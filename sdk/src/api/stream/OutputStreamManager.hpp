/*
 * OutputStreamManager.hpp
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


#ifndef LAUNCHER_PLUGINS_OUTPUT_STREAM_MANAGER_HPP
#define LAUNCHER_PLUGINS_OUTPUT_STREAM_MANAGER_HPP

#include <PImpl.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

// Forward Declarations
class IJobSource;
class OutputStreamRequest;

} // namespace api

namespace comms {

class AbstractLauncherCommunicator;

} // namespace comms

namespace jobs {

class AbstractJobRepository;
class JobStatusNotifier;

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Responsible for managing output streams.
 */
class OutputStreamManager
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobSource               The job source, which creates output streams.
    * @param in_jobRepository           The job repository from which to retrieve jobs.
    * @param in_jobStatusNotifier       The job status notifier from which to receive job status update notifications.
    * @param in_launcherCommunicator    The communicator which may be used to send stream responses to the Launcher.
    */
   OutputStreamManager(
      std::shared_ptr<IJobSource> in_jobSource,
      std::shared_ptr<jobs::AbstractJobRepository> in_jobRepository,
      std::shared_ptr<jobs::JobStatusNotifier> in_jobStatusNotifier,
      std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator);

   /**
    * @brief Handles a stream request.
    *
    * @param in_outputStreamRequest    The Job Output Stream request to be handled.
    */
   void handleStreamRequest(const std::shared_ptr<OutputStreamRequest>& in_outputStreamRequest);

private:
   // The private implementation of OutputStreamManager.
   PRIVATE_IMPL_SHARED(m_impl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
