/*
 * MessageHandler.cpp
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

#include "MessageHandler.hpp"

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace comms {

namespace {

/**
 * @brief Struct which stores persistent data for message parsing.
 */
struct MessageHandlerImpl
{
   /** The size of a message header. 4 bytes. */
   static const size_t MESSAGE_HEADER_SIZE = 4;

   /** The maximum allowable size of a message. Any greater, and the message should be considered garbage. */
   static const size_t MAX_MESSAGE_SIZE = 5242880;

   /** The total size of the current message's payload */
   size_t CurrentPayloadSize;

   /** The number of bytes already processed for the current message */
   size_t BytesRead;

   /** Buffer to store the current message during processing. */
   char* MessageBuffer;
};

MessageHandlerImpl& messageHandler()
{
   static MessageHandlerImpl messageHandler;
   return messageHandler;
}

} // anonymous namespace

std::string MessageHandler::formatMessage(const std::string& message)
{
   std::string formatted;
   return formatted;
}

Error MessageHandler::parseMessages(const char* in_rawData, size_t in_dataLen, std::vector<std::string>& out_messages)
{
   return Success();
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
