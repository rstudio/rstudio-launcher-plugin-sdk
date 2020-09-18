/*
 * ResponseTests.cpp
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

#include <api/Constants.hpp>
#include <api/IJobSource.hpp> // JobSourceConfiguration struct
#include <api/Response.hpp>
#include <json/Json.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

TEST_CASE("Create Bootstrap Response")
{
   json::Object version;
   version[FIELD_VERSION_MAJOR] = API_VERSION_MAJOR;
   version[FIELD_VERSION_MINOR] = API_VERSION_MINOR;
   version[FIELD_VERSION_PATCH] = API_VERSION_PATCH;

   json::Object expectedResult;
   expectedResult[FIELD_MESSAGE_TYPE] = 1;
   expectedResult[FIELD_REQUEST_ID] = 10;
   expectedResult[FIELD_RESPONSE_ID] = 0;
   expectedResult[FIELD_VERSION] = version;

   BootstrapResponse bootstrapResponse(10);
   json::Object bootstrapObject = bootstrapResponse.toJson();

   CHECK(bootstrapObject == expectedResult);
}

TEST_CASE("Create ClusterInfo Response")
{
   json::Object expected;
   expected[FIELD_MESSAGE_TYPE] = 8;
   expected[FIELD_REQUEST_ID] = 26;

   SECTION("No special fields")
   {
      ClusterInfoResponse clusterInfoResponse(26, JobSourceConfiguration());

      expected[FIELD_RESPONSE_ID] = 1;
      expected[FIELD_CONTAINER_SUPPORT] = false;
      expected[FIELD_RESOURCE_LIMITS] = json::Array();
      expected[FIELD_PLACEMENT_CONSTRAINTS] = json::Array();
      expected[FIELD_CONFIG] = json::Array();

      CHECK(clusterInfoResponse.toJson() == expected);
   }

   SECTION("Resource Limits & Queues")
   {
      ResourceLimit limit1(ResourceLimit::Type::CPU_COUNT, "4", "1"),
                    limit2(ResourceLimit::Type::MEMORY, "250", "50"),
                    limit3(ResourceLimit::Type::CPU_TIME, "3600", "60");

      JobSourceConfiguration caps;
      caps.Queues = { "queue1", "QUEUE-TWO" };
      caps.ResourceLimits = { limit1, limit2, limit3 };

      ClusterInfoResponse clusterInfoResponse(26, caps);

      json::Array queues, limits;
      queues.push_back("QUEUE-TWO");
      queues.push_back("queue1");
      limits.push_back(limit1.toJson());
      limits.push_back(limit2.toJson());
      limits.push_back(limit3.toJson());

      expected[FIELD_RESPONSE_ID] = 2;
      expected[FIELD_CONTAINER_SUPPORT] = false;
      expected[FIELD_QUEUES] = queues;
      expected[FIELD_RESOURCE_LIMITS] = limits;
      expected[FIELD_PLACEMENT_CONSTRAINTS] = json::Array();
      expected[FIELD_CONFIG] = json::Array();

      CHECK(clusterInfoResponse.toJson() == expected);
   }

   SECTION("No container support")
   {
      ResourceLimit limit1(ResourceLimit::Type::CPU_COUNT, "4", "1"),
         limit2(ResourceLimit::Type::MEMORY, "250", "50"),
         limit3(ResourceLimit::Type::CPU_TIME, "3600", "60"),
         limit4(ResourceLimit::Type::MEMORY_SWAP, "2048", "512");

      PlacementConstraint constraint1("DiskType", "ssd"),
         constraint2("DiskType", "nvme"),
         constraint3("Region", "us-west"),
         constraint4("Region", "us-east"),
         constraint5("Region", "eu");

      JobConfig config1("CustomConfig1", JobConfig::Type::ENUM),
         config2("CustomConfig2", JobConfig::Type::STRING),
         config3("conf 3", JobConfig::Type::FLOAT);

      JobSourceConfiguration caps;
      caps.CustomConfig = { config1, config2, config3 };
      caps.PlacementConstraints = { constraint1, constraint2, constraint3, constraint4, constraint5 };
      caps.Queues = { "queue1", "QUEUE-TWO", "another queue" };
      caps.ResourceLimits = { limit1, limit2, limit3, limit4 };

      ClusterInfoResponse clusterInfoResponse(26, caps);

      json::Array config, constraints, queues, limits;
      config.push_back(config1.toJson());
      config.push_back(config2.toJson());
      config.push_back(config3.toJson());
      constraints.push_back(constraint1.toJson());
      constraints.push_back(constraint2.toJson());
      constraints.push_back(constraint3.toJson());
      constraints.push_back(constraint4.toJson());
      constraints.push_back(constraint5.toJson());
      queues.push_back("QUEUE-TWO");
      queues.push_back("another queue");
      queues.push_back("queue1");
      limits.push_back(limit1.toJson());
      limits.push_back(limit2.toJson());
      limits.push_back(limit3.toJson());
      limits.push_back(limit4.toJson());

      expected[FIELD_RESPONSE_ID] = 3;
      expected[FIELD_CONTAINER_SUPPORT] = false;
      expected[FIELD_QUEUES] = queues;
      expected[FIELD_CONFIG] = config;
      expected[FIELD_RESOURCE_LIMITS] = limits;
      expected[FIELD_PLACEMENT_CONSTRAINTS] = constraints;

      CHECK(clusterInfoResponse.toJson() == expected);
   }

   SECTION("Container support, no unknown, no default")
   {

      JobSourceConfiguration caps;
      caps.ContainerConfig.SupportsContainers = true;
      caps.ContainerConfig.AllowUnknownImages = false;
      caps.ContainerConfig.ContainerImages = { "image-number-1", "Image2", "  image_three_ " };

      ClusterInfoResponse clusterInfoResponse(26, caps);

      json::Array imageArr;
      imageArr.push_back("  image_three_ ");
      imageArr.push_back("Image2");
      imageArr.push_back("image-number-1");

      expected[FIELD_RESPONSE_ID] = 4;
      expected[FIELD_CONTAINER_SUPPORT] = true;
      expected[FIELD_IMAGES] = imageArr;
      expected[FIELD_ALLOW_UNKNOWN_IMAGES] = false;
      expected[FIELD_RESOURCE_LIMITS] = json::Array();
      expected[FIELD_PLACEMENT_CONSTRAINTS] = json::Array();
      expected[FIELD_CONFIG] = json::Array();

      CHECK(clusterInfoResponse.toJson() == expected);
   }

   SECTION("Container support, unknown, default")
   {
      JobSourceConfiguration caps;
      caps.ContainerConfig.SupportsContainers = true;
      caps.ContainerConfig.AllowUnknownImages = true;
      caps.ContainerConfig.ContainerImages = { "image-number-1", "Image2", "  image_three_ " };
      caps.ContainerConfig.DefaultImage = "  image_three_ ";

      ClusterInfoResponse clusterInfoResponse(26, caps);

      json::Array imageArr;
      imageArr.push_back("  image_three_ ");
      imageArr.push_back("Image2");
      imageArr.push_back("image-number-1");

      expected[FIELD_RESPONSE_ID] = 5;
      expected[FIELD_CONTAINER_SUPPORT] = true;
      expected[FIELD_IMAGES] = imageArr;
      expected[FIELD_ALLOW_UNKNOWN_IMAGES] = true;
      expected[FIELD_DEFAULT_IMAGE] = "  image_three_ ";
      expected[FIELD_RESOURCE_LIMITS] = json::Array();
      expected[FIELD_PLACEMENT_CONSTRAINTS] = json::Array();
      expected[FIELD_CONFIG] = json::Array();

      CHECK(clusterInfoResponse.toJson() == expected);
   }
}

TEST_CASE("Job State Response")
{
   // Input values.
   std::string submitted1Str = "2020-03-31T12:44:32.109485",
               submitted2Str = "2020-03-29T13:05:26.123456",
               last2Str      = "2020-03-30T15:42:20.654321",
               submitted3Str = "2020-03-30T04:27:33.000002",
               last3Str      = "2020-03-30T04:55:46.009387",
               submitted4Str = "2020-03-31T12:48:01.932001";
   system::DateTime submitted1, submitted2, last2, submitted3, last3, submitted4;
   REQUIRE_FALSE(system::DateTime::fromString(submitted1Str, submitted1));
   REQUIRE_FALSE(system::DateTime::fromString(submitted2Str, submitted2));
   REQUIRE_FALSE(system::DateTime::fromString(last2Str, last2));
   REQUIRE_FALSE(system::DateTime::fromString(submitted3Str, submitted3));
   REQUIRE_FALSE(system::DateTime::fromString(last3Str, last3));
   REQUIRE_FALSE(system::DateTime::fromString(submitted4Str, submitted4));

   system::User user2, user3, user4;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_TWO, user2));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_THREE, user3));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FOUR, user4));

   JobPtr job1(new Job()), job2(new Job()), job3(new Job()), job4(new Job());
   job1->Id = "12";
   job1->Name = "Job 1";
   job1->Cluster = "SomeCluster";
   job1->Command = "echo";
   job1->Arguments = { "-n", "Hello world!" };
   job1->User = user3;
   job1->Status = Job::State::PENDING;
   job1->SubmissionTime = submitted1;
   job1->LastUpdateTime = submitted1;
   job1->StandardOutFile = "/path/to/std-1.out";
   job1->StandardOutFile = "/path/to/std-1.err";

   job2->Id = "13";
   job2->Name = "Job 2";
   job2->Cluster = "SomeCluster";
   job2->Exe = "/bin/bash";
   job2->Arguments = { "-c", R"("echo -n Hello, World!")" };
   job2->Status = Job::State::RUNNING;
   job2->User = user2;
   job2->SubmissionTime = submitted2;
   job2->LastUpdateTime = last2;
   job2->StandardOutFile = "/path/to/std-2.out";
   job2->StandardOutFile = "/path/to/std-2.err";

   job3->Id = "14";
   job3->Name = "Job 3";
   job3->Cluster = "SomeCluster";
   job3->Exe = "/bin/myexe";
   job3->Status = Job::State::FINISHED;
   job3->User = user3;
   job3->SubmissionTime = submitted3;
   job3->LastUpdateTime = last3;
   job3->StandardOutFile = "/path/to/std-3.out";
   job3->StandardOutFile = "/path/to/std-3.err";

   job4->Id = "15";
   job4->Name = "Job 4";
   job4->Cluster = "SomeCluster";
   job4->Command = "tail";
   job4->Arguments = { "-n20", "/path/to/my/file.txt" };
   job4->Status = Job::State::FAILED;
   job4->StatusMessage = "tail: cannot open '/path/to/my/file.txt' for reading: No such file or directory";
   job4->User = user4;
   job4->SubmissionTime = submitted4;
   job4->LastUpdateTime = submitted4;
   job4->StandardOutFile = "/path/to/std-4.out";
   job4->StandardOutFile = "/path/to/std-4.err";

   // Expected values. Can rely on Job::toJson as it's tested elsewhere
   json::Array singleJob, jobList;
   singleJob.push_back(job4->toJson());
   jobList.push_back(job1->toJson());
   jobList.push_back(job2->toJson());
   jobList.push_back(job3->toJson());
   jobList.push_back(job4->toJson());

   Optional<std::set<std::string> > noFields;

   SECTION("Single job")
   {
      JobList jobs;
      jobs.push_back(job4);
      JobStateResponse jobStateResponse(54, jobs, noFields);

      json::Object expected;
      expected[FIELD_RESPONSE_ID] = 6;
      expected[FIELD_REQUEST_ID] = 54;
      expected[FIELD_MESSAGE_TYPE] = 2;
      expected[FIELD_JOBS] = singleJob;

      CHECK(jobStateResponse.toJson() == expected);
   }

   SECTION("Multiple jobs")
   {
      JobList jobs;
      jobs.push_back(job1);
      jobs.push_back(job2);
      jobs.push_back(job3);
      jobs.push_back(job4);
      JobStateResponse jobStateResponse(133, jobs, noFields);

      json::Object expected;
      expected[FIELD_RESPONSE_ID] = 7;
      expected[FIELD_REQUEST_ID] = 133;
      expected[FIELD_MESSAGE_TYPE] = 2;
      expected[FIELD_JOBS] = jobList;

      CHECK(jobStateResponse.toJson() == expected);
   }

   SECTION("Multiple jobs w/ field subset")
   {
      JobList jobs;
      jobs.push_back(job1);
      jobs.push_back(job2);
      jobs.push_back(job3);
      jobs.push_back(job4);

      std::set<std::string> fields;
      fields.insert("name");
      fields.insert("user");
      fields.insert("status");
      Optional<std::set<std::string> > fieldsOpt(fields);

      JobStateResponse jobStateResponse(133, jobs, fieldsOpt);

      // Different Expected Job List.
      json::Object job1Obj, job2Obj, job3Obj, job4Obj;
      job1Obj["id"] = "12";
      job1Obj["name"] = "Job 1";
      job1Obj["user"] = USER_THREE;
      job1Obj["status"] = "Pending";

      job2Obj["id"] = "13";
      job2Obj["name"] = "Job 2";
      job2Obj["user"] = USER_TWO;
      job2Obj["status"] = "Running";

      job3Obj["id"] = "14";
      job3Obj["name"] = "Job 3";
      job3Obj["user"] = USER_THREE;
      job3Obj["status"] = "Finished";

      job4Obj["id"] = "15";
      job4Obj["name"] = "Job 4";
      job4Obj["user"] = USER_FOUR;
      job4Obj["status"] = "Failed";
      json::Array jobsArr;
      jobsArr.push_back(job1Obj);
      jobsArr.push_back(job2Obj);
      jobsArr.push_back(job3Obj);
      jobsArr.push_back(job4Obj);

      json::Object expected;
      expected[FIELD_RESPONSE_ID] = 8;
      expected[FIELD_REQUEST_ID] = 133;
      expected[FIELD_MESSAGE_TYPE] = 2;
      expected[FIELD_JOBS] = jobsArr;

      CHECK(jobStateResponse.toJson() == expected);
   }
}

TEST_CASE("Job Status Response Tests")
{
   JobPtr job(new Job());
   job->Id = "58";
   job->Name = "Some Job";

   json::Object expected;
   expected[FIELD_REQUEST_ID] = 0;
   expected[FIELD_MESSAGE_TYPE] = 3;
   expected[FIELD_ID] = "58";
   expected[FIELD_NAME] = "Some Job";

   SECTION("With status message")
   {
      job->Status = Job::State::PENDING;
      job->StatusMessage = "Resources";

      StreamSequences sequences;
      sequences.emplace_back(12, 2);

      json::Object seqObj;
      json::Array seqArr;
      seqObj[FIELD_REQUEST_ID] = 12;
      seqObj[FIELD_SEQUENCE_ID] = 2;
      seqArr.push_back(seqObj);

      expected[FIELD_RESPONSE_ID] = 9;
      expected[FIELD_STATUS] = Job::stateToString(Job::State::PENDING);
      expected[FIELD_STATUS_MESSAGE] = "Resources";
      expected[FIELD_SEQUENCES] = seqArr;

      JobStatusResponse jobStatusResponse(sequences, job);
      CHECK(jobStatusResponse.toJson() == expected);
   }

   SECTION("With status message")
   {
      job->Status = Job::State::RUNNING;

      StreamSequences sequences;
      sequences.emplace_back(12, 2);
      sequences.emplace_back(16, 1);
      sequences.emplace_back(10, 5);
      sequences.emplace_back(11, 33);
      sequences.emplace_back(86, 102);

      json::Object seqObj1, seqObj2, seqObj3, seqObj4, seqObj5;
      json::Array seqArr;
      seqObj1[FIELD_REQUEST_ID] = 12;
      seqObj1[FIELD_SEQUENCE_ID] = 2;
      seqObj2[FIELD_REQUEST_ID] = 16;
      seqObj2[FIELD_SEQUENCE_ID] = 1;
      seqObj3[FIELD_REQUEST_ID] = 10;
      seqObj3[FIELD_SEQUENCE_ID] = 5;
      seqObj4[FIELD_REQUEST_ID] = 11;
      seqObj4[FIELD_SEQUENCE_ID] = 33;
      seqObj5[FIELD_REQUEST_ID] = 86;
      seqObj5[FIELD_SEQUENCE_ID] = 102;
      seqArr.push_back(seqObj1);
      seqArr.push_back(seqObj2);
      seqArr.push_back(seqObj3);
      seqArr.push_back(seqObj4);
      seqArr.push_back(seqObj5);

      expected[FIELD_RESPONSE_ID] = 10;
      expected[FIELD_STATUS] = Job::stateToString(Job::State::RUNNING);
      expected[FIELD_SEQUENCES] = seqArr;

      JobStatusResponse jobStatusResponse(sequences, job);
      CHECK(jobStatusResponse.toJson() == expected);
   }
}

TEST_CASE("Output Stream Response")
{
   json::Object expected;
   expected[FIELD_REQUEST_ID] = 76;
   expected[FIELD_MESSAGE_TYPE] = 5;
   expected[FIELD_COMPLETE] = false;

   SECTION("Complete")
   {
      expected[FIELD_SEQUENCE_ID] = 59;
      expected[FIELD_COMPLETE] = true;
      expected[FIELD_RESPONSE_ID] = 11;

      OutputStreamResponse response(76, 59);
      CHECK(response.toJson() == expected);
   }
   SECTION("Output Type: STDOUT, not complete")
   {
      std::string data = "Some Standard Output\nwith multiple lines\n";
      expected[FIELD_SEQUENCE_ID] = 1;
      expected[FIELD_OUTPUT] = data;
      expected[FIELD_OUTPUT_TYPE] = "0";
      expected[FIELD_RESPONSE_ID] = 12;

      OutputStreamResponse response(76, 1, data, OutputType::STDOUT);
      CHECK(response.toJson() == expected);
   }
   SECTION("Output Type: STDERR, not complete")
   {
      std::string data = "Error: Some error occurred.";
      expected[FIELD_SEQUENCE_ID] = 31;
      expected[FIELD_OUTPUT] = data;
      expected[FIELD_OUTPUT_TYPE] = "1";
      expected[FIELD_RESPONSE_ID] = 13;

      OutputStreamResponse response(76, 31, data, OutputType::STDERR);
      CHECK(response.toJson() == expected);

   }
   SECTION("Output Type: MIXED, not complete")
   {
      std::string data = "Some Standard Output\nError: Some error occurred.\nwith multiple lines\n";
      expected[FIELD_SEQUENCE_ID] = 27;
      expected[FIELD_OUTPUT] = data;
      expected[FIELD_OUTPUT_TYPE] = "2";
      expected[FIELD_RESPONSE_ID] = 14;

      OutputStreamResponse response(76, 27, data, OutputType::BOTH);
      CHECK(response.toJson() == expected);
   }
}

TEST_CASE("Network Response")
{
   json::Object expected;
   expected[FIELD_REQUEST_ID] = 36;
   expected[FIELD_MESSAGE_TYPE] = 7;


   SECTION("All fields")
   {
      NetworkInfo networkInfo;
      networkInfo.Hostname = "myHost";
      networkInfo.IpAddresses = {
         "123.0.2.1",
         "192.0.123.4",
         "10.0.44.37"
      };

      json::Array jsonIps = json::toJsonArray(networkInfo.IpAddresses);

      expected[FIELD_HOST] = "myHost";
      expected[FIELD_IPS] = jsonIps;
      expected[FIELD_RESPONSE_ID] = 15;

      NetworkResponse response(36, networkInfo);
      CHECK(response.toJson() == expected);
   }

   SECTION("Empty IPs")
   {
      NetworkInfo networkInfo;
      networkInfo.Hostname = "myHost";

      expected[FIELD_HOST] = "myHost";
      expected[FIELD_IPS] = json::Array();
      expected[FIELD_RESPONSE_ID] = 16;

      NetworkResponse response(36, networkInfo);
      CHECK(response.toJson() == expected);
   }
}

TEST_CASE("ControlJob Response")
{
   json::Object expected;
   expected[FIELD_REQUEST_ID] = 421;
   expected[FIELD_MESSAGE_TYPE] = 4;

   SECTION("Not complete, non-empty status")
   {
      expected[FIELD_STATUS_MESSAGE] = "Job is not running.";
      expected[FIELD_OPERATION_COMPLETE] = false;
      expected[FIELD_RESPONSE_ID] = 17;

      ControlJobResponse response(421, "Job is not running.", false);
      CHECK(response.toJson() == expected);
   }

   SECTION("Complete, empty status")
   {
      expected[FIELD_STATUS_MESSAGE] = "";
      expected[FIELD_OPERATION_COMPLETE] = true;
      expected[FIELD_RESPONSE_ID] = 18;

      ControlJobResponse response(421, "", true);
      CHECK(response.toJson() == expected);
   }
}

TEST_CASE("ResourceUtilStream Response")
{
   json::Object expected;
   expected[FIELD_MESSAGE_TYPE] = 6;
   expected[FIELD_REQUEST_ID] = 0;

   SECTION("All data empty, complete, one sequence")
   {
      StreamSequences sequences;
      sequences.emplace_back(3, 0);

      json::Object seq;
      seq[FIELD_REQUEST_ID] = 3;
      seq[FIELD_SEQUENCE_ID] = 0;
      json::Array seqs;
      seqs.push_back(seq);

      expected[FIELD_RESPONSE_ID] = 19;
      expected[FIELD_SEQUENCES] = seqs;
      expected[FIELD_COMPLETE] = true;

      ResourceUtilStreamResponse response(sequences, ResourceUtilData(), true);
      CHECK(response.toJson() == expected);
   }

   SECTION("All data existing, not complete, multi sequences")
   {
      StreamSequences sequences;
      sequences.emplace_back(3, 12);
      sequences.emplace_back(12, 0);
      sequences.emplace_back(4, 3);

      json::Array seqsArr;
      for (const StreamSequenceId& seq: sequences)
         seqsArr.push_back(seq.toJson());

      expected[FIELD_RESPONSE_ID] = 20;
      expected[FIELD_SEQUENCES] = seqsArr;
      expected[FIELD_COMPLETE] = false;
      expected[FIELD_CPU_PERCENT] = 95.4;
      expected[FIELD_CPU_SECONDS] = 3561.0;
      expected[FIELD_VIRTUAL_MEM] = 1282.0;
      expected[FIELD_RESIDENT_MEM] = 922.0;

      ResourceUtilData data;
      data.CpuPercent = 95.4;
      data.CpuSeconds = 3561.0;
      data.VirtualMem = 1282.0;
      data.ResidentMem = 922.0;

      ResourceUtilStreamResponse response(sequences, data, false);
      CHECK(response.toJson() == expected);
   }
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
