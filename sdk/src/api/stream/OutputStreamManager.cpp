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

#include <cassert>

#include <api/IJobSource.hpp>
#include <api/Request.hpp>
#include <api/Response.hpp>
#include <api/stream/AbstractOutputStream.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>
#include <jobs/AbstractJobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

struct OutputStream
{
   OutputStream() :
      IsStarted(false)
   {
   }

   OutputStream(
      OutputStreamPtr in_stream,
      jobs::SubscriptionHandle&& in_handle,
      bool in_isStarted) :
      Stream(in_stream),
      SubscriptionHandle(in_handle),
      IsStarted(in_isStarted)
   {
   }

   OutputStreamPtr Stream;
   jobs::SubscriptionHandle SubscriptionHandle;
   bool IsStarted;
};

} // anonymous namespace

// Convenience typedef
typedef std::map<uint64_t, OutputStream> OutputStreamMap;

struct OutputStreamManager::Impl : public std::enable_shared_from_this<Impl>
{
   typedef std::shared_ptr<Impl> SharedThis;
   typedef std::weak_ptr<Impl> WeakThis;

   /**
    * @brief Constructor.
    *
    * @param in_jobSource               The job source, which creates output streams.
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
    * @brief Sends a job output stream completion response to the Launcher.
    *
    * @param in_requestId       The ID of the request for which this response is being sent.
    * @param in_sequenceId      The ID of this response in the sequence of responses for the specified request.
    */
   void sendCompleteResponse(uint64_t in_requestId, uint64_t in_sequenceId)
   {
      UNIQUE_LOCK_MUTEX(Mutex)
      {
         auto itr = ActiveOutputStreams.find(in_requestId);
         if (itr != ActiveOutputStreams.end())
         {
            LauncherCommunicator->sendResponse(OutputStreamResponse(in_requestId, in_sequenceId));
            ActiveOutputStreams.erase(itr);
         }
      }
      END_LOCK_MUTEX
   }

   /**
    * @brief Sends a "Job Not Found" error to the launcher.
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

   /**
    * @brief Sends a "Job Output Not Found" error to the Launcher.
    *
    * If called before the stream starts, holding the lock is not necessary. Otherwise, the lock should be held when
    * this is called.
    *
    * @param in_requestId   The ID of the request for which the job output could not be found.
    * @param in_error       The error which occurred, if any.
    */
   void sendJobOutputNotFoundError(uint64_t in_requestId, const Error& in_error)
   {
      LauncherCommunicator->sendResponse(
         ErrorResponse(
            in_requestId,
            ErrorResponse::Type::JOB_OUTPUT_NOT_FOUND,
            in_error ?  in_error.getSummary() : "Output stream could not be created."));
   }

   /**
    * @brief Sends a job output stream response to the Launcher.
    *
    * @param in_requestId       The ID of the request for which this response is being sent.
    * @param in_sequenceId      The ID of this response in the sequence of responses for the specified request.
    * @param in_output          The output to send.
    * @param in_outputType      The type of the output being sent.
    */
   void sendOutputResponse(
      uint64_t in_requestId,
      uint64_t in_sequenceId,
      const std::string& in_output,
      OutputType in_outputType)
   {
      UNIQUE_LOCK_MUTEX(Mutex)
      {
         if (ActiveOutputStreams.find(in_requestId) != ActiveOutputStreams.end())
            LauncherCommunicator->sendResponse(
               OutputStreamResponse(in_requestId, in_sequenceId, in_output, in_outputType));
      }
      END_LOCK_MUTEX
   }

   /**
    * @brief Sends a "Job Output Not Found" error to the Launcher and removes the output stream.
    *
    * This should only be invoked if an error occurs after the stream has started.
    *
    * @param in_requestId   The ID of the request for which the job output could not be found.
    * @param in_error       The error which occurred.
    */
   void sendStreamErrorResponse(uint64_t in_requestId, const Error& in_error, const std::unique_lock<std::mutex>& in_lock)
   {
      assert(in_lock.owns_lock());
      auto itr = ActiveOutputStreams.find(in_requestId);
      if (itr != ActiveOutputStreams.end())
      {
         sendJobOutputNotFoundError(in_requestId, in_error);
         ActiveOutputStreams.erase(itr);
      }
   }

   /**
    * @brief Sends a "Job Output Not Found" error to the Launcher and removes the output stream.
    *
    * This should only be invoked if an error occurs after the stream has started.
    *
    * @param in_requestId   The ID of the request for which the job output could not be found.
    * @param in_error       The error which occurred.
    */
   void sendStreamErrorResponse(uint64_t in_requestId, const Error& in_error)
   {
      UNIQUE_LOCK_MUTEX(Mutex)
      {
         sendStreamErrorResponse(in_requestId, in_error, uniqueLock);
      }
      END_LOCK_MUTEX
   }

