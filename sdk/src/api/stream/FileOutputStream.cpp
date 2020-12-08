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
 * @brief Checks whether a file exists for a given user.
 * 
 * @param in_file          The file to check for existence.
 * @param in_user          The user for whom to check the file.
 * @param out_wasFound     True if the specified file existed for the given user; false otherwise.
 * 
 * @return Success if the existence of the file could be tested; Error otherwise.
 */
Error fileExistsForUser(const system::FilePath& in_file, const system::User& in_user, bool& out_wasFound)
{
   system::process::ProcessOptions lsOpts;
   lsOpts.Executable = "ls";
   lsOpts.Arguments = { in_file.getAbsolutePath() };
   lsOpts.RunAsUser = in_user;

   system::process::ProcessResult result;
   system::process::SyncChildProcess lsProc(lsOpts);
   Error error = lsProc.run(result);
   if (error)
      return error;

   out_wasFound = result.ExitCode == 0;

   if (!out_wasFound)
   {
      logging::logDebugMessage(
         "'ls " +
         in_file.getAbsolutePath() +
         "' failed with error code (" +
         std::to_string(result.ExitCode) +
         ")" +
         (result.StdError.empty() ? "" : " and stderr \"" + result.StdError + "\""));
   }

   return Success();
}

/**
 * @brief Resolves host mount paths.
 *
 * @param in_strPath    The original path.
 * @param in_mounts     The set of mounts on the job.
 *
 * @return The resolved path.
 */
system::FilePath getRealPath(const system::FilePath& in_strPath, const MountList& in_mounts)
{
   system::FilePath result(in_strPath);
   for (const Mount& mount: in_mounts)
   {
      if (mount.HostSourcePath && result.isWithin(system::FilePath(mount.DestinationPath)))
      {
         std::string relPathStr = boost::trim_left_copy_if(
            in_strPath.getAbsolutePath().substr(mount.DestinationPath.size()),
            boost::is_any_of("/"));

         result = system::FilePath(
            mount.HostSourcePath.getValueOr(HostMountSource()).Path).completeChildPath(relPathStr);
      }
   }

   return result;
}

} // anonymous namespace

typedef std::shared_ptr<FileOutputStream> SharedThis;
typedef std::weak_ptr<FileOutputStream> WeakThis;
typedef std::shared_ptr<system::process::AbstractChildProcess> TailChild;

struct FileOutputStream::Impl
{
   /**
    * @brief Constructor.
    * 
    * @param in_job                 The job for which this output stream is being opened.
    * @param in_findFilesMaxTime    The maximum amount of time to wait for the output files to be created.
    */
   Impl(const api::JobPtr& in_job, system::TimeDuration&& in_findFilesMaxTime) :
      IsStreaming(false),
      IsStopping(false),
      FindFilesRetryCount(0),
      FindFilesMaxWaitTime(in_findFilesMaxTime),
      Mounts(in_job->Mounts),
      StdErrExited(false),
      StdErrExitCode(0),
      StdErrFile(in_job->StandardErrFile),
      StdErrFileFound(false),
      StdOutExited(false),
      StdOutExitCode(0),
      StdOutFile(in_job->StandardOutFile),
      StdOutFileFound(false),
      User(in_job->User),
      WasErrorReported(false),
      WasOutputWritten(false)
   {
   };

