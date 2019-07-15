/*
 * SingularityMain.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include <AbstractMain.hpp>

#include <SingularityPluginApi.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

/**
 * @brief Main class for the Singularity Plugin.
 */
class SingularityMain : public AbstractMain
{
private:
   /**
    * @brief Creates the Launcher Plugin API.
    *
    * @return The Plugin specific Launcher Plugin API.
    */
   std::shared_ptr<AbstractPluginApi> createLauncherPluginApi() const override
   {
      return std::shared_ptr<AbstractPluginApi>(new SingularityPluginApi());
   }
};

} // namespace singularity
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
   rstudio::launcher_plugins::singularity::SingularityMain mainObject;
   return mainObject.run(argc, argv);
}
