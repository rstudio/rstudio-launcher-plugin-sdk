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
#include <api/Response.hpp>
#include <json/Json.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

TEST_CASE("Create Bootstrap Response")
{
   json::Object version;
   version.insert(FIELD_VERSION_MAJOR, json::Value(API_VERSION_MAJOR));
   version.insert(FIELD_VERSION_MINOR, json::Value(API_VERSION_MINOR));
   version.insert(FIELD_VERSION_PATCH, json::Value(API_VERSION_PATCH));

   json::Object expectedResult;
   expectedResult.insert(FIELD_MESSAGE_TYPE, json::Value(1));
   expectedResult.insert(FIELD_REQUEST_ID, json::Value(10));
   expectedResult.insert(FIELD_RESPONSE_ID, json::Value(1));
   expectedResult.insert(FIELD_VERSION, version);

   BootstrapResponse bootstrapResponse(10);
   json::Object bootstrapObject = bootstrapResponse.asJson();

   CHECK(bootstrapObject == expectedResult);
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
