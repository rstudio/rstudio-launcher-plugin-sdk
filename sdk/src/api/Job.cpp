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

#include <mutex>

#include <boost/algorithm/string/trim.hpp>

#include <Error.hpp>
#include <json/Json.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

// Job JSON field constants

// Container
constexpr char const* CONTAINER_IMAGE                 = "image";
constexpr char const* CONTAINER_RUN_AS_USER_ID        = "runAsUserId";
constexpr char const* CONTAINER_RUN_AS_GROUP_ID       = "runAsGroupId";
constexpr char const* CONTAINER_SUPP_GROUP_IDS        = "supplementalGroupIds";

// Exposed Port
constexpr char const* EXPOSED_PORT_TARGET             = "targetPort";
constexpr char const* EXPOSED_PORT_PUBLISHED          = "publishedPort";
constexpr char const* EXPOSED_PORT_PROTOCOL           = "protocol";
constexpr char const* EXPOSED_PORT_PROTOCOL_DEFAULT   = "TCP";

// Environment
constexpr char const* ENVIRONMENT_NAME                = "name";
constexpr char const* ENVIRONMENT_VALUE               = "value";

// Job
constexpr char const* JOB_ARGUMENTS                   = "args";
constexpr char const* JOB_CLUSTER                     = "cluster";
constexpr char const* JOB_COMMAND                     = "command";
constexpr char const* JOB_CONFIG                      = "config";
constexpr char const* JOB_CONTAINER                   = "container";
constexpr char const* JOB_ENVIRONMENT                 = "environment";
constexpr char const* JOB_EXECUTABLE                  = "exe";
constexpr char const* JOB_EXIT_CODE                   = "exitCode";
constexpr char const* JOB_EXPOSED_PORTS               = "exposedPorts";
constexpr char const* JOB_HOST                        = "host";
constexpr char const* JOB_ID                          = "id";
constexpr char const* JOB_LAST_UPDATE_TIME            = "lastUpdateTime";
constexpr char const* JOB_MOUNTS                      = "mounts";
constexpr char const* JOB_NAME                        = "name";
constexpr char const* JOB_PID                         = "pid";
constexpr char const* JOB_PLACEMENT_CONSTRAINTS       = "placementConstraints";
constexpr char const* JOB_QUEUES                      = "queues";
constexpr char const* JOB_RESOURCE_LIMITS             = "resourceLimits";
constexpr char const* JOB_STANDARD_IN                 = "stdin";
constexpr char const* JOB_STANDARD_ERROR_FILE         = "stderrFile";
constexpr char const* JOB_STANDARD_OUTPUT_FILE        = "stdoutFile";
constexpr char const* JOB_STATUS                      = "status";
constexpr char const* JOB_STATUS_MESSAGE              = "statusMessage";
constexpr char const* JOB_SUBMISSION_TIME             = "submissionTime";
constexpr char const* JOB_TAGS                        = "tags";
constexpr char const* JOB_USER                        = "user";
constexpr char const* JOB_WORKING_DIRECTORY           = "workingDirectory";

// Job Config
constexpr char const* JOB_CONFIG_NAME                 = "name";
constexpr char const* JOB_CONFIG_VALUE                = "value";
constexpr char const* JOB_CONFIG_TYPE                 = "valueType";
constexpr char const* JOB_CONFIG_TYPE_ENUM            = "enum";
constexpr char const* JOB_CONFIG_TYPE_FLOAT           = "float";
constexpr char const* JOB_CONFIG_TYPE_INT             = "int";
constexpr char const* JOB_CONFIG_TYPE_STRING          = "string";

// Job Status Values
constexpr const char* JOB_STATUS_CANCELED             = "Canceled";
constexpr const char* JOB_STATUS_FAILED               = "Failed";
constexpr const char* JOB_STATUS_FINISHED             = "Finished";
constexpr const char* JOB_STATUS_KILLED               = "Killed";
constexpr const char* JOB_STATUS_PENDING              = "Pending";
constexpr const char* JOB_STATUS_RUNNING              = "Running";
constexpr const char* JOB_STATUS_SUSPENDED            = "Suspended";

// Mount
constexpr char const* MOUNT_PATH                      = "mountPath";
constexpr char const* MOUNT_READ_ONLY                 = "readOnly";
constexpr char const* MOUNT_TYPE                      = "type";
constexpr char const* MOUNT_SOURCE                    = "source";
constexpr char const* MOUNT_TYPE_AZURE                = "azureFile";
constexpr char const* MOUNT_TYPE_CEPH                 = "cephFs";
constexpr char const* MOUNT_TYPE_GLUSTER              = "glusterFs";
constexpr char const* MOUNT_TYPE_HOST                 = "host";
constexpr char const* MOUNT_TYPE_NFS                  = "nfs";
constexpr char const* MOUNT_TYPE_PASSTHROUGH          = "passthrough";
constexpr char const* MOUNT_SOURCE_ENDPOINTS          = "endpoints";
constexpr char const* MOUNT_SOURCE_HOST               = "host";
constexpr char const* MOUNT_SOURCE_MONITORS           = "monitors";
constexpr char const* MOUNT_SOURCE_PATH               = "path";
constexpr char const* MOUNT_SOURCE_SECRET_FILE        = "secretFile";
constexpr char const* MOUNT_SOURCE_SECRET_NAME        = "secretName";
constexpr char const* MOUNT_SOURCE_SECRET_REF         = "secretRef";
constexpr char const* MOUNT_SOURCE_SHARE_NAME         = "shareName";
constexpr char const* MOUNT_SOURCE_USER               = "user";

