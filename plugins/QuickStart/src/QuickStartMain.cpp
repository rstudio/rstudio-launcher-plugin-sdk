/*
 * QuickstartMain.cpp
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

class QuickStartMain : public AbstractMain
{
   std::shared_ptr<AbstractPluginApi> createLauncherPluginApi() const override
   {
      return std::shared_ptr<AbstractPluginApi>(new QuickStartPluginApi());
   }
};

} // namespace quickstart
} // namespace launcher_plugins
} // namesapce rstudio

int main(int argc, char** argv)
{
   rstudio::launcher_plugins::quickstart::QuickStartMain mainObject;
   return mainObject.run(argc, argv);
}

