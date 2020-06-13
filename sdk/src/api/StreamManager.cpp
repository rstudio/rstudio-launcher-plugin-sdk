/*
 * StreamManager.cpp
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


#include "StreamManager.hpp"

#include <api/AbstractOutputStream.hpp>
#include <api/Request.hpp>

#include "JobStatusStream.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace api {

// Typedefs
typedef std::map<std::string, std::shared_ptr<SingleJobStatusStream> > JobStatusStreamMap;

typedef std::map<std::string, std::shared_ptr<AbstractOutputStream> > OutputStreamMap;

struct StreamManager::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_jobSource               The interface for interacting with the job source.
    * @param in_jobRepository           The job repository from which to retrieve jobs.
    * @param in_jobStatusNotifier       The job status notifier from which to receive job status update notifications.
    * @param in_launcherCommunicator    The communicator which may be used to send stream responses to the Launcher.
    */
   Impl(
      std::shared_ptr<IJobSource>&& in_jobSource,
      jobs::JobRepositoryPtr&& in_jobRepository,
      jobs::JobStatusNotifierPtr&& in_jobStatusNotifier,
      comms::AbstractLauncherCommunicatorPtr&& in_launcherCommunicator) :
         JobRepo(in_jobRepository),
         JobSource(in_jobSource),
         LauncherCommunicator(in_launcherCommunicator),
         Notifier(in_jobStatusNotifier)
   {
   }

   /**
    * @brief Adds a request to the AllJobStatusStream, creating a new stream if necessary.
    *
    * @param in_requestId       The ID of the request for which this stream should be open.
    * @param in_requestUser     The user who made the request.
    *
    * @return Success if the stream could be created and the request added it to; Error otherwise.
    */
   Error addAllJobsStream(uint64_t in_requestId, const system::User& in_requestUser)
   {
      if (AllJobsStream == nullptr)
      {
         AllJobsStream.reset(
            new AllJobStatusStream(JobRepo, Notifier, LauncherCommunicator));

         AllJobsStream->addRequest(in_requestId, in_requestUser);
         Error error = AllJobsStream->initialize();
         if (error)
         {
            AllJobsStream.reset();
            return error;
         }
      }
      else
         AllJobsStream->addRequest(in_requestId, in_requestUser);

      return Success();
   }

   /**
    * @brief Adds a request to the correct SingleJobStatusStream, creating a new stream if necessary.
    *
    * @param in_requestId       The ID of the request for which this stream should be open.
    * @param in_requestUser     The user who made the request.
    *
    * @return Success if the stream could be created and the request added it to; Error otherwise.
    */
   Error addJobStream(uint64_t in_requestId, const std::string& in_jobId, const system::User& in_requestUser)
   {
      if (!JobRepo->getJob(in_jobId, in_requestUser))
      {
         std::string message = "Job " +
            in_jobId +
            " could not be found" +
            (in_requestUser.isAllUsers() ? "" : (" for user " + in_requestUser.getUsername())) + ".";
         LauncherCommunicator->sendResponse(
            ErrorResponse(in_requestId, ErrorResponse::Type::JOB_NOT_FOUND, message));
      }
      else
      {
         auto itr = ActiveJobStreams.find(in_jobId);
         if (itr == ActiveJobStreams.end() || (itr->second == nullptr))
         {
            ActiveJobStreams[in_jobId] = std::make_shared<SingleJobStatusStream>(
               in_jobId,
               JobRepo,
               Notifier,
               LauncherCommunicator);

            itr = ActiveJobStreams.find(in_jobId);
            itr->second->addRequest(in_requestId);
            Error error = itr->second->initialize();
            if (error)
            {
               ActiveJobStreams.erase(itr);
               return error;
            }
         }
         else
            itr->second->addRequest(in_requestId);
      }

      return Success();
   }

   /**
    * @brief Cancels an AllJobStatusStream for the specified request. If the stream is empty, it will be deleted.
    *
    * @param in_requestId   The ID of the request for which to cancel the stream.
    */
   void cancelAllJobsStream(uint64_t in_requestId)
   {
      if (AllJobsStream != nullptr)
      {
         AllJobsStream->removeRequest(in_requestId);
         if (AllJobsStream->isEmpty())
            AllJobsStream.reset();
      }
   }


   /**
    * @brief Cancels the correct SingleJobsStatusStream for the specified request. If the stream is empty,
    *        it will be deleted.
    *
    * @param in_requestId   The ID of the request for which to cancel the stream.
    * @param in_jobId       The ID of the job which was being streamed.
    */
   void cancelJobStream(uint64_t in_requestId, const std::string& in_jobId)
   {
      auto itr = ActiveJobStreams.find(in_jobId);
      if (itr != ActiveJobStreams.end())
      {
         itr->second->removeRequest(in_requestId);
         if (itr->second->isEmpty())
            ActiveJobStreams.erase(itr);
      }
   }

   /** The map of Job IDs to their active job streams. */
   JobStatusStreamMap ActiveJobStreams;

   /** The map of open output streams. */
   OutputStreamMap ActiveOutputStreams;

   /** The mutex to protect the map of active Job status streams. */
   std::mutex ActiveJobStreamsMutex;

   /** The mutex to protect the map of active output streams. */
   std::mutex ActiveOutputStreamsMutex;

   /** The mutex to protect the All Jobs status stream. */
   std::mutex AllJobsMutex;

   /** The stream for "all jobs" stream requests. */
   std::shared_ptr<AllJobStatusStream> AllJobsStream;

   /** The job repository. */
   jobs::JobRepositoryPtr JobRepo;

   /** The job source. */
   std::shared_ptr<IJobSource> JobSource;

   /** The launcher communicator. */
   comms::AbstractLauncherCommunicatorPtr LauncherCommunicator;

   /** The job status notifier. */
   jobs::JobStatusNotifierPtr Notifier;
};

PRIVATE_IMPL_DELETER_IMPL(StreamManager)

StreamManager::StreamManager(
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

void StreamManager::handleStreamRequest(const std::shared_ptr<JobStatusRequest>& in_jobStatusRequest)
{
   if (in_jobStatusRequest->getJobId() == "*")
   {
      LOCK_MUTEX(m_impl->AllJobsMutex)
      {
         if (in_jobStatusRequest->isCancelRequest())
            m_impl->cancelAllJobsStream(in_jobStatusRequest->getId());
         else
         {
            Error error = m_impl->addAllJobsStream(in_jobStatusRequest->getId(), in_jobStatusRequest->getUser());
            if (error)
               m_impl->LauncherCommunicator->sendResponse(
                  ErrorResponse(in_jobStatusRequest->getId(), ErrorResponse::Type::UNKNOWN, error.getSummary()));
         }
      }
      END_LOCK_MUTEX
   }
   else
   {
      LOCK_MUTEX(m_impl->ActiveJobStreamsMutex)
      {
         if (in_jobStatusRequest->isCancelRequest())
            m_impl->cancelJobStream(in_jobStatusRequest->getId(), in_jobStatusRequest->getJobId());
         else
         {
            Error error = m_impl->addJobStream(
               in_jobStatusRequest->getId(),
               in_jobStatusRequest->getJobId(),
               in_jobStatusRequest->getUser());

            if (error)
               m_impl->LauncherCommunicator->sendResponse(
                  ErrorResponse(in_jobStatusRequest->getId(), ErrorResponse::Type::UNKNOWN, error.getSummary()));
         }
      }
      END_LOCK_MUTEX
   }
}

void StreamManager::handleStreamRequest(const std::shared_ptr<OutputStreamRequest>& in_outputStreamRequest)
{

}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
