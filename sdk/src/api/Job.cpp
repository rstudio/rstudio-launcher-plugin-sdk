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
constexpr char const* MOUNT_SOURCE_HOST               = "hostMount";
constexpr char const* MOUNT_SOURCE_NFS                = "nfsMount";
constexpr char const* MOUNT_SOURCE_PATH               = "path";
constexpr char const* MOUNT_SOURCE_NFS_HOST           = "host";

// Placement Constraint
constexpr char const* PLACEMENT_CONSTRAINT_NAME       = "name";
constexpr char const* PLACEMENT_CONSTRAINT_VALUE      = "value";

// Resource Limit
constexpr char const* RESOURCE_LIMIT_DEFAULT          = "defaultValue";
constexpr char const* RESOURCE_LIMIT_MAX              = "maxValue";
constexpr char const* RESOURCE_LIMIT_TYPE             = "type";
constexpr char const* RESOURCE_LIMIT_TYPE_CPU_COUNT   = "cpuCount";
constexpr char const* RESOURCE_LIMIT_TYPE_CPU_TIME    = "cpuTime";
constexpr char const* RESOURCE_LIMIT_TYPE_MEMORY      = "memory";
constexpr char const* RESOURCE_LIMIT_TYPE_MEMORY_SWAP = "memorySwap";
constexpr char const* RESOURCE_LIMIT_VALUE            = "value";

Error jobStatusFromString(const std::string& io_statusStr, Job::State& out_state)
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
      return Error("JobParseError", 1, "Unexpected job status string: " + io_statusStr, ERROR_LOCATION);
   else
      out_state = Job::State::UNKNOWN;

   return Success();
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

template <typename T>
Error fromJsonArray(const json::Array& in_jsonArray, std::vector<T>& out_array)
{
   for (const json::Value& jsonVal: in_jsonArray)
   {
      if (!jsonVal.isObject())
      {
         Error error("JobParseError", 1, "Invalid array value.", ERROR_LOCATION);
         error.addProperty("value", jsonVal.write());
         error.addProperty("array", in_jsonArray.write());
         return error;
      }

      T val;
      Error error = T::fromJson(jsonVal.getObject(), val);
      if (error)
         return error;

      out_array.push_back(val);
   }

   return Success();
}

template <>
Error fromJsonArray(const json::Array& in_jsonArray, EnvironmentList& out_array)
{
   for (const json::Value& jsonVal: in_jsonArray)
   {
      if (!jsonVal.isObject())
      {
         Error error("JobParseError", 1, "Invalid array value.", ERROR_LOCATION);
         error.addProperty("value", jsonVal.write());
         error.addProperty("array", in_jsonArray.write());
         return error;
      }

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
   {
     return updateError(
        JOB_CONTAINER,
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

// Job =================================================================================================================
struct Job::Impl
{
   std::mutex Mutex;
};

PRIVATE_IMPL_DELETER_IMPL(Job)

Job::Job() :
   m_impl(new Impl())
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
   Optional<std::vector<std::string> > arguments;
   Optional<std::set<std::string> > queues, tags;
   Optional<std::string> cluster, command, exe, host, id, lastUpTime, stdIn, stdErr, stdOut, status, statusMessage,
                         submitTime, user, workingDir;
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

   if (command && exe)
   {
      error = Error("JobParseError", 2, R"(Job has conflicting fields "command" and "exe" set.)", ERROR_LOCATION);
      error.addProperty("job", in_json.write());
      return error;
   }

   if (!command && !exe && !containerObj)
   {
      error = Error("JobParseError", 2, R"(Job must have one of fields "command", "exe", and "container" set.)", ERROR_LOCATION);
      error.addProperty("job", in_json.write());
      return error;
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
   result.User = user.getValueOr("");
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

   error = fromJsonArray(config.getValueOr(json::Array()), result.Config);
   if (error)
      return error;

   error = fromJsonArray(env.getValueOr(json::Array()), result.Environment);
   if (error)
      return error;

   error = fromJsonArray(ports.getValueOr(json::Array()), result.ExposedPorts);
   if (error)
      return error;

   error = fromJsonArray(mounts.getValueOr(json::Array()), result.Mounts);
   if (error)
      return error;

   error = fromJsonArray(constraints.getValueOr(json::Array()), result.PlacementConstraints);
   if (error)
      return error;

   error = fromJsonArray(limits.getValueOr(json::Array()), result.ResourceLimits);
   if (error)
      return error;

   error = jobStatusFromString(status.getValueOr(""), result.Status);
   if (error)
      return error;

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
      system::DateTime submissionTime;
      error = system::DateTime::fromString(submitTime.getValueOr(""), submissionTime);
      if (error)
         return updateError("submissionTime", in_json, error);

      result.SubmissionTime = submissionTime;
   }

   out_job = result;
   return Success();
}

Job& Job::operator=(const Job& in_other)
{
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

   if (SubmissionTime)
      jobObj[JOB_SUBMISSION_TIME] = SubmissionTime.getValueOr(system::DateTime()).toString();

   jobObj[JOB_TAGS] = json::toJsonArray(Tags);
   jobObj[JOB_USER] = User;
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
   explicit Impl(std::mutex& in_mutex) :
      Lock(in_mutex)
   {
   }

   std::lock_guard<std::mutex> Lock;
};

PRIVATE_IMPL_DELETER_IMPL(JobLock)

JobLock::JobLock(JobPtr in_job) :
   m_impl(new Impl(in_job->m_impl->Mutex))
{
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
      return updateError(JOB_MOUNTS, in_json, error);

   if (!hostMountSource && !nfsMountSource)
   {
      // TODO: real error set up.
      error = Error("JobParseError", 1, "No mount source specified", ERROR_LOCATION);
      return updateError(JOB_MOUNTS, in_json, error);
   }
   else if (hostMountSource && nfsMountSource)
   {
      error = Error("JobParseError", 1, "Multiple mount sources specified", ERROR_LOCATION);
      return updateError(JOB_MOUNTS, in_json, error);
   }
   else if (hostMountSource)
   {
      HostMountSource mountSource;
      error = HostMountSource::fromJson(hostMountSource.getValueOr(json::Object()), mountSource);
      if (error)
         return updateError(JOB_MOUNTS, in_json, error);

      out_mount.HostSourcePath = mountSource;
   }
   else
   {
      NfsMountSource mountSource;
      error = NfsMountSource::fromJson(nfsMountSource.getValueOr(json::Object()), mountSource);
      if (error)
         return updateError(JOB_MOUNTS, in_json, error);

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

// Placement Constraint ================================================================================================
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
   constraintObj[PLACEMENT_CONSTRAINT_VALUE] = Value;

   return constraintObj;
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
      return updateError(JOB_RESOURCE_LIMITS, in_json, error);

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
         JOB_RESOURCE_LIMITS,
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
