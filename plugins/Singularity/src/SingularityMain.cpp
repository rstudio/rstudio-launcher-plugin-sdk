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
   std::shared_ptr<AbstractPluginApi> createLauncherPluginApi() const override
   {
      return std::shared_ptr<AbstractPluginApi>(new SingularityPluginApi());
   }
};

} // namespace singularity
} // namespace launcher_plugin
} // namesapce rstudio

int main(int argc, char** argv)
{
   rstudio::launcher_plugins::singularity::SingularityMain mainObject;
   return mainObject.run(argc, argv);
}

