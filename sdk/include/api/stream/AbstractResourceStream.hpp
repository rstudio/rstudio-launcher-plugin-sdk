/*
 * AbstractResourceStream.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_RESOURCE_STREAM_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_RESOURCE_STREAM_HPP

#include <api/stream/AbstractMultiStream.hpp>

#include <Error.hpp>
#include <PImpl.hpp>
#include <api/Response.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>
#include <jobs/JobStatusNotifier.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Streams job resource utilization data to the Launcher.
 */
class AbstractResourceStream :
   public AbstractMultiStream<ResourceUtilStreamResponse, ResourceUtilData, bool>
{
public:
   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~AbstractResourceStream() = default;

   /**
    * @brief Adds a request to the stream.
    * 
    * @param in_requestId     The ID of the request.
    * @param in_requestUser   The user who made the request.
    */
   void addRequest(uint64_t in_requestId, const system::User& in_requestUser) override;

   /**
    * @brief Initializes the resource utilization stream.
    * 
    * @return Success if resource utilization streaming was started correctly; Error otherwise.
    */
   virtual Error initialize() = 0;

   /**
    * @brief Notifies that the data stream has completed.
    * 
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    */
   void setStreamComplete();

protected:
   /**
    * @brief Constructor.
    * 
    * @param in_job                    The job for which resource utilization metrics should be streamed.
    * @param in_launcherCommunicator   The communicator through which messages may be sent to the launcher.
    */
   AbstractResourceStream(
      const ConstJobPtr& in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Reports resource utilization data to the Launcher.
    * 
    * @param in_data    The new resource utilization data for this job.
    */
   void reportData(const ResourceUtilData& in_data);

   /**
    * @brief Reports an error to the Launcher.
    *
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    * 
    * @param in_error           The error which occurred.
    */
   void reportError(const Error& in_error);

   /** 
    * @brief The job for which resource utilization metrics should be streamed.
    * 
    * NOTE: To avoid potential deadlock scenarios, the lock for the mutex on the base class, m_mutex, must be held 
    *       when the job lock is acquired, and the job lock must be released before the mutex lock is released. For 
    *       consistency, it is recommended to use the following block of code to acquire both locks:
    * 
    * LOCK_MUTEX_AND_JOB(std::lock_guard, std::mutex, m_mutex, m_job)
    * {
    *    // Do tasks which require the Job Lock.
    * }
    * END_LOCK_MUTEX_AND_JOB
    * 
    */
   const ConstJobPtr m_job;

private:

   // The private implementation of AbstractResourceStream
   PRIVATE_IMPL(m_resBaseImpl);
};

// Convenience Typedef
typedef std::shared_ptr<AbstractResourceStream> AbstractResourceStreamPtr;

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