   /**
    * @brief Starts the output stream.
    *
    * The lock must be held when this method is invoked.
    *
    * @param in_requestId       The ID of the request for which this stream was opened.
    * @param in_job             The job for which the output stream was opened.
    * @param in_outputStream    The output stream to start.
    */
   void startStream(uint64_t in_requestId, const JobPtr& in_job, const OutputStreamPtr& in_outputStream)
   {
      bool isStarted = false;
      if (in_job->Status != Job::State::PENDING)
      {
         Error error = in_outputStream->start();
         if (error)
            return sendJobOutputNotFoundError(in_requestId, error);

         isStarted = true;
      }

      WeakThis weakThis = weak_from_this();
      jobs::SubscriptionHandle handle = Notifier->subscribe(
         in_job->Id,
         [weakThis, in_requestId](const JobPtr& in_job)
         {
            if (SharedThis sharedThis = weakThis.lock())
            {
               // Always lock the stream manager mutex before the job mutex to prevent possible deadlock.
               UNIQUE_LOCK_MUTEX(sharedThis->Mutex)
               {
                  auto itr = sharedThis->ActiveOutputStreams.find(in_requestId);
                  if (itr == sharedThis->ActiveOutputStreams.end())
                     return; // Do nothing if the stream has already been removed.

                  // Lock the job while we check the state.
                  bool startStream = false, closeStream = false;
                  LOCK_JOB(in_job)
                  {
                     startStream = (!itr->second.IsStarted && (in_job->Status != Job::State::PENDING));
                     closeStream = in_job->isCompleted();
                  }
                  END_LOCK_JOB

                  if (startStream)
                  {
                     Error error = itr->second.Stream->start();
                     if (error)
                        return sharedThis->sendStreamErrorResponse(in_requestId, error, uniqueLock);

                     itr->second.IsStarted = true;
                  }

                  if (closeStream)
                  {
                     itr->second.Stream->stop();
                     sharedThis->ActiveOutputStreams.erase(itr);
                  }
               }
               END_LOCK_MUTEX
            }
         });

      ActiveOutputStreams[in_requestId] = OutputStream(in_outputStream, std::move(handle), isStarted);
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

   /** The job status notifier. */
   jobs::JobStatusNotifierPtr Notifier;
};

OutputStreamManager::OutputStreamManager(
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

void OutputStreamManager::handleStreamRequest(const std::shared_ptr<OutputStreamRequest>& in_outputStreamRequest)
{
   uint64_t requestId = in_outputStreamRequest->getId();
   bool isCancel = in_outputStreamRequest->isCancelRequest();
   const std::string& jobId = in_outputStreamRequest->getJobId();
   const system::User& jobUser = in_outputStreamRequest->getUser();

   UNIQUE_LOCK_MUTEX(m_impl->Mutex)
   {
      auto itr = m_impl->ActiveOutputStreams.find(requestId);
      if (itr != m_impl->ActiveOutputStreams.end())
      {
         if (isCancel)
         {
            itr->second.Stream->stop();
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

         // Lock the job while we create the stream.
         LOCK_JOB(job)
         {
            OutputStreamPtr outputStream;
            Impl::WeakThis weakThis = m_impl->weak_from_this();
            Error error = m_impl->JobSource->createOutputStream(
               in_outputStreamRequest->getStreamType(),
               job,
               [weakThis, requestId](
                  const std::string& in_output,
                  OutputType in_outputType,
                  uint64_t in_sequenceId)
               {
                  if (Impl::SharedThis sharedThis = weakThis.lock())
                     sharedThis->sendOutputResponse(requestId, in_sequenceId, in_output, in_outputType);
               },
               [weakThis, requestId](uint64_t in_sequenceId)
               {
                  if (Impl::SharedThis sharedThis = weakThis.lock())
                     sharedThis->sendCompleteResponse(requestId, in_sequenceId);
               },
               [weakThis, requestId](const Error& in_error)
               {
                  if (Impl::SharedThis sharedThis = weakThis.lock())
                     sharedThis->sendStreamErrorResponse(requestId, in_error);
               },
               outputStream);

            if (error || !outputStream)
               m_impl->sendJobOutputNotFoundError(requestId, error);
            else
            {
               m_impl->startStream(requestId, job, outputStream);
            }
         }
         END_LOCK_JOB
      }
   }
   END_LOCK_MUTEX
}

}
} // namespace launcher_plugins
} // namespace rstudio
