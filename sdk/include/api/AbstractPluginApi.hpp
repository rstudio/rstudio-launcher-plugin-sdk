/*
 * AbstractPluginApi.hpp
 * 
 * Copyright (C) 2019 by RStudio, PBC
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_PLUGIN_API_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_PLUGIN_API_HPP

#include <Noncopyable.hpp>

#include <PImpl.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace api {

class IJobSource;

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Base class for the Launcher Plugin API.
 */
class AbstractPluginApi : public Noncopyable
{
public:
   /**
    * @brief Virtual destructor.
    */
   virtual ~AbstractPluginApi() = default;

   /**
    * @brief This method initializes the abstract plugin API.
    *
    * @return Success if the abstract plugin API was successfully initialized; Error otherwise.
    */
   Error initialize();

protected:
   /**
    * @brief Constructor.
    *
    * @param in_launcherCommunicator    The communicator to use for sending and receiving messages from the RStudio
    *                                   Launcher.
    */
   explicit AbstractPluginApi(std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator);

private:
   /**
    * @brief Creates the job source which can communicate with this Plugin's job scheduling system.
    *
    * @return The job source for this Plugin implementation.
    */
   virtual std::shared_ptr<IJobSource> createJobSource() const = 0;

   /**
    * @brief This method is responsible for initializing all components necessary to communicate with the job launching
    *        system supported by this Plugin, such as initializing the communication method (e.g. a TCP socket).
    *
    * @return Success if all components of the Plugin API could be initialized; Error otherwise.
    */
   virtual Error doInitialize() = 0;

   // The private implementation of AbstractPluginApi
   PRIVATE_IMPL(m_abstractPluginImpl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
