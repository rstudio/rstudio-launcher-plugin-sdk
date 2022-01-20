/*
 * QuickStartResourceStream.hpp
 * 
 * Copyright (C) 2019-20 by RStudio, PBC
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

#ifndef LAUNCHER_PLUGINS_QUICK_START_RESOURCE_STREAM_HPP
#define LAUNCHER_PLUGINS_QUICK_START_RESOURCE_STREAM_HPP

#include <api/stream/AbstractResourceStream.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

class QuickStartResourceStream : public api::AbstractResourceStream
{
public:
   /**
    * @brief Constructor.
    * 
    * @param in_job                    The job for which resource utilization metrics should be streamed.
    * @param in_launcherCommunicator   The communicator through which messages may be sent to the launcher.
    */
   QuickStartResourceStream(
      const api::ConstJobPtr& in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Initializes the resource utilization stream.
    * 
    * @return Success if resource utilization streaming was started correctly; Error otherwise.
    */
   Error initialize() override;
};

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio

#endif
