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

// Resource Limit ======================================================================================================
TEST_CASE("From JSON: Resource Limit (cpu count)")
{
   json::Object limitObj;
   limitObj["type"] = "cpuCount";
   limitObj["value"] = "5";

   ResourceLimit limit;
   REQUIRE_FALSE(ResourceLimit::fromJson(limitObj, limit));
   CHECK(limit.ResourceType == ResourceLimit::Type::CPU_COUNT);
   CHECK(limit.Value == "5");
}

TEST_CASE("From JSON: Resource Limit (cpu time)")
{
   json::Object limitObj;
   limitObj["type"] = "cpuTime";
   limitObj["value"] = "6.6";

   ResourceLimit limit;
   REQUIRE_FALSE(ResourceLimit::fromJson(limitObj, limit));
   CHECK(limit.ResourceType == ResourceLimit::Type::CPU_TIME);
   CHECK(limit.Value == "6.6");
}

TEST_CASE("From JSON: Resource Limit (memory)")
{
   json::Object limitObj;
   limitObj["type"] = "memory";
   limitObj["value"] = "128";

   ResourceLimit limit;
   REQUIRE_FALSE(ResourceLimit::fromJson(limitObj, limit));
   CHECK(limit.ResourceType == ResourceLimit::Type::MEMORY);
   CHECK(limit.Value == "128");
}

TEST_CASE("From JSON: Resource Limit (swap)")
{
   json::Object limitObj;
   limitObj["type"] = "memorySwap";
   limitObj["value"] = "2048";

   ResourceLimit limit;
   REQUIRE_FALSE(ResourceLimit::fromJson(limitObj, limit));
   CHECK(limit.ResourceType == ResourceLimit::Type::MEMORY_SWAP);
   CHECK(limit.Value == "2048");
}

TEST_CASE("From JSON: Resource Limit (no value)")
{
   json::Object limitObj;
   limitObj["type"] = "memorySwap";

   ResourceLimit limit;
   REQUIRE(ResourceLimit::fromJson(limitObj, limit));
}

TEST_CASE("From JSON: Resource Limit (no type)")
{
   json::Object limitObj;
   limitObj["value"] = "63.9";

   ResourceLimit limit;
   REQUIRE(ResourceLimit::fromJson(limitObj, limit));
}

TEST_CASE("To JSON: Resource Limit (type only, cpu count)")
{
   json::Object limitObj;
   limitObj["type"] = "cpuCount";

   ResourceLimit limit;
   limit.ResourceType = ResourceLimit::Type::CPU_COUNT;
   CHECK(limit.toJson() == limitObj);
}

TEST_CASE("To JSON: Resource Limit (type and value, cpu time)")
{
   json::Object limitObj;
   limitObj["type"] = "cpuTime";
   limitObj["value"] = "33";

   ResourceLimit limit;
   limit.ResourceType = ResourceLimit::Type::CPU_TIME;
   limit.Value = "33";
   CHECK(limit.toJson() == limitObj);
}

TEST_CASE("To JSON: Resource Limit (type and default, memory)")
{
   json::Object limitObj;
   limitObj["type"] = "memory";
   limitObj["defaultValue"] = "100";

   ResourceLimit limit;
   limit.ResourceType = ResourceLimit::Type::MEMORY;
   limit.DefaultValue = "100";
   CHECK(limit.toJson() == limitObj);
}

TEST_CASE("To JSON: Resource Limit (type and max, swap)")
{
   json::Object limitObj;
   limitObj["type"] = "memorySwap";
   limitObj["maxValue"] = "250";

   ResourceLimit limit;
   limit.ResourceType = ResourceLimit::Type::MEMORY_SWAP;
   limit.MaxValue = "250";
   CHECK(limit.toJson() == limitObj);
   CHECK(limit.toJson().write() == limitObj.write());
}

TEST_CASE("To JSON: Resource Limit (type, value, and default)")
{
   json::Object limitObj;
   limitObj["type"] = "memory";
   limitObj["value"] = "55";
   limitObj["defaultValue"] = "100";

   ResourceLimit limit;
   limit.ResourceType = ResourceLimit::Type::MEMORY;
   limit.Value = "55";
   limit.DefaultValue = "100";
   CHECK(limit.toJson() == limitObj);
}

TEST_CASE("To JSON: Resource Limit (type, value, and max)")
{
   json::Object limitObj;
   limitObj["type"] = "memory";
   limitObj["value"] = "55";
   limitObj["maxValue"] = "250";

   ResourceLimit limit;
   limit.ResourceType = ResourceLimit::Type::MEMORY;
   limit.Value = "55";
   limit.MaxValue = "250";
   CHECK(limit.toJson() == limitObj);
   CHECK(limit.toJson().write() == limitObj.write());
}

TEST_CASE("To JSON: Resource Limit (type, max, and default)")
{
   json::Object limitObj;
   limitObj["type"] = "cpuTime";
   limitObj["defaultValue"] = "90";
   limitObj["maxValue"] = "180";

   ResourceLimit limit(ResourceLimit::Type::CPU_TIME, "180", "90");
   CHECK(limit.toJson() == limitObj);
   CHECK(limit.toJson().write() == limitObj.write());
}

TEST_CASE("To JSON: Resource Limit (type, value, max, and default)")
{
   json::Object limitObj;
   limitObj["type"] = "cpuTime";
   limitObj["value"] = "127";
   limitObj["defaultValue"] = "90";
   limitObj["maxValue"] = "180";

   ResourceLimit limit(ResourceLimit::Type::CPU_TIME, "180", "90");
   limit.Value = "127";
   CHECK(limit.toJson() == limitObj);
   CHECK(limit.toJson().write() == limitObj.write());
}

