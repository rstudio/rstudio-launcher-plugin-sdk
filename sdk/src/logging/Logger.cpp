/*
 * Logger.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "logging/Logger.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <sstream>

// We do a little special handling for syslog because it does its own formatting.
#include "SyslogDestination.hpp"
#include "logging/ILogDestination.hpp"
#include "system/DateTime.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace logging {

namespace {

std::string formatLogMessage(
   LogLevel in_logLevel,
   const std::string& in_message,
   const std::string& in_programId,
   bool in_formatForSyslog = false)
{
   std::string levelPrefix;
   switch (in_logLevel)
   {
      case LogLevel::ERROR:
      {
         levelPrefix = "ERROR: ";
         break;
      }
      case LogLevel::WARNING:
      {
         levelPrefix = "WARNING: ";
         break;
      }
      case LogLevel::DEBUG:
      {
         levelPrefix = "DEBUG: " ;
         break;
      }
      case LogLevel::INFO:
      {
         levelPrefix = "INFO: ";
         break;
      }
      case LogLevel::OFF:
      default:
      {
         // This shouldn't be possible. If it happens don't add a log level.
         assert(false);
         break;
      }
   }

   if (!in_formatForSyslog)
   {
      // Add the time.
      using namespace boost::posix_time;
      ptime time = microsec_clock::universal_time();

      std::ostringstream oss;
      oss << system::date_time::format(time, "%d %b %Y %H:%M:%S")
          << " [" << in_programId << "] "
          << levelPrefix
          << in_message << std::endl;
      return oss.str();
   }

   // Formatting for syslog.
   std::string formattedMessage = levelPrefix + in_message;
   boost::algorithm::replace_all(formattedMessage, "\n", "|||");
   return formattedMessage;
}

} // anonymous namespace

Error logLevelFromString(const std::string& in_logLevelStr, LogLevel& out_logLevel)
{
   std::string trimmedStr = boost::trim_copy(in_logLevelStr);
   if (boost::iequals(trimmedStr, "OFF") || (trimmedStr == "0"))
   {
      out_logLevel = LogLevel::OFF;
      return Success();
   }
   if (boost::iequals(trimmedStr, "ERROR") || (trimmedStr == "1"))
   {
      out_logLevel = LogLevel::ERROR;
      return Success();
   }
   if (boost::iequals(trimmedStr, "WARNING") || (trimmedStr == "2"))
   {
      out_logLevel = LogLevel::WARNING;
      return Success();
   }
   if (boost::iequals(trimmedStr, "DEBUG") || (trimmedStr == "3"))
   {
      out_logLevel = LogLevel::DEBUG;
      return Success();
   }
   if (boost::iequals(trimmedStr, "INFO") || (trimmedStr == "4"))
   {
      out_logLevel = LogLevel::INFO;
      return Success();
   }

   // TODO: we need a category of errors for this.
   return Error();
}

// Public ==============================================================================================================
Logger& Logger::getInstance(const std::string& in_programId)
{
   // This is thread-safe as of C++11. See section 6.7 paragraph 4 of the C++ standard.
   static Logger logger(in_programId);
   return logger;
}

void Logger::setLogLevel(LogLevel in_logLevel)
{
   m_logLevel = in_logLevel;
}

void Logger::addLogDestination(std::unique_ptr<ILogDestination> in_destination)
{
   if (m_logDestinations.find(in_destination->getId()) == m_logDestinations.end())
   {
      m_logDestinations.insert(std::make_pair(in_destination->getId(), std::move(in_destination)));
   }
   else
   {
      logDebugMessage(
         "Attempted to register a log destination that has already been registered with id" +
         std::to_string(in_destination->getId()));
   }
}

void Logger::removeLogDestination(unsigned int in_destinationId)
{
   auto iter = m_logDestinations.find(in_destinationId);
   if (iter != m_logDestinations.end())
   {
      m_logDestinations.erase(iter);
   }
   else
   {
      logDebugMessage(
         "Attempted to unregister a log destination that has not been registered with id" +
         std::to_string(in_destinationId));
   }
}

void Logger::logError(const Error& in_error)
{
   if (m_logLevel >= LogLevel::ERROR)
   {
      writeMessageToAllDestinations(LogLevel::ERROR, in_error.asString());
   }
}

void Logger::logErrorAsWarning(const Error& in_error)
{
   if (m_logLevel >= LogLevel::WARNING)
   {
      writeMessageToAllDestinations(LogLevel::WARNING, in_error.asString());
   }
}

void Logger::logErrorAsInfo(const Error& in_error)
{
   if (m_logLevel >= LogLevel::INFO)
   {
      writeMessageToAllDestinations(LogLevel::INFO, in_error.asString());
   }
}

void Logger::logErrorAsDebug(const Error& in_error)
{
   if (m_logLevel >= LogLevel::DEBUG)
   {
      writeMessageToAllDestinations(LogLevel::DEBUG, in_error.asString());
   }
}

void Logger::logErrorMessage(const std::string& in_message)
{
   if (m_logLevel >= LogLevel::ERROR)
      writeMessageToAllDestinations(LogLevel::ERROR, in_message);
}

void Logger::logWarningMessage(const std::string& in_message)
{
   if (m_logLevel >= LogLevel::WARNING)
      writeMessageToAllDestinations(LogLevel::WARNING, in_message);
}

void Logger::logInfoMessage(const std::string& in_message)
{
   if (m_logLevel >= LogLevel::INFO)
      writeMessageToAllDestinations(LogLevel::INFO, in_message);
}

void Logger::logDebugMessage(const std::string& in_message)
{
   if (m_logLevel >= LogLevel::DEBUG)
      writeMessageToAllDestinations(LogLevel::DEBUG, in_message);
}

// Private =============================================================================================================
Logger::Logger(std::string in_programId) :
   m_logLevel(LogLevel::INFO),
   m_programId(std::move(in_programId))
{
}

void Logger::writeMessageToAllDestinations(LogLevel in_logLevel, const std::string& in_message)
{
   // Preformat the message for non-syslog loggers.
   std::string formattedMessage = formatLogMessage(in_logLevel, in_message, m_programId);

   const auto destEnd = m_logDestinations.end();
   for (auto iter = m_logDestinations.begin(); iter != destEnd; ++iter)
   {
      if (iter->first == SyslogDestination::getSyslogId())
      {
         iter->second->writeLog(in_logLevel, formatLogMessage(in_logLevel, in_message, m_programId, true));
      }
      else
      {
         iter->second->writeLog(in_logLevel, formattedMessage);
      }
   }
}

} // namespace logging
} // namespace launcher_plugins
} // namespace rstudio

