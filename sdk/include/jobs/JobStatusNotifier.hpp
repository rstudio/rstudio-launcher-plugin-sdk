/*
 * JobStatusNotifier.hpp
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

#ifndef LAUNCHER_PLUGINS_JOB_STATUS_NOTIFIER_HPP
#define LAUNCHER_PLUGINS_JOB_STATUS_NOTIFIER_HPP

#include <Noncopyable.hpp>

#include <string>
#include <functional>
#include <memory>

#include <PImpl.hpp>
#include <api/Job.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

// Forward declarations
class AbstractJobRepository;

struct Subscription;
typedef std::shared_ptr<Subscription> SubscriptionHandle;

/** @brief Function which is invoked whe a subscribed job is updated. */
typedef std::function<void(const api::JobPtr&)> OnJobStatusUpdate;

/**
 * @brief Class which notifies subscribers when a job updates.
 */
class JobStatusNotifier final :
   public Noncopyable,
   public std::enable_shared_from_this<JobStatusNotifier>
{
public:
   /**
    * @brief Constructor.
    */
   JobStatusNotifier();

   /**
    * @brief Subscribes to all jobs.
    *
    * @param in_onJobStatusUpdate   The function to be invoked when any job is updated.
    *
    * @return A handle to the job subscription. To end the subscription, allow the handle to fall out of scope.
    */
   SubscriptionHandle subscribe(const OnJobStatusUpdate& in_onJobStatusUpdate);


   /**
    * @brief Subscribes to a specific job.
    *
    * @param in_jobId               The ID of the job to subscribe to.
    * @param in_onJobStatusUpdate   The function to be invoked when the job is updated.
    *
    * @return A handle to the job subscription. To end the subscription, allow the handle to fall out of scope.
    */
   SubscriptionHandle subscribe(const std::string& in_jobId, const OnJobStatusUpdate& in_onJobStatusUpdate);

   /**
    * @brief Updates the status of a job with a new status and optionally a new status message.
    *
    * @param in_job                 The job to be updated.
    * @param in_newStatus           The new status of the job.
    * @param in_statusMessage       The new status message of the job. Default: "".
    * @param in_invocationTime      The time at which this method was invoked. Default: Current Time. If there is
    *                               concern about time differences between the RStudio Launcher Host and the Job
    *                               Scheduling System, this may be overridden with the  time from the point of view of
    *                               the Job Scheduling System.
    */
   void updateJob(
      const api::JobPtr& in_job,
      api::Job::State in_newStatus,
      const std::string& in_statusMessage = "",
      const system::DateTime& in_invocationTime = system::DateTime());

private:
   // The private implementation of JobStatusNotifier.
   PRIVATE_IMPL(m_impl);

   friend class Subscription;
};

/** Convenience typedef */
typedef std::shared_ptr<JobStatusNotifier> JobStatusNotifierPtr;

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio

#endif