// Container ===========================================================================================================
TEST_CASE("From JSON: Container (image only)")
{
   json::Object containerObj;
   containerObj["image"] = "name-of-a-container-image-1234";

   Container container;
   REQUIRE_FALSE(Container::fromJson(containerObj, container));
   CHECK(container.Image == "name-of-a-container-image-1234");
   CHECK_FALSE(container.RunAsUserId);
   CHECK_FALSE(container.RunAsGroupId);
   CHECK(container.SupplementalGroupIds.empty());
}

TEST_CASE("From JSON: Container (image and run-as user)")
{
   json::Object containerObj;
   containerObj["image"] = "name-of-a-container-image-1234";
   containerObj["runAsUserId"] = 1033;

   Container container;
   REQUIRE_FALSE(Container::fromJson(containerObj, container));
   CHECK(container.Image == "name-of-a-container-image-1234");
   CHECK(container.RunAsUserId);
   CHECK(container.RunAsUserId.getValueOr(0) == 1033);
   CHECK_FALSE(container.RunAsGroupId);
   CHECK(container.SupplementalGroupIds.empty());
}

TEST_CASE("From JSON: Container (image and run-as group)")
{
   json::Object containerObj;
   containerObj["image"] = "name-of-a-container-image-1234";
   containerObj["runAsGroupId"] = 1257;

   Container container;
   REQUIRE_FALSE(Container::fromJson(containerObj, container));
   CHECK(container.Image == "name-of-a-container-image-1234");
   CHECK_FALSE(container.RunAsUserId);
   CHECK(container.RunAsGroupId);
   CHECK(container.RunAsGroupId.getValueOr(0) == 1257);
   CHECK(container.SupplementalGroupIds.empty());

}

TEST_CASE("From JSON: Container (all fields)")
{
   json::Array groupIds;
   groupIds.push_back(1000);
   groupIds.push_back(1009);

   json::Object containerObj;
   containerObj["image"] = "name-of-a-container-image-1234";
   containerObj["runAsUserId"] = 1033;
   containerObj["runAsGroupId"] = 1257;
   containerObj["supplementalGroupIds"] = groupIds;

   Container container;
   REQUIRE_FALSE(Container::fromJson(containerObj, container));
   CHECK(container.Image == "name-of-a-container-image-1234");
   CHECK(container.RunAsUserId);
   CHECK(container.RunAsUserId.getValueOr(0) == 1033);
   CHECK(container.RunAsGroupId);
   CHECK(container.RunAsGroupId.getValueOr(0) == 1257);
   REQUIRE(container.SupplementalGroupIds.size() == 2);
   CHECK(container.SupplementalGroupIds[0] == 1000);
   CHECK(container.SupplementalGroupIds[1] == 1009);
}

TEST_CASE("From JSON: Container (no image)")
{
   json::Array groupIds;
   groupIds.push_back(1000);
   groupIds.push_back(1009);

   json::Object containerObj;
   containerObj["runAsUserId"] = 1033;
   containerObj["runAsGroupId"] = 1257;
   containerObj["supplementalGroupIds"] = groupIds;

   Container container;
   REQUIRE(Container::fromJson(containerObj, container));
}

TEST_CASE("To JSON: Container (image only)")
{
   json::Object containerObj;
   containerObj["image"] = "some-image_!";

   Container container;
   container.Image = "some-image_!";
   CHECK(container.toJson() == containerObj);
}

TEST_CASE("To JSON: Container (all fields)")
{
   json::Array groupIds;
   groupIds.push_back(1048);
   groupIds.push_back(1298);
   groupIds.push_back(364);

   json::Object containerObj;
   containerObj["image"] = "some-image_!";
   containerObj["runAsUserId"] = 999;
   containerObj["runAsGroupId"] = 999;
   containerObj["supplementalGroupIds"] = groupIds;

   Container container;
   container.Image = "some-image_!";
   container.RunAsUserId = 999;
   container.RunAsGroupId = 999;
   container.SupplementalGroupIds.push_back(1048);
   container.SupplementalGroupIds.push_back(1298);
   container.SupplementalGroupIds.push_back(364);

   CHECK(container.toJson() == containerObj);
}

// Placement Constraint ================================================================================================
TEST_CASE("From JSON: Placement Constraint")
{
   json::Object constraintObj;
   constraintObj["name"] = "someName";
   constraintObj["value"] = "a-value";

   PlacementConstraint constraint;
   REQUIRE_FALSE(PlacementConstraint::fromJson(constraintObj, constraint));
   CHECK(constraint.Name == "someName");
   CHECK(constraint.Value == "a-value");
}

TEST_CASE("From JSON: Placement Constraint (no name)")
{
   json::Object constraintObj;
   constraintObj["value"] = "a-value";

   PlacementConstraint constraint;
   REQUIRE(PlacementConstraint::fromJson(constraintObj, constraint));
}

TEST_CASE("From JSON: Placement Constraint (no value)")
{
   json::Object constraintObj;
   constraintObj["name"] = "someName";

   PlacementConstraint constraint;
   REQUIRE(PlacementConstraint::fromJson(constraintObj, constraint));
}

TEST_CASE("To JSON: Placement Constraint")
{
   json::Object constraintObj;
   constraintObj["name"] = "someName";
   constraintObj["value"] = "a-value";

   PlacementConstraint constraint("someName", "a-value");
   REQUIRE(constraint.toJson() == constraintObj);
}

