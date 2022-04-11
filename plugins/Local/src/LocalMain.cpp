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

#include <climits>
#include <unistd.h>

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
    *
    * @return The Plugin specific Launcher Plugin API.
    */
   std::shared_ptr<api::AbstractPluginApi> createLauncherPluginApi(
      std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator) const override
   {
      return std::shared_ptr<api::AbstractPluginApi>(
         new LocalPluginApi(m_hostname, in_launcherCommunicator));
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
    * @brief Gets the unique program ID for this plugin.
    *
    * @return The unique program ID for this plugin.
    */
   std::string getProgramId() const override
   {
      // Include the hostname in the program ID for load balanced scenarios.
      return "rstudio-local-launcher-" + m_hostname;
   }


   /**
    * @brief Initializes the main process, including custom options.
    *
    * @return Success if the process could be initialized; Error otherwise.
    */
    Error initialize() override
    {
       // Get the hostname of the machine running this instance of the Local Plugin.
       Error error = getHostname(m_hostname);
       if (error)
          return error;

       // Ensure Local specific options are initialized.
       LocalOptions::getInstance().initialize();
       return Success();
    }

private:
   std::string m_hostname;
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
