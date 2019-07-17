/*
 * AbstractMain.cpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include <AbstractMain.hpp>

#include <iostream>

#include <logging/Logger.hpp>

#include "logging/StderrLogDestination.hpp"
#include "logging/SyslogDestination.hpp"

namespace rstudio {
namespace launcher_plugins {

int AbstractMain::run(int, char**)
{
   // Set up the logger.
   using namespace logging;
   Logger& logger = Logger::getInstance(getProgramId());
   logger.setLogLevel(LogLevel::INFO);

   logger.addLogDestination(std::move(std::unique_ptr<ILogDestination>(new SyslogDestination(getProgramId()))));
   if (StderrDestination::isStderrTty())
      logger.addLogDestination(std::move(std::unique_ptr<ILogDestination>(new StderrDestination())));

   logger.logInfoMessage("Starting " + getProgramId() + "...");
   return 0;
}

} // namespace launcher_plugins
} // namespace rstudio