// Placement Constraint
constexpr char const* PLACEMENT_CONSTRAINT_NAME       = "name";
constexpr char const* PLACEMENT_CONSTRAINT_VALUE      = "value";

// Resource Limit
constexpr char const* RESOURCE_LIMIT_DEFAULT          = "defaultValue";
constexpr char const* RESOURCE_LIMIT_MAX              = "maxValue";
constexpr char const* RESOURCE_LIMIT_TYPE             = "type";
constexpr char const* RESOURCE_LIMIT_VALUE            = "value";

enum class JobParseError
{
   SUCCESS = 0,
   INVALID_VALUE = 1,
   MISSING_VALUE = 2,
   CONFLICTING_VALUES = 3
};

Error jobParseError(
   JobParseError in_code,
   const std::string& in_details,
   const std::string& in_objName,
   const json::Value& in_json,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation)
{
   std::string message;
   switch (in_code)
   {
      case JobParseError::SUCCESS:
         return Success();
      case JobParseError::INVALID_VALUE:
      {
         message = "Invalid value: ";
         break;
      }
      case JobParseError::MISSING_VALUE:
      {
         message = "Required value was not set: ";
         break;
      }
      case JobParseError::CONFLICTING_VALUES:
      {
         message = "Multiple conflicting values set: ";
         break;
      }
   }
   Error error;
   if (in_cause)
      error = Error("JobParseError", static_cast<int>(in_code), message + in_details, in_cause, in_errorLocation);
   else
      error = Error("JobParseError", static_cast<int>(in_code), message + in_details, in_errorLocation);

   error.addProperty(in_objName, in_json.write());
   return error;
}

Error jobParseError(
   JobParseError in_code,
   const std::string& in_details,
   const std::string& in_objName,
   const json::Value& in_json,
   const ErrorLocation& in_errorLocation)
{
   return jobParseError(in_code, in_details, in_objName, in_json, Success(), in_errorLocation);
}

inline std::string quoteStr(const char* in_str)
{
   return std::string("\"").append(in_str).append("\"");
}

inline std::string quoteStr(const std::string& in_str)
{
   return quoteStr(in_str.c_str());
}

bool jobStatusFromString(const std::string& io_statusStr, Job::State& out_state)
{
   std::string statusStr = boost::trim_copy(io_statusStr);
   if (statusStr == JOB_STATUS_CANCELED)
      out_state = Job::State::CANCELED;
   else if (statusStr == JOB_STATUS_FAILED)
      out_state =  Job::State::FAILED;
   else if (statusStr == JOB_STATUS_FINISHED)
      out_state =  Job::State::FINISHED;
   else if (statusStr == JOB_STATUS_KILLED)
      out_state =  Job::State::KILLED;
   else if (statusStr == JOB_STATUS_PENDING)
      out_state =  Job::State::PENDING;
   else if (statusStr == JOB_STATUS_RUNNING)
      out_state =  Job::State::RUNNING;
   else if (statusStr == JOB_STATUS_SUSPENDED)
      out_state =  Job::State::SUSPENDED;
   else if (!statusStr.empty())
      return false;
   else
      out_state = Job::State::UNKNOWN;

   return true;
}

std::string jobStatusToString(const Job::State& in_state)
{
   switch (in_state)
   {
      case Job::State::CANCELED:
         return JOB_STATUS_CANCELED;
      case Job::State::FAILED:
         return JOB_STATUS_FAILED;
      case Job::State::FINISHED:
         return JOB_STATUS_FINISHED;
      case Job::State::KILLED:
         return JOB_STATUS_KILLED;
      case Job::State::PENDING:
         return JOB_STATUS_PENDING;
      case Job::State::RUNNING:
         return JOB_STATUS_RUNNING;
      case Job::State::SUSPENDED:
         return JOB_STATUS_SUSPENDED;
      case Job::State::UNKNOWN:
      default:
         return "";
   }
}

std::string mountTypeToString(MountSource::Type in_type, const std::string& in_customType)
{
   switch (in_type)
   {
      case MountSource::Type::AZURE_FILE:
         return MOUNT_TYPE_AZURE;
      case MountSource::Type::CEPH_FS:
         return MOUNT_TYPE_CEPH;
      case MountSource::Type::GLUSTER_FS:
         return MOUNT_TYPE_GLUSTER;
      case MountSource::Type::HOST:
         return MOUNT_TYPE_HOST;
      case MountSource::Type::NFS:
         return MOUNT_TYPE_NFS;
   };

   if (!in_customType.empty())
      return in_customType;

   return MOUNT_TYPE_PASSTHROUGH;
}

template <typename T>
Error fromJsonArray(const std::string& in_arrayName, const json::Array& in_jsonArray, std::vector<T>& out_array)
{
   for (const json::Value& jsonVal: in_jsonArray)
   {
      if (!jsonVal.isObject())
         return jobParseError(
            JobParseError::INVALID_VALUE,
            "value " + quoteStr(jsonVal.write()) + " has an invalid type",
            in_arrayName,
            in_jsonArray,
            ERROR_LOCATION);

      T val;
      Error error = T::fromJson(jsonVal.getObject(), val);
      if (error)
         return error;

      out_array.push_back(val);
   }

   return Success();
}

