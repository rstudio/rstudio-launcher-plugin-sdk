/*
 * IJobSource.hpp
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


#ifndef LAUNCHER_PLUGINS_IJOBSOURCE_HPP
#define LAUNCHER_PLUGINS_IJOBSOURCE_HPP

#include "Job.hpp"

#include <set>

#include <Error.hpp>
#include <jobs/JobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

class User;

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/** @brief Describes the container configuration of the Job Source. */
struct ContainerConfiguration
{
   /**
    * @brief Default constructor.
    */
   ContainerConfiguration() :
      AllowUnknownImages(false),
      SupportsContainers(false)
   {
   }

   /** Whether users may select unknown images when launching a job. */
   bool AllowUnknownImages;

   /** The list of known images. */
   std::set<std::string> ContainerImages;

   /** The default image. */
   std::string DefaultImage;

   /** Whether this Job Source supports containers. Default: false. */
   bool SupportsContainers;
};

/** @brief Describes the capabilities and configuration of this Job Source. */
struct JobSourceConfiguration
{
   /** The container configuration of this Job Source. */
   ContainerConfiguration ContainerConfig;

   /** The custom configuration values supported by this Job Source. */
   JobConfigList CustomConfig;

   /** The set of job placement constraints which may be set when launching a job. */
   PlacementConstraintList PlacementConstraints;

   /** The set of queues on which jobs may be run. */
   std::set<std::string> Queues;

   /**
    * The set of resource limit types, optionally with maximum and default values, which user may set when launching a
    * job.
    */
   ResourceLimitList ResourceLimits;
};

/** @brief Generic interface for communicating with a Job Source. Implementation is plugin specific. */
class IJobSource
{
public:
   /**
    * @brief Virtual Destructor.
    */
   virtual ~IJobSource() = default;

   /**
    * @brief Initializes the Job Source.
    *
    * This function should return an error if communication with the job source fails.
    *
    * @return Success if the job source could be initialized; Error otherwise.
    */
   virtual Error initialize() = 0;

   /**
    * @brief Gets the configuration and capabilities of this Job Source for the specified user.
    *
    * This function controls the options that will be available to users when launching jobs.
    *
    * NOTE: Many of the values here should most likely be controllable by Launcher administrators when they configure
    *       the Launcher. For more details, see the RStudio Launcher Plugin SDK QuickStart Guide TODO #7.
    *
    * @param in_user                The user who made the request to see the configuration and capabilities of the
    *                               Cluster. This may be used to return a different configuration based on any
    *                               configured user profiles. For more information about user profiles, see the
    *                               'User Profiles' subsection of the 'Advanced Features' section of the RStudio
    *                               Launcher Plugin SDK Developer's Guide.
    * @param out_configuration      The configuration and capabilities of this Job Source, for the specified user.
    *
    * @return Success if the configuration and capabilities for this Job Source could be populated; Error otherwise.
    */
   virtual Error getConfiguration(const system::User& in_user, JobSourceConfiguration& out_configuration) const = 0;

   /**
    * @brief Gets all RStudio jobs currently in the job scheduling system.
    *
    * @param out_jobs   All RStudio jobs currently in the job scheduling system.
    *
    * @return Success if all jobs could be retrieved; Error otherwise.
    */
   virtual Error getJobs(JobList& out_jobs) const = 0;

   /**
    * @brief Submits a job to the Job Scheduling System.
    *
    * @param io_job     The Job to be submitted. On successful submission, the Job should be updated with relevant
    *                   details, such as the ID of the job, the Submission time, the actual Job Queue (if applicable),
    *                   and the current status.
    *
    * @return Success if the job could be submitted to the Job Scheduling System; Error otherwise.
    */
   virtual Error submitJob(JobPtr io_job) const = 0;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository           The job repository, from which to look up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post or from which to receive job status
    *                                   updates.
    */
   IJobSource(jobs::JobRepositoryPtr in_jobRepository, jobs::JobStatusNotifierPtr in_jobStatusNotifier) :
      m_jobRepository(std::move(in_jobRepository)),
      m_jobStatusNotifier(std::move(in_jobStatusNotifier))
   {
   }

   /** The job repository, from which to look up jobs. */
   jobs::JobRepositoryPtr m_jobRepository;

   /** The job status notifier to which to post or from which to receive job status updates. */
   jobs::JobStatusNotifierPtr m_jobStatusNotifier;
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
