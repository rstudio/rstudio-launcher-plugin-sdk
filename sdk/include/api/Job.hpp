/*
 * Job.hpp
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

#ifndef LAUNCHER_PLUGINS_JOB_HPP
#define LAUNCHER_PLUGINS_JOB_HPP

#include <ctime>
#include <set>
#include <string>
#include <vector>

#include <Optional.hpp>
#include <system/DateTime.hpp>
#include <system/User.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace json {

class Object;

} // namespace json
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/** Convenience typedef for an Environment Variable. */
typedef std::pair<std::string, std::string> EnvVariable;

// Forward Declarations
struct JobConfig;
struct Mount;
struct NfsMountSource;
struct ResourceLimit;
struct PlacementConstraint;

/** @brief Struct which represents the container to use when launching a containerized job. */
struct Container
{
   /**
    * @brief Constructs a Container from a JSON object which represents the container.
    *
    * @param in_json            The JSON object which represents the container.
    * @param out_container      The populated container value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a Container; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, Container& out_container);

   /**
    * @brief Converts this Container to a JSON object which represents it.
    *
    * @return The JSON object which represents this Container.
    */
   json::Object toJson() const;

   /** The name of the image to use. */
   std::string Image;

   /** The optional user ID to run the container as. */
   Optional<int> RunAsUserId;

   /** The optional group ID to run the container as. */
   Optional<int> RunAsGroupId;

   /** The optional set of supplemental group IDs for the run-as user, to pass to the container on launch. */
   std::vector<int> SupplementalGroupIds;
};

/** @brief Struct which represents an exposed port on a containerized job. */
struct ExposedPort
{
   /**
    * @brief Constructs an ExposedPort from a JSON object which represents the exposed port.
    *
    * @param in_json            The JSON object which represents the exposed port.
    * @param out_exposedPort    The populated exposed port value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as an ExposedPort; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, ExposedPort& out_exposedPort);

   /**
    * @brief Converts this ExposedPort to a JSON object which represents it.
    *
    * @return The JSON object which represents this ExposedPort.
    */
   json::Object toJson() const;

   /** The published port. */
   Optional<int> PublishedPort;

   /** The protocol of the port. */
   std::string Protocol;

   /** The target port. */
   int TargetPort;
};

/** @brief Struct which represents the source path of an local host system Mount. */
struct HostMountSource
{
   /**
    * @brief Constructs a HostMountSource from a JSON object which represents the host mount source.
    *
    * @param in_json            The JSON object which represents the host mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a HostMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, HostMountSource& out_mountSource);

   /**
    * @brief Converts this HostMountSource to a JSON object which represents it.
    *
    * @return The JSON object which represents this HostMountSource.
    */
   json::Object toJson() const;

   /** The source path for the mount. */
   std::string Path;
};

/** @brief Structure which represents a job. */
struct Job
{
   /**
    * @enum Job::State
    *
    * @brief An enum which describes the possible states a job may have.
    */
   enum class State
   {
      /** The job was canceled by the user. */
      CANCELED,

      /** The job failed to launch. */
      FAILED,

      /** The job finished running, successfully or not. */
      FINISHED,

      /** The job was killed. */
      KILLED,

      /** The job is queued in the job scheduling system and has not started yet. */
      PENDING,

      /** The job is currently running. */
      RUNNING,

      /** The job has been suspended. */
      SUSPENDED,

      /** The job status is unknown. */
      UNKNOWN
   };

   /**
    * @brief Constructs a Job from a JSON object which represents the job.
    *
    * @param in_json        The JSON object which represents the job.
    * @param out_job        The populated job value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a Job; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, Job& out_job);

   /**
    * @brief Gets a job configuration value, if it exists.
    *
    * @param in_name    The name of the configuration option to retrieve.
    *
    * @return The value of the configuration option, if any.
    */
   Optional<std::string> getJobConfigValue(const std::string& in_name) const;

   /**
    * @brief Checks whether the job has all of the supplied tags.
    *
    * @param in_tags    The desired set of tags to filter jobs by.
    *
    * @return True if this job has all of the supplied tags; false otherwise.
    */
   bool matchesTags(const std::set<std::string>& in_tags) const;

   /**
    * @brief Converts this Job to a JSON object which represents it.
    *
    * @return The JSON object which represents this Job.
    */
   json::Object toJson() const;

   /** The arguments to supply to the Command or Exe. */
   std::vector<std::string> Arguments;

   /** The name of the cluster which should run this job. */
   std::string Cluster;