template <>
Error fromJsonArray(const std::string& in_arrayName, const json::Array& in_jsonArray, EnvironmentList& out_array)
{
   for (const json::Value& jsonVal: in_jsonArray)
   {
      if (!jsonVal.isObject())
         return jobParseError(
            JobParseError::INVALID_VALUE,
            "value " + quoteStr(jsonVal.write()) + " has an invalid type",
            in_arrayName,
            in_jsonArray,
            ERROR_LOCATION);

      std::string name, value;
      Error error = json::readObject(jsonVal.getObject(),
         ENVIRONMENT_NAME, name,
         ENVIRONMENT_VALUE, value);

      if (error)
         return error;

      out_array.push_back(std::make_pair(name, value));
   }

   return Success();
}

template <typename T>
json::Array toJsonArray(const std::vector<T>& in_vector)
{
   json::Array arr;

   for (const T& val: in_vector)
      arr.push_back(val.toJson());

   return arr;
}

template <>
json::Array toJsonArray(const EnvironmentList& in_vector)
{
   json::Array arr;

   for (const EnvVariable& val: in_vector)
   {
      json::Object envObj;
      envObj[ENVIRONMENT_NAME] = val.first;
      envObj[ENVIRONMENT_VALUE] = val.second;
      arr.push_back(envObj);
   }

   return arr;
}

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
      return updateError(JOB_CONTAINER, in_json, error);

   if (supplementalGroupIds &&
      !supplementalGroupIds.getValueOr(json::Array()).toVectorInt(out_container.SupplementalGroupIds))
      return jobParseError(
         JobParseError::INVALID_VALUE,
         quoteStr(CONTAINER_SUPP_GROUP_IDS) + " contains a value with an invalid type.",
         JOB_CONTAINER,
         in_json,
         ERROR_LOCATION);
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

// Job =================================================================================================================
struct Job::Impl
{
   std::recursive_mutex Mutex;
};

PRIVATE_IMPL_DELETER_IMPL(Job)

Job::Job() :
   m_impl(new Impl()),
   User(true) // Create with an empty user.
{
}

Job::Job(const Job& in_other) :
   m_impl(new Impl()), // Don't copy the mutex.
   Arguments(in_other.Arguments),
   Cluster(in_other.Cluster),
   Command(in_other.Command),
   Config(in_other.Config),
   ContainerDetails(in_other.ContainerDetails),
   Environment(in_other.Environment),
   Exe(in_other.Exe),
   ExitCode(in_other.ExitCode),
   ExposedPorts(in_other.ExposedPorts),
   Host(in_other.Host),
   Id(in_other.Id),
   LastUpdateTime(in_other.LastUpdateTime),
   Mounts(in_other.Mounts),
   Name(in_other.Name),
   Pid(in_other.Pid),
   PlacementConstraints(in_other.PlacementConstraints),
   Queues(in_other.Queues),
   ResourceLimits(in_other.ResourceLimits),
   StandardIn(in_other.StandardIn),
   StandardErrFile(in_other.StandardErrFile),
   StandardOutFile(in_other.StandardOutFile),
   Status(in_other.Status),
   StatusMessage(in_other.StatusMessage),
   SubmissionTime(in_other.SubmissionTime),
   Tags(in_other.Tags),
   User(in_other.User),
   WorkingDirectory(in_other.WorkingDirectory)
{
}

Job::Job(Job&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl)), // Don't copy the mutex.
   Arguments(std::move(in_other.Arguments)),
   Cluster(std::move(in_other.Cluster)),
   Command(std::move(in_other.Command)),
   Config(std::move(in_other.Config)),
   ContainerDetails(std::move(in_other.ContainerDetails)),
   Environment(std::move(in_other.Environment)),
   Exe(std::move(in_other.Exe)),
   ExitCode(std::move(in_other.ExitCode)),
   ExposedPorts(std::move(in_other.ExposedPorts)),
   Host(std::move(in_other.Host)),
   Id(std::move(in_other.Id)),
   LastUpdateTime(std::move(in_other.LastUpdateTime)),
   Mounts(std::move(in_other.Mounts)),
   Name(std::move(in_other.Name)),
   Pid(std::move(in_other.Pid)),
   PlacementConstraints(std::move(in_other.PlacementConstraints)),
   Queues(std::move(in_other.Queues)),
   ResourceLimits(std::move(in_other.ResourceLimits)),
   StandardIn(std::move(in_other.StandardIn)),
   StandardErrFile(std::move(in_other.StandardErrFile)),
   StandardOutFile(std::move(in_other.StandardOutFile)),
   Status(in_other.Status),
   StatusMessage(std::move(in_other.StatusMessage)),
   SubmissionTime(std::move(in_other.SubmissionTime)),
   Tags(std::move(in_other.Tags)),
   User(std::move(in_other.User)),
   WorkingDirectory(std::move(in_other.WorkingDirectory))
{
}

