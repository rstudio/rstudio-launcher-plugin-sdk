/*
 * FileOutputStream.cpp
 *
 * Copyright (C) 2020 by RStudio, PBC
 *
 * Unless you have received this program directly from RStudio pursuant to the terms of a commercial license agreement
 * with RStudio, then this program is licensed to you under the following terms:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#include <api/stream/FileOutputStream.hpp>

#include <boost/algorithm/string/trim.hpp>

#include <system/Asio.hpp>
#include <system/FilePath.hpp>
#include <system/User.hpp>
#include <system/Process.hpp>
#include <utils/MutexUtils.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

/**
 * @brief Resolves host mount paths.
 *
 * @param in_strPath    The original path.
 * @param in_mounts     The set of mounts on the job.
 *
 * @return The resolved path.
 */
system::FilePath getRealPath(const std::string& in_strPath, const MountList& in_mounts)
{
   system::FilePath result(in_strPath);
   for (const Mount& mount: in_mounts)
   {
      if (mount.HostSourcePath && result.isWithin(system::FilePath(mount.DestinationPath)))
      {
         std::string relPathStr = boost::trim_left_copy_if(
            in_strPath.substr(mount.DestinationPath.size()),
            boost::is_any_of("/"));

         result = system::FilePath(
            mount.HostSourcePath.getValueOr(HostMountSource()).Path).completeChildPath(relPathStr);
      }
   }

   return result;
}

} // anonymous namespace

typedef std::function<void(const std::string&, OutputType)> OnTypedOutput; 
typedef std::shared_ptr<FileOutputStream> SharedThis;
typedef std::shared_ptr<system::process::AbstractChildProcess> TailChild;

struct FileOutputStream::Impl
{
   explicit Impl(api::JobPtr& in_job) :
      IsStreaming(false),
      IsStopping(false),
      JobObj(in_job),
      StdErrExited(false),
      StdErrExitCode(0),
      StdOutExited(false),
      StdOutExitCode(0),
      WasErrorReported(false),
      WasOutputWritten(false)
   {
   };

   /**
    * @brief Starts streaming output from the specified file as the specified output type.
    *
    * @param in_outputType      The type of output that will be streamed.
    * @param in_file            The file from which to read the output.
    *
    * @return Success if the file could be read; Error otherwise.
    */
   Error startChildStream(OutputType in_outputType, const system::FilePath& in_file, SharedThis in_sharedThis)
   {
      system::process::ProcessOptions procOpts;
      procOpts.Executable = "tail";
      procOpts.IsShellCommand = true;

      if (IsStreaming)
         procOpts.Arguments.emplace_back("-f");
      procOpts.Arguments.emplace_back("-n+1");
      procOpts.Arguments.emplace_back(in_file.getAbsolutePath());

      procOpts.RunAsUser = JobObj->User;

      system::process::AsyncProcessCallbacks callbacks;
      {
         using namespace std::placeholders;
         callbacks.OnError = [in_sharedThis](const Error& in_error)
         {
            UNIQUE_LOCK_MUTEX(in_sharedThis->m_impl->Mutex)
            {
               if (!in_sharedThis->m_impl->WasOutputWritten && !in_sharedThis->m_impl->WasErrorReported)
               {
                  in_sharedThis->m_impl->WasErrorReported = true;
                  in_sharedThis->reportError(in_error);
               }

               logging::logError(in_error);
            }
            END_LOCK_MUTEX
         };

         callbacks.OnStandardOutput = [in_sharedThis, in_outputType](const std::string& in_output)
         {
            UNIQUE_LOCK_MUTEX(in_sharedThis->m_impl->Mutex)
            {
               if (!in_sharedThis->m_impl->WasErrorReported)
               {
                  in_sharedThis->m_impl->WasOutputWritten = true;
                  in_sharedThis->onOutput(in_output, in_outputType);
               }
            }
            END_LOCK_MUTEX
         };

         callbacks.OnStandardError = [in_sharedThis](const std::string& in_output)
         {
            std::string message = "Error output received for OutputStream tail command: " + in_output;
            logging::logErrorMessage(
               message,
               ERROR_LOCATION);

            UNIQUE_LOCK_MUTEX(in_sharedThis->m_impl->Mutex)
            {
               if (!in_sharedThis->m_impl->WasErrorReported && !in_sharedThis->m_impl->WasOutputWritten)
               {
                  in_sharedThis->m_impl->WasErrorReported = true;
                  in_sharedThis->reportError(
                     Error(
                        "FileOutputStreamError",
                        2,
                        message,
                        ERROR_LOCATION));
               }
            }
            END_LOCK_MUTEX
         };

         callbacks.OnExit = std::bind(FileOutputStream::onExitCallback, in_sharedThis, in_outputType, _1);
      }

      TailChild& child = (in_outputType == OutputType::STDERR ?  StdErrChild : StdOutChild);
      Error error = system::process::ProcessSupervisor::runAsyncProcess(procOpts, callbacks, &child);
      if (error)
         return error;

      return Success();
   }

