/*
 * AbstractMain.cpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
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

   addLogDestination(std::unique_ptr<ILogDestination>(new SyslogDestination(LogLevel::INFO, getProgramId())));
   addLogDestination(std::unique_ptr<ILogDestination>(new StderrLogDestination(LogLevel::INFO)));

   logInfoMessage("Starting " + getProgramId() + "...");

   // Create the LauncherPluginApi.
   std::shared_ptr<AbstractPluginApi> pluginApi = createLauncherPluginApi();
   Error error = pluginApi->initialize();
   if (error)
   {
      logError(error);
      return error.getCode();
   }

   // Read the options.
   options::Options& options = options::Options::getInstance();
   error = options.readOptions(in_argc, in_argv, getConfigFile());
   if (error)
   {
      logError(error);
      return error.getCode();
   }

   // Now that we've read the options, add the default file log destination.
   addLogDestination(
      std::unique_ptr<ILogDestination>(
         new FileLogDestination(
            3,
            LogLevel::INFO,
            getProgramId(),
            options.getScratchPath().completeChildPath(getPluginName()))));

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