Error Job::fromJson(const json::Object& in_json, Job& out_job)
{
   // Everything but the name is optional.
   Job result;
   Optional<std::string> submitTime;
   Optional<std::vector<std::string> > arguments;
   Optional<std::set<std::string> > queues, tags;
   Optional<std::string> cluster, command, exe, host, id, lastUpTime, stdIn, stdErr, stdOut, status, statusMessage,
                         user, workingDir;
   Optional<json::Object> containerObj;
   Optional<json::Array> config, env, ports, mounts, constraints, limits;

   Error error = json::readObject(in_json,
                                  JOB_ARGUMENTS, arguments,
                                  JOB_CLUSTER, cluster,
                                  JOB_COMMAND, command,
                                  JOB_CONFIG, config,
                                  JOB_CONTAINER, containerObj,
                                  JOB_ENVIRONMENT, env,
                                  JOB_EXECUTABLE, exe,
                                  JOB_EXIT_CODE, result.ExitCode,
                                  JOB_EXPOSED_PORTS, ports,
                                  JOB_HOST, host,
                                  JOB_ID, id,
                                  JOB_LAST_UPDATE_TIME, lastUpTime,
                                  JOB_MOUNTS, mounts,
                                  JOB_NAME, result.Name,
                                  JOB_PID, result.Pid,
                                  JOB_PLACEMENT_CONSTRAINTS, constraints,
                                  JOB_QUEUES, queues,
                                  JOB_RESOURCE_LIMITS, limits,
                                  JOB_STANDARD_IN, stdIn,
                                  JOB_STANDARD_ERROR_FILE, stdErr,
                                  JOB_STANDARD_OUTPUT_FILE, stdOut,
                                  JOB_STATUS, status,
                                  JOB_STATUS_MESSAGE, statusMessage,
                                  JOB_SUBMISSION_TIME, submitTime,
                                  JOB_TAGS, tags,
                                  JOB_USER, user,
                                  JOB_WORKING_DIRECTORY, workingDir);

   if (error)
      return error;

   // If both command and exe are non-empty, it's ambiguous which command should be run.
   if ((command && !command.getValueOr("").empty()) && (exe && !exe.getValueOr("").empty()))
      return jobParseError(
         JobParseError::CONFLICTING_VALUES,
         quoteStr(JOB_COMMAND) + " and " + quoteStr(JOB_EXECUTABLE),
         "job",
         in_json,
         ERROR_LOCATION);

   // If all of these are empty, there's nothing to run.
   if ((!command || command.getValueOr("").empty()) &&
      (!exe || exe.getValueOr("").empty()) &&
      (!containerObj || containerObj.getValueOr(json::Object()).isEmpty()))
      return jobParseError(
         JobParseError::MISSING_VALUE,
         "one of " +
            quoteStr(JOB_CONTAINER) +
            " and/or one of " +
            quoteStr(JOB_COMMAND) +
            " and " +
            quoteStr(JOB_EXECUTABLE),
         "job",
         in_json,
         ERROR_LOCATION);

   if (!user || user.getValueOr("").empty())
   {
      result.User = system::User(true); // Make an empty user. Possible on Job submission.
   }
   else if (user.getValueOr("") == "*")
      result.User = system::User(); // Default user is all users.
   else
   {
      error = system::User::getUserFromIdentifier(user.getValueOr(""), result.User);
      if (error)
         return jobParseError(
            JobParseError::INVALID_VALUE,
            quoteStr(user.getValueOr("")) + " is not a valid user.",
            "job",
            in_json,
            error,
            ERROR_LOCATION);
   }

   result.Arguments = arguments.getValueOr({});
   result.Cluster = cluster.getValueOr("");
   result.Command = command.getValueOr("");
   result.Exe = exe.getValueOr("");
   result.Host = host.getValueOr("");
   result.Id = id.getValueOr("");
   result.Queues = queues.getValueOr({});
   result.StandardIn = stdIn.getValueOr("");
   result.StandardErrFile = stdErr.getValueOr("");
   result.StandardOutFile = stdOut.getValueOr("");
   result.StatusMessage = statusMessage.getValueOr("");
   result.Tags = tags.getValueOr({});
   result.WorkingDirectory = workingDir.getValueOr("");

   if (containerObj)
   {
      Container container;
      error = Container::fromJson(containerObj.getValueOr(json::Object()), container);
      if (error)
         return error;

      result.ContainerDetails = container;
   }

   error = fromJsonArray(JOB_CONFIG, config.getValueOr(json::Array()), result.Config);
   if (error)
      return error;

   error = fromJsonArray(JOB_ENVIRONMENT, env.getValueOr(json::Array()), result.Environment);
   if (error)
      return error;

   error = fromJsonArray(JOB_EXPOSED_PORTS, ports.getValueOr(json::Array()), result.ExposedPorts);
   if (error)
      return error;

   error = fromJsonArray(JOB_MOUNTS, mounts.getValueOr(json::Array()), result.Mounts);
   if (error)
      return error;

   error = fromJsonArray(JOB_PLACEMENT_CONSTRAINTS, constraints.getValueOr(json::Array()), result.PlacementConstraints);
   if (error)
      return error;

   error = fromJsonArray(JOB_RESOURCE_LIMITS, limits.getValueOr(json::Array()), result.ResourceLimits);
   if (error)
      return error;

   if (!jobStatusFromString(status.getValueOr(""), result.Status))
      return jobParseError(
         JobParseError::INVALID_VALUE,
         quoteStr(status.getValueOr("")) + " is not a valid job status",
         "job",
         in_json,
         ERROR_LOCATION);


   if (lastUpTime)
   {
      system::DateTime lastUpdateTime;
      error = system::DateTime::fromString(lastUpTime.getValueOr(""), lastUpdateTime);
      if (error)
         return updateError("lastUpdateTime", in_json, error);

      result.LastUpdateTime = lastUpdateTime;
   }

   if (submitTime)
   {
      error = system::DateTime::fromString(submitTime.getValueOr(""), result.SubmissionTime);
      if (error)
         return updateError("submissionTime", in_json, error);
   }

   out_job = result;
   return Success();
}

