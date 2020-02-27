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
#include <boost/algorithm/string/trim.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

// Job JSON field constants

// Exposed Port
constexpr char const* EXPOSED_PORT                    = "exposedPort";
constexpr char const* EXPOSED_PORT_TARGET             = "targetPort";
constexpr char const* EXPOSED_PORT_PUBLISHED          = "publishedPort";
constexpr char const* EXPOSED_PORT_PROTOCOL           = "protocol";
constexpr char const* EXPOSED_PORT_PROTOCOL_DEFAULT   = "TCP";

// Job Config
constexpr char const* JOB_CONFIG                      = "config";
constexpr char const* JOB_CONFIG_NAME                 = "name";
constexpr char const* JOB_CONFIG_VALUE                = "value";
constexpr char const* JOB_CONFIG_TYPE                 = "valueType";
constexpr char const* JOB_CONFIG_TYPE_ENUM            = "enum";
constexpr char const* JOB_CONFIG_TYPE_FLOAT           = "float";
constexpr char const* JOB_CONFIG_TYPE_INT             = "int";
constexpr char const* JOB_CONFIG_TYPE_STRING          = "string";

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
   Optional<std::string> protocol;
   Error error = json::readObject(in_json,
      EXPOSED_PORT_TARGET, out_exposedPort.TargetPort,
      EXPOSED_PORT_PROTOCOL, protocol,
      EXPOSED_PORT_PUBLISHED, out_exposedPort.PublishedPort);

   if (error)
      return error;

   out_exposedPort.Protocol = protocol.getValueOr(EXPOSED_PORT_PROTOCOL_DEFAULT);
   return Success();
}

json::Object ExposedPort::toJson() const
{
   json::Object exposedPortObj;
   exposedPortObj[EXPOSED_PORT_TARGET] = TargetPort;

   if (PublishedPort)
      exposedPortObj[EXPOSED_PORT_PUBLISHED] = PublishedPort.getValueOr(0);

   exposedPortObj[EXPOSED_PORT_PROTOCOL] = Protocol;

   return exposedPortObj;
}

// Job Config ==========================================================================================================
JobConfig::JobConfig(const std::string& in_name, Type in_type) :
   Name(in_name),
   ValueType(in_type)
{
}

Error JobConfig::fromJson(const json::Object& in_json, JobConfig& out_jobConfig)
{
   Optional<std::string> optStrType;
   Error error = json::readObject(in_json,
      JOB_CONFIG_NAME, out_jobConfig.Name,
      JOB_CONFIG_VALUE, out_jobConfig.Value,
      JOB_CONFIG_TYPE, optStrType);

   if (error)
      return updateError(JOB_CONFIG, in_json, error);

   if (optStrType)
   {
      std::string strType = optStrType.getValueOr("");
      boost::trim(strType);
      if (strType == JOB_CONFIG_TYPE_ENUM)
         out_jobConfig.ValueType = Type::ENUM;
      else if (strType == JOB_CONFIG_TYPE_FLOAT)
         out_jobConfig.ValueType = Type::FLOAT;
      else if (strType == JOB_CONFIG_TYPE_INT)
         out_jobConfig.ValueType = Type::INT;
      else if (strType == JOB_CONFIG_TYPE_STRING)
         out_jobConfig.ValueType = Type::STRING;
      else
         return updateError(
            JOB_CONFIG,
            in_json,
            error = Error(
               "JobConfigParseError",
               1,
               "Invalid Job Config Value Type (" + strType + ")", ERROR_LOCATION));
   }

   return Success();
}

json::Object JobConfig::toJson() const
{
   json::Object confObj;
   confObj[JOB_CONFIG_NAME] = Name;

   if (ValueType)
   {
      // It should never use the default value here, since ValueType is set.
      switch (ValueType.getValueOr(Type::ENUM))
      {
         case Type::ENUM:
         {
            confObj[JOB_CONFIG_TYPE] = JOB_CONFIG_TYPE_ENUM;
            break;
         }
         case Type::FLOAT:
         {
            confObj[JOB_CONFIG_TYPE] = JOB_CONFIG_TYPE_FLOAT;
            break;
         }
         case Type::INT:
         {
            confObj[JOB_CONFIG_TYPE] = JOB_CONFIG_TYPE_INT;
            break;
         }
         case Type::STRING:
         {
            confObj[JOB_CONFIG_TYPE] = JOB_CONFIG_TYPE_STRING;
            break;
         }
         default:
         {
            // We can only reach this if a new Type was added but this switch wasn't updated - do nothing in release, but
            // assert. It _should_ be caught by unit tests.
            assert(false);
         }
      }
   }

   if (!Value.empty())
      confObj[JOB_CONFIG_VALUE] = Value;

   return confObj;
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
