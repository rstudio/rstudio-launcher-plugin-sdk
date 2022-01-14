/*
 * MockLogDestination.hpp
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

#ifndef LAUNCHER_PLUGINS_MOCK_LOG_DESTINATION_HPP
#define LAUNCHER_PLUGINS_MOCK_LOG_DESTINATION_HPP

#include <logging/ILogDestination.hpp>

#include <queue>

namespace rstudio {
namespace launcher_plugins {
namespace logging {

/**
 * @brief Struct which represents a message that was logged.
 */
struct LogRecord
{
   /**
    * @brief Constructor.
    *
    * @param in_level       The level of detail at which the message was logged.
    * @param in_message     The message which was logged.
    */
   LogRecord(LogLevel in_level, std::string in_message) :
      Level(in_level),
      Message(std::move(in_message))
   {
   }

   /** The level of detail at which the message was logged. **/
   const LogLevel Level;

   /** The message which was logged. **/
   const std::string Message;
};

/**
 * @brief Log destination class which can be used for capturing the messages that are logged during a test.
 */
class MockLogDestination : public ILogDestination
{
public:

   /**
    * @brief Constructor.
    * 
    * @param in_id       The level of detail at which the message was logged.
    * @param in_message     The message which was logged.
    * The most detailed log level is always used
    */
   MockLogDestination() :
      ILogDestination("", LogLevel::DEBUG, LogMessageFormatType::PRETTY, false)
   {
   }

  /**
  * @brief Gets the unique ID of the log destination.
  *
  * @return The unique ID of the log destination.
  */
  unsigned int getId() const 
  {
     // Choose a random number, but this destination should only be used alone, for testing.
     return 10;
  };

   /**
    * @brief Gets the number of log records that are currently stored.
    *
    * @return The number of log records that are currently stored.
    */
   size_t getSize() const
   {
      return m_records.size();
   }

   /**
    * @brief Peeks at the first log record.
    *
    * @return The first log record.
    */
   const LogRecord& peek() const
   {
      return m_records.front();
   }

   /**
    * @brief Pops the first log record and returns it.
    *
    * @return The first log record.
    */
   LogRecord pop()
   {
      const LogRecord record = peek();
      m_records.pop();
      return record;
   }

   /**
    * @brief Reloads the log destintation. Ensures that the log does not have any stale file handles.
    */
   void reload() 
   {
      // Nothing to do.
   }

   /**
    * @brief Writes a message to this log destination.
    *
    * @param in_logLevel    The log level of the message to write.
    * @param in_message     The message to write to the destination.
    */
   void writeLog(LogLevel in_logLevel, const std::string& in_message) override
   {
      m_records.emplace(in_logLevel, in_message);
   };

private:
   /** The log records, stored in the order they were written. **/
   std::queue<LogRecord> m_records;
};

/** Typedef for a shared_ptr to a MockLogDestination. */
typedef std::shared_ptr<MockLogDestination> MockLogPtr;

/**
 * @brief Function which either initializes or clears and then returns a MockLogDestination.
 *
 * @return The clean MockLogDestination.
 */
MockLogPtr getMockLogDest()
{
   static MockLogPtr logDest(new MockLogDestination());
   static bool added = false;

   if (!added)
   {
      logging::addLogDestination(logDest);
      added = true;
   }

   // Clear out any old logs.
   while (logDest->getSize() > 0) logDest->pop();

   return logDest;
}

} // namespace logging
} // namespace launcher_plugins
} // namespace rstudio

#endif