Error Job::stateFromString(const std::string& in_statusString, State& out_status)
{
   if (!jobStatusFromString(in_statusString, out_status))
      return Error(
         "StateParseError",
         1,
         quoteStr(in_statusString) + " is not a valid job status",
         ERROR_LOCATION);

   return Success();
}

std::string Job::stateToString(State in_status)
{
   return jobStatusToString(in_status);
}

Job& Job::operator=(const Job& in_other)
{
   if (this == &in_other)
      return *this;

   this->Arguments = in_other.Arguments;
   this->Cluster = in_other.Cluster;
   this->Command = in_other.Command;
   this->Config = in_other.Config;
   this->ContainerDetails = in_other.ContainerDetails;
   this->Environment = in_other.Environment;
   this->ExitCode = in_other.ExitCode;
   this->Exe = in_other.Exe;
   this->ExposedPorts = in_other.ExposedPorts;
   this->Host = in_other.Host;
   this->Id = in_other.Id;
   this->LastUpdateTime = in_other.LastUpdateTime;
   this->Mounts = in_other.Mounts;
   this->Name = in_other.Name;
   this->Pid = in_other.Pid;
   this->PlacementConstraints = in_other.PlacementConstraints;
   this->Queues = in_other.Queues;
   this->ResourceLimits = in_other.ResourceLimits;
   this->StandardIn = in_other.StandardIn;
   this->StandardErrFile = in_other.StandardErrFile;
   this->StandardOutFile = in_other.StandardOutFile;
   this->Status = in_other.Status;
   this->StatusMessage = in_other.StatusMessage;
   this->SubmissionTime = in_other.SubmissionTime;
   this->Tags = in_other.Tags;
   this->User = in_other.User;
   this->WorkingDirectory = in_other.WorkingDirectory;
   return *this;
}

Job& Job::operator=(Job&& in_other) noexcept
{
   if (this == &in_other)
      return *this;

   this->m_impl.reset();
   this->m_impl.swap(in_other.m_impl);

   this->Arguments = std::move(in_other.Arguments);
   this->Cluster = std::move(in_other.Cluster);
   this->Command = std::move(in_other.Command);
   this->Config = std::move(in_other.Config);
   this->ContainerDetails = std::move(in_other.ContainerDetails);
   this->Environment = std::move(in_other.Environment);
   this->ExitCode = std::move(in_other.ExitCode);
   this->Exe = std::move(in_other.Exe);
   this->ExposedPorts = std::move(in_other.ExposedPorts);
   this->Host = std::move(in_other.Host);
   this->Id = std::move(in_other.Id);
   this->LastUpdateTime = std::move(in_other.LastUpdateTime);
   this->Mounts = std::move(in_other.Mounts);
   this->Name = std::move(in_other.Name);
   this->Pid = std::move(in_other.Pid);
   this->PlacementConstraints = std::move(in_other.PlacementConstraints);
   this->Queues = std::move(in_other.Queues);
   this->ResourceLimits = std::move(in_other.ResourceLimits);
   this->StandardIn = std::move(in_other.StandardIn);
   this->StandardErrFile = std::move(in_other.StandardErrFile);
   this->StandardOutFile = std::move(in_other.StandardOutFile);
   this->Status = in_other.Status;
   this->StatusMessage = std::move(in_other.StatusMessage);
   this->SubmissionTime = std::move(in_other.SubmissionTime);
   this->Tags = std::move(in_other.Tags);
   this->User = std::move(in_other.User);
   this->WorkingDirectory = std::move(in_other.WorkingDirectory);
   return *this;
}

Optional<std::string> Job::getJobConfigValue(const std::string& in_name) const
{
   Optional<std::string> value;

   for (const JobConfig& conf: Config)
   {
      if (conf.Name == in_name)
      {
         value = conf.Value;
         break;
      }
   }

   return value;
}

bool Job::isCompleted() const
{
   return (Status == State::FINISHED) ||
      (Status == State::KILLED) ||
      (Status == State::CANCELED) ||
      (Status == State::FAILED);
}

bool Job::matchesTags(const std::set<std::string>& in_tags) const
{
   if (in_tags.size() > Tags.size())
      return false;

   for (const std::string& searchTag: in_tags)
      if (Tags.find(searchTag) == Tags.end())
         return false;

   return true;
}

json::Object Job::toJson() const
{
   json::Object jobObj;

   jobObj[JOB_ARGUMENTS] = json::toJsonArray(Arguments);

   if (!Cluster.empty())
      jobObj[JOB_CLUSTER] = Cluster;

   jobObj[JOB_COMMAND] = Command;
   jobObj[JOB_CONFIG] = toJsonArray(Config);

   if (ContainerDetails)
      jobObj[JOB_CONTAINER] = ContainerDetails.getValueOr(Container()).toJson();

   jobObj[JOB_ENVIRONMENT] = toJsonArray(Environment);
   jobObj[JOB_EXECUTABLE] = Exe;
   jobObj[JOB_EXPOSED_PORTS] = toJsonArray(ExposedPorts);

   if (ExitCode)
      jobObj[JOB_EXIT_CODE] = ExitCode.getValueOr(-1);

   jobObj[JOB_HOST] = Host;
   jobObj[JOB_ID] = Id;

   if (LastUpdateTime)
      jobObj[JOB_LAST_UPDATE_TIME] = LastUpdateTime.getValueOr(system::DateTime()).toString();

   jobObj[JOB_MOUNTS] = toJsonArray(Mounts);
   jobObj[JOB_NAME] = Name;

   if (Pid)
      jobObj[JOB_PID] = Pid.getValueOr(-1);

   jobObj[JOB_PLACEMENT_CONSTRAINTS] = toJsonArray(PlacementConstraints);
   jobObj[JOB_QUEUES] = json::toJsonArray(Queues);
   jobObj[JOB_RESOURCE_LIMITS] = toJsonArray(ResourceLimits);
   jobObj[JOB_STANDARD_IN] = StandardIn;
   jobObj[JOB_STANDARD_ERROR_FILE] = StandardErrFile;
   jobObj[JOB_STANDARD_OUTPUT_FILE] = StandardOutFile;
   jobObj[JOB_STATUS] = jobStatusToString(Status);

   if (!StatusMessage.empty())
      jobObj[JOB_STATUS_MESSAGE] = StatusMessage;

  jobObj[JOB_SUBMISSION_TIME] = SubmissionTime.toString();

   jobObj[JOB_TAGS] = json::toJsonArray(Tags);
   jobObj[JOB_USER] = User.getUsername();
   jobObj[JOB_WORKING_DIRECTORY] = WorkingDirectory;

   return jobObj;
}

