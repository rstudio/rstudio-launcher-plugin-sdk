/*
 * ResourceStreamManager.cpp
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

#include "ResourceStreamManager.hpp"

#include <map>

#include <logging/Logger.hpp>
#include <api/IJobSource.hpp>
#include <api/Request.hpp>
#include <api/stream/AbstractResourceStream.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>
#include <jobs/AbstractJobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

struct ResourceStream
{
   ResourceStream(AbstractResourceStreamPtr in_stream, jobs::SubscriptionHandle in_handle) :
      Stream(in_stream),
      SubHandle(in_handle),
      IsInitialized(false)
   {
   }

   ResourceStream() = default;

   AbstractResourceStreamPtr Stream;
   jobs::SubscriptionHandle SubHandle;
   bool IsInitialized;
};

typedef std::map<std::string, ResourceStream> ResourceStreamMap;

struct ResourceStreamManager::Impl : public std::enable_shared_from_this<Impl>
{
   typedef std::shared_ptr<Impl> SharedThis;
   typedef std::weak_ptr<Impl> WeakThis;

   Impl(
      std::shared_ptr<IJobSource>&& in_jobSource,
      jobs::JobRepositoryPtr&& in_jobRepository,
      jobs::JobStatusNotifierPtr&& in_notifier,
      comms::AbstractLauncherCommunicatorPtr&& in_launcherCommunicator) :
         JobSource(in_jobSource),
         JobRepo(in_jobRepository),
         Notifier(in_notifier),
         LauncherCommunicator(in_launcherCommunicator)
   {
   }

   /**
    * @brief Sends a "Job Not Found" error to the Launcher.
    *
    * @param in_requestId       The ID of the request for which this response should be sent.
    * @param in_jobId           The ID of the job which could not be found.
    * @param in_requestUser     The user who made the request.
    */
   void sendJobNotFoundError(uint64_t in_requestId, const std::string& in_jobId, const system::User& in_requestUser)
   {
      std::string message = "Job " +
                            in_jobId +
                            " could not be found" +
                            (in_requestUser.isAllUsers() ? "" : (" for user " + in_requestUser.getUsername())) + ".";
      LauncherCommunicator->sendResponse(
         ErrorResponse(in_requestId, ErrorResponse::Type::JOB_NOT_FOUND, message));
   }

   jobs::SubscriptionHandle watchJob(const std::string& in_jobId)
   {
      WeakThis weakThis = weak_from_this();
      return Notifier->subscribe(in_jobId, [weakThis](ConstJobPtr in_job)
      {
         if (SharedThis sharedThis = weakThis.lock())
         {
            LOCK_MUTEX(sharedThis->Mutex)
            {
               LOCK_JOB(in_job)
               {
                  auto itr = sharedThis->ActiveStreams.find(in_job->Id);
                  if (itr == sharedThis->ActiveStreams.end())
                     return;

                  // If the job newly entered a completed state, cancel the stream and forget about it.
                  if (in_job->isCompleted())
                  {
                     itr->second.Stream->setStreamComplete();
                     sharedThis->ActiveStreams.erase(itr);
                  }
                  // If the job recently entered the running state, ensure the stream is initialized.
                  else if ((in_job->Status == Job::State::RUNNING) && !itr->second.IsInitialized)
                  {
                     Error error = itr->second.Stream->initialize();
                     if (error)
                     {
                        logging::logErrorMessage(
                           "An error occurred while initializing resource utilization metric streaming for Job " +
                              in_job->Id);
                        logging::logError(error);
                        itr->second.Stream->setStreamComplete();
                        sharedThis->ActiveStreams.erase(itr);
                        return;
                     }

                     itr->second.IsInitialized = true;
                  }
               }
               END_LOCK_JOB
            }
            END_LOCK_MUTEX
         }
      });
   }

   std::shared_ptr<IJobSource> JobSource;
   jobs::JobRepositoryPtr JobRepo;
   jobs::JobStatusNotifierPtr Notifier;
   comms::AbstractLauncherCommunicatorPtr LauncherCommunicator;

   std::mutex Mutex;
   ResourceStreamMap ActiveStreams;
};

ResourceStreamManager::ResourceStreamManager(
      std::shared_ptr<IJobSource> in_jobSource,
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
         m_impl(
            new Impl(
               std::move(in_jobSource),
               std::move(in_jobRepository),
               std::move(in_jobStatusNotifier),
               std::move(in_launcherCommunicator)))
{
}

void ResourceStreamManager::handleStreamRequest(
   const std::shared_ptr<ResourceUtilStreamRequest>& in_resourceUtilStreamRequest)
{
   uint64_t id = in_resourceUtilStreamRequest->getId();
   const std::string& jobId = in_resourceUtilStreamRequest->getJobId();
   const system::User& user = in_resourceUtilStreamRequest->getUser();

   LOCK_MUTEX(m_impl->Mutex)
   {
      ConstJobPtr job = m_impl->JobRepo->getJob(jobId, user);
      if (!job)
         return m_impl->sendJobNotFoundError(id, jobId, user);

      auto itr = m_impl->ActiveStreams.find(jobId);
      if (itr == m_impl->ActiveStreams.end())
      {
         AbstractResourceStreamPtr stream;
         Error error = m_impl->JobSource->createResourceStream(job, m_impl->LauncherCommunicator, stream);
         if (error)
         {
            return m_impl->LauncherCommunicator->sendResponse(
               ErrorResponse(id, ErrorResponse::Type::UNKNOWN, error.getSummary()));
         }

         LOCK_JOB(job)
         {
            if (in_resourceUtilStreamRequest->isCancelRequest() || job->isCompleted())
               return;

            if (job->Status == Job::State::RUNNING)
               stream->initialize();

            m_impl->ActiveStreams[jobId] = ResourceStream(stream, m_impl->watchJob(jobId));
         }
         END_LOCK_JOB
      }
      else
      {
         if (in_resourceUtilStreamRequest->isCancelRequest())
         {
            itr->second.Stream->removeRequest(id);
            if (itr->second.Stream->isEmpty())
               m_impl->ActiveStreams.erase(itr);
         }
         else
            itr->second.Stream->addRequest(id, in_resourceUtilStreamRequest->getUser());
      }
   }
   END_LOCK_MUTEX
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio