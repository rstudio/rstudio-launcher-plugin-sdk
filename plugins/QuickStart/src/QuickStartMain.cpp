/*
 * QuickStartMain.cpp
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

#include <QuickStartOptions.hpp>
#include <QuickStartPluginApi.hpp>

// TODO #1: Change the namespaces based on your organization and plugin name.
namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

/**
 * @brief The Main class of the QuickStart Launcher Plugin.
 */
// TODO #2: Rename all classes from QuickStart* to MyPluginName*
class QuickStartMain : public AbstractMain
{
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
      out_pluginApi.reset(new QuickStartPluginApi(in_launcherCommunicator));
      return Success();
   }

   /**
    * @brief Returns the unique program ID for this plugin.
    *
    * @return The unique program ID for this plugin.
    */
   std::string getPluginName() const override
   {
      // TODO #3: Change this from quickstart to myplugin.
      return "quickstart";
   }

   // TODO #4: Optionally override getProgramId() to change the programID from rstudio-<pluginName>-launcher to
   //          myorg-<pluginName>-launcher or something similar.

   /**
    * @brief Initializes the main process, including custom options.
    *
    * @return Success if the process could be initialized; Error otherwise.
    */
   Error initialize() override
   {
      QuickStartOptions::getInstance().initialize();
      return Success();
   }
};

} // namespace quickstart
} // namespace launcher_plugins
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
   rstudio::launcher_plugins::quickstart::QuickStartMain mainObject;
   return mainObject.run(argc, argv);
}