   /**
    * @brief The shell command to run.
    *
    * This should be run using a shell such as /bin/sh, as opposed to Exe, which should be invoked directly.
    *
    * Only one of Command and Exe may be set per job. Jobs which have both set should have been rejected by the
    * Launcher.
    */
   std::string Command;

   /** The custom job scheduling specific configuration options that were set by the user for this job. */
   std::vector<JobConfig> Config;

   /** The container to run the job in. Only used for containerized jobs. */
   Optional<Container> ContainerDetails;

   /** Environment variables to set on the job's run environment. */
   std::vector<EnvVariable> Environment;

   /**
    * @brief The executable to run.
    *
    * This should be invoked directly. It is the user's responsibility to ensure that the provided executable is either
    * fully qualified or on the PATH within the job environment.
    *
    * Only one of Command and Exe may be set per job. Jobs which have both set should have been rejected by the
    * Launcher.
    */
   std::string Exe;

   /** The exit code of the job, if applicable. */
   Optional<int> ExitCode;

   /** The ports which were exposed for this job. Only used with containerized jobs. */
   std::vector<ExposedPort> ExposedPorts;

   /** The host on which the job was or is being run. */
   std::string Host;

   /** The unique ID of the job in the scheduling system. */
   std::string Id;

   /** The last time the job was updated. */
   Optional<system::DateTime> LastUpdateTime;

   /** The file system mounts to set when launching this job. */
   std::vector<Mount> Mounts;

   /** The name of the job. */
   std::string Name;

   /** The PID of the job, if applicable. */
   Optional<pid_t> Pid;

   /** Custom placement constraints for the job scheduling system that were set by the user for this job. */
   std::vector<PlacementConstraint> PlacementConstraints;

   /** The set of queues on which this job may be run, or the queue which ran the job. */
   std::vector<std::string> Queues;

   /** The resource limits that were set by the user for this job. */
   std::vector<ResourceLimit> ResourceLimits;

   /** Data which should be supplied to the job via standard in. */
   std::string StandardIn;

   /** The file to which the job's standard error output was written. */
   std::string StandardErrFile;

   /** The file to which the job's standard output was written. */
   std::string StandardOutFile;

   /** The status of the job. */
   State Status;

   /** The reason for the status, if any. */
   std::string StatusMessage;

   /** The time at which the job was submitted to the job scheduling system. */
   Optional<system::DateTime> SubmissionTime;

   /** The tags which were set on the job by the user. Can be used for filtering jobs based on tags. */
   std::set<std::string> Tags;

   /** The user who ran the job. */
   std::string User;

   /** The working directory from which to run the job. */
   std::string WorkingDirectory;
};

/**
 * @brief Struct which represents a custom configuration setting for jobs launched with a given Plugin.
 *
 * JobConfig values should be used only when there is a necessary per-job configuration that cannot be covered by
 * another aspect of the Job structure, such as a ResourceLimit or PlacementConstraint.
 */
struct JobConfig
{
   /**
    * @enum JobConfig::Type
    *
    * @brief Enum which represents the Type of a JobConfig value.
    */
   enum class Type
   {
      /** Enumeration type. */
         ENUM,

      /** Floating point value type. */
         FLOAT,

      /** Integer type. */
         INT,

      /** String type. */
         STRING
   };

   /**
    * @brief Default constructor.
    */
   JobConfig() = default;

   /**
    * @brief Constructor.
    *
    * @param in_name    The name of the custom job configuration value.
    * @param in_type    The type of the custom job configuration value.
    */
   JobConfig(const std::string& in_name, Type in_type);

   /**
    * @brief Constructs a JobConfig from a JSON object which represents the job config.
    *
    * @param in_json            The JSON object which represents the job config.
    * @param out_jobConfig      The populated job config value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a JobConfig; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, JobConfig& out_jobConfig);

   /**
    * @brief Converts this JobConfig to a JSON object which represents it.
    *
    * @return The JSON object which represents this JobConfig.
    */
   json::Object toJson() const;

   /** The name of the custom job configuration value. */
   std::string Name;

   /** The type of the custom job configuration value. */
   Optional<Type> ValueType;

   /** The value of the custom job configuration value. */
   std::string Value;
};

/** @brief Struct which represents an file system mount available to a job. */
struct Mount
{
   /**
    * @brief Constructs a Mount from a JSON object which represents the mount.
    *
    * @param in_json        The JSON object which represents the mount.
    * @param out_mount      The populated mount value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a Mount; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, Mount& out_mount);

   /**
    * @brief Converts this Mount to a JSON object which represents it.
    *
    * @return The JSON object which represents this Mount.
    */
   json::Object toJson() const;

