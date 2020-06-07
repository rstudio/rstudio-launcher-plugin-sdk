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
   LocalJobRunner(
      const std::string& in_hostname,
      jobs::JobStatusNotifierPtr in_notifier,
      std::shared_ptr<job_store::LocalJobStorage> in_jobStorage);

   Error initialize();

   Error runJob(api::JobPtr& io_job, api::ErrorResponse::Type& out_errorType);

private:
   typedef std::weak_ptr<LocalJobRunner> WeakLocalJobRunner;

   static void onJobExitCallback(WeakLocalJobRunner in_weakThis, int in_exitCode, api::JobPtr io_job);

   static void onProcessWatchDeadline(WeakLocalJobRunner in_weakThis, int in_count, api::JobPtr io_job);

   /** The name of the host running this job. */
   const std::string& m_hostname;

   /** The job storage. */
   std::shared_ptr<job_store::LocalJobStorage> m_jobStorage;

   /** The job status notifier, to update the status of the job on exit. */
   jobs::JobStatusNotifierPtr m_notifier;

   std::shared_ptr<system::AsyncDeadlineEvent> m_processWatchEvent;

   /** The secure cookie. */
   LocalSecureCookie m_secureCookie;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
