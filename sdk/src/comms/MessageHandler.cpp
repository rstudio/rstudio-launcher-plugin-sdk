/*
 * MessageHandler.cpp
 *
 * Copyright (C) 2019-20 by RStudio, PBC
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

#include <boost/asio/detail/socket_ops.hpp>

#include <Error.hpp>
#include <logging/Logger.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace comms {

/**
 * @brief Struct which stores persistent data for message parsing.
 */
struct MessageHandler::Impl
{
   /**
    * @brief Constructor.
    * @param in_maxMessageSize
    */
   explicit Impl(size_t in_maxMessageSize = DEFAULT_MAX_MESSAGE_SIZE) :
      MaxMessageSize(in_maxMessageSize),
      CurrentPayloadSize(0),
      BytesRead(0)
   {
      MessageBuffer = new char[MaxMessageSize];
   }

   /**
    * @brief Destructor.
    */
   ~Impl()
   {
      delete[] MessageBuffer;
   }

   /** The size of a message header. 4 bytes. */
   static const size_t MESSAGE_HEADER_SIZE = 4;

   /** The default maximum allowable size of a message. Any greater, and the message should be considered garbage. */
   static const size_t DEFAULT_MAX_MESSAGE_SIZE = 5242880;

   /** The maximum allowable size of a message. */
   const size_t MaxMessageSize;

   /** The total size of the current message's payload */
   size_t CurrentPayloadSize;

   /** The number of bytes already processed for the current message */
   size_t BytesRead;

   /** Buffer to store the current message during processing. */
   char* MessageBuffer;
};

PRIVATE_IMPL_DELETER_IMPL(MessageHandler)

MessageHandler::MessageHandler() :
   m_impl(new Impl())
{
}

MessageHandler::MessageHandler(size_t in_maxMessageSize) :
   m_impl(new Impl(in_maxMessageSize))
{
}

std::string MessageHandler::formatMessage(const std::string& message)
{
   // Log a debug message if the given message is larger than the maximum.
   size_t messageSize = message.size();
   if (messageSize > m_impl->MaxMessageSize)
   {
      logging::logDebugMessage(
         "Plugin generated message (" +
         std::to_string(messageSize) +
         " B) is larger than the maximum message size (" +
         std::to_string(m_impl->MaxMessageSize) +
         " B).",
         ERROR_LOCATION);
   }

   // Get the size of the message in big endian, regardless of the OS endianness
   size_t payloadSize = boost::asio::detail::socket_ops::host_to_network_long(messageSize);

   // Reinterpret the message size as an array of 4 chars and put it at the front of the payload, followed by the
   // message itself.
   std::string payload(reinterpret_cast<char*>(&payloadSize), Impl::MESSAGE_HEADER_SIZE);
   return payload.append(message);
}

Error MessageHandler::processBytes(const char* in_rawData, size_t in_dataLen, std::vector<std::string>& out_messages)
{
   do
   {
      // Keeps track of the number of bytes of in_rawData that have been processed for this iteration. Before the next
      // iteration in_rawData will be advanced by bytesProcessed and in_dataLen will be reduced by the same amount.
      size_t bytesProcessed = 0;

      if (m_impl->BytesRead < Impl::MESSAGE_HEADER_SIZE)
      {
         bytesProcessed += processHeader(in_rawData, in_dataLen);

         if (m_impl->BytesRead == Impl::MESSAGE_HEADER_SIZE)
         {
            // Convert the message size back from network endianess to OS endianess.
            m_impl->CurrentPayloadSize =
               boost::asio::detail::socket_ops::network_to_host_long(m_impl->CurrentPayloadSize);

            if (m_impl->CurrentPayloadSize > m_impl->MaxMessageSize)
            {
               Error error = systemError(boost::system::errc::protocol_error,
                                         "Received message with size " +
                                         std::to_string(m_impl->CurrentPayloadSize) +
                                         " which is greater than maximum allowed message size " +
                                         std::to_string(m_impl->MaxMessageSize),
                                         ERROR_LOCATION);
               return error;
            }
         }
         else
         {
            // Part of the message header only - finished for now.
            return Success();
         }
      }

      // Calculate how much we still have to read for the message, how many bytes have already been written to the
      // buffer, and how many bytes can actually be written.
      size_t writtenMessageBytes = m_impl->BytesRead - Impl::MESSAGE_HEADER_SIZE;
      size_t remainingMessageBytes = m_impl->CurrentPayloadSize - writtenMessageBytes;
      size_t bytesToWrite = std::min(remainingMessageBytes, (in_dataLen - bytesProcessed));

      if (bytesToWrite > 0)
      {
         // Write the unwritten bytes to the buffer after the bytes that were already written there.
         std::memcpy(
            m_impl->MessageBuffer + writtenMessageBytes,
            in_rawData + bytesProcessed,
            bytesToWrite);

         // Update the number of bytes we've read.
         m_impl->BytesRead += bytesToWrite;
         bytesProcessed += bytesToWrite;
      }

      // If we've read a full message add the new message to the buffer and clean up all the tracking variables.
      if (m_impl->BytesRead == (m_impl->CurrentPayloadSize + Impl::MESSAGE_HEADER_SIZE))
      {
         out_messages.emplace_back(m_impl->MessageBuffer, m_impl->CurrentPayloadSize);
         m_impl->BytesRead = 0;
         m_impl->CurrentPayloadSize = 0;
      }

      // Advance the raw data pointer and reduce the length by the number of bytes we just processed.
      in_rawData += bytesProcessed;
      in_dataLen -= bytesProcessed;

   // Iterate if there are any more messages to read.
   } while (in_dataLen > 0);

   return Success();
}

int MessageHandler::processHeader(const char* in_rawData, size_t in_rawDataLength)
{
   // No-op if we've already processed this message's whole header
   if (m_impl->BytesRead >= m_impl->MaxMessageSize)
      return 0;

   // Figure out the number of bytes left in the current message's header and read at most that many bytes.
   size_t bytesToRead = std::min(Impl::MESSAGE_HEADER_SIZE - m_impl->BytesRead, in_rawDataLength);

   // Calculate the size of the payload from the header (or continue to).
   for (size_t i = 0; i < bytesToRead; ++i)
   {
      // The message header is a 32 bit big-endian unsigned integer, stored as 4 character bytes. To reconstruct the
      // size left-shift each byte by its position in the array * 8 (the first bit of the first byte is in position 0
      // of the integer, and the first bit of the second byte is in position 8 of the integer, and so on).
      // Example: payload size          = 41 987 332
      //          message header binary = "00000100 10101101 10000000 00000010"
      //          reconstructed size at each iteration of the loop:
      //             1. 00000100 << 0 == 4
      //             2. 00000100 |= (10000000 << 8) == 1010110100000100 = 44 292
      //             3. 1010110100000100 |= (10000000 << 16) == 100000001010110100000100 = 8 432 900
      //             4. 100000001010110100000100 |= (00000010 << 24) == 00000010100000001010110100000100 = 41 987 332
      m_impl->CurrentPayloadSize |= static_cast<unsigned char>(in_rawData[i]) << (m_impl->BytesRead * 8);
      ++m_impl->BytesRead;
   }

   return bytesToRead;
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
