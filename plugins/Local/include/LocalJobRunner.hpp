/*
 * LocalJobRunner.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_JOB_RUNNER_HPP
#define LAUNCHER_PLUGINS_LOCAL_JOB_RUNNER_HPP

#include <memory>

#include <api/Job.hpp>
#include <api/Response.hpp>
#include <jobs/JobStatusNotifier.hpp>

#include <LocalSecureCookie.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace system {

class AsyncDeadlineEvent;

} // namespace system

namespace local {
namespace job_store {

class LocalJobStorage;

} // namespace job_store
} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Runs jobs on the local machine.
 */
class LocalJobRunner : public std::enable_shared_from_this<LocalJobRunner>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_hostname        The hostname of the machine on which jobs will be run (this machine).
    * @param in_notifier        The job status notifier, for posting job status updates.
    * @param in_jobStorage      The job storage, for saving jobs and job output.
    */
   LocalJobRunner(
      const std::string& in_hostname,
      jobs::JobStatusNotifierPtr in_notifier,
      std::shared_ptr<job_store::LocalJobStorage> in_jobStorage);

   /**
    * @brief Initializes the job runner.
    *
    * @return Success if the job runner could be initialized; Error otherwise.
    */
   Error initialize();

   /**
    * @brief Runs the specified job.
    *
    * @param io_job                 The Job to be run.
    * @param out_wasInvalidJob      Whether the error that occurred was because the requested Job was invalid.
    *
    * @return Success if the job could be run; Error otherwise.
    */
   Error runJob(api::JobPtr& io_job, bool& out_wasInvalidJob);

private:
   // Convenience typedefs.
   typedef std::weak_ptr<LocalJobRunner> WeakLocalJobRunner;
   typedef std::map<std::string, std::shared_ptr<system::AsyncDeadlineEvent> > ProcessWatchEvents;

   static void onJobErrorCallback(api::JobPtr in_job, const std::string& in_errorStr);

   /**
    * @brief Callback to be invoked when a Job exits.
    *
    * @param in_weakThis    A weak pointer to this LocalJobRunner.
    * @param in_exitCode    The exit code of the Job.
    * @param io_job         The Job that has exited.
    */
   static void onJobExitCallback(WeakLocalJobRunner in_weakThis, int in_exitCode, api::JobPtr io_job);

   /**
    * @brief Callback to be invoked after a set amount of time to check whether the Job is running yet.
    *
    * @param in_weakThis    A weak pointer to this LocalJobRunner.
    * @param in_count       The number of times the callback has been invoked for the specified Job.
    * @param io_job         The Job which must be watched.
    */
   static void onProcessWatchDeadline(WeakLocalJobRunner in_weakThis, int in_count, api::JobPtr io_job);

   /**
    * @brief Adds or updates a process watch event.
    *
    * @param in_id                      The ID of the job for which the event was created.
    * @param in_processWatchEvent       The process watch event.
    */
   void addProcessWatchEvent(
      const std::string& in_id,
      const std::shared_ptr<system::AsyncDeadlineEvent>& in_processWatchEvent);

   /**
    * @brief Removes a process watch event.
    *
    * @param in_id      The ID of the job for which the event was created.
    */
   void removeWatchEvent(const std::string& in_id);

   /** The name of the host running this job. */
   const std::string& m_hostname;

   /** The job storage. */
   std::shared_ptr<job_store::LocalJobStorage> m_jobStorage;

   /** The mutex to protect the process watch events. */
   std::mutex m_mutex;

   /** The job status notifier, to update the status of the job on exit. */
   jobs::JobStatusNotifierPtr m_notifier;

   /** The watch events for each process that has not started running yet. */
   ProcessWatchEvents m_processWatchEvents;

   /** The secure cookie. */
   LocalSecureCookie m_secureCookie;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
