/*
 * MessageHandlerTests.cpp
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

#include <TestMain.hpp>

#include <Error.hpp>
#include <MockLogDestination.hpp>

#include <boost/asio/detail/socket_ops.hpp>

#include <comms/MessageHandler.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace comms {

namespace {

const size_t MESSAGE_HEADER_SIZE = 4;

std::string convertHeader(size_t payloadSize)
{
   size_t beSize = boost::asio::detail::socket_ops::host_to_network_long(payloadSize);
   std::string header;
   header.append(
      reinterpret_cast<char*>(&beSize),
      MESSAGE_HEADER_SIZE);

   return header;
}

} // anonymous namespace

TEST_CASE("Empty message is formatted correctly")
{
   MessageHandler msgHandler;
   std::string formatted = msgHandler.formatMessage("");
   REQUIRE(formatted == convertHeader(0));
}

TEST_CASE("Message is formatted correctly")
{
   MessageHandler msgHandler;
   std::string formatted = msgHandler.formatMessage("Hello!");
   std::string expected = convertHeader(6) + "Hello!";
   REQUIRE(formatted == expected);
}

TEST_CASE("Generated message is too large")
{
   // Set up a log dest to catch the message.
   using namespace rstudio::launcher_plugins::logging;
   std::shared_ptr<MockLogDestination> logDest(new MockLogDestination());
   addLogDestination(logDest);

   // Set up the expected log message
   const std::string expectedLog = "Plugin generated message (20 B) is larger than the maximum message size (10 B).";
   const std::string expectedMessage = convertHeader(20) + "This message is 20 B";

   // Configure the limit to be 10 B so we can hit it easily.
   MessageHandler msgHandler(10);
   std::string formatted = msgHandler.formatMessage("This message is 20 B");

   REQUIRE(formatted == expectedMessage);
   REQUIRE(logDest->getSize() == 1);
   CHECK(logDest->peek().Level == LogLevel::DEBUG);
   CHECK(logDest->peek().Message.find(expectedLog) != std::string::npos);

   // Clean up the log destination.
   removeLogDestination(logDest->getId());
}

TEST_CASE("Simple message is processed")
{
   std::string message = "Hello, world!";
   std::string payload = convertHeader(message.size()) + message;

   MessageHandler msgHandler;
   std::vector<std::string> messages;
   Error error = msgHandler.parseMessages(payload.c_str(), payload.size(), messages);

   REQUIRE_FALSE(error);
   REQUIRE(messages.size() == 1);
   CHECK(messages.front() == message);
}

TEST_CASE("Header bytes are correctly processed one-by-one")
{
   std::string message = "Hello, world!";
   std::string payload = convertHeader(message.size()) + message;

   MessageHandler msgHandler;
   std::vector<std::string> messages;

   // Parse each byte of the header one at a time.
   Error error = msgHandler.parseMessages(payload.c_str(), 1, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(payload.c_str() + 1, 1, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(payload.c_str() + 2, 1, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(payload.c_str() + 3, 1, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   // Parse the rest of the message
   error = msgHandler.parseMessages(payload.c_str() + 4, payload.size() - 4, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.size() == 1);
   CHECK(messages.front() == message);
}

TEST_CASE("Complex message processed correctly piecemeal")
{
   std::string messagePart1 = "Hello, world! ";
   std::string messagePart2 = "This is a little paragraph. ";
   std::string messagePart3 = "I hope you like it. ";
   std::string messagePart4 = "Also some unicode: ταБЬℓσ";

   std::string header = convertHeader(
      messagePart1.size() +
      messagePart2.size() +
      messagePart3.size() +
      messagePart4.size());

   MessageHandler msgHandler;
   std::vector<std::string> messages;

   // Parse the complex message in pieces
   Error error = msgHandler.parseMessages(header.c_str(), 2,  messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(header.c_str() + 2, 2, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(messagePart1.c_str(), messagePart1.size(), messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(messagePart2.c_str(), messagePart2.size(), messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(messagePart3.c_str(), messagePart3.size(), messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.empty());

   error = msgHandler.parseMessages(messagePart4.c_str(), messagePart4.size(), messages);
   REQUIRE_FALSE(error);

   REQUIRE(messages.size() == 1);
   CHECK(messages.front() == (messagePart1 + messagePart2 + messagePart3 + messagePart4));
}

TEST_CASE("Multiple messages in one buffer")
{
   std::string compoundBuffer;
   for (int i = 0; i < 10; ++i)
   {
      std::string message = "This is message #" + std::to_string(i);
      compoundBuffer += (convertHeader(message.size()) + message);
   }

   MessageHandler msgHandler;
   std::vector<std::string> messages;

   Error error = msgHandler.parseMessages(compoundBuffer.c_str(), compoundBuffer.size(), messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.size() == 10);

   for (int i = 0; i < 10; ++i)
   {
      std::string message = "This is message #" + std::to_string(i);
      CHECK(messages.at(i) == message);
   }
}

TEST_CASE("Multiple complex messages in one buffer processed in two parts")
{
   std::string message1Part1 = "Hello, world! ";
   std::string message1Part2 = "This is a little paragraph. ";
   std::string message2Part1 = "I hope you like it. ";
   std::string message2Part2 = "Also some unicode: ταБЬℓσ" ;
   std::string message3Part1 = "This is the last and final message. ";
   std::string message3Part2 = "Isn't it great?";
   std::string compoundMessage1 = message1Part1 + message1Part2;
   std::string compoundMessage2 = message2Part1 + message2Part2;
   std::string compoundMessage3 = message3Part1 + message3Part2;

   std::string compoundBuffer =
      convertHeader(compoundMessage1.size()).append(compoundMessage1).append(
      convertHeader(compoundMessage2.size())).append(compoundMessage2).append(
      convertHeader(compoundMessage3.size())).append(compoundMessage3);

   // Header size is 4.
   size_t firstChunkSize = 4 + compoundMessage1.size() + 4;
   size_t secondChunkSize = compoundBuffer.size() - firstChunkSize;

   MessageHandler msgHandler;
   std::vector<std::string> messages;
   Error error = msgHandler.parseMessages(compoundBuffer.c_str(), firstChunkSize, messages);
   REQUIRE_FALSE(error);
   CHECK(messages.size() == 1);

   error = msgHandler.parseMessages(compoundBuffer.c_str() + firstChunkSize, secondChunkSize, messages);
   REQUIRE_FALSE(error);
   REQUIRE(messages.size() == 3);

   CHECK(messages.at(0) == compoundMessage1);
   CHECK(messages.at(1) == compoundMessage2);
   CHECK(messages.at(2) == compoundMessage3);
}

TEST_CASE("Received message is too large")
{
   std::string message = convertHeader(20).append("This message is 20 B");
   std::string expectedErrorDesc = "Received message with size 20 which is greater than maximum allowed message size 10";

   MessageHandler msgHandler(10);
   std::vector<std::string> messages;
   Error error = msgHandler.parseMessages(message.c_str(), message.size(), messages);

   CHECK(error);
   CHECK(error.getCode() == boost::system::errc::protocol_error);
   CHECK(error.getName() == "SystemError");
   CHECK(error.getMessage() == "Protocol error");
   CHECK(messages.empty());
   REQUIRE(error.getProperties().size() == 1);
   CHECK(error.getProperty("description") == expectedErrorDesc);
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
