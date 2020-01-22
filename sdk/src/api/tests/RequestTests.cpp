/*
 * RequestTests.cpp
 *
 * Copyright (C) 2020 by RStudio, Inc.
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
#include <api/Request.hpp>
#include <json/Json.hpp>
#include <logging/Logger.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

using namespace logging;
typedef std::shared_ptr<MockLogDestination> LogPtr;

LogPtr getLogDest()
{
   static LogPtr logDest(new MockLogDestination());
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


TEST_CASE("Parse valid bootstrap request")
{
   LogPtr logDest = getLogDest();

   json::Object versionObj;
   versionObj.insert("major", json::Value(2));
   versionObj.insert("minor", json::Value(11));
   versionObj.insert("patch", json::Value(375));

   json::Object requestObj;
   requestObj.insert("version", versionObj);
   requestObj.insert("messageType", json::Value(static_cast<int>(Request::Type::BOOTSTRAP)));
   requestObj.insert("requestId", json::Value(6));

   std::shared_ptr<Request> request;
   Error error = Request::fromJson(requestObj, request);

   REQUIRE_FALSE(error);
   REQUIRE(request != nullptr);
   CHECK(request->getType() == Request::Type::BOOTSTRAP);
   CHECK(request->getId() == 6);

   BootstrapRequest* bootstrapRequest = dynamic_cast<BootstrapRequest*>(request.get());
   REQUIRE(bootstrapRequest != nullptr);
   CHECK(bootstrapRequest->getMajorVersion() == 2);
   CHECK(bootstrapRequest->getMinorVersion() == 11);
   CHECK(bootstrapRequest->getPatchNumber() == 375);
   CHECK(logDest->getSize() == 0);
}

TEST_CASE("Parse invalid bootstrap request")
{
   LogPtr logDest = getLogDest();

   json::Object versionObj;
   versionObj.insert("major", json::Value(2));
   versionObj.insert("patch", json::Value(375));

   json::Object requestObj;
   requestObj.insert("version", versionObj);
   requestObj.insert("messageType", json::Value(static_cast<int>(Request::Type::BOOTSTRAP)));
   requestObj.insert("requestId", json::Value(6));

   std::shared_ptr<Request> request;
   Error error = Request::fromJson(requestObj, request);

   REQUIRE(error);
   CHECK(error.getMessage().find("Invalid request received from launcher") != std::string::npos);
   REQUIRE(logDest->getSize() == 1);
   CHECK(logDest->peek().Level == LogLevel::ERR);
   CHECK(logDest->pop().Message.find("minor") != std::string::npos);
}

TEST_CASE("Parse invalid request - missing message type")
{
   json::Object requestObj;
   requestObj.insert("requestId", json::Value(6));


   std::shared_ptr<Request> request;
   Error error = Request::fromJson(requestObj, request);

   REQUIRE(error);
   REQUIRE(error.getMessage().find("messageType") != std::string::npos);
}

TEST_CASE("Parse invalid request - missing request request ID")
{
   LogPtr logDest = getLogDest();
   json::Object requestObj;
   requestObj.insert("messageType", json::Value(static_cast<int>(Request::Type::BOOTSTRAP)));


   std::shared_ptr<Request> request;
   Error error = Request::fromJson(requestObj, request);

   REQUIRE(error);
   CHECK(error.getMessage().find("Invalid request received from launcher") != std::string::npos);
   REQUIRE(logDest->getSize() == 2);
   CHECK(logDest->peek().Level == LogLevel::ERR);
   CHECK(logDest->pop().Message.find("requestId") != std::string::npos); // Base constructor runs first.
   CHECK(logDest->peek().Level == LogLevel::ERR);
   CHECK(logDest->pop().Message.find("version") != std::string::npos); // Then bootstrap.
}

TEST_CASE("Parse invalid request - negative message type")
{
   LogPtr logDest = getLogDest();
   json::Object requestObj;
   requestObj.insert("messageType", json::Value(-4));
   requestObj.insert("requestId", json::Value(6));

   std::shared_ptr<Request> request;
   Error error = Request::fromJson(requestObj, request);

   REQUIRE(error);
   CHECK(error.getMessage().find("-4") != std::string::npos);
   CHECK(logDest->getSize() == 0);
}

TEST_CASE("Parse invalid request - message type too large")
{
   LogPtr logDest = getLogDest();
   json::Object requestObj;
   requestObj.insert("messageType", json::Value(568));
   requestObj.insert("requestId", json::Value(6));

   std::shared_ptr<Request> request;
   Error error = Request::fromJson(requestObj, request);

   REQUIRE(error);
   REQUIRE(error.getMessage().find("568") != std::string::npos);
   CHECK(logDest->getSize() == 0);
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