// Job =================================================================================================================
TEST_CASE("From JSON: Job (name and command only)")
{
   json::Object jobObj;
   jobObj["command"] = "run-tests";
   jobObj["name"] = "First Job";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command == "run-tests");
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe.empty());
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "First Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::UNKNOWN);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name, exe, and state, canceled)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "Second-Job";
   jobObj["status"] = "Canceled";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe == "/bin/my-exe");
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "Second-Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::CANCELED);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name and state, failed)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "3rd_Job";
   jobObj["status"] = "Failed";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe == "/bin/my-exe");
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "3rd_Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::FAILED);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name and state, finished)")
{
   json::Object container;
   container["image"] = "do-task-container";

   json::Object jobObj;
   jobObj["container"] = container;
   jobObj["name"] = "another!Job";
   jobObj["status"] = "Finished";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK(((job.ContainerDetails &&
          job.ContainerDetails.getValueOr(Container()).Image == "do-task-container") &&
          !job.ContainerDetails.getValueOr(Container()).RunAsUserId &&
          !job.ContainerDetails.getValueOr(Container()).RunAsGroupId &&
          job.ContainerDetails.getValueOr(Container()).SupplementalGroupIds.empty()));
   CHECK(job.Environment.empty());
   CHECK(job.Exe.empty());
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "another!Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::FINISHED);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name and state, killed)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "some&Job";
   jobObj["status"] = "Killed";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe == "/bin/my-exe");
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "some&Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::KILLED);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name and state, pending)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "A really really, really really, really really really long job name";
   jobObj["status"] = "Pending";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe == "/bin/my-exe");
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "A really really, really really, really really really long job name");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::PENDING);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name and state, running)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "First Job";
   jobObj["status"] = "Running";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe == "/bin/my-exe");
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "First Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::RUNNING);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (name and state, suspended, extra whitespace)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "First Job";
   jobObj["status"] = "  Suspended  ";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   CHECK(job.Arguments.empty());
   CHECK(job.Cluster.empty());
   CHECK(job.Command.empty());
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe == "/bin/my-exe");
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "First Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::SUSPENDED);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (invalid status)")
{
   json::Object jobObj;
   jobObj["exe"] = "/bin/my-exe";
   jobObj["name"] = "First Job";
   jobObj["status"] = "Not a job status";

   Job job;
   REQUIRE(Job::fromJson(jobObj, job));
}

