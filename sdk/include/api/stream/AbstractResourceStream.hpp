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
#include <api/stream/IDataStream.hpp>

#include <Error.hpp>
#include <PImpl.hpp>
#include <api/Response.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Streams job resource utilization data to the Launcher.
 */
class AbstractResourceStream :
   public IDataStream<ResourceUtilData>,
   public AbstractMultiStream<ResourceUtilStreamResponse, ResourceUtilData, bool>
{
public:
   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~AbstractResourceStream() = default;

   /**
    * @brief Initializes the resource utilization stream.
    * 
    * @return Success if resource utilization streaming was started correctly; Error otherwise.
    */
   virtual Error initialize() = 0;

protected:
   /**
    * @brief Constructor.
    * 
    * @param in_job                    The job for which resource utilization metrics should be streamed.
    * @param in_launcherCommunicator   The communicator through which messages may be sent to the launcher.
    */
   explicit AbstractResourceStream(
      const ConstJobPtr& in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Reports resource utilization data to the Launcher.
    * 
    * @param in_data    The new resource utilization data for this job.
    */
   void reportData(const ResourceUtilData& in_data) final;

   /**
    * @brief Reports an error to the Launcher.
    *
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    * 
    * @param in_error           The error which occurred.
    */
   void reportError(const Error& in_error) final;

   /**
    * @brief Notifies that the data stream has completed.
    * 
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    */
   void setStreamComplete() final;

   /** The job for which resource utilization metrics should be streamed. */
   const ConstJobPtr m_job;

private:
   // The private implementation of AbstractResourceStream
   PRIVATE_IMPL(m_resBaseImpl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
