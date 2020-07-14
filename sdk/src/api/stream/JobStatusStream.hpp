/*
 * JobStatusStream.hpp
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

#ifndef LAUNCHER_PLUGINS_JOB_STATUS_STREAM_HPP
#define LAUNCHER_PLUGINS_JOB_STATUS_STREAM_HPP

#include <api/stream/AbstractMultiStream.hpp>

#include <PImpl.hpp>
#include <api/Response.hpp>
#include <jobs/JobStatusNotifier.hpp>
#include <jobs/JobRepository.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

/** Convenience typedef. */
typedef AbstractMultiStream<JobStatusResponse, api::JobPtr> AbstractJobStatusStream;

/**
 * @brief Responsible for streaming Job Status data for a specific Job.
 */
class SingleJobStatusStream final :
   public AbstractJobStatusStream,
   public std::enable_shared_from_this<SingleJobStatusStream>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobId                   The ID of the job whose status should be streamed.
    * @param in_jobRepository           The job repository, from which the initial job state will be retrieved.
    * @param in_jobStatusNotifier       The job status notifier that will post updates about the job.
    * @param in_launcherCommunicator    The launcher communicator which will send the responses.
    */
   SingleJobStatusStream(
      std::string in_jobId,
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Adds a request to this JobStatusStream.
    *
    * @param in_requestId
    * @param in_requestUser
    */
   void addRequest(uint64_t in_requestId);

   /**
    * @brief Initializes the response stream.
    *
    * @return Success if the response stream could be initialized; Error otherwise.
    */
   Error initialize() override;

private:
   /**
    * @brief Sends the initial states for the given request, or all requests if none is specified.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_requestId   The ID of the request to send the initial job state to.
    */
   void sendInitialState(uint64_t in_requestId = 0);

   // The private implementation of JobStatusStream.
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Responsible for streaming Job Status data for all Jobs.
 */
class AllJobStatusStream final :
   public AbstractJobStatusStream,
   public std::enable_shared_from_this<AllJobStatusStream>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository           The job repository, from which the initial job states will be retrieved.
    * @param in_jobStatusNotifier       The job status notifier that will post updates about the jobs.
    * @param in_launcherCommunicator    The launcher communicator which will send the responses.
    */
   AllJobStatusStream(
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Adds a request to this JobStatusStream.
    *
    * @param in_requestId
    * @param in_requestUser
    */
   void addRequest(uint64_t in_requestId, const system::User& in_requestUser);

   /**
    * @brief Initializes the response stream.
    *
    * @return Success if the response stream could be initialized; Error otherwise.
    */
   Error initialize() override;

   /**
    * @brief Removes the request from this stream. Should be invoked when the request is canceled by the user.
    *
    * @param in_requestId   The ID of the request which is no longer listening to this stream.
    */
   void removeRequest(uint64_t in_requestId) override;

private:
   /**
    * @brief Gets the IDs of the requests which should be given information about the specified job.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_job     The job which was updated.
    *
    * @return The set of request IDs with permission to see the specified job's details.
    */
   std::set<uint64_t> getRequestIdsForJob(const JobPtr& in_job) const;

   /**
    * @brief Sends the initial states for the given request, or all requests if none is specified.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_requestId       The ID of the request for which to send all current states.
    */
   void sendInitialStates(uint64_t in_requestId = 0);

   // The private implementation of JobStatusStream.
   PRIVATE_IMPL(m_impl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