   /** Whether the output stream is stopping by request. */
   bool IsStopping;

   /** Whether the output stream is really being streamed (as opposed to dumping all the output data at once). */
   bool IsStreaming;

   api::JobPtr JobObj;

   /** Mutex to protect members. */
   std::mutex Mutex;

   OnTypedOutput OnOutputFunc;

   /** The StdErr tail command child process. */
   TailChild StdErrChild;

   /** The exit code of StdErr tail command child process. */
   int StdErrExitCode;

   /** Whether the StdErr tail command child process has exited. */
   bool StdErrExited;

   /**
    * The StdOut tail command child process. If the output type is BOTH and the stdout and stderr files are the same,
    * this is also the child process.
    */
   TailChild StdOutChild;

   /** The exit code of StdOut or mixed tail command child process. */
   int StdOutExitCode;

   /** Whether the StdOut or mixed tail command child process has exited. */
   bool StdOutExited;

   /** Whether an error has already been reported to the parent class. */
   bool WasErrorReported;

   /** Whether output data has been sent to the launcher. */
   bool WasOutputWritten;

   /** Deadline event to give the job a couple more seconds to finish output. */
   std::shared_ptr<system::AsyncDeadlineEvent> WaitForEndEvent;
};

PRIVATE_IMPL_DELETER_IMPL(FileOutputStream)

FileOutputStream::FileOutputStream(
   OutputType in_outputType,
   api::JobPtr in_job,
   AbstractOutputStream::OnOutput in_onOutput,
   AbstractOutputStream::OnComplete in_onComplete,
   AbstractOutputStream::OnError in_onError) :
      AbstractOutputStream(
         in_outputType,
         std::move(in_job),
         std::move(in_onOutput),
         std::move(in_onComplete),
         std::move(in_onError)),
      m_impl(new Impl(m_job))
{
}

Error FileOutputStream::start()
{
   UNIQUE_LOCK_MUTEX(m_impl->Mutex)
   {
      bool outputFileEmpty = m_job->StandardOutFile.empty();
      bool errorFileEmpty = m_job->StandardErrFile.empty();
      system::FilePath outputFile = getRealPath(m_job->StandardOutFile, m_job->Mounts);
      system::FilePath errorFile = getRealPath(m_job->StandardErrFile, m_job->Mounts);

      LOCK_JOB(m_job)
      {
         SharedThis sharedThis = shared_from_this();
         m_impl->IsStreaming = !m_job->isCompleted();
         if (m_outputType == OutputType::BOTH)
         {
            if ((outputFile == errorFile) && !outputFileEmpty)
            {
               // The StdErr stream was never started, so it already exited.
               m_impl->StdErrExited = true;
               return m_impl->startChildStream(m_outputType, outputFile, sharedThis);
            }
            else
            {
               if (!outputFileEmpty)
               {
                  Error error = m_impl->startChildStream(OutputType::STDOUT, outputFile, sharedThis);
                  if (error)
                     return error;
               }
               else
                  m_impl->StdOutExited = true;
               if (!errorFileEmpty)
               {
                  Error error = m_impl->startChildStream(OutputType::STDERR, errorFile, sharedThis);
                  if (error)
                     return error;
               }
               else
                  m_impl->StdErrExited = true;
            }
         }
         else if ((m_outputType == OutputType::STDOUT) && !outputFileEmpty)
         {
            m_impl->StdErrExited = true;
            Error error = m_impl->startChildStream(OutputType::STDOUT, outputFile, sharedThis);
            if (error)
               return error;
         }
         else if ((m_outputType == OutputType::STDERR) && !errorFileEmpty)
         {
            m_impl->StdOutExited = true;
            Error error = m_impl->startChildStream(OutputType::STDERR, errorFile, sharedThis);
            if (error)
               return error;
         }
      }
      END_LOCK_JOB
   }
   END_LOCK_MUTEX

   return Success();
}

