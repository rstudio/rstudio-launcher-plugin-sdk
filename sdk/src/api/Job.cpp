/*
 * Job.cpp
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

#include <api/Job.hpp>

#include <Error.hpp>
#include <json/Json.hpp>
#include <json/JsonUtils.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

Error& updateError(const std::string& in_name, const json::Object& in_object, Error& io_error)
{
   if (io_error)
   {
      std::string description = io_error.getProperty("description");
      description += " on object " + in_name + ": " + in_object.write();
      io_error.addOrUpdateProperty("description", description);
   }

   return io_error;
}

} // anonymous namespace

// Exposed Port ========================================================================================================
Error ExposedPort::fromJson(const json::Object& in_json, ExposedPort& out_exposedPort)
{
   Error error = json::readObject(in_json, "targetPort", out_exposedPort.TargetPort);
   if (error)
      return updateError("exposedPort", in_json, error);

   Optional<std::string> protocol;
   error = json::readObject(in_json, "protocol", protocol);
   if (error)
      return updateError("exposedPort", in_json, error);

   out_exposedPort.Protocol = protocol.getValueOr("TCP");

   error = json::readObject(in_json, "publishedPort", out_exposedPort.PublishedPort);
   if (error)
      return updateError("exposedPort", in_json, error);

   return Success();
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
