/*
 * JobTests.cpp
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

#include <Error.hpp>
#include <api/Job.hpp>
#include <json/Json.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

TEST_CASE("From JSON: Exposed port with only target port")
{
   json::Object exposedPortObj;
   exposedPortObj.insert("targetPort", 2345);

   ExposedPort exposedPort;

   REQUIRE_FALSE(ExposedPort::fromJson(exposedPortObj, exposedPort));
   CHECK(exposedPort.TargetPort == 2345);
   CHECK(exposedPort.Protocol == "TCP");
   CHECK_FALSE(exposedPort.PublishedPort);
}

TEST_CASE("From JSON: Exposed port with target port and protocol")
{
   json::Object exposedPortObj;
   exposedPortObj.insert("targetPort", 2345);
   exposedPortObj.insert("protocol", "HTTP");

   ExposedPort exposedPort;

   REQUIRE_FALSE(ExposedPort::fromJson(exposedPortObj, exposedPort));
   CHECK(exposedPort.TargetPort == 2345);
   CHECK(exposedPort.Protocol == "HTTP");
   CHECK_FALSE(exposedPort.PublishedPort);
}

TEST_CASE("From JSON: Exposed port with all fields")
{
   json::Object exposedPortObj;
   exposedPortObj.insert("targetPort", 5432);
   exposedPortObj.insert("protocol", "HTTPS");
   exposedPortObj.insert("publishedPort", 6978);

   ExposedPort exposedPort;

   REQUIRE_FALSE(ExposedPort::fromJson(exposedPortObj, exposedPort));
   CHECK(exposedPort.TargetPort == 5432);
   CHECK(exposedPort.Protocol == "HTTPS");
   REQUIRE(exposedPort.PublishedPort);
   CHECK(exposedPort.PublishedPort.getValueOr(0) == 6978);
}

TEST_CASE("From JSON: Exposed port without target port")
{
   json::Object exposedPortObj;
   exposedPortObj.insert("protocol", "HTTPS");
   exposedPortObj.insert("publishedPort", 6978);

   ExposedPort exposedPort;

   REQUIRE(ExposedPort::fromJson(exposedPortObj, exposedPort));
}


TEST_CASE("To JSON: Exposed port with only target port")
{
}

TEST_CASE("To JSON: Exposed port with target port and protocol")
{
}

TEST_CASE("To JSON: Exposed port with all fields")
{
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
