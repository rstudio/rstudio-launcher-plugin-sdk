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

// Container
constexpr char const* CONTAINER                       = "container";
constexpr char const* CONTAINER_IMAGE                 = "image";
constexpr char const* CONTAINER_RUN_AS_USER_ID        = "runAsUserId";
constexpr char const* CONTAINER_RUN_AS_GROUP_ID       = "runAsGroupId";
constexpr char const* CONTAINER_SUPP_GROUP_IDS        = "supplementalGroupIds";

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

// Mount
constexpr char const* MOUNT                           = "mount";
constexpr char const* MOUNT_PATH                      = "mountPath";
constexpr char const* MOUNT_READ_ONLY                 = "readOnly";
constexpr char const* MOUNT_SOURCE_HOST               = "hostMount";
constexpr char const* MOUNT_SOURCE_NFS                = "nfsMount";
constexpr char const* MOUNT_SOURCE_PATH               = "path";
constexpr char const* MOUNT_SOURCE_NFS_HOST           = "host";

// Resource Limit
constexpr char const* RESOURCE_LIMITS                 = "resourceLimits";
constexpr char const* RESOURCE_LIMIT_DEFAULT          = "defaultValue";
constexpr char const* RESOURCE_LIMIT_MAX              = "maxValue";
constexpr char const* RESOURCE_LIMIT_TYPE             = "type";
constexpr char const* RESOURCE_LIMIT_TYPE_CPU_COUNT   = "cpuCount";
constexpr char const* RESOURCE_LIMIT_TYPE_CPU_TIME    = "cpuTime";
constexpr char const* RESOURCE_LIMIT_TYPE_MEMORY      = "memory";
constexpr char const* RESOURCE_LIMIT_TYPE_MEMORY_SWAP = "memorySwap";
constexpr char const* RESOURCE_LIMIT_VALUE            = "value";

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

// Container ===========================================================================================================
Error Container::fromJson(const json::Object& in_json, Container& out_container)
{
   Optional<json::Array> supplementalGroupIds;

   Error error = json::readObject(in_json,
      CONTAINER_IMAGE, out_container.Image,
      CONTAINER_RUN_AS_USER_ID, out_container.RunAsUserId,
      CONTAINER_RUN_AS_GROUP_ID, out_container.RunAsGroupId,
      CONTAINER_SUPP_GROUP_IDS, supplementalGroupIds);

   if (error)
      return updateError(CONTAINER, in_json, error);

   if (supplementalGroupIds &&
      !supplementalGroupIds.getValueOr(json::Array()).toVectorInt(out_container.SupplementalGroupIds))
   {
     return updateError(
        CONTAINER,
        in_json,
        error = Error("JobParseError", 1, "Invalid type for supplemental group ids", ERROR_LOCATION));
   }

   return Success();
}

json::Object Container::toJson() const
{
   json::Object containerObj;
   containerObj[CONTAINER_IMAGE] = Image;

   if (RunAsUserId)
      containerObj[CONTAINER_RUN_AS_USER_ID] = RunAsUserId.getValueOr(0);
   if (RunAsGroupId)
      containerObj[CONTAINER_RUN_AS_GROUP_ID] = RunAsGroupId.getValueOr(0);
   if (!SupplementalGroupIds.empty())
      containerObj[CONTAINER_SUPP_GROUP_IDS] = json::toJsonArray(SupplementalGroupIds);

   return containerObj;
}

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

// Host Mount Source ===================================================================================================
Error HostMountSource::fromJson(const json::Object& in_json, HostMountSource& out_mountSource)
{
   Error error = json::readObject(in_json, MOUNT_SOURCE_PATH, out_mountSource.Path);
   if (error)
      return updateError(MOUNT_SOURCE_HOST, in_json, error);

   return Success();
}