TEST_CASE("From JSON: Job (all fields, exe)")
{
   json::Array argsArr, configArr, envArr, portsArr, mountsArr, placArr, queuesArr, limitsArr, tagsArr;
   argsArr.push_back("-c");
   argsArr.push_back("--arg=value");
   argsArr.push_back("some arg with spaces");

   json::Object confObj1, confObj2, confObj3;
   confObj1["name"] = "option";
   confObj1["value"] = "val";
   confObj2["name"] = "numericalOpt";
   confObj2["value"] = "4";
   confObj3["name"] = "lastOpt";
   confObj3["value"] = "val with spaces";

   configArr.push_back(confObj1);
   configArr.push_back(confObj2);
   configArr.push_back(confObj3);

   Container container;
   container.Image = "Image-Name";
   container.RunAsUserId = 22;
   container.SupplementalGroupIds.push_back(130);
   container.SupplementalGroupIds.push_back(141);

   json::Object envVal1, envVal2;
   envVal1.insert("name","PATH");
   envVal1.insert("value", ".;/some/locations;/other/locations");
   envVal2.insert("name","LD_LIBRARY_PATH");
   envVal2.insert("value", "/libs;/usr/libs;");
   envArr.push_back(envVal1);
   envArr.push_back(envVal2);

   json::Object port1, port2, port3, port4;
   port1["protocol"] = "HTTP";
   port1["targetPort"] = 5557;
   port2["publishedPort"] = 8989;
   port2["targetPort"] = 5432;
   port3["publishedPort"] = 1234;
   port3["protocol"] = "HTTPS";
   port3["targetPort"] = 4321;
   port4["targetPort"] = 6767;
   portsArr.push_back(port1);
   portsArr.push_back(port2);
   portsArr.push_back(port3);
   portsArr.push_back(port4);

   NfsMountSource nfsMountSource;
   nfsMountSource.Host = "nfsHost:72";
   nfsMountSource.Path = "/source/path";
   HostMountSource hostMountSource;
   hostMountSource.Path = "/read/only/path";
   json::Object mount1, mount2;
   mount1["mountPath"] = "/dest/path";
   mount1["nfsMount"] = nfsMountSource.toJson();
   mount2["mountPath"] = "/read/only/dest/path";
   mount2["readOnly"] = true;
   mount2["hostMount"] = hostMountSource.toJson();
   mountsArr.push_back(mount1);
   mountsArr.push_back(mount2);

   placArr.push_back(PlacementConstraint("customConstraint1", "diskType1").toJson());
   placArr.push_back(PlacementConstraint("otherConstraint", "1029").toJson());

   queuesArr.push_back("possibleQueue1");
   queuesArr.push_back("queue2");
   queuesArr.push_back("other-queue");
   queuesArr.push_back("queue with spaces  ");

   ResourceLimit limit1, limit2, limit3, limit4;
   limit1.ResourceType = ResourceLimit::Type::CPU_COUNT;
   limit1.Value = "3";
   limit2.ResourceType = ResourceLimit::Type::CPU_TIME;
   limit2.Value = "180";
   limit3.ResourceType = ResourceLimit::Type::MEMORY;
   limit3.Value = "150";
   limit4.ResourceType = ResourceLimit::Type::MEMORY_SWAP;
   limit4.Value = "2048";
   limitsArr.push_back(limit1.toJson());
   limitsArr.push_back(limit2.toJson());
   limitsArr.push_back(limit3.toJson());
   limitsArr.push_back(limit4.toJson());

   tagsArr.push_back("tag1");
   tagsArr.push_back("another tag");
   tagsArr.push_back("4th_tag");

   json::Object jobObj;
   jobObj["args"] = argsArr;
   jobObj["cluster"] = "ClusterName";
   jobObj["config"] = configArr;
   jobObj["container"] = container.toJson();
   jobObj["environment"] = envArr;
   jobObj["exe"] = "/path/to/exe";
   jobObj["exitCode"] = 0;
   jobObj["exposedPorts"] = portsArr;
   jobObj["host"] = "clusterMachine12";
   jobObj["id"] = "56";
   jobObj["lastUpdateTime"] = "2020-01-14T04:22:47.069381Z";
   jobObj["mounts"] = mountsArr;
   jobObj["name"] = "Complete_Job#";
   jobObj["pid"] = 18375;
   jobObj["placementConstraints"] = placArr;
   jobObj["queues"] = queuesArr;
   jobObj["resourceLimits"] = limitsArr;
   jobObj["stdin"] = "Pass this to the exe on standard in.";
   jobObj["stderrFile"] = "/path/to/errorFile.txt";
   jobObj["stdoutFile"] = "/path/to/outputFile.txt";
   jobObj["status"] = "Finished";
   jobObj["statusMessage"] = "Exited successfully.";
   jobObj["submissionTime"] = "2020-01-14T04:20:13Z";
   jobObj["tags"] = tagsArr;
   jobObj["user"] = "bobg";
   jobObj["workingDirectory"] = "/current/dir";

   Job job;
   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   REQUIRE(job.Arguments.size() == 3);
   CHECK(job.Arguments[0] == "-c");
   CHECK(job.Arguments[1] == "--arg=value");
   CHECK(job.Arguments[2] == "some arg with spaces");
   CHECK(job.Cluster == "ClusterName");
   CHECK(job.Command.empty());
   REQUIRE(job.Config.size() == 3);
   CHECK(((job.Config[0].Name == "option") && (job.Config[0].Value == "val") && !job.Config[0].ValueType));
   CHECK(((job.Config[1].Name == "numericalOpt") && (job.Config[1].Value == "4") && !job.Config[1].ValueType));
   CHECK(((job.Config[2].Name == "lastOpt") && (job.Config[2].Value == "val with spaces") && !job.Config[2].ValueType));
   CHECK(job.ContainerDetails);
   CHECK(job.ContainerDetails.getValueOr(Container()).Image == "Image-Name");
   CHECK(job.ContainerDetails.getValueOr(Container()).RunAsUserId.getValueOr(0) == 22);
   CHECK_FALSE(job.ContainerDetails.getValueOr(Container()).RunAsGroupId);
   REQUIRE(job.ContainerDetails.getValueOr(Container()).SupplementalGroupIds.size() == 2);
   CHECK(job.ContainerDetails.getValueOr(Container()).SupplementalGroupIds[0] == 130);
   CHECK(job.ContainerDetails.getValueOr(Container()).SupplementalGroupIds[1] == 141);
   REQUIRE(job.Environment.size() == 2);
   CHECK(((job.Environment[0].first == "PATH") && (job.Environment[0].second == ".;/some/locations;/other/locations")));
   CHECK(((job.Environment[1].first == "LD_LIBRARY_PATH") && (job.Environment[1].second == "/libs;/usr/libs;")));
   CHECK(job.Exe == "/path/to/exe");
   CHECK((job.ExitCode && (job.ExitCode.getValueOr(-1) == 0)));
   REQUIRE(job.ExposedPorts.size() == 4);
   CHECK(job.ExposedPorts[0].Protocol == "HTTP");
   CHECK(job.ExposedPorts[0].TargetPort == 5557);
   CHECK_FALSE(job.ExposedPorts[0].PublishedPort);
   CHECK(job.ExposedPorts[1].Protocol == "TCP");
   CHECK(job.ExposedPorts[1].TargetPort == 5432);
   CHECK(job.ExposedPorts[1].PublishedPort.getValueOr(0) == 8989);
   CHECK(job.ExposedPorts[2].Protocol == "HTTPS");
   CHECK(job.ExposedPorts[2].TargetPort == 4321);
   CHECK(job.ExposedPorts[2].PublishedPort.getValueOr(0) == 1234);
   CHECK(job.ExposedPorts[3].Protocol == "TCP");
   CHECK(job.ExposedPorts[3].TargetPort == 6767);
   CHECK_FALSE(job.ExposedPorts[3].PublishedPort);
   CHECK(job.Host == "clusterMachine12");
   CHECK(job.Id  == "56");
   CHECK((job.LastUpdateTime && job.LastUpdateTime.getValueOr(system::DateTime()).toString() == "2020-01-14T04:22:47.069381Z"));
   REQUIRE(job.Mounts.size() == 2);
   CHECK((job.Mounts[0].NfsSourcePath &&
         !job.Mounts[0].HostSourcePath &&
         job.Mounts[0].NfsSourcePath.getValueOr(NfsMountSource()).Host == "nfsHost:72" &&
         job.Mounts[0].NfsSourcePath.getValueOr(NfsMountSource()).Path == "/source/path" &&
         job.Mounts[0].DestinationPath == "/dest/path" &&
         !job.Mounts[0].IsReadOnly));
   CHECK((!job.Mounts[1].NfsSourcePath &&
          job.Mounts[1].HostSourcePath &&
          job.Mounts[1].HostSourcePath.getValueOr(HostMountSource()).Path == "/read/only/path" &&
          job.Mounts[1].DestinationPath == "/read/only/dest/path" &&
          job.Mounts[1].IsReadOnly));
   CHECK(job.Name  == "Complete_Job#");
   CHECK((job.Pid && job.Pid.getValueOr(0) == 18375));
   REQUIRE(job.PlacementConstraints.size() == 2);
   CHECK((job.PlacementConstraints[0].Name == "customConstraint1" && job.PlacementConstraints[0].Value == "diskType1"));
   CHECK((job.PlacementConstraints[1].Name == "otherConstraint" && job.PlacementConstraints[1].Value == "1029"));
   REQUIRE(job.Queues.size() == 4);
   CHECK((job.Queues.find("possibleQueue1") != job.Queues.end() &&
          job.Queues.find("queue2") != job.Queues.end() &&
          job.Queues.find("other-queue") != job.Queues.end() &&
          job.Queues.find("queue with spaces  ") != job.Queues.end()));
   REQUIRE(job.ResourceLimits.size() == 4);
   CHECK((job.ResourceLimits[0].ResourceType == ResourceLimit::Type::CPU_COUNT &&
          job.ResourceLimits[0].Value == "3" &&
          job.ResourceLimits[0].MaxValue.empty() &&
          job.ResourceLimits[0].DefaultValue.empty()));
   CHECK((job.ResourceLimits[1].ResourceType == ResourceLimit::Type::CPU_TIME &&
          job.ResourceLimits[1].Value == "180" &&
          job.ResourceLimits[1].MaxValue.empty() &&
          job.ResourceLimits[1].DefaultValue.empty()));
   CHECK((job.ResourceLimits[2].ResourceType == ResourceLimit::Type::MEMORY &&
          job.ResourceLimits[2].Value == "150" &&
          job.ResourceLimits[2].MaxValue.empty() &&
          job.ResourceLimits[2].DefaultValue.empty()));
   CHECK((job.ResourceLimits[3].ResourceType == ResourceLimit::Type::MEMORY_SWAP &&
          job.ResourceLimits[3].Value == "2048" &&
          job.ResourceLimits[3].MaxValue.empty() &&
          job.ResourceLimits[3].DefaultValue.empty()));
   CHECK(job.StandardIn == "Pass this to the exe on standard in.");
   CHECK(job.StandardErrFile == "/path/to/errorFile.txt");
   CHECK(job.StandardOutFile == "/path/to/outputFile.txt");
   CHECK(job.Status == Job::State::FINISHED);
   CHECK(job.StatusMessage == "Exited successfully.");
   CHECK((job.SubmissionTime && job.SubmissionTime.getValueOr(system::DateTime()).toString() == "2020-01-14T04:20:13Z"));
   REQUIRE(job.Tags.size() == 3);
   CHECK((job.Tags.find("tag1") != job.Tags.end() &&
          job.Tags.find("another tag") != job.Tags.end() &&
          job.Tags.find("4th_tag") != job.Tags.end()));
   CHECK(job.User == "bobg");
   CHECK(job.WorkingDirectory == "/current/dir");
}

