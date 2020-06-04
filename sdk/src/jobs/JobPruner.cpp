/*
 * JobPruner.cpp
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

#include "JobPruner.hpp"

#include <options/Options.hpp>
#include <system/Asio.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

namespace {

/**
 * @brief Checks whether a job state is a completed state.
 *
 * @param in_state      The state to check.
 *
 * @return True if the state is Canceled, Finished, or Failed; false otherwise.
 */
bool isCompletedState(api::Job::State in_state)
{
   return (in_state == api::Job::State::CANCELED) ||
      (in_state == api::Job::State::FAILED) ||
      (in_state == api::Job::State::FINISHED);
}

// Typedef for the map of Prune Deadline Events.
typedef std::map<std::string, std::shared_ptr<system::AsyncDeadlineEvent>> PruningMap;

} // namespace

struct JobPruner::Impl: public std::enable_shared_from_this<JobPruner::Impl>
{
   typedef std::shared_ptr<JobPruner::Impl> SharedThis;
   typedef std::weak_ptr<JobPruner::Impl> WeakThis;

   /**
    * @brief Constructor.
    *
    * @param in_jobRepository       The job repository from which pruned jobs should be removed.
    * @param in_jobStatusNotifier   The job status notifier.
    */
   Impl(
      JobRepositoryPtr in_jobRepository,
      JobStatusNotifierPtr in_jobStatusNotifier) :
         JobExpiryTime(std::move(options::Options::getInstance().getJobExpiryHours())),
         JobRepo(std::move(in_jobRepository)),
         Notifier(std::move(in_jobStatusNotifier))
   {
   }

   /**
    * @brief Prunes the job with the specified ID.
    *
    * @param in_jobId   The ID of the job to prune.
    */
   void pruneJob(const std::string& in_jobId)
   {
      LOCK_MUTEX(Mutex)
      {
         // Get the job as an admin user. If it doesn't exist, there's nothing to do.
         api::JobPtr job = JobRepo->getJob(in_jobId, system::User());
         if (job == nullptr)
            return;

         bool removeJob = false;
         LOCK_JOB(job)
         {
            // Check if we should remove the job.
            system::DateTime expiry = job->LastUpdateTime.getValueOr(job->SubmissionTime) + JobExpiryTime;
            removeJob = expiry <= system::DateTime();
            if (removeJob)
               JobRepo->removeJob(in_jobId);
         }
         END_LOCK_JOB

         if (removeJob)
         {
            auto itr = ActivePruneTasks.find(in_jobId);
            if (itr != ActivePruneTasks.end())
               ActivePruneTasks.erase(itr);
         }
      }
      END_LOCK_MUTEX
   }

   /**
    * @brief Callback function for job status notifier subscription. Schedules jobs for prune, if necessary.
    *
    * @param in_job     The job that was recently updated.
    */
   void onJobUpdate(const api::JobPtr& in_job)
   {
      LOCK_MUTEX(Mutex)
      {
         LOCK_JOB(in_job)
         {
            if (isCompletedState(in_job->Status))
            {
               const std::string& jobId = in_job->Id;
               WeakThis weakThis = shared_from_this();
               system::AsioFunction onDeadline = [weakThis, jobId]()
               {
                  if (SharedThis sharedThis = weakThis.lock())
                  {
                     sharedThis->pruneJob(jobId);
                  }
               };

               auto pruneEvent = std::make_shared<system::AsyncDeadlineEvent>(
                  onDeadline,
                  in_job->LastUpdateTime.getValueOr(in_job->SubmissionTime) + JobExpiryTime);
               ActivePruneTasks[jobId] = pruneEvent;
               pruneEvent->start();
            }
         }
         END_LOCK_JOB
      }
      END_LOCK_MUTEX
   }

   /**
    * @brief Starts the pruner. Should be called once from the main thread.
    */
   void start()
   {
      WeakThis weakThis = shared_from_this();
      OnJobStatusUpdate onJobStatusUpdate = [weakThis](const api::JobPtr& in_job)
      {
         if (SharedThis sharedThis = weakThis.lock())
            sharedThis->onJobUpdate(in_job);
      };

      AllJobsSubHandle = Notifier->subscribe(onJobStatusUpdate);
   }

   /**
    * @brief The map of actively scheduled prune tasks.
    */
   PruningMap ActivePruneTasks;

   /**
    * @brief The subscription handle which needs to be kept alive to keep getting job status update notifications.
    */
   SubscriptionHandle AllJobsSubHandle;

   /**
    * @brief The amount of time from the last update of a job until it should be pruned.
    */
   system::TimeDuration JobExpiryTime;

   /**
    * @brief The Job Repository to remove jobs from.
    */
   JobRepositoryPtr JobRepo;

   /**
    * @brief Mutex to protect the PruningMap.
    */
   std::mutex Mutex;

   /**
    * @brief The Job Status Notifier.
    */
   JobStatusNotifierPtr Notifier;
};

JobPruner::JobPruner(
   JobRepositoryPtr in_jobRepository,
   JobStatusNotifierPtr in_jobStatusNotifier) :
   m_impl(new Impl(std::move(in_jobRepository), std::move(in_jobStatusNotifier)))
{
   m_impl->start();
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
