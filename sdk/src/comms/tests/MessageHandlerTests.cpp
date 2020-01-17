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

#include <MockLogDestination.hpp>

#include <boost/asio/detail/socket_ops.hpp>

#include <comms/MessageHandler.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace comms {

namespace {

static const size_t MESSAGE_HEADER_SIZE = 4;

std::string convertHeader(int payloadSize)
{
   size_t leSize = boost::asio::detail::socket_ops::host_to_network_long(payloadSize);
   std::string header;
   header.append(
      reinterpret_cast<char*>(&leSize),
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

TEST_CASE("Message is too large")
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
   REQUIRE(logDest->peek().Level == LogLevel::DEBUG);
   REQUIRE(logDest->peek().Message.find(expectedLog) != std::string::npos);

   // Clean up the log destination.
   removeLogDestination(logDest->getId());
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