TEST_CASE("From JSON: Job (some fields, command)")
{
   json::Array argsArr;
   argsArr.push_back("-n");
   argsArr.push_back("Hello!");

   json::Object jobObj;
   jobObj["name"] = "First Job";
   jobObj["status"] = "Running";
   jobObj["command"] = "echo";
   jobObj["args"] = argsArr;

   Job job;

   REQUIRE_FALSE(Job::fromJson(jobObj, job));
   REQUIRE(job.Arguments.size() == 2);
   CHECK((job.Arguments[0] == "-n" && job.Arguments[1] == "Hello!"));
   CHECK(job.Cluster.empty());
   CHECK(job.Command == "echo");
   CHECK(job.Config.empty());
   CHECK_FALSE(job.ContainerDetails);
   CHECK(job.Environment.empty());
   CHECK(job.Exe.empty());
   CHECK_FALSE(job.ExitCode);
   CHECK(job.ExposedPorts.empty());
   CHECK(job.Host.empty());
   CHECK(job.Id.empty());
   CHECK_FALSE(job.LastUpdateTime);
   CHECK(job.Mounts.empty());
   CHECK(job.Name  == "First Job");
   CHECK_FALSE(job.Pid);
   CHECK(job.PlacementConstraints.empty());
   CHECK(job.Queues.empty());
   CHECK(job.ResourceLimits.empty());
   CHECK(job.StandardIn.empty());
   CHECK(job.StandardErrFile.empty());
   CHECK(job.StandardOutFile.empty());
   CHECK(job.Status == Job::State::RUNNING);
   CHECK(job.StatusMessage.empty());
   CHECK_FALSE(job.SubmissionTime);
   CHECK(job.Tags.empty());
   CHECK(job.User.empty());
   CHECK(job.WorkingDirectory.empty());
}

