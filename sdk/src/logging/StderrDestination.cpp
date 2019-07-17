/*
 * StderrDestination.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include <logging/StderrLogDestination.hpp>

#include <iostream>
#include <unistd.h>

namespace rstudio {
namespace launcher_plugins {
namespace logging {

bool StderrDestination::isStderrTty()
{
   return ::isatty(STDERR_FILENO) == 1;
}

unsigned int StderrDestination::getStderrId()
{
   return 0;
}

unsigned int StderrDestination::getId() const
{
   return getStderrId();
}

void StderrDestination::writeLog(LogLevel, const std::string& in_message)
{
   if (isStderrTty())
      std::cerr << in_message;
}

} // namespace logger
} // namespace launcher_plugins
} // namespace rstudio