   /** The path to which to mount the source path. */
   std::string DestinationPath;

   /** Whether the mounted path is read only. */
   bool IsReadOnly;

   /** The source path to mount. Only one of this and NfsSourcePath may be set per Mount object. */
   Optional<HostMountSource> HostSourcePath;

   /** The source path to mount. Only one of this and HostSourcePath may be set per Mount object. */
   Optional<NfsMountSource> NfsSourcePath;
};

/** @brief Struct which represents the source path of an NFS Mount. */
struct NfsMountSource
{
   /**
    * @brief Constructs an NfsMountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the NFS mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as an NfsMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, NfsMountSource& out_mountSource);

   /**
    * @brief Converts this NfsMountSource to a JSON object which represents it.
    *
    * @return The JSON object which represents this NfsMountSource.
    */
   json::Object toJson() const;

   /** The host of the source path. */
   std::string Host;

   /** The source path for the mount. */
   std::string Path;
};

/**
 * @brief Struct which represents a custom placement constraint for the job.
 *
 * This may be used to allow users to request other resource limits than those supported by ResourceLimit, or it may be
 * used for any other constraint that can affect where a job is run.
 *
 * There should be a PlacementConstraint for every value of a given constraint type. For example, if the constraint is
 * the AWS region and the allowed AWS regions are us-east-1, us-west-1, and us-west-2, there should be the following
 * PlacementConstraints in the ClusterInfo response:
 * { "name": "region", "value": "us-east-1" }
 * { "name": "region", "value": "us-west-1" }
 * { "name": "region", "value": "us-west-2" }
 *
 * For more details, see ClusterInfoResponse or PlacementConstraint in the RStudio Job Launcher Documentation:
 * https://docs.rstudio.com/job-launcher/latest/creating-plugins.html#plugin-messages.
 */
struct PlacementConstraint
{
   /**
    * @brief Default constructor.
    */
   PlacementConstraint() = default;

   /**
    * @brief Constructor.
    *
    * @param in_name        The name of the placement constraint.
    * @param in_value       One of the possible values for the placement constraint with the specified name.
    */
   PlacementConstraint(std::string in_name, std::string in_value);

   /**
    * @brief Constructs a PlacementConstraint from a JSON object which represents the placement constraint.
    *
    * @param in_json                    The JSON object which represents the placement constraint.
    * @param out_placementConstraint    The populated placement constraint value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a PlacementConstraint; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, PlacementConstraint& out_placementConstraint);

   /**
    * @brief Converts this PlacementConstraint to a JSON object which represents it.
    *
    * @return The JSON object which represents this PlacementConstraint.
    */
   json::Object toJson() const;

   /** The name of this placement constraint. */
   std::string Name;

   /** The value of this placement constraint. */
   std::string Value;
};

/** @brief Struct which represents a resource limit for a job. */
struct ResourceLimit
{
   /**
    * @enum ResourceLimit::Type
    *
    * @brief The type of resource limit.
    */
   enum class Type
   {
      /** The required number of CPUs for a job. */
      CPU_COUNT,

      /** The required amount of CPU time for a job, in seconds. */
      CPU_TIME,

      /** The required amount of memory for a job, in MB. */
      MEMORY,

      /** The required amount of swap space for a job, in MB. */
      MEMORY_SWAP
   };

   /**
    * @brief Default constructor.
    */
   ResourceLimit() = default;

   /**
    * @brief Constructor.
    *
    * @param in_limitType       The type of the resource limit.
    * @param in_maxValue        The maximum value of the resource limit. Default: no maximum.
    * @param in_defaultValue    The default value of the resource limit. Default: no default.
    */
   explicit ResourceLimit(Type in_limitType, std::string in_maxValue = "", std::string in_defaultValue = "");

   /**
    * @brief Constructs a ResourceLimit from a JSON object which represents the resource limit.
    *
    * @param in_json                The JSON object which represents the resource limit.
    * @param out_resourceLimit      The populated resource limit value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a ResourceLimit; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, ResourceLimit& out_resourceLimit);

   /**
    * @brief Converts this ResourceLimit to a JSON object which represents it.
    *
    * @return The JSON object which represents this ResourceLimit.
    */
   json::Object toJson() const;

   /** The type of resource to limit. */
   Type ResourceType;

   /** The value of the resource limit. */
   std::string Value;

   /** The maximum value that can be set for this type of resource. */
   std::string MaxValue;

   /** The default value that will be set for this type of resource. */
   std::string DefaultValue;
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