TEST_CASE("From JSON: Job (exe and command)")
{
   json::Object jobObj;
   jobObj["name"] = "First Job";
   jobObj["exe"] = "/bin/some/exe";
   jobObj["command"] = "shell-command";

   Job job;
   REQUIRE(Job::fromJson(jobObj, job));
}

TEST_CASE("From JSON: Job (no name)")
{
   json::Object jobObj;
   jobObj["id"] ="job-22";
   jobObj["command"] = "echo";

   Job job;
   REQUIRE(Job::fromJson(jobObj, job));
}

TEST_CASE("From JSON: Job (no exe, command, or container)")
{
   json::Object jobObj;
   jobObj["name"] = "job-name";

   Job job;
   REQUIRE(Job::fromJson(jobObj, job));
}

TEST_CASE("From JSON: Job (invalid types)")
{
   json::Object jobObj;
   jobObj["command"] = "echo";
   jobObj["name"] = "Hello World Job";

   SECTION("String array w/ int")
   {
      json::Array args;
      args.push_back(1);
      args.push_back("2");

      jobObj["args"] = args;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }

   SECTION("String array w/ obj")
   {
      json::Object obj;
      obj["some"] = "fields";
      obj["and"] = 10;

      json::Array queues;
      queues.push_back("a queue");
      queues.push_back(obj);

      jobObj["queues"] = queues;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }

   SECTION("Non-obj on Obj array")
   {
      ResourceLimit limit(ResourceLimit::Type::MEMORY, "250", "50");
      json::Array limits;
      limits.push_back(limit.toJson());
      limits.push_back("a string");

      jobObj["resourceLimits"] = limits;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }

   SECTION("Wrong obj in array")
   {
      JobConfig conf;
      conf.Name = "confName";
      conf.Value = "32";
      ResourceLimit limit(ResourceLimit::Type::MEMORY, "250", "50");
      json::Array configs;
      configs.push_back(limit.toJson());
      configs.push_back(conf.toJson());

      jobObj["config"] = configs;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }

   SECTION("Non-obj as object")
   {
      jobObj["container"] = false;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }

   SECTION("Array as object")
   {
      json::Array arr;
      arr.push_back(false);
      arr.push_back(2);
      arr.push_back("str");
      jobObj["container"] = arr;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }

   SECTION("Array as int")
   {
      json::Array arr;
      jobObj["pid"] = arr;

      Job job;
      CHECK(Job::fromJson(jobObj, job));
   }
}


