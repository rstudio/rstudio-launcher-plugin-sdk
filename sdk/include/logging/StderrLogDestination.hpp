/*
 * StdErrLogDestination.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_STD_ERR_LOG_DESTINATION_HPP
#define LAUNCHER_PLUGINS_STD_ERR_LOG_DESTINATION_HPP

#include "ILogDestination.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace logging {

/**
 * @brief A class which logs messages to stderr.
 *
 * If stderr is not a TTY, no logs will be written. In that case, it is better not to register the destination.
 * Only one of these should be created per program.
 */
class StderrDestination : public ILogDestination
{
public:
   /**
    * @brief Checks whether stderr is a TTY
    */
   static bool isStderrTty();

   /**
    * @brief Gets the unique ID for the stderr destination. There should only be one stderr destination for the whole
    *        program.
    *
    * @return The unique ID of the stderr destination.
    */
   static unsigned int getStderrId();

   /**
    * @brief Gets the unique ID of the stderr destination.
    *
    * @return The unique ID of the stderr destination.
    */
   unsigned int getId() const override;

   /**
    * @brief Writes a message to stderr.
    *
    * @param in_logLevel    The log level of the message to write. Filtering is done prior to this call. This is for
    *                       informational purposes only.
    * @param in_message     The message to write to stderr.
    */
   void writeLog(LogLevel in_logLevel, const std::string& in_message) override;
};

} // namespace logger
} // namespace launcher_plugins
} // namespace rstudio

#endif
