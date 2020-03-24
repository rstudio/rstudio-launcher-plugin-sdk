/*
 * LocalMain.cpp
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

#include <AbstractMain.hpp>

#include <unistd.h>
#include <zconf.h>

#include <LocalOptions.hpp>
#include <LocalPluginApi.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

namespace {

/**
 * @brief Gets the hostname of the machine running this process.
 *
 * @param out_hostname      The hostname of this machine.
 *
 * @return Success if the hostname could be retrieved; Error otherwise.
 */
Error getHostname(std::string& out_hostname)
{
   char hostname[HOST_NAME_MAX + 1];

   int result = gethostname(hostname, HOST_NAME_MAX + 1);
   if (result != 0)
      return systemError(errno, ERROR_LOCATION);

   out_hostname = hostname;
   return Success();
}

} // anonymous namespace

/**
 * @brief Main class for the Local Plugin.
 */
class LocalMain : public AbstractMain
{
private:
   /**
    * @brief Creates the Launcher Plugin API.
    *
    * @param in_launcherCommunicator    The communicator that will be used to send and receive messages from the RStudio
    *                                   Launcher.
    * @param out_pluginApi              The Plugin specific Launcher Plugin API.
    *
    * @return Success if the plugin API could be created; Error otherwise.
    */
   Error createLauncherPluginApi(
      std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator,
      std::shared_ptr<api::AbstractPluginApi>& out_pluginApi) const override
   {
      std::string hostname;
      Error error = getHostname(hostname);
      if (error)
         return error;

      out_pluginApi.reset(new LocalPluginApi(hostname, in_launcherCommunicator));
      return Success();
   }

   /**
    * @brief Returns the name of this plugin.
    *
    * @return The name of this plugin.
    */
    std::string getPluginName() const override
    {
       return "local";
    }

   /**
    * @brief Initializes the main process, including custom options.
    *
    * @return Success if the process could be initialized; Error otherwise.
    */
    Error initialize() override
    {
       // Ensure Local specific options are initialized.
       LocalOptions::getInstance().initialize();
       return Success();
    }
};

} // namespace local
} // namespace launcher_plugin
} // namespace rstudio

/**
 * @brief The main function.
 *
 * @param argc      The number of arguments supplied to the program.
 * @param argv      The list of arguments supplied to the program.
 *
 * @return 0 on success; non-zero exit code otherwise.
 */
int main(int argc, char** argv)
{
   rstudio::launcher_plugins::local::LocalMain mainObject;
   return mainObject.run(argc, argv);
}
