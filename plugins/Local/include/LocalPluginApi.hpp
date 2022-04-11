/*
 * LocalPluginApi.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_PLUGIN_API_HPP
#define LAUNCHER_PLUGINS_LOCAL_PLUGIN_API_HPP

#include <api/AbstractPluginApi.hpp>

#include "LocalJobSource.hpp"
#include "LocalJobRepository.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace comms {

class AbstractLauncherCommunicator;

}
}
}

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Launcher Plugin API for the Local Plugin.
 */
class LocalPluginApi : public api::AbstractPluginApi
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_hostname                The name of the host running this instance of the Local Plugin.
    * @param in_launcherCommunicator    The communicator to use for sending and receiving messages from the RStudio
    *                                   Launcher.
    */
   LocalPluginApi(
      std::string in_hostname,
      std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator);

private:
   /**
    * @brief Creates the job repository which stores any RStudio Launcher jobs currently in the job scheduling system.
    *
    * @param in_jobStatusNotifier       The job status notifier, which is used by the AbstractJobRepository to keep
    *                                   track of new jobs.
    *
    * @return The job repository.
    */
   jobs::JobRepositoryPtr createJobRepository(
      const jobs::JobStatusNotifierPtr& in_jobStatusNotifier) const override;
   /**
    * @brief Creates the job source which can communicate with the Local system.
    *
    * @param in_jobRepository           The job repository, from which to look up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post or from which to receive job status
    *                                   updates.
    *
    * @return The job source for the Local Plugin implementation.
    */
   std::shared_ptr<api::IJobSource> createJobSource(
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier) const override;

   /**
    * @brief This method is responsible for initializing all components necessary to communicate with the job launching
    *        system supported by this Plugin, such as initializing the communication method (e.g. a TCP socket).
    *
    * @return Success if all components of the Plugin API could be initialized; Error otherwise.
    */
   Error doInitialize() override;

   /** The hostname of the machine running this instance of the Local Plugin. */
   std::string m_hostname;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
