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
   std::string getProgramId() const override
   {
      // TODO #3: Change this from rstudio-quickstart-launcher to myorg-myplugin-launcher, or something similar.
      return "rstudio-quickstart-launcher";
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
