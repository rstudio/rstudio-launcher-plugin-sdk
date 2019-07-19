/*
 * ILogDestination.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_I_LOG_DESTINATION_HPP
#define LAUNCHER_PLUGINS_I_LOG_DESTINATION_HPP

#include <boost/noncopyable.hpp>

#include <string>

#include "logging/Logger.hpp"


namespace rstudio {
namespace launcher_plugins {
namespace logging {

/**
 * @brief Interface which allows a logger to write a log message to a destination.
 *
 * Log destinations IDs 0 - 100 are reserved for SDK provided log destinations.
 */
class ILogDestination : boost::noncopyable
{
public:
   /**
    * @brief Virtual destructor to allow for inheritance.
    */
    virtual ~ILogDestination() = default;

   /**
    * @brief Gets the unique ID of the log destination.
    *
    * @return The unique ID of the log destination.
    */
   virtual unsigned int getId() const = 0;

   /**
    * @brief Writes a message to this log destination.
    *
    * @param in_logLevel    The log level of the message to write. Filtering is done prior to this call. This is for
    *                       informational purposes only.
    * @param in_message     The message to write to the destination.
    */
   virtual void writeLog(LogLevel in_level, const std::string& in_message) = 0;
};

} // namespace logging
} // namespace launcher_plugins
} // namespace rstudio

#endif
