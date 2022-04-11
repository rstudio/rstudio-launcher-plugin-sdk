/*
 * ILogDestination.hpp
 * 
 * Copyright (C) 2020 by RStudio, PBC
 *
 * Unless you have received this program directly from RStudio pursuant to the terms of a commercial license agreement
 * with RStudio, then this program is licensed to you under the following terms:
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

#ifndef LAUNCHER_PLUGINS_I_LOG_DESTINATION_HPP
#define LAUNCHER_PLUGINS_I_LOG_DESTINATION_HPP

#include <Noncopyable.hpp>

#include <string>

#include <logging/Logger.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace logging {

/**
 * @brief Interface which allows a logger to write a log message to a destination.
 *
 * Log destinations IDs 0 - 100 are reserved for SDK provided log destinations.
 */
class ILogDestination : Noncopyable
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_logLevel    The most detailed level of log to be written to this log destination.
    */
   explicit ILogDestination(LogLevel in_logLevel) : m_logLevel(in_logLevel) {};

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
    * @brief Reloads the log destintation. Ensures that the log does not have any stale file handles.
    */
   virtual void reload() = 0;

   /**
    * @brief Gets the maximum level of logs that will be written to this log destination.
    *
    * @return This log destination's maximum log level.
    */
   LogLevel getLogLevel() { return m_logLevel; };

   /**
    * @brief Writes a message to this log destination.
    *
    * @param in_logLevel    The log level of the message to write.
    * @param in_message     The message to write to the destination.
    */
   virtual void writeLog(LogLevel in_logLevel, const std::string& in_message) = 0;

protected:
   /**
    * @brief The maximum level of log messages to write for this logger.
    */
    LogLevel m_logLevel;
};

} // namespace logging
} // namespace launcher_plugins
} // namespace rstudio

#endif