void FileOutputStream::stop()
{
   SharedThis sharedThis = shared_from_this();
   OnStreamEnd onStreamEnd = [sharedThis]()
   {
      UNIQUE_LOCK_MUTEX(sharedThis->m_impl->Mutex)
      {
         if (sharedThis->m_impl->StdOutChild)
         {
            Error error = sharedThis->m_impl->StdOutChild->terminate();
            if (error)
               logging::logError(error, ERROR_LOCATION);

            sharedThis->m_impl->StdOutChild.reset();
         }

         if (sharedThis->m_impl->StdErrChild)
         {
            Error error = sharedThis->m_impl->StdErrChild->terminate();
            if (error)
               logging::logError(error, ERROR_LOCATION);

            sharedThis->m_impl->StdErrChild.reset();
         }
      }
      END_LOCK_MUTEX
   };

   UNIQUE_LOCK_MUTEX(m_impl->Mutex)
   {
      m_impl->IsStopping = true;
   }
   END_LOCK_MUTEX

   if (m_job->isCompleted())
      waitForStreamEnd(onStreamEnd);
   else
   {
      onStreamEnd();
   }
}

void FileOutputStream::onExitCallback(SharedThis in_sharedThis, OutputType in_outputType, int in_exitCode)
{
   Impl& impl = *in_sharedThis->m_impl;
   UNIQUE_LOCK_MUTEX(impl.Mutex)
   {
      // If the streams were stopped explicitly it's expected that the children will exit.
      if (impl.IsStopping)
         return;

      // Log a warning message if the streamed tail process exited on its own (that should never happen)
      if (impl.IsStreaming)
         logging::logWarningMessage("OutputStream tail command exited with code " + std::to_string(in_exitCode));

      if (in_outputType == OutputType::STDERR)
      {
         impl.StdErrExitCode = in_exitCode;
         impl.StdErrExited = true;
      }
      else
      {
         impl.StdOutExitCode = in_exitCode;
         impl.StdOutExited = true;

         if (in_outputType == OutputType::BOTH)
            impl.StdErrExitCode = in_exitCode;
      }

      if (impl.StdOutExited && impl.StdErrExited)
      {
         if (!impl.WasOutputWritten && ((impl.StdOutExitCode != 0) || (impl.StdErrExitCode != 0)))
         {
            if (!impl.WasErrorReported)
               in_sharedThis->reportError(
                  Error(
                     "FileOutputStreamError",
                     1,
                     "OutputStream exited unexpectedly with codes: (stdout stream: " +
                        std::to_string(impl.StdOutExitCode) +
                        ", stderr stream: " +
                        std::to_string(impl.StdErrExitCode) +
                        ")",
                     ERROR_LOCATION));
         }
         else
            in_sharedThis->setStreamComplete();
      }
   }
   END_LOCK_MUTEX
}

void FileOutputStream::onOutput(const std::string& in_output, OutputType in_outputType)
{
   reportData(in_output, in_outputType);
}

void FileOutputStream::waitForStreamEnd(const OnStreamEnd& in_onStreamEnd)
{
   UNIQUE_LOCK_MUTEX(m_impl->Mutex)
   {
      m_impl->WaitForEndEvent.reset(new system::AsyncDeadlineEvent(in_onStreamEnd, system::TimeDuration::Seconds(2)));
      m_impl->WaitForEndEvent->start();
   }
   END_LOCK_MUTEX
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