// Job Config ==========================================================================================================
JobConfig::JobConfig(std::string in_name, Type in_type) :
   Name(std::move(in_name)),
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

// JobLock =============================================================================================================
struct JobLock::Impl
{
   explicit Impl(std::recursive_mutex& in_mutex) :
      Lock(in_mutex)
   {
   }

   std::lock_guard<std::recursive_mutex> Lock;
};

PRIVATE_IMPL_DELETER_IMPL(JobLock)

JobLock::JobLock(JobPtr in_job) :
   m_impl(new Impl(in_job->m_impl->Mutex))
{
}

// Mount Source ========================================================================================================
Error MountSource::fromJson(const json::Object& in_json, MountSource& out_mountSource)
{
   std::string type;
   json::Object source;
   Error error = readObject(in_json, MOUNT_TYPE, type, MOUNT_SOURCE, source);
   if (error)
      return updateError("mountSource", in_json, error);

   if (type == MOUNT_TYPE_AZURE)
   {
      AzureFileMountSource mountSource;
      error = AzureFileMountSource::fromJson(source, mountSource);
      if (error)
         return error;

      out_mountSource = mountSource;
   }
   else if (type == MOUNT_TYPE_CEPH)
   {
      CephFsMountSource mountSource;
      error = CephFsMountSource::fromJson(source, mountSource);
      if (error)
         return error;

      out_mountSource = mountSource;
   }
   else if (type == MOUNT_TYPE_GLUSTER)
   {
      GlusterFsMountSource mountSource;
      error = GlusterFsMountSource::fromJson(source, mountSource);
      if (error)
         return error;

      out_mountSource = mountSource;
   }
   else if (type == MOUNT_TYPE_HOST)
   {
      HostMountSource mountSource;
      error = HostMountSource::fromJson(source, mountSource);
      if (error)
         return error;

      out_mountSource = mountSource;
   }
   else if (type == MOUNT_TYPE_NFS)
   {
      NfsMountSource mountSource;
      error = NfsMountSource::fromJson(source, mountSource);
      if (error)
         return error;

      out_mountSource = mountSource;
   }
   else
   {
      if (type != MOUNT_TYPE_PASSTHROUGH)
         out_mountSource.CustomType = type;

      out_mountSource.SourceType = Type::PASSTHROUGH;
      out_mountSource.SourceObject = source;
   }

   return Success();
}

AzureFileMountSource& MountSource::asAzureFileMountSource()
{
   return const_cast<AzureFileMountSource&>(const_cast<const MountSource*>(this)->asAzureFileMountSource());
}

const AzureFileMountSource& MountSource::asAzureFileMountSource() const
{
   if (!isAzureFileMountSource())
   {
      throw std::logic_error(
         "Attempting to convert a mount of type " +
            mountTypeToString(SourceType, CustomType) +
            "Mount to " +
            MOUNT_TYPE_AZURE +
            "Mount");
   }

   return static_cast<const AzureFileMountSource&>(*this);
}

CephFsMountSource& MountSource::asCephFsMountSource()
{
   return const_cast<CephFsMountSource&>(const_cast<const MountSource*>(this)->asCephFsMountSource());
}

const CephFsMountSource& MountSource::asCephFsMountSource() const
{
   if (!isCephFsMountSource())
   {
      throw std::logic_error(
         "Attempting to convert a mount of type " +
            mountTypeToString(SourceType, CustomType) +
            "Mount to " +
            MOUNT_TYPE_CEPH +
            "Mount");
   }

   return static_cast<const CephFsMountSource&>(*this);
}

GlusterFsMountSource& MountSource::asGlusterFsMountSource()
{
   return const_cast<GlusterFsMountSource&>(const_cast<const MountSource*>(this)->asGlusterFsMountSource());
}

const GlusterFsMountSource& MountSource::asGlusterFsMountSource() const
{
   if (!isGlusterFsMountSource())
   {
      throw std::logic_error(
         "Attempting to convert a mount of type " +
            mountTypeToString(SourceType, CustomType) +
            "Mount to " +
            MOUNT_TYPE_GLUSTER +
            "Mount");
   }

   return static_cast<const GlusterFsMountSource&>(*this);
}

HostMountSource& MountSource::asHostMountSource()
{
   return const_cast<HostMountSource&>(const_cast<const MountSource*>(this)->asHostMountSource());
}