   /**
    * @brief Once the files have been found, starts streaming their contents to the caller.
    * 
    * @param in_sharedThis       A shared pointer to the parent FileOutputStream object.
    * @param in_lock             The owned Mutex lock.
    * @param in_jobLock          The owned lock for the Job.
    */
   void onFilesFound(
      SharedThis in_sharedThis,
      const std::unique_lock<std::recursive_mutex>& in_lock,
      const api::JobLock& in_jobLock)
   {
      assert(in_lock.owns_lock());
      bool outputFileEmpty = StdOutFile.isEmpty();
      bool errorFileEmpty = StdErrFile.isEmpty();
      system::FilePath outputFile = getRealPath(StdOutFile, Mounts);
      system::FilePath errorFile = getRealPath(StdErrFile, Mounts);

      if (in_sharedThis->m_outputType == OutputType::BOTH)
      {
         if ((outputFile == errorFile) && !outputFileEmpty)
         {
            // The StdErr stream was never started, so it already exited.
            StdErrExited = true;
            Error error = startChildStream(in_sharedThis->m_outputType, outputFile, in_sharedThis);
            if (error)
            {
               logging::logError(error);
               in_sharedThis->reportError(error);
            }
         }
         else
         {
            if (!outputFileEmpty)
            {
               Error error = startChildStream(OutputType::STDOUT, outputFile, in_sharedThis);
               if (error)
               {
                  logging::logError(error);
                  in_sharedThis->reportError(error);
               }
            }
            else
               StdOutExited = true;
            if (!errorFileEmpty)
            {
               Error error = startChildStream(OutputType::STDERR, errorFile, in_sharedThis);
               if (error)
                  in_sharedThis->reportError(error);
            }
            else
               StdErrExited = true;
         }
      }
      else if ((in_sharedThis->m_outputType == OutputType::STDOUT) && !outputFileEmpty)
      {
         StdErrExited = true;
         Error error = startChildStream(OutputType::STDOUT, outputFile, in_sharedThis);
         if (error)
         {
            logging::logError(error);
            in_sharedThis->reportError(error);
         }
      }
      else if ((in_sharedThis->m_outputType == OutputType::STDERR) && !errorFileEmpty)
      {
         StdOutExited = true;
         Error error = startChildStream(OutputType::STDERR, errorFile, in_sharedThis);
         if (error)
         {
            logging::logError(error);
            in_sharedThis->reportError(error);
         }
      }
   }

   /**
    * @brief Checks for the existence of the Job's output files, updating internal flags accordingly.
    * 
    * @param in_lock    The owned Mutex lock.
    * 
    * @return Success if the existence of the files could be checked; Error on failure to check for the files.
    */
   Error onFindFilesTimer(const std::unique_lock<std::recursive_mutex>& in_lock)
   {
      assert(in_lock.owns_lock());

      // If the files haven't be found, test them for existince again.
      if (!StdOutFileFound)
      {
         Error error = fileExistsForUser(StdOutFile, User, StdOutFileFound);
         if (error)
         {
            logging::logError(error, ERROR_LOCATION);
            return error;
         }

         if (StdOutFile == StdErrFile)
            StdErrFileFound = StdOutFileFound;
      }

      if (!StdErrFileFound)
      {
         Error error = fileExistsForUser(StdErrFile, User, StdErrFileFound);
         if (error)
         {
            logging::logError(error, ERROR_LOCATION);
            return error;
         }
      }

      // No errors testing for existence - return success
      return Success();
   }

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

      procOpts.RunAsUser = User;

      system::process::AsyncProcessCallbacks callbacks;
      {
         using namespace std::placeholders;
         callbacks.OnError = [in_sharedThis](const Error& in_error)
         {
            UNIQUE_LOCK_RECURSIVE_MUTEX(in_sharedThis->m_impl->Mutex)
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
            UNIQUE_LOCK_RECURSIVE_MUTEX(in_sharedThis->m_impl->Mutex)
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

            UNIQUE_LOCK_RECURSIVE_MUTEX(in_sharedThis->m_impl->Mutex)
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

         callbacks.OnExit = std::bind(FileOutputStream::onExitCallback, WeakThis(in_sharedThis), in_outputType, _1);
      }

      TailChild& child = (in_outputType == OutputType::STDERR ?  StdErrChild : StdOutChild);
      Error error = system::process::ProcessSupervisor::runAsyncProcess(procOpts, callbacks, &child);
      if (error)
         return error;

      return Success();
   }

