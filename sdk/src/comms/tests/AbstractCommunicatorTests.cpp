/*
 * AbstractCommunicatorTests.cpp
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

#include <TestMain.hpp>

#include <queue>

#include <boost/asio/detail/socket_ops.hpp>

#include <Error.hpp>
#include <MockLogDestination.hpp>
#include <api/Constants.hpp>
#include <api/Response.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>
#include <json/Json.hpp>
#include <sstream>
#include <system/Asio.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace comms {

namespace {

void initAsioService()
{
   static bool isInit = false;
   if (!isInit)
   {
      system::AsioService::startThreads(2);
      isInit = true;
   }
}

class MockCommunicator : public AbstractLauncherCommunicator
{
public:
   explicit MockCommunicator(size_t in_maxMessageSize = 5242880) :
      AbstractLauncherCommunicator(in_maxMessageSize, [](const Error& in_error) { logging::logError(in_error); })
   {
      initAsioService();
   }

   void receiveData(const std::string& in_data)
   {
      onDataReceived(in_data.c_str(), in_data.size());
   }

   void waitForExit() override
   { };

   std::queue<std::string> SentMessages;

private:
   void writeResponse(const std::string& in_responseMessage) override
   {
      SentMessages.push(in_responseMessage);
   }
};

typedef std::shared_ptr<MockCommunicator> CommsPtr;

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

TEST_CASE("Send a simple response")
{
   logging::MockLogPtr mockLog = logging::getMockLogDest();

   api::BootstrapResponse response(4);
   std::string strResponse = response.toJson().write();
   std::string expectedResult = convertHeader(strResponse.size()).append(strResponse);

   CommsPtr comms(new MockCommunicator());
   comms->sendResponse(response);

   CHECK(mockLog->getSize() == 0);
   REQUIRE(comms->SentMessages.size() == 1);
   CHECK(comms->SentMessages.front() == expectedResult);
}

TEST_CASE("Receive a simple request")
{
   logging::MockLogPtr mockLog = logging::getMockLogDest();

   json::Object version;
   version.insert(api::FIELD_VERSION_MAJOR, json::Value(5));
   version.insert(api::FIELD_VERSION_MINOR, json::Value(99));
   version.insert(api::FIELD_VERSION_PATCH, json::Value(26));

   json::Object requestObj;
   requestObj.insert(api::FIELD_VERSION, version);
   requestObj.insert(api::FIELD_REQUEST_ID, json::Value(33));
   requestObj.insert(api::FIELD_MESSAGE_TYPE, json::Value(static_cast<int>(api::Request::Type::BOOTSTRAP)));
   std::string requestMsg = requestObj.write();

   RequestHandler handler = [](const std::shared_ptr<api::Request>& in_request)
   {
      REQUIRE(in_request != nullptr);
      CHECK(in_request->getId() == 33);
      REQUIRE(in_request->getType() == api::Request::Type::BOOTSTRAP);

      api::BootstrapRequest* bootstrapRequest = dynamic_cast<api::BootstrapRequest*>(in_request.get());
      REQUIRE(bootstrapRequest != nullptr);
      CHECK(bootstrapRequest->getMajorVersion() == 5);
      CHECK(bootstrapRequest->getMinorVersion() == 99);
      CHECK(bootstrapRequest->getPatchNumber() == 26);
   };

   CommsPtr comms(new MockCommunicator());
   comms->registerRequestHandler(api::Request::Type::BOOTSTRAP, handler);
   comms->receiveData(convertHeader(requestMsg.size()).append(requestMsg));

   CHECK(mockLog->getSize() == 0);
}

TEST_CASE("Receive a request for a type that doesn't have a handler")
{
   logging::MockLogPtr mockLog = logging::getMockLogDest();

   int bootstrapType = static_cast<int>(api::Request::Type::BOOTSTRAP);

   json::Object version;
   version.insert(api::FIELD_VERSION_MAJOR, json::Value(5));
   version.insert(api::FIELD_VERSION_MINOR, json::Value(99));
   version.insert(api::FIELD_VERSION_PATCH, json::Value(26));

   json::Object requestObj;
   requestObj.insert(api::FIELD_VERSION, version);
   requestObj.insert(api::FIELD_REQUEST_ID, json::Value(33));
   requestObj.insert(api::FIELD_MESSAGE_TYPE, json::Value(bootstrapType));
   std::string requestMsg = requestObj.write();

   CommsPtr comms(new MockCommunicator());
   comms->receiveData(convertHeader(requestMsg.size()).append(requestMsg));

   std::ostringstream expectedStr;
   expectedStr << "request type " << api::Request::Type::BOOTSTRAP;

   // Wait a couple of seconds to ensure the other threads finish.
   sleep(2);

   REQUIRE(mockLog->getSize() == 1);
   CHECK(mockLog->peek().Level == logging::LogLevel::DEBUG);
   CHECK(mockLog->pop().Message.find(expectedStr.str()) != std::string::npos);

   std::string expectedMessage = api::ErrorResponse(
      33,
      api::ErrorResponse::Type::REQUEST_NOT_SUPPORTED,
      "Request not supported").toJson().write();

   REQUIRE(comms->SentMessages.size() == 1);
   CHECK(comms->SentMessages.front() == convertHeader(expectedMessage.size()).append(expectedMessage));

   comms->SentMessages.pop();
}

TEST_CASE("Register request handler for same request type")
{
   logging::MockLogPtr mockLog = logging::getMockLogDest();

   int bootstrapType = static_cast<int>(api::Request::Type::BOOTSTRAP);

   json::Object version;
   version.insert(api::FIELD_VERSION_MAJOR, json::Value(5));
   version.insert(api::FIELD_VERSION_MINOR, json::Value(99));
   version.insert(api::FIELD_VERSION_PATCH, json::Value(26));

   json::Object requestObj;
   requestObj.insert(api::FIELD_VERSION, version);
   requestObj.insert(api::FIELD_REQUEST_ID, json::Value(33));
   requestObj.insert(api::FIELD_MESSAGE_TYPE, json::Value(bootstrapType));
   std::string requestMsg = requestObj.write();

   RequestHandler badHandler = [](const std::shared_ptr<api::Request>&)
   {
      CHECK(false);
   };

   RequestHandler handler = [](std::shared_ptr<api::Request> in_request)
   {
      REQUIRE(in_request != nullptr);
      CHECK(in_request->getId() == 33);
      REQUIRE(in_request->getType() == api::Request::Type::BOOTSTRAP);

      api::BootstrapRequest* bootstrapRequest = dynamic_cast<api::BootstrapRequest*>(in_request.get());
      REQUIRE(bootstrapRequest != nullptr);
      CHECK(bootstrapRequest->getMajorVersion() == 5);
      CHECK(bootstrapRequest->getMinorVersion() == 99);
      CHECK(bootstrapRequest->getPatchNumber() == 26);
   };

   CommsPtr comms(new MockCommunicator());
   comms->registerRequestHandler(api::Request::Type::BOOTSTRAP, badHandler);
   comms->registerRequestHandler(api::Request::Type::BOOTSTRAP, handler);
   comms->receiveData(convertHeader(requestMsg.size()).append(requestMsg));

   std::ostringstream expectedStr;
   expectedStr << "request type " << api::Request::Type::BOOTSTRAP;

   REQUIRE(mockLog->getSize() == 1);
   CHECK(mockLog->peek().Level == logging::LogLevel::DEBUG);
   CHECK(mockLog->pop().Message.find(expectedStr.str()) != std::string::npos);

}

TEST_CASE("Bad request - not JSON")
{
   logging::MockLogPtr logPtr = logging::getMockLogDest();
   std::string message = convertHeader(20).append("This message is 20 B");

   CommsPtr comms(new MockCommunicator());
   comms->receiveData(message);

   // Wait a couple of seconds to ensure the other threads finish.
   sleep(2);

   REQUIRE(logPtr->getSize() == 1);
   CHECK(logPtr->peek().Level == logging::LogLevel::ERR);
   CHECK(logPtr->pop().Message.find("JsonParseError") != std::string::npos);
}

TEST_CASE("Bad request - invalid type")
{
   logging::MockLogPtr logPtr = logging::getMockLogDest();
   json::Object request;
   request.insert(api::FIELD_MESSAGE_TYPE, json::Value(-1));
   request.insert(api::FIELD_REQUEST_ID, json::Value(2));

   std::string message = convertHeader(request.write().size()).append(request.write());

   CommsPtr comms(new MockCommunicator());
   comms->receiveData(message);

   // Wait a couple of seconds to ensure the other threads finish.
   sleep(2);

   REQUIRE(logPtr->getSize() == 1);
   CHECK(logPtr->peek().Level == logging::LogLevel::ERR);
   CHECK(logPtr->pop().Message.find("Could not deserialize launcher message: " + request.write()) != std::string::npos);
}

TEST_CASE("Bad request - invalid request")
{
   logging::MockLogPtr logPtr = logging::getMockLogDest();
   json::Object request;
   request.insert(api::FIELD_MESSAGE_TYPE, json::Value(static_cast<int>(api::Request::Type::BOOTSTRAP)));

   std::string message = convertHeader(request.write().size()).append(request.write());

   CommsPtr comms(new MockCommunicator());
   comms->receiveData(message);

   // Wait a couple of seconds to ensure the other threads finish.
   sleep(2);

   REQUIRE(logPtr->getSize() == 3);
   // The order of the error messages here should be deterministic. First base request missing value, then bootstrap
   // missing value, then comm error.
   CHECK(logPtr->peek().Level == logging::LogLevel::ERR);
   CHECK(logPtr->pop().Message.find(api::FIELD_REQUEST_ID) != std::string::npos);
   CHECK(logPtr->peek().Level == logging::LogLevel::ERR);
   CHECK(logPtr->pop().Message.find(api::FIELD_VERSION) != std::string::npos);
   CHECK(logPtr->peek().Level == logging::LogLevel::ERR);
   CHECK(logPtr->pop().Message.find("Received invalid launcher message: " + request.write()) != std::string::npos);
}

// This test case must always come last.
TEST_CASE("Clean up")
{
   system::AsioService::stop();
   system::AsioService::waitForExit();
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
