/*
 * AbstractJobStatusWatcher.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_JOB_STATUS_WATCHER_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_JOB_STATUS_WATCHER_HPP

#include <Noncopyable.hpp>

#include <PImpl.hpp>
#include <api/Job.hpp>
#include <jobs/JobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

/**
 * @brief Manages posting job status updates to the notifier.
 */
class AbstractJobStatusWatcher : public Noncopyable
{
public:
   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~AbstractJobStatusWatcher() = default;

   /**
    * @brief Starts the job status watcher.
    */
   virtual Error start() = 0;

   /**
    * @brief Stops the job status watcher.
    */
   virtual void stop() = 0;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository           The job repository, from which to look-up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post job updates.
    */
   AbstractJobStatusWatcher(JobRepositoryPtr in_jobRepository, JobStatusNotifierPtr in_jobStatusNotifier);

   /**
    * @brief Updates the job status for the specified job.
    *
    * Attempts to look up the job in the repository. If the job cannot be found getJobDetails(...) will be invoked.
    *
    * @param in_jobId               The ID of the job which should be updated.
    * @param in_newStatus           The new status of the job.
    * @param in_statusMessage       The new status message of the job, if any.
    * @param in_invocationTime      The time at which the job was updated, if different from now.
    *
    * @return Success if the job could be found and updated; Error otherwise.
    */
   Error updateJobStatus(
      const std::string& in_jobId,
      api::Job::State in_newStatus,
      const std::string& in_statusMessage = "",
      const system::DateTime& in_invocationTime = system::DateTime());

   /**
    * @brief Updates the job status for the specified job.
    *
    * @param in_job                 The job which should be updated.
    * @param in_newStatus           The new status of the job.
    * @param in_statusMessage       The new status message of the job, if any.
    * @param in_invocationTime      The time at which the job was updated, if different from now.
    */
   void updateJobStatus(
      const api::JobPtr& in_job,
      api::Job::State in_newStatus,
      const std::string& in_statusMessage = "",
      const system::DateTime& in_invocationTime = system::DateTime());

private:
   /**
    * @brief Gets the job details for the specified job.
    *
    * @param in_jobId   The ID of the job to retrieve.
    * @param out_job    The populated Job object.
    *
    * @return Success if the job details could be retrieved and parsed; Error otherwise.
    */
   virtual Error getJobDetails(const std::string& in_jobId, api::JobPtr& out_job) const = 0;

   // The private implementation of AbstractJobStatusWatcher.
   PRIVATE_IMPL(m_baseImpl);
};

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio

#endif
