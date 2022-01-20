/*
 * AbstractTimedJobStatusWatcher.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_TIMED_JOB_STATUS_WATCHER_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_TIMED_JOB_STATUS_WATCHER_HPP

#include <jobs/AbstractJobStatusWatcher.hpp>

#include <memory>

#include <PImpl.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

/**
 * @brief Responsible for polling job statuses on a timer.
 */
class AbstractTimedJobStatusWatcher :
   public AbstractJobStatusWatcher,
   public std::enable_shared_from_this<AbstractTimedJobStatusWatcher>
{
public:
   /**
    * @brief Virtual destructor for inheritance. Invokes stop().
    */
   ~AbstractTimedJobStatusWatcher() noexcept override;

   /**
    * @brief Starts the timed job status watcher.
    */
   Error start() final;

   /**
    * @brief Stops the timed job status watcher.
    */
   void stop() final;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_frequency               The frequency at which job statuses should be polled.
    * @param in_jobRepository           The job repository, from which to look-up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post job updates.
    */
   AbstractTimedJobStatusWatcher(
      system::TimeDuration in_frequency,
      JobRepositoryPtr in_jobRepository,
      JobStatusNotifierPtr in_jobStatusNotifier);

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
   virtual Error pollJobStatus() = 0;

   PRIVATE_IMPL(m_timedBaseImpl);
};

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio

#endif