   /**
    * @brief Stops all the child processes of this output stream.
    */
   void stopChildProcesses()
   {
      UNIQUE_LOCK_RECURSIVE_MUTEX(Mutex)
      {
         stopChildProcesses(uniqueLock);
      }
      END_LOCK_MUTEX
   }


   /**
    * @brief Stops all the child processes of this output stream.
    * 
    * @param in_lock    The owned Mutex lock.
    */
   void stopChildProcesses(const std::unique_lock<std::recursive_mutex>& in_lock)
   {
      assert(in_lock.owns_lock());

      IsStopping = true;
      if (StdOutChild)
      {
         Error error = StdOutChild->terminate();
         if (error)
            logging::logError(error, ERROR_LOCATION);

         StdOutChild.reset();
      }

      if (StdErrChild)
      {
         Error error = StdErrChild->terminate();
         if (error)
            logging::logError(error, ERROR_LOCATION);

         StdErrChild.reset();
      }
   }

   /** Whether the output stream is stopping by request. */
   bool IsStopping;

   /** Whether the output stream is really being streamed (as opposed to dumping all the output data at once). */
   bool IsStreaming;

   /** The maximum amount of time to wait for the output files to be created. */
   system::TimeDuration FindFilesMaxWaitTime;

   /** The number of times we have checked for the existence of the output files. */
   uint64_t FindFilesRetryCount;

   /** The time at which we started checking for the existence of the output files. */
   system::DateTime FindFilesStartTime;

   /** The deadline event that triggers a search for the output files. */
   std::shared_ptr<system::AsyncDeadlineEvent> FindFilesTimer;

   /** A reference to the list of mounts for the Job. */
   const api::MountList& Mounts;

   /** Mutex to protect members. */
   std::recursive_mutex Mutex;

   /** The StdErr tail command child process. */
   TailChild StdErrChild;

   /** The exit code of StdErr tail command child process. */
   int StdErrExitCode;

   /** Whether the StdErr tail command child process has exited. */
   bool StdErrExited;

   /** The location of the standard error output file. */
   system::FilePath StdErrFile;

   /** Whether the StdErr file has been found or not. */
   bool StdErrFileFound;

   /**
    * The StdOut tail command child process. If the output type is BOTH and the stdout and stderr files are the same,
    * this is also the child process.
    */
   TailChild StdOutChild;

   /** The exit code of StdOut or mixed tail command child process. */
   int StdOutExitCode;

   /** Whether the StdOut or mixed tail command child process has exited. */
   bool StdOutExited;

   /** The location of the standard output file. */
   system::FilePath StdOutFile;

   /** Whether the StdOut file has been found or not. */
   bool StdOutFileFound;

   /** The user who owns the Job. */
   system::User User;

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
      m_impl(new Impl(m_job, system::TimeDuration::Seconds(10)))
{
}

Error FileOutputStream::start()
{
   UNIQUE_LOCK_RECURSIVE_MUTEX(m_impl->Mutex)
   {
      Error error = m_impl->onFindFilesTimer(uniqueLock);
      if (error)
         return error;

      if (m_impl->StdOutFileFound && m_impl->StdErrFileFound)
      {
         LOCK_JOB(m_job)
         {
            m_impl->onFilesFound(shared_from_this(), uniqueLock, jobLock);
         }
         END_LOCK_JOB
      }
      else
      {
         system::TimeDuration firstWaitTime = system::TimeDuration::Microseconds(50000);
         m_impl->FindFilesTimer.reset(
            new system::AsyncDeadlineEvent(
               std::bind(&FileOutputStream::onFindFileTimerCallback, weak_from_this()),
               (m_impl->FindFilesMaxWaitTime > firstWaitTime) ? firstWaitTime : m_impl->FindFilesMaxWaitTime));

         ++m_impl->FindFilesRetryCount;
         m_impl->FindFilesStartTime = system::DateTime();
         m_impl->FindFilesTimer->start();
      }
   }
   END_LOCK_MUTEX
   return Success();
}

void FileOutputStream::stop()
{
   SharedThis sharedThis = shared_from_this();
   OnStreamEnd onStreamEnd = [sharedThis]()
   {
      sharedThis->m_impl->stopChildProcesses();
   };

   if (m_job->isCompleted())
      waitForStreamEnd(onStreamEnd);
   else
   {
      onStreamEnd();
   }
}

void FileOutputStream::onExitCallback(WeakThis in_weakThis, OutputType in_outputType, int in_exitCode)
{
   SharedThis sharedThis = in_weakThis.lock();
   if (!sharedThis)
      return;

   Impl& impl = *sharedThis->m_impl;
   UNIQUE_LOCK_RECURSIVE_MUTEX(impl.Mutex)
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
               sharedThis->reportError(
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
            sharedThis->setStreamComplete();
      }
   }
   END_LOCK_MUTEX
}

