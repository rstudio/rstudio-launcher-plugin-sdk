/*
 * SyslogDestination.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_SYS_LOG_DESTINATION_HPP
#define LAUNCHER_PLUGINS_SYS_LOG_DESTINATION_HPP

#include "ILogDestination.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace logging {

/**
 * @brief A class which logs messages to syslog.
 *
 * Only one of these should be created per program.
 */
class SyslogDestination : public ILogDestination
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_programId       The ID of this program.
    */
   explicit SyslogDestination(const std::string& in_programId);

   /**
    * @brief Destructor.
    */
   ~SyslogDestination() override;

   /**
    * @brief Gets the unique ID for the syslog destination. There should only be one syslog destination for the whole
    *        program.
    *
    * @return The unique ID of the syslog destination.
    */
   static unsigned int getSyslogId();

   /**
    * @brief Gets the unique ID of the syslog destination.
    *
    * @return The unique ID of the syslog destination.
    */
   unsigned int getId() const override;

   /**
    * @brief Writes a message to syslog.
    *
    * @param in_logLevel    The log level of the message to write. Filtering is done prior to this call. This is for
    *                       informational purposes only.
    * @param in_message     The message to write to syslog.
    */
   void writeLog(LogLevel in_logLevel, const std::string& in_message) override;
};

} // namespace logger
} // namespace launcher_plugins
} // namespace rstudio

#endif
