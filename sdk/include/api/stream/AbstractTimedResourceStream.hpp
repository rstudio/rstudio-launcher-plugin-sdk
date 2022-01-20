/*
 * AbstractTimedResourceStream.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_TIMED_RESOURCE_STREAM_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_TIMED_RESOURCE_STREAM_HPP

#include <api/stream/AbstractResourceStream.hpp>

#include <memory>

#include <PImpl.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

class AbstractTimedResourceStream :
   public AbstractResourceStream,
   public std::enable_shared_from_this<AbstractTimedResourceStream>
{
public:
   /**
    * @brief Virtual destructor.
    */
   virtual ~AbstractTimedResourceStream();

   /**
    * @brief Initializes the timed resource utilization stream.
    * 
    * @return Success if resource utilization streaming was started correctly; Error otherwise.
    */
   Error initialize() override;

protected:
   /**
    * @brief Constructor.
    * 
    * @param in_frequency              The frequency at which job resource utilization metrics should be polled.
    * @param in_job                    The job for which resource utilization metrics should be streamed.
    * @param in_launcherCommunicator   The communicator through which messages may be sent to the launcher.
    */
   AbstractTimedResourceStream(
      system::TimeDuration in_frequency,
      const ConstJobPtr& in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

private:
   /**
    * @brief This method will be invoked when initialized is called on the base class, allowing the inheriting class to 
    *        optionally do any initialization steps necessarfy.
    * 
    * @return Success if the inheriting class initialized correctly; the Error that occurred otherwise.
    */
   virtual Error onInitialize();

   /**
    * @brief Polls resource utilization of the job.
    * 
    * This method will be invoked once every configured interval.
    * 
    * @param out_data      The current resource utilization data of the job.
    * 
    * @return Error if it was not possible to retrieve resource utilization for any reason; Success otherwise.
    */
   virtual Error pollResourceUtilData(ResourceUtilData& out_data) = 0;

   PRIVATE_IMPL(m_timedBaseImpl);
};

} // namespace api 
} // namespace launcher_plugins 
} // namespace rstudio 

#endif
