/*
 * ResponseTests.cpp
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

#include <api/Constants.hpp>
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
      ClusterInfoResponse clusterInfoResponse(26);

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

      ClusterInfoResponse clusterInfoResponse(
         26,
         { "queue1", "QUEUE-TWO" },
         { limit1, limit2, limit3 });

      json::Array queues, limits;
      queues.push_back("queue1");
      queues.push_back("QUEUE-TWO");
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

      ClusterInfoResponse clusterInfoResponse(
         26,
         { "queue1", "QUEUE-TWO", "another queue" },
         { limit1, limit2, limit3, limit4 },
         { constraint1, constraint2, constraint3, constraint4, constraint5 },
         { config1, config2, config3 });

      json::Array config, constraints, queues, limits;
      config.push_back(config1.toJson());
      config.push_back(config2.toJson());
      config.push_back(config3.toJson());
      constraints.push_back(constraint1.toJson());
      constraints.push_back(constraint2.toJson());
      constraints.push_back(constraint3.toJson());
      constraints.push_back(constraint4.toJson());
      constraints.push_back(constraint5.toJson());
      queues.push_back("queue1");
      queues.push_back("QUEUE-TWO");
      queues.push_back("another queue");
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
      std::set<std::string> images = { "image-number-1", "Image2", "  image_three_ " };

      ClusterInfoResponse clusterInfoResponse(
         26,
         images);

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
      std::set<std::string> images = { "image-number-1", "Image2", "  image_three_ " };

      ClusterInfoResponse clusterInfoResponse(
         26,
         images,
         "  image_three_ ",
         true);

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

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
