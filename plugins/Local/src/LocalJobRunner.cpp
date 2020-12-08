/*
 * LocalJobRunner.cpp
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

#include <LocalJobRunner.hpp>

#include <cmath>

#include <boost/algorithm/string.hpp>

#include <json/Json.hpp>
#include <system/Asio.hpp>
#include <system/DateTime.hpp>
#include <system/Crypto.hpp>
#include <system/Process.hpp>

#include <LocalConstants.hpp>
#include <LocalError.hpp>
#include <LocalJobRepository.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

using State = api::Job::State;

typedef std::shared_ptr<LocalJobRunner> SharedThis;

namespace {

Error decryptPassword(const api::JobPtr& in_job, const std::string& in_key, std::string& out_password)
{
   Optional<std::string> encryptedPasswordOpt = in_job->getJobConfigValue(s_encryptedPassword);
   if (!encryptedPasswordOpt)
   {
      std::string encryptedPassword = encryptedPasswordOpt.getValueOr("");

      Optional<std::string> initVectorOpt = in_job->getJobConfigValue(s_initializationVector);
      if (!initVectorOpt)
         return createError(
            LocalError::INVALID_JOB_CONFIG,
            "required field 'initializationVector' missing",
            ERROR_LOCATION);

      std::string iv = initVectorOpt.getValueOr("");
      if (iv.size() < 8)
         return createError(
            LocalError::INVALID_JOB_CONFIG,
            "required field 'initializationVector' is too short - must be at least 8 bytes",
            ERROR_LOCATION);

      Error error = system::crypto::decryptAndBase64Decode(encryptedPassword, in_key, iv, out_password);
      if (error)
         return createError(
            LocalError::INVALID_JOB_CONFIG,
            "'encryptedPassword' field or 'initializationVector' field has invalid format",
            error,
            ERROR_LOCATION);
   }

   return Success();
}

Error generateJobId(std::string& out_id)
{
   // The ID just needs to be unique, so generate some random data and then base-64 encode it so it's writable to file
   // and to be used in a file name.
   std::vector<unsigned char> randomData;
   Error error = system::crypto::random(16, randomData);
   if (error)
      return error;

   std::string id;
   error = system::crypto::base64Encode(randomData, id);
   if (error)
      return error;

   // Don't allow / in the ID, as it will be used as part of a file name.
   out_id = boost::replace_all_copy(id, "/", "-");
   return Success();
}

Error populateProcessOptions(
   const api::JobPtr& in_job,
   const std::string& in_secureCookieKey,
   system::process::ProcessOptions& out_procOpts)
{
   // Pam profile and password first, since they can result in an error.
   out_procOpts.PamProfile = in_job->getJobConfigValue(s_pamProfile).getValueOr("");
   if (!out_procOpts.PamProfile.empty())
   {
      Error error = decryptPassword(in_job, in_secureCookieKey, out_procOpts.Password);
      if (error)
         return error;
   }

   // Deal with mounts next, since there could be an invalid one.
   for (const api::Mount& mount: in_job->Mounts)
   {
      if (!mount.Source.isHostMountSource())
         return createError(
            LocalError::INVALID_MOUNT_TYPE,
            "Invalid mount: " + mount.toJson().write() + " - only host mount types are supported.",
            ERROR_LOCATION);

      out_procOpts.Mounts.push_back(mount);
   }

   // Copy the argument and environment arrays.
   std::copy(in_job->Arguments.begin(), in_job->Arguments.end(), std::back_inserter(out_procOpts.Arguments));
   std::copy(in_job->Environment.begin(), in_job->Environment.end(), std::back_inserter(out_procOpts.Environment));

   // Populate the rest of the job details.
   out_procOpts.IsShellCommand = !in_job->Command.empty();
   out_procOpts.Executable = out_procOpts.IsShellCommand ? in_job->Command : in_job->Exe;
   out_procOpts.RunAsUser = in_job->User;
   out_procOpts.StandardInput = in_job->StandardIn;

   if (!in_job->StandardErrFile.empty())
      out_procOpts.StandardErrorFile = system::FilePath(in_job->StandardErrFile);
   if (!in_job->StandardOutFile.empty())
      out_procOpts.StandardOutputFile = system::FilePath(in_job->StandardOutFile);
   if (!in_job->WorkingDirectory.empty())
      out_procOpts.WorkingDirectory = system::FilePath(in_job->WorkingDirectory);

   return Success();
}

} // anonymous namespace

LocalJobRunner::LocalJobRunner(
   const std::string& in_hostname,
   jobs::JobStatusNotifierPtr in_notifier,
   std::shared_ptr<LocalJobRepository> in_jobRepository) :
   m_hostname(in_hostname),
   m_jobRepo(std::move(in_jobRepository)),
   m_notifier(std::move(in_notifier))
{
}

Error LocalJobRunner::initialize()
{
   return m_secureCookie.initialize();
}

Error LocalJobRunner::runJob(api::JobPtr& io_job, bool& out_wasInvalidJob)
{
   // Give the job an ID.
   Error error = generateJobId(io_job->Id);
   if (error)
      return error;

   // Set the submission time and the hostname.
   io_job->SubmissionTime = system::DateTime();
   io_job->Host = m_hostname;

   // Set the output files for the job, if required.
   error = m_jobRepo->setJobOutputPaths(io_job);
   if (error)
      return error;

   // Start building the process options.
   system::process::ProcessOptions procOpts;
   error = populateProcessOptions(io_job, m_secureCookie.getKey(), procOpts);
   if (error)
   {
      // If the process options couldn't be populated, the job must have been invalid.
      out_wasInvalidJob = true;
      return error;
   }

   // Set up the onExit and onStderr (for logging) callbacks.
   system::process::AsyncProcessCallbacks callbacks;
   callbacks.OnExit = std::bind(
      LocalJobRunner::onJobExitCallback,
      weak_from_this(),
      std::placeholders::_1,
      io_job);

   const std::string& jobId = io_job->Id;
   callbacks.OnStandardError = std::bind(LocalJobRunner::onJobErrorCallback, io_job, std::placeholders::_1);

   // Run the process. The SDK locks the job before calling submit job, which prevents the job going from non-existent
   // in the system directly to the FINISHED status if the job is very quick.
   std::shared_ptr<system::process::AbstractChildProcess> childProcess;
   error = system::process::ProcessSupervisor::runAsyncProcess(procOpts, callbacks, &childProcess);
   if (error || (childProcess == nullptr))
      return createError(
         LocalError::JOB_LAUNCH_ERROR,
         "Could not launch process for job " + jobId,
         error,
         ERROR_LOCATION);

   // Set the PID and then notify about the PENDING status update.
   io_job->Pid = childProcess->getPid();
   m_notifier->updateJob(io_job, State::PENDING);

   auto jobWatchEvent = std::make_shared<system::AsyncDeadlineEvent>(
      std::bind(LocalJobRunner::onProcessWatchDeadline, weak_from_this(), 1, io_job),
      system::TimeDuration::Microseconds(100000));
   addProcessWatchEvent(io_job->Id, jobWatchEvent);
   jobWatchEvent->start();

   return Success();
}

void LocalJobRunner::onJobErrorCallback(api::JobPtr in_job, const std::string& in_errorStr)
{
   logging::logDebugMessage("Standard error for job " + in_job->Id + ": " + in_errorStr);

   // If there's a stderr file for the job, write the error there as well.
   if (!in_job->StandardErrFile.empty())
   {
      system::process::ProcessOptions procOpts;
      procOpts.Executable = "echo";
      procOpts.Arguments = { in_errorStr };
      procOpts.IsShellCommand = true;
      procOpts.StandardOutputFile = system::FilePath(in_job->StandardErrFile);
      procOpts.StandardErrorFile = procOpts.StandardOutputFile;

      system::process::SyncChildProcess childProcess(procOpts);
      system::process::ProcessResult result;

      Error error = childProcess.run(result);

      std::string errMsg = "Could not write rsandbox error to job output file " + in_job->StandardErrFile;
      if (error)
      {
         error.addProperty("description", errMsg);
         logging::logError(error, ERROR_LOCATION);
      }

      if (result.ExitCode != 0)
      {
         if (!result.StdError.empty())
            errMsg.append(" - ").append(result.StdError);

         logging::logErrorMessage(errMsg, ERROR_LOCATION);
      }
   }
}

void LocalJobRunner::onJobExitCallback(WeakLocalJobRunner in_weakThis, int in_exitCode, api::JobPtr io_job)
{
   if (SharedThis sharedThis = in_weakThis.lock())
   {
      LOCK_JOB(io_job)
      {
         logging::logDebugMessage(
            "Job " +
            io_job->Id +
            "(pid " +
            std::to_string(io_job->Pid.getValueOr(-1)) +
            ") exited with code " +
            std::to_string(in_exitCode));

         io_job->ExitCode = in_exitCode;

         // If the job was explicitly killed, the status doesn't need to be changed so there's no need to notify.
         // Normally notifying the status update will save the job, so save the job manually this time. Otherwise,
         // update the status appropriately.
         if (io_job->Status == State::KILLED)
         {
            io_job->LastUpdateTime = system::DateTime();
            sharedThis->m_jobRepo->saveJob(io_job);
         }
         else
         {
            // If the job is currently pending (i.e. it exited really quickly, and we never saw the running state),
            // notify that it is running first.
            if (io_job->Status == State::PENDING)
            {
               sharedThis->m_notifier->updateJob(io_job, State::RUNNING);
            }

            sharedThis->m_notifier->updateJob(io_job, State::FINISHED);
         }
      }
      END_LOCK_JOB
   }
}

void LocalJobRunner::onProcessWatchDeadline(WeakLocalJobRunner in_weakThis, int in_count, api::JobPtr io_job)
{
   if (SharedThis sharedThis = in_weakThis.lock())
   {
      // Give up at this point.
      if (in_count > 100)
      {
         logging::logErrorMessage(
            "Job " + io_job->Id + " did not transition to a running state within a reasonable time.");
         return;
      }

      LOCK_JOB(io_job)
      {
         // Check the job status. If it already exited, just stop watching.
         if ((io_job->Status == State::KILLED) || (io_job->Status == State::FINISHED))
         {
            // Remove the watch event to prevent an ever-growing map.
            sharedThis->removeWatchEvent(io_job->Id);
            return;
         }

         system::process::ProcessInfo procInfo;
         Error error = system::process::ProcessInfo::getProcessInfo(io_job->Pid.getValueOr(0), procInfo);
         if (error)
         {
            logging::logError(error, ERROR_LOCATION);

            // Remove the watch event to prevent an ever-growing map.
            sharedThis->removeWatchEvent(io_job->Id);
            return;
         }

         // If the process name has changed from rsandbox, then the job is running. Update the status and exit.
         if (procInfo.Executable != "rsandbox")
         {
            sharedThis->m_notifier->updateJob(io_job, State::RUNNING);

            // Remove the watch event to prevent an ever-growing map.
            sharedThis->removeWatchEvent(io_job->Id);
            return;
         }
      }
      END_LOCK_JOB

      // If we haven't exited at this point, the job isn't running yet. Retry.
      // For the first 5 tries, wait (2^in_count) * 100 milliseconds (i.e. 200 milliseconds, then 400 milliseconds,
      // then 800 milliseconds, then 1.6 seconds, then 3.2 seconds). After that, just wait 5 seconds each time.
      system::TimeDuration waitTime = (in_count > 5) ?
         system::TimeDuration::Seconds(5) :
         system::TimeDuration::Microseconds(::pow(2, in_count) * 100000); // 100 milliseconds = 100000 microseconds.

      auto jobWatchEvent = std::make_shared<system::AsyncDeadlineEvent>(
         std::bind(LocalJobRunner::onProcessWatchDeadline, in_weakThis, in_count + 1, io_job),
         waitTime);
      sharedThis->addProcessWatchEvent(io_job->Id, jobWatchEvent);
      jobWatchEvent->start();
   }
}

void LocalJobRunner::addProcessWatchEvent(
   const std::string& in_id,
   const std::shared_ptr<system::AsyncDeadlineEvent>& in_processWatchEvent)
{
   LOCK_MUTEX(m_mutex)
   {
      m_processWatchEvents[in_id] = in_processWatchEvent;
   }
   END_LOCK_MUTEX
}

void LocalJobRunner::removeWatchEvent(const std::string& in_id)
{
   LOCK_MUTEX(m_mutex)
   {
      auto itr = m_processWatchEvents.find(in_id);
      if (itr != m_processWatchEvents.end())
         m_processWatchEvents.erase(itr);
   }
   END_LOCK_MUTEX
}

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
