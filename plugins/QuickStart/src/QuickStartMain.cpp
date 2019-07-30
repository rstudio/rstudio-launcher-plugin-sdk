/*
 * QuickStartMain.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include <AbstractMain.hpp>

#include <QuickStartPluginApi.hpp>


namespace rstudio {
namespace launcher_plugins {
namespace quickstart {
// TODO #1: Change the namespaces based on your organization and plugin name.

/**
 * @brief The Main class of the QuickStart Launcher Plugin.
 */
// TODO #2: Rename all classes from QuickStart* to MyPluginName*
class QuickStartMain : public AbstractMain
{
   /**
    * @brief Creates the QuickStart Launcher Plugin API.
    *
    * @return The QuickStart Launcher Plugin API.
    */
   std::shared_ptr<AbstractPluginApi> createLauncherPluginApi() const override
   {
      return std::shared_ptr<AbstractPluginApi>(new QuickStartPluginApi());
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