const HostMountSource& MountSource::asHostMountSource() const
{
   if (!isHostMountSource())
   {
      throw std::logic_error(
         "Attempting to convert a mount of type " +
            mountTypeToString(SourceType, CustomType) +
            "Mount to " +
            MOUNT_TYPE_HOST +
            "Mount");
   }

   return static_cast<const HostMountSource&>(*this);
}

NfsMountSource& MountSource::asNfsMountSource()
{
   return const_cast<NfsMountSource&>(const_cast<const MountSource*>(this)->asNfsMountSource());
}

const NfsMountSource& MountSource::asNfsMountSource() const
{
   if (!isNfsMountSource())
   {
      throw std::logic_error(
         "Attempting to convert a mount of type " +
            mountTypeToString(SourceType, CustomType) +
            "Mount to " +
            MOUNT_TYPE_NFS +
            "Mount");
   }

   return static_cast<const NfsMountSource&>(*this);
}

bool MountSource::isAzureFileMountSource() const
{
   return SourceType == Type::AZURE_FILE;
}

bool MountSource::isCephFsMountSource() const
{
   return SourceType == Type::CEPH_FS;
}

bool MountSource::isGlusterFsMountSource() const
{
   return SourceType == Type::GLUSTER_FS;
}

bool MountSource::isHostMountSource() const
{
   return SourceType == Type::HOST;
}

bool MountSource::isNfsMountSource() const
{
   return SourceType == Type::NFS;
}

bool MountSource::isPassthroughMountSource() const
{
   return SourceType == Type::PASSTHROUGH;
}

json::Object MountSource::toJson() const
{
   return SourceObject;
}

// Azure File Mount Source =============================================================================================
AzureFileMountSource::AzureFileMountSource()
{
   SourceType = MountSource::Type::AZURE_FILE;
}

Error AzureFileMountSource::fromJson(const json::Object& in_json, AzureFileMountSource& out_mountSource)
{
   std::string secretName, shareName;
   Error error = json::readObject(in_json, MOUNT_SOURCE_SECRET_NAME, secretName, MOUNT_SOURCE_SHARE_NAME, shareName);
   if (error)
      return updateError(std::string(MOUNT_TYPE_AZURE) + "Mount", in_json, error);

   out_mountSource.SourceObject = in_json.clone().getObject();
   return Success();
}

std::string AzureFileMountSource::getSecretName() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_SECRET_NAME));
   return (*SourceObject.find(MOUNT_SOURCE_SECRET_NAME)).getValue().getString();
}

std::string AzureFileMountSource::getShareName() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_SHARE_NAME));
   return (*SourceObject.find(MOUNT_SOURCE_SHARE_NAME)).getValue().getString();
}

// Ceph FS Mount Source =============================================================================================
CephFsMountSource::CephFsMountSource()
{
   SourceType = MountSource::Type::CEPH_FS;
}

Error CephFsMountSource::fromJson(const json::Object& in_json, CephFsMountSource& out_mountSource)
{
   std::vector<std::string> monitors;
   Optional<std::string> path, user, secretFile, secretRef;
   Error error = json::readObject(in_json,
      MOUNT_SOURCE_MONITORS, monitors,
      MOUNT_SOURCE_PATH, path,
      MOUNT_SOURCE_USER, user,
      MOUNT_SOURCE_SECRET_FILE, secretFile,
      MOUNT_SOURCE_SECRET_REF, secretRef);
   if (error)
      return updateError(std::string(MOUNT_TYPE_CEPH) + "Mount", in_json, error);

   out_mountSource.SourceObject = in_json.clone().getObject();
   return Success();
}

std::vector<std::string> CephFsMountSource::getMonitors() const
{
   // We should have returned an error during `fromJson` if this is not true.
   std::vector<std::string> monitors;
   Error error = json::readObject(SourceObject, MOUNT_SOURCE_MONITORS, monitors);
   assert(!error);

   return std::move(monitors);
}

std::string CephFsMountSource::getPath() const
{
   if (SourceObject.hasMember(MOUNT_SOURCE_PATH))
      return (*SourceObject.find(MOUNT_SOURCE_PATH)).getValue().getString();

   return "";
}

std::string CephFsMountSource::getUser() const
{
   if (SourceObject.hasMember(MOUNT_SOURCE_USER))
      return (*SourceObject.find(MOUNT_SOURCE_USER)).getValue().getString();

   return "";
}

std::string CephFsMountSource::getSecretFile() const
{
   if (SourceObject.hasMember(MOUNT_SOURCE_SECRET_FILE))
      return (*SourceObject.find(MOUNT_SOURCE_SECRET_FILE)).getValue().getString();

   return "";
}

std::string CephFsMountSource::getSecretRef() const
{
   if (SourceObject.hasMember(MOUNT_SOURCE_SECRET_REF))
      return (*SourceObject.find(MOUNT_SOURCE_SECRET_REF)).getValue().getString();

   return "";
}

// Gluster FS Mount Source =============================================================================================
GlusterFsMountSource::GlusterFsMountSource()
{
   SourceType = MountSource::Type::GLUSTER_FS;
}

Error GlusterFsMountSource::fromJson(const json::Object& in_json, GlusterFsMountSource& out_mountSource)
{
   std::string endpoints, path;
   Error error = json::readObject(in_json, MOUNT_SOURCE_ENDPOINTS, endpoints, MOUNT_SOURCE_PATH, path);
   if (error)
      return updateError(std::string(MOUNT_TYPE_GLUSTER) + "Mount", in_json, error);

   out_mountSource.SourceObject = in_json.clone().getObject();
   return Success();
}

