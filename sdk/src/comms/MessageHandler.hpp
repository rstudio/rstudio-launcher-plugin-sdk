/*
 * MessageHandler.hpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
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

#ifndef LAUNCHER_PLUGINS_MESSAGE_HANDLER
#define LAUNCHER_PLUGINS_MESSAGE_HANDLER

#include <cinttypes>
#include <string>
#include <vector>

#include <PImpl.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

}
}

namespace rstudio {
namespace launcher_plugins {
namespace comms {

/**
 * @brief Parses messages from the launcher and formats messages to send to the launcher.
 */
class MessageHandler
{
public:
   /**
    * @brief Default constructor. Maximum message size is 5 GB.
    */
   MessageHandler();

   /**
    * @brief Constructor.
    *
    * @param in_maxMessageSize   The maximum allowable size of a message, in bytes.
    */
   explicit MessageHandler(size_t in_maxMessageSize);

   /**
    * @brief Formats a message to be sent to the launcher.
    *
    * @param in_message     The body of the message to format.
    *
    * @return The formatted message.
    */
   std::string formatMessage(const std::string& in_message);

   /**
    * @brief Parses messages from the raw bytes received on the input stream.
    *
    * Each complete message in the input data will be added to the out_messages vector separately. Partial messages will
    * be stored for further processing on the next call to this function.
    *
    * @param in_rawData         The buffer containing the raw data.
    * @param in_dataLen         The length of in_rawData.
    * @param out_messages       The vector to which complete messages will be added.
    *
    * @return Success if the raw data is valid and no messages exceed the maximum message size (5 MB); Error otherwise.
    */
   Error parseMessages(const char* in_rawData, size_t in_dataLen, std::vector<std::string>& out_messages);

private:
   // The private implemenation of MessageHandler.
   PRIVATE_IMPL(m_impl);

   /**
    * @brief Parses the header of the next message from in_rawData.
    *
    * @param in_rawData            The raw data to process.
    * @param in_rawDataLength      The length of in_rawData.
    *
    * @return The number of bytes that were processed.
    */
   int processHeader(const char* in_rawData, size_t in_rawDataLength);

};

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio

#endif
