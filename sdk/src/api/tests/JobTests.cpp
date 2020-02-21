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

// Exposed Port ========================================================================================================
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
   ExposedPort exposedPort;
   exposedPort.TargetPort = 56;

   json::Object expected;
   expected["targetPort"] = 56;
   expected["protocol"] = "";

   REQUIRE(exposedPort.toJson() == expected);
}

TEST_CASE("To JSON: Exposed port with target port and protocol")
{
   ExposedPort exposedPort;
   exposedPort.TargetPort = 382;
   exposedPort.Protocol = "HTTP";

   json::Object expected;
   expected["targetPort"] = 382;
   expected["protocol"] = "HTTP";

   REQUIRE(exposedPort.toJson() == expected);
}

TEST_CASE("To JSON: Exposed port with all fields")
{
   ExposedPort exposedPort;
   exposedPort.TargetPort = 9091;
   exposedPort.Protocol = "other";
   exposedPort.PublishedPort = 667;

   json::Object expected;
   expected["targetPort"] = 9091;
   expected["protocol"] = "other";
   expected["publishedPort"] = 667;

   REQUIRE(exposedPort.toJson() == expected);
}

// Job Config ==========================================================================================================
TEST_CASE("From JSON: JobConfig name and type (float)")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "a name";
   jobConfigObj["valueType"] = "float";

   JobConfig jobConfig;
   REQUIRE_FALSE(JobConfig::fromJson(jobConfigObj, jobConfig));
   CHECK(jobConfig.Name == "a name");
   CHECK(jobConfig.ValueType == JobConfig::Type::FLOAT);
}

TEST_CASE("From JSON: JobConfig all fields (int)")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "customConfigValue";
   jobConfigObj["valueType"] = "int";
   jobConfigObj["value"] = "13";

   JobConfig jobConfig;
   REQUIRE_FALSE(JobConfig::fromJson(jobConfigObj, jobConfig));
   CHECK(jobConfig.Name == "customConfigValue");
   CHECK(jobConfig.ValueType == JobConfig::Type::INT);
   CHECK(jobConfig.Value == "13");
}

TEST_CASE("From JSON: JobConfig all fields (enum)")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "anotherName";
   jobConfigObj["valueType"] = "enum";
   jobConfigObj["value"] = "ENUM_VAL";

   JobConfig jobConfig;
   REQUIRE_FALSE(JobConfig::fromJson(jobConfigObj, jobConfig));
   CHECK(jobConfig.Name == "anotherName");
   CHECK(jobConfig.ValueType == JobConfig::Type::ENUM);
   CHECK(jobConfig.Value == "ENUM_VAL");
}

TEST_CASE("From JSON: JobConfig all fields (string)")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "lastName";
   jobConfigObj["valueType"] = "string";
   jobConfigObj["value"] = "Hello, World!";

   JobConfig jobConfig;
   REQUIRE_FALSE(JobConfig::fromJson(jobConfigObj, jobConfig));
   CHECK(jobConfig.Name == "lastName");
   CHECK(jobConfig.ValueType == JobConfig::Type::STRING);
   CHECK(jobConfig.Value == "Hello, World!");
}

TEST_CASE("From JSON: JobConfig missing name")
{
   json::Object jobConfigObj;
   jobConfigObj["valueType"] = "string";
   jobConfigObj["value"] = "Hello, World!";

   JobConfig jobConfig;
   REQUIRE(JobConfig::fromJson(jobConfigObj, jobConfig));
}

TEST_CASE("From JSON: JobConfig invalid type")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "lastName";
   jobConfigObj["valueType"] = "string but not";
   jobConfigObj["value"] = "Hello, World!";

   JobConfig jobConfig;
   REQUIRE(JobConfig::fromJson(jobConfigObj, jobConfig));
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
