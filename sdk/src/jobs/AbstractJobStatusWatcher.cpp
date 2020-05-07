/*
 * AbstractJobStatusWatcher.cpp
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

#include <jobs/AbstractJobStatusWatcher.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

struct AbstractJobStatusWatcher::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository           The job repository, from which to look-up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post job updates.
    */
   Impl(JobRepositoryPtr in_jobRepository, JobStatusNotifierPtr in_jobStatusNotifier) :
      JobRepo(std::move(in_jobRepository)),
      Notifier(std::move(in_jobStatusNotifier))
   {
   }

   /** The job repository. */
   JobRepositoryPtr  JobRepo;

   /** The job status notifier. */
   JobStatusNotifierPtr Notifier;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractJobStatusWatcher)

AbstractJobStatusWatcher::AbstractJobStatusWatcher(
   JobRepositoryPtr in_jobRepository,
   JobStatusNotifierPtr in_jobStatusNotifier) :
      m_baseImpl(new Impl(std::move(in_jobRepository), std::move(in_jobStatusNotifier)))
{
}

Error AbstractJobStatusWatcher::updateJobStatus(
   const std::string& in_jobId,
   api::Job::State in_newStatus,
   const std::string& in_statusMessage,
   const system::DateTime& in_invocationTime)
{
   api::JobPtr job = m_baseImpl->JobRepo->getJob(in_jobId);
   if (!job)
   {
      Error error = getJobDetails(in_jobId, job);
      if (error)
         return error;
   }

   updateJobStatus(job, in_newStatus, in_statusMessage, in_invocationTime);
   return Success();
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
