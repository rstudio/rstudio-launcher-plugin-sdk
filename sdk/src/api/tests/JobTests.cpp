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
TEST_CASE("From JSON: JobConfig missing value")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "a name";
   jobConfigObj["valueType"] = "float";

   JobConfig jobConfig;
   REQUIRE(JobConfig::fromJson(jobConfigObj, jobConfig));
}

TEST_CASE("From JSON: JobConfig name and value")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "a name";
   jobConfigObj["value"] = "a config value";

   JobConfig jobConfig;
   REQUIRE_FALSE(JobConfig::fromJson(jobConfigObj, jobConfig));
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
   CHECK(jobConfig.ValueType);
   CHECK(jobConfig.ValueType.getValueOr(JobConfig::Type::INT) == JobConfig::Type::ENUM);
   CHECK(jobConfig.Value == "ENUM_VAL");
}

TEST_CASE("From JSON: JobConfig all fields (float)")
{
   json::Object jobConfigObj;
   jobConfigObj["name"] = "some+conf+val";
   jobConfigObj["valueType"] = "float";
   jobConfigObj["value"] = "12.27";

   JobConfig jobConfig;
   REQUIRE_FALSE(JobConfig::fromJson(jobConfigObj, jobConfig));
   CHECK(jobConfig.Name == "some+conf+val");
   CHECK(jobConfig.ValueType);
   CHECK(jobConfig.ValueType.getValueOr(JobConfig::Type::INT) == JobConfig::Type::FLOAT);
   CHECK(jobConfig.Value == "12.27");
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
   CHECK(jobConfig.ValueType);
   CHECK(jobConfig.ValueType.getValueOr(JobConfig::Type::ENUM) == JobConfig::Type::INT);
   CHECK(jobConfig.Value == "13");
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
   CHECK(jobConfig.ValueType);
   CHECK(jobConfig.ValueType.getValueOr(JobConfig::Type::FLOAT) == JobConfig::Type::STRING);
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

TEST_CASE("To JSON: JobConfig name and type (float)")
{
   JobConfig config("confVal", JobConfig::Type::FLOAT);

   json::Object expected;
   expected["name"] = "confVal";
   expected["valueType"] = "float";

   CHECK(config.toJson() == expected);
}

TEST_CASE("To JSON: JobConfig all fields (int)")
{
   JobConfig config("confVal2", JobConfig::Type::INT);
   config.Value = "38";

   json::Object expected;
   expected["name"] = "confVal2";
   expected["valueType"] = "int";
   expected["value"] = "38";

   CHECK(config.toJson() == expected);
}

TEST_CASE("To JSON: JobConfig all fields (enum)")
{
   JobConfig config("some-conf-val", JobConfig::Type::ENUM);
   config.Value = "ENUM_VAL_2";

   json::Object expected;
   expected["name"] = "some-conf-val";
   expected["valueType"] = "enum";
   expected["value"] = "ENUM_VAL_2";

   CHECK(config.toJson() == expected);
}

TEST_CASE("To JSON: JobConfig all fields (string)")
{
   JobConfig config("conf3Val", JobConfig::Type::STRING);
   config.Value = "a string of words";

   json::Object expected;
   expected["name"] = "conf3Val";
   expected["valueType"] = "string";
   expected["value"] = "a string of words";

   CHECK(config.toJson() == expected);
}

// Mount ===============================================================================================================
TEST_CASE("From JSON: Host Mount Source")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";

   HostMountSource mountSource;
   REQUIRE_FALSE(HostMountSource::fromJson(mountSourceObj, mountSource));
   CHECK(mountSource.Path == "/path/to/mount/folder");
}

TEST_CASE("From JSON: Host Mount Source no path")
{
   json::Object mountSourceObj;

   HostMountSource mountSource;
   REQUIRE(HostMountSource::fromJson(mountSourceObj, mountSource));
}

TEST_CASE("To JSON: Host Mount Source")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";

   HostMountSource mountSource;
   mountSource.Path = "/path/to/mount/folder";
   CHECK(mountSource.toJson() == mountSourceObj);
}

TEST_CASE("From JSON: Nfs Mount Source")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/source/path";
   mountSourceObj["host"] = "192.168.22.1";

   NfsMountSource nfsMountSource;
   REQUIRE_FALSE(NfsMountSource::fromJson(mountSourceObj, nfsMountSource));
   CHECK(nfsMountSource.Path == "/source/path");
   CHECK(nfsMountSource.Host == "192.168.22.1");
}

TEST_CASE("From JSON: Nfs Mount Source (no host)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/source/path";

   NfsMountSource nfsMountSource;
   REQUIRE(NfsMountSource::fromJson(mountSourceObj, nfsMountSource));
}