void FileOutputStream::onFindFileTimerCallback(std::weak_ptr<FileOutputStream> in_weakThis)
{
   if (SharedThis sharedThis = in_weakThis.lock())
   {
      Impl& impl = *sharedThis->m_impl;
      UNIQUE_LOCK_RECURSIVE_MUTEX(impl.Mutex)
      {
         Error error = impl.onFindFilesTimer(uniqueLock);
         if (error)
            return sharedThis->reportError(error);

         //
         if (impl.StdOutFileFound && impl.StdErrFileFound)
         {
            LOCK_JOB(sharedThis->m_job)
            {
               impl.onFilesFound(sharedThis, uniqueLock, jobLock);
            }
            END_LOCK_JOB
         }
         else
         {
            // Determine if we've run out of time to wait for the files to be created.
            if ((system::DateTime() - impl.FindFilesStartTime) > impl.FindFilesMaxWaitTime)
            {
               std::string errorMessage = "Could not find ";
               if (!impl.StdOutFileFound)
                  errorMessage.append("ouput (").append(impl.StdOutFile.getAbsolutePath()).append(")");

               if (!impl.StdErrFileFound)
               {
                  std::string errFileMsg = "error (" + impl.StdErrFile.getAbsolutePath() + ")";
                  if (!impl.StdOutFileFound)
                     errorMessage.append(" and ").append(errFileMsg).append(" files");
                  else
                     errorMessage.append(errFileMsg).append(" file");
               }
               else
                  errorMessage.append(" file");

               errorMessage.append(" within timeout.");
               
               logging::logErrorMessage(errorMessage);
               return sharedThis->reportError(Error("FileOutputStreamError", 1, errorMessage, ERROR_LOCATION));
            }

            // Otherwise, wait a while and retry. Back-off by 50 milliseconds every time we retry, until we hit .5
            // seconds. After that, try every second.
            ++impl.FindFilesRetryCount;
            system::TimeDuration waitTime = (impl.FindFilesRetryCount <= 10) ? 
               system::TimeDuration::Microseconds(50000 * impl.FindFilesRetryCount) :
               (system::TimeDuration::Seconds(1));

            impl.FindFilesTimer.reset(
               new system::AsyncDeadlineEvent(
                  std::bind(&FileOutputStream::onFindFileTimerCallback, in_weakThis),
                  waitTime));
            
            impl.FindFilesTimer->start();
         }

      }
      END_LOCK_MUTEX
   }
}

void FileOutputStream::onOutput(const std::string& in_output, OutputType in_outputType)
{
   reportData(in_output, in_outputType);
}

void FileOutputStream::waitForStreamEnd(const OnStreamEnd& in_onStreamEnd)
{
   UNIQUE_LOCK_RECURSIVE_MUTEX(m_impl->Mutex)
   {
      m_impl->WaitForEndEvent.reset(new system::AsyncDeadlineEvent(in_onStreamEnd, system::TimeDuration::Seconds(2)));
      m_impl->WaitForEndEvent->start();
   }
   END_LOCK_MUTEX
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