std::string GlusterFsMountSource::getEndpoints() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_ENDPOINTS));
   return (*SourceObject.find(MOUNT_SOURCE_ENDPOINTS)).getValue().getString();
}

std::string GlusterFsMountSource::getPath() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_PATH));
   return (*SourceObject.find(MOUNT_SOURCE_PATH)).getValue().getString();
}

// Host Mount Source ===================================================================================================
HostMountSource::HostMountSource()
{
   SourceType = MountSource::Type::HOST;
}

Error HostMountSource::fromJson(const json::Object& in_json, HostMountSource& out_mountSource)
{
   std::string path;
   Error error = json::readObject(in_json, MOUNT_SOURCE_PATH, path);
   if (error)
      return updateError(std::string(MOUNT_TYPE_HOST) + "Mount", in_json, error);

   out_mountSource.SourceObject = in_json.clone().getObject();
   return Success();
}

std::string HostMountSource::getPath() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_PATH));
   return (*SourceObject.find(MOUNT_SOURCE_PATH)).getValue().getString();
}

// NFS Mount Source ====================================================================================================
NfsMountSource::NfsMountSource()
{
   SourceType = MountSource::Type::NFS;
}

Error NfsMountSource::fromJson(const json::Object& in_json, NfsMountSource& out_mountSource)
{
   std::string path, host;
   Error error = json::readObject(in_json,
      MOUNT_SOURCE_PATH, path,
      MOUNT_SOURCE_HOST, host);
   if (error)
      return updateError("nfsMount", in_json, error);

   return Success();
}

std::string NfsMountSource::getHost() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_HOST));
   return (*SourceObject.find(MOUNT_SOURCE_HOST)).getValue().getString();
}

std::string NfsMountSource::getPath() const
{
   // We should have returned an error during `fromJson` if this is not true.
   assert(SourceObject.hasMember(MOUNT_SOURCE_PATH));
   return (*SourceObject.find(MOUNT_SOURCE_PATH)).getValue().getString();
}

// Mount ===============================================================================================================
Error Mount::fromJson(const json::Object& in_json, Mount& out_mount)
{
   Optional<bool> isReadOnly;
   Error error = json::readObject(in_json,
      MOUNT_PATH, out_mount.Destination,
      MOUNT_READ_ONLY, isReadOnly);

   if (error)
      return updateError(JOB_MOUNTS, in_json, error);

   error = MountSource::fromJson(in_json, out_mount.Source);
   if (error)
      return error;

   out_mount.IsReadOnly = isReadOnly.getValueOr(false);

   return Success();
}

json::Object Mount::toJson() const
{
   json::Object mountObj;
   mountObj[MOUNT_PATH] = Destination;
   mountObj[MOUNT_READ_ONLY] = IsReadOnly;
   mountObj[MOUNT_TYPE] = mountTypeToString(Source.SourceType, Source.CustomType);
   mountObj[MOUNT_SOURCE] = Source.SourceObject;

   return mountObj;
}

// Placement Constraint ================================================================================================
PlacementConstraint::PlacementConstraint(std::string in_name) :
   Name(std::move(in_name))
{
}

PlacementConstraint::PlacementConstraint(std::string in_name, std::string in_value) :
   Name(std::move(in_name)),
   Value(std::move(in_value))
{
}

Error PlacementConstraint::fromJson(const json::Object& in_json, PlacementConstraint& out_placementConstraint)
{
   Error error = json::readObject(in_json,
      PLACEMENT_CONSTRAINT_NAME, out_placementConstraint.Name,
      PLACEMENT_CONSTRAINT_VALUE, out_placementConstraint.Value);

   if (error)
      return updateError(JOB_PLACEMENT_CONSTRAINTS, in_json, error);

   return Success();
}

json::Object PlacementConstraint::toJson() const
{
   json::Object constraintObj;
   constraintObj[PLACEMENT_CONSTRAINT_NAME] = Name;
   if (!Value.empty())
      constraintObj[PLACEMENT_CONSTRAINT_VALUE] = Value;

   return constraintObj;
}

// Resource Limit ======================================================================================================
const char* const ResourceLimit::Type::CPU_COUNT   = "cpuCount";
const char* const ResourceLimit::Type::CPU_TIME    = "cpuTime";
const char* const ResourceLimit::Type::MEMORY      = "memory";
const char* const ResourceLimit::Type::MEMORY_SWAP = "memorySwap";

ResourceLimit::ResourceLimit(std::string in_limitType,  std::string in_maxValue, std::string in_defaultValue) :
   ResourceType(std::move(in_limitType)),
   MaxValue(std::move(in_maxValue)),
   DefaultValue(std::move(in_defaultValue))
{
}

Error ResourceLimit::fromJson(const json::Object& in_json, ResourceLimit& out_resourceLimit)
{
   Error error = json::readObject(in_json,
      RESOURCE_LIMIT_TYPE, out_resourceLimit.ResourceType,
      RESOURCE_LIMIT_VALUE, out_resourceLimit.Value);

   if (error)
      return updateError(JOB_RESOURCE_LIMITS, in_json, error);

   return Success();
}

json::Object ResourceLimit::toJson() const
{
   json::Object limitObj;

   limitObj[RESOURCE_LIMIT_TYPE] = ResourceType;

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