TEST_CASE("To JSON: Job (all fields)")
{
   // NOTE: In the toJson side, we don't validate the job.
   JobConfig config1, config2;
   config1.Name = "strConfVal";
   config1.Value = "someVal";
   config1.ValueType = JobConfig::Type::STRING;
   config2.Name = "intConfVal";
   config2.Value = "13";
   config2.ValueType = JobConfig::Type::INT;

   Container container;
   container.Image = "some-image-name-123";
   container.SupplementalGroupIds.push_back(123);
   container.SupplementalGroupIds.push_back(4039);
   container.RunAsGroupId = 2222;
   container.RunAsUserId = 2222;

   ExposedPort port;
   port.Protocol = "HTTPS";
   port.TargetPort = 8787;
   port.PublishedPort = 443;

   system::DateTime last, submitted;
   REQUIRE_FALSE(system::DateTime::fromString("1987-04-03T13:51:19.000381Z", last));
   REQUIRE_FALSE(system::DateTime::fromString("1987-04-03T13:21:05.412398Z", submitted));

   NfsMountSource nfsSource;
   nfsSource.Host = "some.nfs.machine:3321";
   nfsSource.Path = "/usr/home/username";
   HostMountSource hostSource1, hostSource2;
   hostSource1.Path = "/a/location";
   hostSource2.Path = "/another/loc/ation";
   Mount mount1, mount2, mount3;
   mount1.NfsSourcePath = nfsSource;
   mount1.IsReadOnly = false;
   mount1.DestinationPath = "/home";
   mount2.HostSourcePath = hostSource1;
   mount2.IsReadOnly = true;
   mount2.DestinationPath = "/a/different/loc";
   mount3.HostSourcePath = hostSource2;
   mount3.IsReadOnly = false;
   mount3.DestinationPath = "/another/diff/loc/ation";

   PlacementConstraint const1, const2;
   const1.Name = "customConstraint";
   const1.Value = "57";
   const2.Name = "otherContraint";
   const2.Value = " a value- with spaces and--__STUFF";

   ResourceLimit limit1, limit2;
   limit1.ResourceType = ResourceLimit::Type::MEMORY_SWAP;
   limit1.Value = "2048";
   limit2.ResourceType = ResourceLimit::Type::CPU_COUNT;
   limit2.Value = "6";

   Job job;
   job.Arguments.emplace_back("-n");
   job.Arguments.emplace_back("Hello\nWorld!");
   job.Cluster = "some_-cluster-";
   job.Command = "echo";
   job.Config.push_back(config1);
   job.Config.push_back(config2);
   job.ContainerDetails = container;
   job.Environment.emplace_back("PATH", "/A/path;/another/path/;;");
   job.Environment.emplace_back("SOME_VAR", "TRUE");
   job.Exe = "/conflicting/exe";
   job.ExitCode = 1;
   job.ExposedPorts.push_back(port);
   job.Host = "computer1.domain.com";
   job.Id = "cluster-job-357";
   job.LastUpdateTime = last;
   job.Mounts.push_back(mount1);
   job.Mounts.push_back(mount2);
   job.Mounts.push_back(mount3);
   job.Name = "RStudio Launcher Job (echo)";
   job.Pid = 1096;
   job.PlacementConstraints.push_back(const1);
   job.PlacementConstraints.push_back(const2);
   job.Queues.insert("hello");
   job.Queues.insert("world");
   job.Queues.insert("keep adding");
   job.Queues.insert("MORE_QUEUES");
   job.ResourceLimits.push_back(limit1);
   job.ResourceLimits.push_back(limit2);
   job.ResourceLimits.emplace_back(ResourceLimit::Type::MEMORY, "100", "20");
   job.StandardIn = "Some Standard Input String";
   job.StandardErrFile = "/home/cluster-job-357.err";
   job.StandardOutFile = "/home/cluster-job-357.out";
   job.Status = Job::State::FAILED;
   job.StatusMessage = "Unrecognized option '-n' for exe.";
   job.SubmissionTime = submitted;
   job.Tags.insert("tag 1");
   job.Tags.insert("tag");
   job.Tags.insert("1");
   job.Tags.insert("RStudio");
   job.Tags.insert("Job Launcher");
   job.User = "annem";
   job.WorkingDirectory = "/home";

   json::Object env1, env2;
   env1["name"] = "PATH";
   env1["value"] = "/A/path;/another/path/;;";
   env2["name"] = "SOME_VAR";
   env2["value"] = "TRUE";

   json::Array args, config, env, ports, mounts, constraints, queues, limits, tags;
   args.push_back("-n");
   args.push_back("Hello\nWorld!");
   config.push_back(config1.toJson());
   config.push_back(config2.toJson());
   env.push_back(env1);
   env.push_back(env2);
   ports.push_back(port.toJson());
   mounts.push_back(mount1.toJson());
   mounts.push_back(mount2.toJson());
   mounts.push_back(mount3.toJson());
   constraints.push_back(const1.toJson());
   constraints.push_back(const2.toJson());
   queues.push_back("MORE_QUEUES");
   queues.push_back("hello");
   queues.push_back("keep adding");
   queues.push_back("world");
   limits.push_back(limit1.toJson());
   limits.push_back(limit2.toJson());
   limits.push_back(ResourceLimit(ResourceLimit::Type::MEMORY, "100", "20").toJson());

   // Tags are a set and will be alphabetized
   tags.push_back("1");
   tags.push_back("Job Launcher");
   tags.push_back("RStudio");
   tags.push_back("tag");
   tags.push_back("tag 1");

   json::Object expected;
   expected["args"] = args;
   expected["cluster"] = "some_-cluster-";
   expected["command"] = "echo";
   expected["config"] = config;
   expected["container"] = container.toJson();
   expected["environment"] = env;
   expected["exe"] = "/conflicting/exe";
   expected["exitCode"] = 1;
   expected["exposedPorts"] = ports;
   expected["host"] = "computer1.domain.com";
   expected["id"] = "cluster-job-357";
   expected["lastUpdateTime"] = "1987-04-03T13:51:19.000381Z";
   expected["mounts"] = mounts;
   expected["name"] = "RStudio Launcher Job (echo)";
   expected["pid"] = 1096;
   expected["placementConstraints"] = constraints;
   expected["queues"] = queues;
   expected["resourceLimits"] = limits;
   expected["stdin"] = "Some Standard Input String";
   expected["stderrFile"] = "/home/cluster-job-357.err";
   expected["stdoutFile"] = "/home/cluster-job-357.out";
   expected["status"] = "Failed";
   expected["statusMessage"] = "Unrecognized option '-n' for exe.";
   expected["submissionTime"] = "1987-04-03T13:21:05.412398Z";
   expected["tags"] = tags;
   expected["user"] = "annem";
   expected["workingDirectory"] = "/home";

   CHECK(job.toJson() == expected);
}

TEST_CASE("To JSON: Job (some fields)")
{
   system::DateTime last, submitted;
   REQUIRE_FALSE(system::DateTime::fromString("1987-04-03T13:51:19.000381Z", last));
   REQUIRE_FALSE(system::DateTime::fromString("1987-04-03T13:21:05.412398Z", submitted));

   ResourceLimit limit1, limit2;
   limit1.ResourceType = ResourceLimit::Type::MEMORY_SWAP;
   limit1.Value = "2048";
   limit2.ResourceType = ResourceLimit::Type::CPU_COUNT;
   limit2.Value = "6";

   Job job;
   job.Cluster = "some_-cluster-";
   job.Command = "echo";
   job.ExitCode = 1;
   job.Host = "computer1.domain.com";
   job.Id = "cluster-job-357";
   job.LastUpdateTime = last;
   job.Name = "RStudio Launcher Job (echo)";
   job.Pid = 1096;
   job.ResourceLimits.push_back(limit1);
   job.ResourceLimits.push_back(limit2);
   job.ResourceLimits.emplace_back(ResourceLimit::Type::MEMORY, "100", "20");
   job.StandardIn = "Some Standard Input String";
   job.StandardErrFile = "/home/cluster-job-357.err";
   job.StandardOutFile = "/home/cluster-job-357.out";
   job.Status = Job::State::RUNNING;
   job.SubmissionTime = submitted;
   job.User = "user38";
   job.WorkingDirectory = "/home/user38";

   json::Array limits;
   limits.push_back(limit1.toJson());
   limits.push_back(limit2.toJson());
   limits.push_back(ResourceLimit(ResourceLimit::Type::MEMORY, "100", "20").toJson());

   json::Object expected;
   expected["args"] = json::Array();
   expected["cluster"] = "some_-cluster-";
   expected["command"] = "echo";
   expected["config"] = json::Array();
   expected["environment"] = json::Array();
   expected["exe"] = "";
   expected["exposedPorts"] = json::Array();
   expected["exitCode"] = 1;
   expected["host"] = "computer1.domain.com";
   expected["id"] = "cluster-job-357";
   expected["lastUpdateTime"] = "1987-04-03T13:51:19.000381Z";
   expected["mounts"] = json::Array();
   expected["name"] = "RStudio Launcher Job (echo)";
   expected["pid"] = 1096;
   expected["placementConstraints"] = json::Array();
   expected["queues"] = json::Array();
   expected["resourceLimits"] = limits;
   expected["stdin"] = "Some Standard Input String";
   expected["stderrFile"] = "/home/cluster-job-357.err";
   expected["stdoutFile"] = "/home/cluster-job-357.out";
   expected["status"] = "Running";
   expected["submissionTime"] = "1987-04-03T13:21:05.412398Z";
   expected["tags"] = json::Array();
   expected["user"] = "user38";
   expected["workingDirectory"] = "/home/user38";

   CHECK(job.toJson() == expected);
}

