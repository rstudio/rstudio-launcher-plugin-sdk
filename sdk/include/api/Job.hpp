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

#include <Noncopyable.hpp>
#include <Optional.hpp>
#include <PImpl.hpp>
#include <json/Json.hpp>
#include <system/DateTime.hpp>
#include <system/User.hpp>
#include <utils/MutexUtils.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {


// Forward Declarations
struct AzureFileMountSource;
struct CephFsMountSource;
struct Container;
struct ExposedPort;
struct GlusterFsMountSource;
struct HostMountSource;
struct Job;
struct JobConfig;
class JobLock;
struct Mount;
struct MountSource;
struct NfsMountSource;
struct ResourceLimit;
struct PlacementConstraint;

// Convenience Typedefs
typedef std::shared_ptr<Job> JobPtr;

typedef std::pair<std::string, std::string> EnvVariable;
typedef std::vector<EnvVariable> EnvironmentList;
typedef std::vector<ExposedPort> ExposedPortList;
typedef std::vector<JobConfig> JobConfigList;
typedef std::vector<JobPtr> JobList;
typedef std::vector<Mount> MountList;
typedef std::vector<PlacementConstraint> PlacementConstraintList;
typedef std::vector<ResourceLimit> ResourceLimitList;

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
    * @brief Constructor.
    */
   Job();

   /**
    * @brief Copy constructor.
    *
    * @param in_other       The job to copy.
    */
   Job(const Job& in_other);

   /**
    * @brief Move constructor.
    *
    * @param in_other       The job to move into this job.
    */
   Job(Job&& in_other) noexcept;

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
    * @brief Converts a status string into its equivalent Job::State enum value.
    *
    * @param in_statusString    The string to convert.
    * @param out_status         The converted status, if no error occurred.
    *
    * @return Success if in_statusString is a valid job state; Error otherwise.
    */
   static Error stateFromString(const std::string& in_statusString, State& out_status);

   /**
    * @brief Converts a Job::State enum value into its string representation.
    *
    * @param in_status   The Job::State value to be converted to string.
    *
    * @return The string representation of the specified Job::State.
    */
   static std::string stateToString(State in_status);

   /**
    * @brief Assignment operator.
    *
    * @param in_other       The Job to copy into this Job.
    *
    * @return A reference to this Job.
    */
   Job& operator=(const Job& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other       The Job to move into this Job.
    *
    * @return A reference to this Job.
    */
   Job& operator=(Job&& in_other) noexcept;

   /**
    * @brief Gets a job configuration value, if it exists.
    *
    * @param in_name    The name of the configuration option to retrieve.
    *
    * @return The value of the configuration option, if any.
    */
   Optional<std::string> getJobConfigValue(const std::string& in_name) const;

   /**
    * @brief Checks whether the job has completed (i.e. the job's state is a completed state).
    *
    * @return True if the job has completed; false otherwise.
    */
   bool isCompleted() const;

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
   JobConfigList Config;

   /** The container to run the job in. Only used for containerized jobs. */
   Optional<Container> ContainerDetails;

   /** Environment variables to set on the job's run environment. */
   EnvironmentList Environment;

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
   ExposedPortList ExposedPorts;

   /** The host on which the job was or is being run. */
   std::string Host;

   /** The unique ID of the job in the scheduling system. */
   std::string Id;

   /** The last time the job was updated. */
   Optional<system::DateTime> LastUpdateTime;

   /** The file system mounts to set when launching this job. */
   MountList Mounts;

   /** The name of the job. */
   std::string Name;

   /** The PID of the job, if applicable. */
   Optional<pid_t> Pid;

   /** Custom placement constraints for the job scheduling system that were set by the user for this job. */
   PlacementConstraintList PlacementConstraints;

   /** The set of queues on which this job may be run, or the queue which ran the job. */
   std::set<std::string> Queues;

   /** The resource limits that were set by the user for this job. */
   ResourceLimitList ResourceLimits;

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
   system::DateTime SubmissionTime;

   /** The tags which were set on the job by the user. Can be used for filtering jobs based on tags. */
   std::set<std::string> Tags;

   /** The user who ran the job. */
   system::User User;

   /** The working directory from which to run the job. */
   std::string WorkingDirectory;

private:
   friend class JobLock;

   // The  private implementation of a Job object.
   PRIVATE_IMPL(m_impl);
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
   JobConfig(std::string in_name, Type in_type);

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

/** @brief RAII class for locking access to a Job object. Should be used every time a Job is modified. */
class JobLock : Noncopyable
{
public:
   /**
    * @brief Constructor.
    *
    * May throw a std::system_error.
    *
    * @param in_job     The job to lock.
    */
   explicit JobLock(JobPtr in_job);

private:
   // The private implementation of JobLock.
   PRIVATE_IMPL(m_impl);
};

/** @brief Struct which represents the source path of an NFS Mount. */
struct MountSource
{
   /** @brief Constants representing the support types of MountSource. */
   enum class Type
   {
      /** Represents an Azure File Mount Source. */
      AZURE_FILE,

      /** Represents a Ceph File System Mount Source. */
      CEPH_FS,

      /** Represents a Gluster File System Mount Source. */
      GLUSTER_FS,

      /** Represents a Host Mount Source. */
      HOST,

      /** Represents an NFS Mount Source. */
      NFS,

      /** Represents a Mount Source that will be passed through to the Plugin to handle. */
      PASSTHROUGH
   };
   
   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~MountSource() = default;

   /**
    * @brief Constructs a MountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a MountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, MountSource& out_mountSource);

   /**
    * @brief Gets this MountSource as an AzureFileMountSource. 
    * 
    * @throw std::logic_error if isAzureFileMountSource() would return false.
    * 
    * @return This MountSource as an AzureFileMountSource.
    */
   AzureFileMountSource& asAzureFileMountSource();

   /**
    * @brief Gets this MountSource as an AzureFileMountSource. 
    * 
    * @throw std::logic_error if isAzureFileMountSource() would return false.
    * 
    * @return This MountSource as an AzureFileMountSource.
    */
   const AzureFileMountSource& asAzureFileMountSource() const;

   /**
    * @brief Gets this MountSource as an CephFsMountSource. 
    * 
    * @throw std::logic_error if isCephFsMountSource() would return false.
    * 
    * @return This MountSource as an CephFsMountSource.
    */
   CephFsMountSource& asCephFsMountSource();

   /**
    * @brief Gets this MountSource as an CephFsMountSource. 
    * 
    * @throw std::logic_error if isCephFsMountSource() would return false.
    * 
    * @return This MountSource as an CephFsMountSource.
    */
   const CephFsMountSource& asCephFsMountSource() const;

   /**
    * @brief Gets this MountSource as an GlusterFsMountSource. 
    * 
    * @throw std::logic_error if isGlusterFsMountSource() would return false.
    * 
    * @return This MountSource as an GlusterFsMountSource.
    */
   GlusterFsMountSource& asGlusterFsMountSource();

   /**
    * @brief Gets this MountSource as an GlusterFsMountSource. 
    * 
    * @throw std::logic_error if isGlusterFsMountSource() would return false.
    * 
    * @return This MountSource as an GlusterFsMountSource.
    */
   const GlusterFsMountSource& asGlusterFsMountSource() const;

   /**
    * @brief Gets this MountSource as an HostMountSource. 
    * 
    * @throw std::logic_error if isHostMountSource() would return false.
    * 
    * @return This MountSource as an HostMountSource.
    */
   HostMountSource& asHostMountSource();

   /**
    * @brief Gets this MountSource as an HostMountSource. 
    * 
    * @throw std::logic_error if isHostMountSource() would return false.
    * 
    * @return This MountSource as an HostMountSource.
    */
   const HostMountSource& asHostMountSource() const;

   /**
    * @brief Gets this MountSource as an NfsMountSource. 
    * 
    * @throw std::logic_error if isNfsMountSource() would return false.
    * 
    * @return This MountSource as an NfsMountSource.
    */
   NfsMountSource& asNfsMountSource();

   /**
    * @brief Gets this MountSource as an NfsMountSource. 
    * 
    * @throw std::logic_error if isNfsMountSource() would return false.
    * 
    * @return This MountSource as an NfsMountSource.
    */
   const NfsMountSource& asNfsMountSource() const;

   /**
    * @brief Checks whether this MountSource is an AzureFileMountSource.
    * 
    * @return True if this MountSource is an AzureFileMountSource; false otherwise.
    */
   bool isAzureFileMountSource() const;

   /**
    * @brief Checks whether this MountSource is an CephFsMountSource.
    * 
    * @return True if this MountSource is an CephFsMountSource; false otherwise.
    */
   bool isCephFsMountSource() const;

   /**
    * @brief Checks whether this MountSource is an GlusterFsMountSource.
    * 
    * @return True if this MountSource is an GlusterFsMountSource; false otherwise.
    */
   bool isGlusterFsMountSource() const;

   /**
    * @brief Checks whether this MountSource is an HostMountSource.
    * 
    * @return True if this MountSource is an HostMountSource; false otherwise.
    */
   bool isHostMountSource() const;

   /**
    * @brief Checks whether this MountSource is an NfsMountSource.
    * 
    * @return True if this MountSource is an NfsMountSource; false otherwise.
    */
   bool isNfsMountSource() const;

   /**
    * @brief Checks whether this MountSource is an PassthroughMountSource.
    * 
    * @return True if this MountSource is an PassthroughMountSource; false otherwise.
    */
   bool isPassthroughMountSource() const;

   /**
    * @brief Converts this NfsMountSource to a JSON object which represents it.
    *
    * @return The JSON object which represents this NfsMountSource.
    */
   json::Object toJson() const;

   /** The type of the Mount Source for this Mount. */
   Type SourceType;

   /** 
    * An optional field, to represent a custom mount source type. If the type field is not recongized SourceType will 
    * be set to `MountSource::Type::PASSTHROUGH` and CustomType will hold the original value of the field. */
   std::string CustomType;

   /** The JSON object that describes the Mount Source for this Mount. */
   json::Object SourceObject;
};

/** @brief Represents an Azure File Mount Source. */
struct AzureFileMountSource final : MountSource
{
   /**
    * @brief Constructs an AzureMountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as an AzureMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, AzureFileMountSource& out_mountSource);

   /**
    * @brief Gets the name of the Azure Secret used to connect to the Azure File Mount Source.
    * 
    * @return The name of the Azure Secret used to connect to the Azure File Mount Source.
    */
   std::string getSecretName() const;

   /**
    * @brief Gets the name of the share in Azure to which to connect.
    * 
    * @return The name of the share in Azure to which to connect.
    */
   std::string getShareName() const;

private:
   /**
    * @brief Constructor.
    */
   AzureFileMountSource();

   friend class MountSource;
};


/** @brief Represents a Ceph File System Mount Source. */
struct CephFsMountSource final : MountSource
{
   /**
    * @brief Constructs a CephFsMountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a CephFsMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, CephFsMountSource& out_mountSource);

   /**
    * @brief Gets
    * 
    * @return  
    */
   std::vector<std::string> getMonitors() const;

   /**
    * @brief Gets the path to mount.
    * 
    * @return The path to mount.
    */
   std::string getPath() const;

   /**
    * @brief Gets the user to mount the path as.
    * 
    * @return The user to mount the path as.
    */
   std::string getUser() const;

   /**
    * @brief Gets
    * 
    * @return  
    */
   std::string getSecretFile() const;

   /**
    * @brief Gets
    * 
    * @return  
    */
   std::string getSecretRef() const;

private:
   /**
    * @brief Constructor.
    */
   CephFsMountSource();

   friend class MountSource;
};


struct GlusterFsMountSource final : MountSource
{
   /**
    * @brief Constructor.
    */
   GlusterFsMountSource();
   
   /**
    * @brief Constructs a GlusterFsMountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a GlusterFsMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, GlusterFsMountSource& out_mountSource);

   /**
    * @brief Gets the Glusterfs endpoints to which to connect when mounting the path.
    * 
    * @return The Glusterfs endpoints to which to connect when mounting the path.
    */
   std::string getEndpoints() const;

   /**
    * @brief Gets the path to mount.
    * 
    * @return The path to mount.
    */
   std::string getPath() const;
};

/** @brief Represents a path to mount on the same host as the Job. */
struct HostMountSource final : MountSource
{
   /**
    * @brief Constructs a HostMountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as a HostMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, HostMountSource& out_mountSource);

   /**
    * @brief Gets the path on the current host to be mounted.
    * 
    * @return The path on the current host to be mounted.
    */
   std::string getPath() const;

private:
   /**
    * @brief Constructor.
    */
   HostMountSource();
   
   friend class MountSource;
};

/** @brief Represents an NFS Mount Source. */
struct NfsMountSource final : MountSource
{
   /**
    * @brief Constructs an NfsMountSource from a JSON object which represents the mount source.
    *
    * @param in_json            The JSON object which represents the mount source.
    * @param out_mountSource    The populated mount source value. Not valid if an error is returned.
    *
    * @return Success if in_json could be parsed as an NfsMountSource; Error otherwise.
    */
   static Error fromJson(const json::Object& in_json, NfsMountSource& out_mountSource);

   /**
    * @brief Gets the NFS host.
    * 
    * @return The NFS host.
    */
   std::string getHost() const;

   /**
    * @brief Gets the path on the NFS host to be mounted.
    * 
    * @return The path on the NFS host to be mounted.
    */
   std::string getPath() const;

private:
   /**
    * @brief Constructor.
    */
   NfsMountSource();
   
   friend class MountSource;
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
   std::string Destination;

   /** Whether the mounted path is read only. */
   bool IsReadOnly;

   /** The source to mount. */
   MountSource Source;
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
    * Creates a free-form placement constraint, which allows the user to enter any text value.
    *
    * @param in_name        The name of the placement constraint.
    */
   explicit PlacementConstraint(std::string in_name);

   /**
    * @brief Constructor.
    *
    * Creates an enumeration placement constraint, which allows to
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
   struct Type
   {
      /** The required number of CPUs for a job. */
      static const char* const CPU_COUNT;

      /** The required amount of CPU time for a job, in seconds. */
      static const char* const CPU_TIME;

      /** The required amount of memory for a job, in MB. */
      static const char* const MEMORY;

      /** The required amount of swap space for a job, in MB. */
      static const char* const MEMORY_SWAP;
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
   explicit ResourceLimit(std::string in_limitType, std::string in_maxValue = "", std::string in_defaultValue = "");

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
   std::string ResourceType;

   /** The value of the resource limit. */
   std::string Value;

   /** The maximum value that can be set for this type of resource. */
   std::string MaxValue;

   /** The default value that will be set for this type of resource. */
   std::string DefaultValue;
};

#define LOCK_JOB(in_job)                                    \
try                                                         \
{                                                           \
   rstudio::launcher_plugins::api::JobLock jobLock(in_job); \


#define END_LOCK_JOB END_LOCK_MUTEX

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
