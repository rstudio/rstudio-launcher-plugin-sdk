/*
 * AbstractMain.cpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "AbstractMain.hpp"

#include <iostream>
#include <options/Options.hpp>

#include "logging/Logger.hpp"
#include "logging/FileLogDestination.hpp"
#include "logging/StderrLogDestination.hpp"
#include "logging/SyslogDestination.hpp"

namespace rstudio {
namespace launcher_plugins {

int AbstractMain::run(int in_argc, char** in_argv)
{
   // Set up the logger.
   using namespace logging;
   setProgramId(getProgramId());
   setLogLevel(LogLevel::INFO);

   addLogDestination(std::unique_ptr<ILogDestination>(new SyslogDestination(getProgramId())));
   if (StderrDestination::isStderrTty())
      addLogDestination(std::unique_ptr<ILogDestination>(new StderrDestination()));

   logInfoMessage("Starting " + getProgramId() + "...");

   // Create the LauncherPluginApi.
   std::shared_ptr<AbstractPluginApi> pluginApi = createLauncherPluginApi();
   Error error = pluginApi->initialize();
   if (error)
   {
      logError(error);
      return error.getErrorCode();
   }

   // Read the options.
   options::Options& options = options::Options::getInstance();
   error = options.readOptions(in_argc, in_argv, getConfigFile());
   if (error)
   {
      logError(error);
      return error.getErrorCode();
   }

   // Now that we've read the options, add the default file log destination.
   addLogDestination(
      std::unique_ptr<ILogDestination>(
         new FileLogDestination(3, getProgramId(), options.getScratchPath().childPath(getPluginName()))));

   logInfoMessage("Shutting down " + getProgramId() + "...");

   return 0;
}

std::string AbstractMain::getProgramId() const
{
   return "rstudio-" + getPluginName() + "-launcher";
}

system::FilePath AbstractMain::getConfigFile() const
{
   return system::FilePath("/etc/rstudio/launcher." + getPluginName() + ".conf");
}

} // namespace launcher_plugins
} // namespace rstudio