TEST_CASE("To JSON: Job (each state type)")
{
   Job job;

   json::Object expected;
   expected["args"] = json::Array();
   expected["command"] = "";
   expected["config"] = json::Array();
   expected["environment"] = json::Array();
   expected["exe"] = "";
   expected["exposedPorts"] = json::Array();
   expected["host"] = "";
   expected["id"] = "";
   expected["mounts"] = json::Array();
   expected["name"] = "";
   expected["placementConstraints"] = json::Array();
   expected["queues"] = json::Array();
   expected["resourceLimits"] = json::Array();
   expected["stdin"] = "";
   expected["stderrFile"] = "";
   expected["stdoutFile"] = "";
   expected["tags"] = json::Array();
   expected["user"] = "";
   expected["workingDirectory"] = "";

   SECTION("Canceled")
   {
      job.Status = Job::State::CANCELED;
      expected["status"] = "Canceled";

      CHECK(job.toJson() == expected);
   }

   SECTION("Failed")
   {
      job.Status = Job::State::FAILED;
      expected["status"] = "Failed";

      CHECK(job.toJson() == expected);
   }

   SECTION("Finished")
   {
      job.Status = Job::State::FINISHED;
      expected["status"] = "Finished";

      CHECK(job.toJson() == expected);
   }

   SECTION("Killed")
   {
      job.Status = Job::State::FINISHED;
      expected["status"] = "Finished";

      CHECK(job.toJson() == expected);
   }

   SECTION("Pending")
   {
      job.Status = Job::State::PENDING;
      expected["status"] = "Pending";

      CHECK(job.toJson() == expected);
   }

   SECTION("Running")
   {
      job.Status = Job::State::RUNNING;
      expected["status"] = "Running";

      CHECK(job.toJson() == expected);
   }

   SECTION("Suspended")
   {
      job.Status = Job::State::SUSPENDED;
      expected["status"] = "Suspended";

      CHECK(job.toJson() == expected);
   }

   SECTION("None")
   {
      job.Status = Job::State::UNKNOWN;
      expected["status"] = "";

      CHECK(job.toJson() == expected);
   }
}

TEST_CASE("Get Job Config Value")
{
   JobConfig config1, config2;
   config1.Name = "type1";
   config1.Value = "some-val";
   config2.Name = "type2";
   config2.Value = "4";

   Job job;
   job.Config.push_back(config1);
   job.Config.push_back(config2);

   auto val1 = job.getJobConfigValue("type1");
   auto val2 = job.getJobConfigValue("type2");
   auto val3 = job.getJobConfigValue("type3");
   CHECK((val1 && val1.getValueOr("") == "some-val"));
   CHECK((val2 && val2.getValueOr("") == "4"));
   CHECK_FALSE(val3);
}

TEST_CASE("Matches tags")
{
   Job job;
   job.Tags.insert("tag 1");
   job.Tags.insert("Job Launcher");
   job.Tags.insert("Session");
   job.Tags.insert("RStudio Session");

   SECTION("Exactly one match")
   {
      CHECK(job.matchesTags({ "tag 1" }));
   }

   SECTION("No match (str is a prefix)")
   {
      CHECK_FALSE(job.matchesTags({ "RStudio" }));
   }

   SECTION("Multiple matches")
   {
      CHECK(job.matchesTags({ "Session", "tag 1" }));
   }

   SECTION("All match")
   {
      CHECK(job.matchesTags({ "Job Launcher", "RStudio Session", "tag 1", "Session" }));
   }

   SECTION("Duplicate tag")
   {
      CHECK(job.matchesTags({ "tag 1", "tag 1", "Job Launcher" }));
   }

   SECTION("Duplicate tags, more tags than the job has")
   {
      CHECK(job.matchesTags({ "Job Launcher", "tag 1", "tag 1", "Session", "Job Launcher", "tag 1" }));
   }

   SECTION("Some match, some don't")
   {
      CHECK_FALSE(job.matchesTags({ "RStudio Session", "tag 1", "tag", "session" }));
   }

   SECTION("More tags than the job has, no duplicates")
   {
      CHECK_FALSE(job.matchesTags({ "Job Launcher", "tag-1", "RStudio Session", "tag 1", "Session", "not a tag" }));
   }

   SECTION("No tags")
   {
      CHECK(job.matchesTags({ }));
   }
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