json::Object HostMountSource::toJson() const
{
   json::Object mountSourceObj;
   mountSourceObj[MOUNT_SOURCE_PATH] = Path;

   return mountSourceObj;
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
               "JobParseError",
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

// Mount ===============================================================================================================
Error Mount::fromJson(const json::Object& in_json, Mount& out_mount)
{
   Optional<json::Object> hostMountSource, nfsMountSource;
   Optional<bool> isReadOnly;
   Error error = json::readObject(in_json,
      MOUNT_PATH, out_mount.DestinationPath,
      MOUNT_SOURCE_HOST, hostMountSource,
      MOUNT_SOURCE_NFS, nfsMountSource,
      MOUNT_READ_ONLY, isReadOnly);

   if (error)
      return updateError(MOUNT, in_json, error);

   if (!hostMountSource && !nfsMountSource)
   {
      // TODO: real error set up.
      error = Error("JobParseError", 1, "No mount source specified", ERROR_LOCATION);
      return updateError(MOUNT, in_json, error);
   }
   else if (hostMountSource && nfsMountSource)
      {
         error = Error("JobParseError", 1, "Multiple mount sources specified", ERROR_LOCATION);
         return updateError(MOUNT, in_json, error);
      }
      else if (hostMountSource)
         {
            HostMountSource mountSource;
            error = HostMountSource::fromJson(hostMountSource.getValueOr(json::Object()), mountSource);
            if (error)
               return updateError(MOUNT, in_json, error);

            out_mount.HostSourcePath = mountSource;
         }
         else
         {
            NfsMountSource mountSource;
            error = NfsMountSource::fromJson(nfsMountSource.getValueOr(json::Object()), mountSource);
            if (error)
               return updateError(MOUNT, in_json, error);

            out_mount.NfsSourcePath = mountSource;
         }

   out_mount.IsReadOnly = isReadOnly.getValueOr(false);

   return Success();
}

json::Object Mount::toJson() const
{
   // There should be exactly one mount source. If both are set, it's a programmer error. Assert, but in release mode
   // just set both (or no) mount sources on the object.
   assert((NfsSourcePath && !HostSourcePath) || (!NfsSourcePath && HostSourcePath));

   json::Object mountObj;
   mountObj[MOUNT_PATH] = DestinationPath;
   mountObj[MOUNT_READ_ONLY] = IsReadOnly;

   if (HostSourcePath)
      mountObj[MOUNT_SOURCE_HOST] = HostSourcePath.getValueOr(HostMountSource()).toJson();

   if (NfsSourcePath)
      mountObj[MOUNT_SOURCE_NFS] = NfsSourcePath.getValueOr(NfsMountSource()).toJson();

   return mountObj;
}

// NFS Mount Source ====================================================================================================
Error NfsMountSource::fromJson(const json::Object& in_json, NfsMountSource& out_mountSource)
{
   Error error = json::readObject(in_json,
      MOUNT_SOURCE_PATH, out_mountSource.Path,
      MOUNT_SOURCE_NFS_HOST, out_mountSource.Host);
   if (error)
      return updateError(MOUNT_SOURCE_NFS, in_json, error);

   return Success();
}

json::Object NfsMountSource::toJson() const
{
   json::Object mountSourceObj;
   mountSourceObj[MOUNT_SOURCE_PATH] = Path;
   mountSourceObj[MOUNT_SOURCE_NFS_HOST] = Host;

   return mountSourceObj;
}

// Resource Limit ======================================================================================================
ResourceLimit::ResourceLimit(Type in_limitType,  std::string in_maxValue, std::string in_defaultValue) :
   ResourceType(in_limitType),
   MaxValue(std::move(in_maxValue)),
   DefaultValue(std::move(in_defaultValue))
{
}

Error ResourceLimit::fromJson(const json::Object& in_json, ResourceLimit& out_resourceLimit)
{
   std::string strType;
   Error error = json::readObject(in_json,
      RESOURCE_LIMIT_TYPE, strType,
      RESOURCE_LIMIT_VALUE, out_resourceLimit.Value);

   if (error)
      return updateError(RESOURCE_LIMITS, in_json, error);

   boost::trim(strType);
   if (strType == RESOURCE_LIMIT_TYPE_CPU_COUNT)
      out_resourceLimit.ResourceType = Type::CPU_COUNT;
   else if (strType == RESOURCE_LIMIT_TYPE_CPU_TIME)
      out_resourceLimit.ResourceType = Type::CPU_TIME;
   else if (strType == RESOURCE_LIMIT_TYPE_MEMORY)
      out_resourceLimit.ResourceType = Type::MEMORY;
   else if (strType == RESOURCE_LIMIT_TYPE_MEMORY_SWAP)
      out_resourceLimit.ResourceType = Type::MEMORY_SWAP;
   else
      return updateError(
         RESOURCE_LIMITS,
         in_json,
         error = Error("JobParseError", 1, "Invalid resource type", ERROR_LOCATION));

   return Success();
}

json::Object ResourceLimit::toJson() const
{
   json::Object limitObj;

   switch (ResourceType)
   {
      case Type::CPU_COUNT:
      {
         limitObj[RESOURCE_LIMIT_TYPE] = RESOURCE_LIMIT_TYPE_CPU_COUNT;
         break;
      }
      case Type::CPU_TIME:
      {
         limitObj[RESOURCE_LIMIT_TYPE] = RESOURCE_LIMIT_TYPE_CPU_TIME;
         break;
      }
      case Type::MEMORY:
      {
         limitObj[RESOURCE_LIMIT_TYPE] = RESOURCE_LIMIT_TYPE_MEMORY;
         break;
      }
      case Type::MEMORY_SWAP:
      {
         limitObj[RESOURCE_LIMIT_TYPE] = RESOURCE_LIMIT_TYPE_MEMORY_SWAP;
         break;
      }
      default:
      {
         // This should only happen if a resource type is added and this method isn't updated.
         assert(false);
      }
   }

   if (!Value.empty())
      limitObj[RESOURCE_LIMIT_VALUE] = Value;
   if (!DefaultValue.empty())
      limitObj[RESOURCE_LIMIT_DEFAULT] = DefaultValue;
   if (!MaxValue.empty())
      limitObj[RESOURCE_LIMIT_MAX] = MaxValue;

   return limitObj;
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