TEST_CASE("From JSON: Nfs Mount Source (no path)")
{
   json::Object mountSourceObj;
   mountSourceObj["host"] = "192.168.22.1";

   NfsMountSource nfsMountSource;
   REQUIRE(NfsMountSource::fromJson(mountSourceObj, nfsMountSource));
}

TEST_CASE("To JSON: Nfs Mount Source")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";
   mountSourceObj["host"] = "192.168.22.1";

   NfsMountSource mountSource;
   mountSource.Path = "/path/to/mount/folder";
   mountSource.Host = "192.168.22.1";
   CHECK(mountSource.toJson() == mountSourceObj);
}

TEST_CASE("From JSON: Mount (host source)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";

   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["hostMount"] = mountSourceObj;

   Mount mount;
   REQUIRE_FALSE(Mount::fromJson(mountObj, mount));
   REQUIRE(mount.HostSourcePath);
   CHECK_FALSE(mount.NfsSourcePath);
   CHECK(mount.HostSourcePath.getValueOr(HostMountSource()).Path == "/path/to/mount/folder");
   CHECK_FALSE(mount.IsReadOnly);
}

TEST_CASE("From JSON: Mount (nfs source)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";
   mountSourceObj["host"] = "123.65.8.22";

   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["nfsMount"] = mountSourceObj;

   Mount mount;
   REQUIRE_FALSE(Mount::fromJson(mountObj, mount));
   REQUIRE(mount.NfsSourcePath);
   CHECK_FALSE(mount.HostSourcePath);
   CHECK(mount.NfsSourcePath.getValueOr(NfsMountSource()).Path == "/path/to/mount/folder");
   CHECK(mount.NfsSourcePath.getValueOr(NfsMountSource()).Host == "123.65.8.22");
   CHECK_FALSE(mount.IsReadOnly);
}

TEST_CASE("From JSON: Mount (nfs source with read-only)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";
   mountSourceObj["host"] = "123.65.8.22";

   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["nfsMount"] = mountSourceObj;
   mountObj["readOnly"] = true;

   Mount mount;
   REQUIRE_FALSE(Mount::fromJson(mountObj, mount));
   REQUIRE(mount.NfsSourcePath);
   CHECK_FALSE(mount.HostSourcePath);
   CHECK(mount.NfsSourcePath.getValueOr(NfsMountSource()).Path == "/path/to/mount/folder");
   CHECK(mount.NfsSourcePath.getValueOr(NfsMountSource()).Host == "123.65.8.22");
   CHECK(mount.IsReadOnly);
}

TEST_CASE("From JSON: Mount (no source)")
{
   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["readOnly"] = true;

   Mount mount;
   REQUIRE(Mount::fromJson(mountObj, mount));
}

TEST_CASE("From JSON: Mount (both sources)")
{
   json::Object nfsMountSourceObj;
   nfsMountSourceObj["path"] = "/path/to/mount/folder";
   nfsMountSourceObj["host"] = "123.65.8.22";

   json::Object hostMountSourceObj;
   hostMountSourceObj["path"] = "/another/path/";

   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["nfsMount"] = nfsMountSourceObj;
   mountObj["hostMount"] = hostMountSourceObj;
   mountObj["readOnly"] = false;

   Mount mount;
   REQUIRE(Mount::fromJson(mountObj, mount));
}

TEST_CASE("From JSON: Mount (no destination)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";
   mountSourceObj["host"] = "123.65.8.22";

   json::Object mountObj;
   mountObj["nfsMount"] = mountSourceObj;
   mountObj["readOnly"] = true;

   Mount mount;
   REQUIRE(Mount::fromJson(mountObj, mount));
}

TEST_CASE("To JSON: Mount (host source w/ read only)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";

   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["hostMount"] = mountSourceObj;
   mountObj["readOnly"] = true;

   HostMountSource mountSource;
   mountSource.Path = "/path/to/mount/folder";

   Mount mount;
   mount.DestinationPath = "/path/to/dest/folder";
   mount.HostSourcePath = mountSource;
   mount.IsReadOnly = true;

   CHECK(mount.toJson() == mountObj);
}

TEST_CASE("To JSON: Mount (nfs source w/ false read only)")
{
   json::Object mountSourceObj;
   mountSourceObj["path"] = "/path/to/mount/folder";
   mountSourceObj["host"] = "123.65.8.22";

   json::Object mountObj;
   mountObj["mountPath"] = "/path/to/dest/folder";
   mountObj["nfsMount"] = mountSourceObj;
   mountObj["readOnly"] = false;

   NfsMountSource mountSource;
   mountSource.Path = "/path/to/mount/folder";
   mountSource.Host = "123.65.8.22";

   Mount mount;
   mount.DestinationPath = "/path/to/dest/folder";
   mount.NfsSourcePath = mountSource;
   mount.IsReadOnly = false;

   CHECK(mount.toJson() == mountObj);
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
