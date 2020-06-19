/*
 * OutputStreamManager.cpp
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


#include "OutputStreamManager.hpp"

#include <api/IJobSource.hpp>
#include <api/Request.hpp>
#include <api/Response.hpp>
#include <api/stream/AbstractOutputStream.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>
#include <jobs/JobRepository.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {
typedef std::map<uint64_t, std::shared_ptr<AbstractOutputStream> > OutputStreamMap;

struct OutputStreamManager::Impl : public std::enable_shared_from_this<Impl>
{
   typedef std::shared_ptr<Impl> SharedThis;
   typedef std::weak_ptr<Impl> WeakThis;

   void removeOutputStream(uint64_t in_requestId)
   {
      auto itr = ActiveOutputStreams.find(in_requestId);
      if (itr != ActiveOutputStreams.end())
         ActiveOutputStreams.erase(itr);
   }

   /**
    * @brief Sends a job not found error to the launcher.
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

   /** The mutex to protect the map of active output streams. */
   std::mutex Mutex;

   /** The map of open output streams. */
   OutputStreamMap ActiveOutputStreams;

   /** The job repository. */
   jobs::JobRepositoryPtr JobRepo;

   /** The job source. */
   std::shared_ptr<IJobSource> JobSource;

   /** The launcher communicator. */
   comms::AbstractLauncherCommunicatorPtr LauncherCommunicator;
};

void OutputStreamManager::handleStreamRequest(const std::shared_ptr<OutputStreamRequest>& in_outputStreamRequest)
{
   uint64_t requestId = in_outputStreamRequest->getId();
   bool isCancel = in_outputStreamRequest->isCancelRequest();
   const std::string& jobId = in_outputStreamRequest->getJobId();
   const system::User& jobUser = in_outputStreamRequest->getUser();

   LOCK_MUTEX(m_impl->Mutex)
   {
      auto itr = m_impl->ActiveOutputStreams.find(requestId);
      if (itr != m_impl->ActiveOutputStreams.end())
      {
         if (isCancel)
         {
            itr->second->stop();
            m_impl->ActiveOutputStreams.erase(itr);
         }
         else
         {
            logging::logDebugMessage(
               "Received duplicate output stream request (" +
               std::to_string(requestId) +
               ") for job " +
               jobId);
         }
      }
      else if (!isCancel)
      {
         JobPtr job = m_impl->JobRepo->getJob(jobId, jobUser);
         if (!job)
            return m_impl->sendJobNotFoundError(requestId, jobId, jobUser);

         Impl::WeakThis weakThis = m_impl->weak_from_this();
         AbstractOutputStream::OnOutput onOutput = [weakThis, requestId](
            const std::string& in_output,
            OutputType in_outputType,
            uint64_t in_sequenceId)
         {
            if (Impl::SharedThis sharedThis = weakThis.lock())
            {
               LOCK_MUTEX(sharedThis->Mutex)
               {
                  if (sharedThis->ActiveOutputStreams.find(requestId) != sharedThis->ActiveOutputStreams.end())
                     sharedThis->LauncherCommunicator->sendResponse(
                        OutputStreamResponse(requestId, in_sequenceId, in_output, in_outputType));
               }
               END_LOCK_MUTEX
            }
         };

         AbstractOutputStream::OnComplete onComplete = [weakThis, requestId](uint64_t in_sequenceId)
         {
            if (Impl::SharedThis sharedThis = weakThis.lock())
            {
               LOCK_MUTEX(sharedThis->Mutex)
               {
                  auto itr = sharedThis->ActiveOutputStreams.find(requestId);
                  if (itr != sharedThis->ActiveOutputStreams.end())
                  {
                     sharedThis->LauncherCommunicator->sendResponse(OutputStreamResponse(requestId, in_sequenceId));
                     sharedThis->ActiveOutputStreams.erase(itr);
                  }
               }
               END_LOCK_MUTEX
            }
         };

         AbstractOutputStream::OnError onError = [weakThis, requestId](const Error& in_error)
         {
            if (Impl::SharedThis sharedThis = weakThis.lock())
            {
               LOCK_MUTEX(sharedThis->Mutex)
               {
                  auto itr = sharedThis->ActiveOutputStreams.find(requestId);
                  if (itr != sharedThis->ActiveOutputStreams.end())
                  {
                     sharedThis->LauncherCommunicator->sendResponse(
                        ErrorResponse(requestId, ErrorResponse::Type::JOB_OUTPUT_NOT_FOUND, in_error.getSummary()));
                     sharedThis->ActiveOutputStreams.erase(itr);
                  }
               }
               END_LOCK_MUTEX
            }
         };

         OutputStreamPtr outputStream;
         Error error = m_impl->JobSource->createOutputStream(
            in_outputStreamRequest->getStreamType(),
            job,
            onOutput,
            onComplete,
            onError,
            outputStream);

         if (error || !outputStream)
         {
            m_impl->LauncherCommunicator->sendResponse(
               ErrorResponse(
                  requestId,
                  ErrorResponse::Type::JOB_OUTPUT_NOT_FOUND,
                  (error ?  error.getSummary() : "Output stream could not be created." )));
         }
         else
         {

         }
      }
   }
   END_LOCK_MUTEX
}

}
} // namespace launcher_plugins
} // namespace rstudio
