/*
 * QuickStartJobStatusWatcher.hpp
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

#ifndef LAUNCHER_PLUGINS_QUICK_START_JOB_STATUS_WATCHER_HPP
#define LAUNCHER_PLUGINS_QUICK_START_JOB_STATUS_WATCHER_HPP

#include <jobs/AbstractTimedJobStatusWatcher.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

class QuickStartJobStatusWatcher : public jobs::AbstractTimedJobStatusWatcher
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_frequency               The frequency at which job statuses should be polled.
    * @param in_jobRepository           The job repository, from which to look-up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post job updates.
    */
   QuickStartJobStatusWatcher(
      system::TimeDuration in_frequency,
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier);

private:
   /**
    * @brief Polls job statuses.
    *
    * Note: If this method returns an error, polling will be stopped. An error should be returned if it will not be
    * possible to poll job information whatsoever. If the polling problem is temporary, it may be advisable to log an
    * error or a warning and return Success instead.
    *
    * This method should invoke one of the AbstractJobStatusWatcher::updateJobStatus methods for each job that was
    * updated.
    *
    * @return Success if the jobs could be polled; Error if an unrecoverable polling issue occurred.
    */
   Error pollJobStatus() override;

   /**
    * @brief Gets the job details for the specified job.
    *
    * @param in_jobId   The ID of the job to retrieve.
    * @param out_job    The populated Job object.
    *
    * @return Success if the job details could be retrieved and parsed; Error otherwise.
    */
   Error getJobDetails(const std::string& in_jobId, api::JobPtr& out_job) const override;
};

/** Convenience typedef. */
typedef std::shared_ptr<QuickStartJobStatusWatcher> QuickStartJobStatusWatcherPtr;

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio

#endif
