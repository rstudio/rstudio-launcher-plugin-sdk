/*
 * QuickStartJobSource.cpp
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

#include <QuickStartJobSource.hpp>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

QuickStartJobSource::QuickStartJobSource(
   const jobs::JobRepositoryPtr& in_jobRepository,
   const jobs::JobStatusNotifierPtr& in_jobStatusNotifier) :
   api::IJobSource(in_jobRepository, in_jobStatusNotifier),
   m_jobStatusWatcher(
         new QuickStartJobStatusWatcher(
               system::TimeDuration::Minutes(1),
               in_jobRepository,
               in_jobStatusNotifier))
{
   // TODO #11: Adjust the job status watcher frequency.
}

Error QuickStartJobSource::initialize()
{
   // TODO #6: Initialize communication with the job scheduling system. If communication fails, return an error.
   Error error = m_jobStatusWatcher->start();
   return error;
}

Error QuickStartJobSource::getConfiguration(
   const system::User& in_user,
   api::JobSourceConfiguration& out_configuration) const
{
   // TODO #7: Define cluster configuration.
   return Success();
}

Error QuickStartJobSource::getJobs(api::JobList &out_jobs) const
{
   // TODO #8: Pull all RStudio jobs from the job scheduling system synchronously.
   return Success();
}

Error QuickStartJobSource::submitJob(api::JobPtr io_job, bool& out_wasInvalidRequest) const
{
   // TODO #12: Submit and then update the job.
   out_wasInvalidRequest = true;
   return Error(
      "NotImplemented",
      1,
      "Method QuickStartJobSource::submitJob is not implemented.",
      ERROR_LOCATION);
}

Error QuickStartJobSource::createOutputStream(
   api::OutputType in_outputType,
   api::JobPtr in_job,
   api::AbstractOutputStream::OnOutput in_onOutput,
   api::AbstractOutputStream::OnComplete in_onComplete,
   api::AbstractOutputStream::OnError in_onError,
   api::OutputStreamPtr& out_outputStream)
{
   // TODO #13: Create an output stream.
   return Error(
      "NotImplemented",
      2,
      "Method QuickStartJobSource::createOutputStream is not implemented.",
      ERROR_LOCATION);
}

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio
