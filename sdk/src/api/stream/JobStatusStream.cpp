/*
 * JobStatusStream.cpp
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

#include "JobStatusStream.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace api {

typedef std::shared_ptr<SingleJobStatusStream> SharedSingle;
typedef std::weak_ptr<SingleJobStatusStream> WeakSingle;
typedef std::shared_ptr<AllJobStatusStream> SharedAll;
typedef std::weak_ptr<AllJobStatusStream> WeakAll;

typedef std::map<uint64_t, system::User> RequestUserMap;

// Single Job Status Stream ============================================================================================
struct SingleJobStatusStream::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_jobId                   The ID of the job whose status should be streamed.
    * @param in_jobRepository           The job repository, from which the initial job state will be retrieved.
    * @param in_jobStatusNotifier       The job status notifier that will post updates about the job.
    */
   Impl(
      std::string in_jobId,
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier) :
         IsInitialized(false),
         JobId(std::move(in_jobId)),
         JobRepo(std::move(in_jobRepository)),
         Notifier(std::move(in_jobStatusNotifier))
   {
   }

   /** The JobStatus Subscription Handle. */
   jobs::SubscriptionHandle Handle;

   /** Whether or not the stream has been initialized. */
   bool IsInitialized;

   /** The ID of the job for which to stream status updates. */
   std::string JobId;

   /** The job repository from which to pull jobs. */
   jobs::JobRepositoryPtr JobRepo;

   /** The job status notifier, which will notify about new job updates. */
   jobs::JobStatusNotifierPtr Notifier;
};

PRIVATE_IMPL_DELETER_IMPL(SingleJobStatusStream)

SingleJobStatusStream::SingleJobStatusStream(
   std::string in_jobId,
   jobs::JobRepositoryPtr in_jobRepository,
   jobs::JobStatusNotifierPtr in_jobStatusNotifier,
   comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
      AbstractJobStatusStream(std::move(in_launcherCommunicator)),
      m_impl(new Impl(std::move(in_jobId), std::move(in_jobRepository), std::move(in_jobStatusNotifier)))
{
}

void SingleJobStatusStream::addRequest(uint64_t in_requestId, const system::User&)
{
   LOCK_MUTEX(m_mutex)
   {
      onAddRequest(in_requestId);
      if (m_impl->IsInitialized)
         sendInitialState(in_requestId);
   }
   END_LOCK_MUTEX
}

Error SingleJobStatusStream::initialize()
{
   // No need to lock the mutex here - this happens before we've initialized the multi-threaded aspect of this class
   // (i.e. The JobStatus Subscription.)

   // First send the initial job state. User permissions should have been validated before this was created.
   sendInitialState();

   // Then register for updates.
   WeakSingle weakThis = shared_from_this();
   jobs::OnJobStatusUpdate onJobStatusUpdate = [weakThis](const api::JobPtr& in_job)
   {
      if (SharedSingle sharedThis = weakThis.lock())
      {
         LOCK_MUTEX(sharedThis->m_mutex)
         {
            // If somehow we get a notification for the wrong job, just skip it.
            if (in_job->Id == sharedThis->m_impl->JobId)
            {
               LOCK_JOB(in_job)
               {
                  sharedThis->sendResponse(in_job);
               }
               END_LOCK_JOB
            }
         }
         END_LOCK_MUTEX
      }
   };

   m_impl->Handle = m_impl->Notifier->subscribe(m_impl->JobId, onJobStatusUpdate);

   m_impl->IsInitialized = true;
   return Success();
}

void SingleJobStatusStream::sendInitialState(uint64_t in_requestId)
{
   api::JobPtr job = m_impl->JobRepo->getJob(m_impl->JobId, system::User());

   LOCK_JOB(job)
   {
      if (in_requestId == 0)
         sendResponse(job);
      else
         sendResponse({ in_requestId }, job);
   }
   END_LOCK_JOB
}

// All Jobs Status Stream ==============================================================================================
struct AllJobStatusStream::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository           The job repository, from which the initial job state will be retrieved.
    * @param in_jobStatusNotifier       The job status notifier that will post updates about the job.
    */
   Impl(
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier):
         IsInitialized(false),
         JobRepo(std::move(in_jobRepository)),
         Notifier(std::move(in_jobStatusNotifier))
   {
   }

   /** The JobStatus Subscription Handle. */
   jobs::SubscriptionHandle Handle;

   /** Whether or not the stream has been initialized. */
   bool IsInitialized;

   /** The job repository from which to pull jobs. */
   jobs::JobRepositoryPtr JobRepo;

   /** The job status notifier, which will notify about new job updates. */
   jobs::JobStatusNotifierPtr Notifier;

   /** The map from Request ID to User. Used to filter job status responses based on permissions. */
   RequestUserMap RequestUsers;
};

PRIVATE_IMPL_DELETER_IMPL(AllJobStatusStream)

AllJobStatusStream::AllJobStatusStream(
   jobs::JobRepositoryPtr in_jobRepository,
   jobs::JobStatusNotifierPtr in_jobStatusNotifier,
   comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
      AbstractJobStatusStream(std::move(in_launcherCommunicator)),
      m_impl(new Impl(std::move(in_jobRepository), std::move(in_jobStatusNotifier)))
{
}

void AllJobStatusStream::addRequest(uint64_t in_requestId, const system::User& in_requestUser)
{
   LOCK_MUTEX(m_mutex)
   {
      auto itr = m_impl->RequestUsers.find(in_requestId);
      if (itr == m_impl->RequestUsers.end())
         m_impl->RequestUsers.emplace(in_requestId, in_requestUser);

      onAddRequest(in_requestId);
      if (m_impl->IsInitialized)
         sendInitialStates(in_requestId);
   }
   END_LOCK_MUTEX
}

Error AllJobStatusStream::initialize()
{
   // No need to lock the mutex here - this happens before we've initialized the multi-threaded aspect of this class
   // (i.e. The JobStatus Subscription.)

   // First send the initial job states.
   sendInitialStates();

   // Then register for updates.
   WeakAll weakThis = shared_from_this();
   jobs::OnJobStatusUpdate onJobStatusUpdate = [weakThis](const api::JobPtr& in_job)
   {
      if (SharedAll sharedThis = weakThis.lock())
      {
         LOCK_MUTEX(sharedThis->m_mutex)
         {
            LOCK_JOB(in_job)
            {
               sharedThis->sendResponse(sharedThis->getRequestIdsForJob(in_job), in_job);
            }
            END_LOCK_JOB
         }
         END_LOCK_MUTEX
      }
   };
   m_impl->Handle = m_impl->Notifier->subscribe(onJobStatusUpdate);

   m_impl->IsInitialized = true;
   return Success();
}

void AllJobStatusStream::removeRequest(uint64_t in_requestId)
{
   LOCK_MUTEX(m_mutex)
   {
      auto itr = m_impl->RequestUsers.find(in_requestId);
      if (itr != m_impl->RequestUsers.end())
         m_impl->RequestUsers.erase(in_requestId);

      onRemoveRequest(in_requestId);
   }
   END_LOCK_MUTEX
}

std::set<uint64_t> AllJobStatusStream::getRequestIdsForJob(const JobPtr& in_job) const
{
   std::set<uint64_t> requestIds;
   for (const auto& requestUser: m_impl->RequestUsers)
   {
      if (requestUser.second.isAllUsers() || requestUser.second == in_job->User)
         requestIds.insert(requestUser.first);
   }

   return requestIds;
}

void AllJobStatusStream::sendInitialStates(uint64_t in_requestId)
{
   if (in_requestId != 0)
   {
      const auto& itr = m_impl->RequestUsers.find(in_requestId);
      if (itr != m_impl->RequestUsers.end())
      {
         const JobList& jobs = m_impl->JobRepo->getJobs(itr->second);
         for (const auto& job: jobs)
         {
            LOCK_JOB(job)
            {
               sendResponse({ in_requestId }, job);
            }
            END_LOCK_JOB
         }
      }
   }
   else
   {
      // Get all the jobs and filter based on permission using getRequestIdsForJob.
      const JobList& jobs = m_impl->JobRepo->getJobs(system::User());
      for (const auto& job: jobs)
      {
         LOCK_JOB(job)
         {
            sendResponse(getRequestIdsForJob(job), job);
         }
         END_LOCK_JOB
      }
   }
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
