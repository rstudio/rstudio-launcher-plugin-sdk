/*
 * Logger.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_LOGGER_HPP
#define LAUNCHER_PLUGINS_LOGGER_HPP

#include <boost/noncopyable.hpp>

#include <map>
#include <string>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace logging {

class ILogDestination;

/**
 * @brief Enum which represents the level of detail at which to log messages.
 */
enum class LogLevel
{
   OFF = 0,       // No messages will be logged.
   ERROR = 1,     // Error messages will be logged.
   WARNING = 2,   // Warning and error messages will be logged.
   DEBUG = 3,     // Debug, warning, and error messages will be logged.
   INFO = 4       // All messages will be logged.
};

// TODO: move this to config. That should be the only place we read this from string and will have the right errors.
Error logLevelFromString(const std::string& in_logLevelStr, LogLevel& out_logLevel);

/**
 * @brief Class which logs messages to destinations.
 *
 * Multiple destinations may be added to this logger in order to write the same message to each destination.
 * The default log level is ERROR.
 */
class Logger : boost::noncopyable
{
public:
   /**
    * @brief Gets the single instance of the logger for the whole program.
    *
    * @param in_programId   The ID of this program.
    *
    * @return The single instance of the logger for the whole program.
    */
   static Logger& getInstance(const std::string& in_programId);

   /**
    * @brief Sets the level of detail at which to log messages.
    *
    * @param in_logLevel    The level of detail at which to log messages.
    */
   void setLogLevel(LogLevel in_logLevel);

   /**
    * @brief Adds a log destination to the logger.
    *
    * If a duplicate destination is added, the duplicate will be ignored.
    *
    * @param in_destination     The destination to add.
    */
   void addLogDestination(std::unique_ptr<ILogDestination> in_destination);

   /**
    * @brief Removes a log destination from the logger.
    *
    * If a log destination does not exist with the given ID, no destination will be removed.
    *
    * @param in_destinationID   The ID of the destination to remove.
    */
    void removeLogDestination(unsigned int in_destinationId);

    // TODO: see if I can make these const.
    /**
     * @brief Logs an error to all registered destinations.
     *
     * If no destinations are registered, no log will be written.
     * If the log level is below LogLevel::ERROR, no log will be written.
     *
     * @param in_error      The error to log.
     */
    void logError(const Error& in_error);

   /**
    * @brief Logs an error as a warning to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::WARNING, no log will be written.
    *
    * @param in_error      The error to log as a warning.
    */
   void logErrorAsWarning(const Error& in_error);

   /**
    * @brief Logs an error as a debug message to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::DEBUG, no log will be written.
    *
    * @param in_error      The error to log as a debug message.
    */
   void logErrorAsDebug(const Error& in_error);

   /**
    * @brief Logs an error as an info message to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::INFO, no log will be written.
    *
    * @param in_error      The error to log as an info message.
    */
   void logErrorAsInfo(const Error& in_error);
   /**
    * @brief Logs an error message to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::ERROR, no log will be written.
    *
    * @param in_message      The message to log as an error.
    */
   void logErrorMessage(const std::string& in_message);

   /**
    * @brief Logs a warning message to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::WARNING, no log will be written.
    *
    * @param in_message      The message to log as a warning.
    */
   void logWarningMessage(const std::string& in_message);

   /**
    * @brief Logs a debug message to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::DEBUG, no log will be written.
    *
    * @param in_message      The message to log as a debug message.
    */
   void logDebugMessage(const std::string& in_message);

   /**
    * @brief Logs an info message to all registered destinations.
    *
    * If no destinations are registered, no log will be written.
    * If the log level is below LogLevel::INFO, no log will be written.
    *
    * @param in_error      The message to log as an info message.
    */
   void logInfoMessage(const std::string& in_message);

private:
   /**
    * @brief Private constructor to prevent multiple instances of Logger.
    *
    * @param in_programId   The ID of the program for which this logger is being created.
    */
   explicit Logger(std::string in_programId);

   /**
    * @brief Writes a pre-formatted message to all registered destinations.
    *
    * @param in_logLevel    The log level of the message, which is passed to the destination for informational purposes.
    * @param in_message     The pre-formatted message.
    */
   void writeMessageToAllDestinations(LogLevel in_logLevel, const std::string& in_message);

   // The maximum level of message to write.
   LogLevel m_logLevel;

   // The ID of the program fr which to write logs.
   std::string m_programId;

   // The registered log destinations.
   std::map<unsigned int, std::unique_ptr<ILogDestination> > m_logDestinations;
};

} // namespace logger
} // namespace launcher_plugins
} // namespace rstudio

#endif
