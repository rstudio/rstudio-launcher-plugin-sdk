/*
 * QuickStartJobStatusWatcher.cpp
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

#include <QuickStartJobStatusWatcher.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

QuickStartJobStatusWatcher::QuickStartJobStatusWatcher(
   system::TimeDuration in_frequency,
   jobs::JobRepositoryPtr in_jobRepository,
   jobs::JobStatusNotifierPtr in_jobStatusNotifier) :
      jobs::AbstractTimedJobStatusWatcher(
         std::move(in_frequency),
         std::move(in_jobRepository),
         std::move(in_jobStatusNotifier))
{
}

Error QuickStartJobStatusWatcher::pollJobStatus()
{
   // TODO #9: Poll the Job Scheduling System for job status updates. Invoke AbstractJobStatusWatcher::updateJobStatus
   //          for each updated job.
   return Success();
}

Error QuickStartJobStatusWatcher::getJobDetails(const std::string& in_jobId, api::JobPtr& out_job) const
{
   // TODO #10: Get the full details of the requested job from the Job Scheduling System, and remove the placeholder
   //           error below.
   return Error(
      "NotImplemented",
      1,
      "Method QuickStartJobStatusWatcher::getJobDetails is not implemented.",
      ERROR_LOCATION);
}

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio
