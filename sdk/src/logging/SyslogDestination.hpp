/*
 * SyslogDestination.hpp
 * 
 * Copyright (C) 2022 by RStudio, PBC
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

#ifndef LAUNCHER_PLUGINS_SYS_LOG_DESTINATION_HPP
#define LAUNCHER_PLUGINS_SYS_LOG_DESTINATION_HPP

#include <logging/ILogDestination.hpp>

#include <PImpl.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace logging {

/**
 * @brief A class which logs messages to syslog.
 *
 * Only one of these should be created per program.
 */
class SyslogDestination : public logging::ILogDestination
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_id              The unique ID of this log destination.
    * @param in_logLevel        The most detailed level of log to be written to syslog.
    * @param in_formatType      The format type for log messages.
    * @param in_programId       The ID of this program.
    * @param in_reloadable      Whether or not the destination is reloadable. If so, reloading of logging configuration
    *                           will cause the log destination to be removed. Set this to true only for log destinations
    *                           that are intended to be hot-reconfigurable, such as the global default logger.
    */
   SyslogDestination(const std::string& in_id,
                     logging::LogLevel in_logLevel,
                     logging::LogMessageFormatType in_formatType,
                     const std::string& in_programId,
                     bool in_reloadable = false);

   /**
    * @brief Destructor.
    */
   ~SyslogDestination() override;

   /**
    * @brief Refreshes the log destintation. Ensures that the log does not have any stale file handles.
    *
    * @param in_refreshParams   Refresh params to use when refreshing the log destinations (if applicable).
    */
   void refresh(const logging::RefreshParams& in_refreshParams = logging::RefreshParams()) override;

   /**
    * @brief Writes a message to syslog.
    *
    * @param in_logLevel    The log level of the message to write. Filtering is done prior to this call. This is for
    *                       informational purposes only.
    * @param in_message     The message to write to syslog.
    */
   void writeLog(logging::LogLevel in_logLevel, const std::string& in_message) override;

private:
   // The private implementation of SyslogDestination
   PRIVATE_IMPL(m_impl);
};

} // namespace logging
} // namespace launcher_plugins
} // namespace rstudio

#endif
