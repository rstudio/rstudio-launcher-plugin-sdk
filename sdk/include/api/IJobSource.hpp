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

/** Generic interface for communicating with a Job Source. Implementation is plugin specific. */
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
    * @brief If this job source supports containers, returns whether unknown images may be selected by users when
    *        launching jobs.
    *
    * This function controls Cluster capabilities.
    *
    * NOTE: This should most likely be controllable by Launcher administrators when they configure the Launcher.
    *
    * @param in_user                    The user who made the request to see the capabilities of the Cluster. This may
    *                                   be used to return different capabilities based on the configured user profiles.
    * @param out_allowUnknownImages     Whether or not unknown images should be allowed.
    *
    * @return Success if the value of the allow unknown images setting could be checked; Error otherwise.
    */
   virtual Error allowUnknownImages(const system::User& in_user, bool& out_allowUnknownImages) const
   {
      out_allowUnknownImages = false;
      return Success();
   }

   /**
    * @brief If this job source supports containers, gets the container images which are available to run jobs.
    *
    * This function controls Cluster capabilities.
    *
    * @param in_user        The user who made the request to see the capabilities of the Cluster. This may be used to
    *                       return different capabilities based on the configured user profiles.
    * @param out_images     The set of container images on which jobs may be run.
    *
    * @return Success if the set of container images could be retrieved, error others.
    */
   virtual Error getContainerImages(const system::User& in_user, std::set<std::string>& out_images) const
   {
      out_images = {};
      return Success();
   }

   /**
    * @brief Gets the custom configuration values which may be set on the jobs, if any.
    *
    * This function controls Cluster capabilities.
    *
    * NOTE: Custom configuration values should be used sparingly. If possible, find other means to surface values that
    *       could be surfaced here. For example, if a custom configuration value could be set by a system administrator,
    *       consider adding it to the Plugin's Options.
    *
    * @param in_user                The user who made the request to see the capabilities of the Cluster. This may be
    *                               used to return different capabilities based on the configured user profiles.
    * @param out_customConfig       The custom configuration settings available to set on jobs.
    *
    * @return Success if the custom configuration settings could be populated; Error otherwise.
    */
   virtual Error getCustomConfig(const system::User& in_user, std::vector<JobConfig>& out_customConfig) const
   {
      out_customConfig = {};
      return Success();
   }

   /**
    * @brief If this job source supports containers, returns the default container image to use when a job is launched,
    *        if any.
    *
    * This function controls Cluster capabilities.
    *
    * @param in_user            The user who made the request to see the capabilities of the Cluster. This may be used
    *                           to return different capabilities based on the configured user profiles.
    * @param out_defaultImage   The default container image, if any.
    *
    * @return Success if the default image could be populated; Error otherwise.
    */
   virtual Error getDefaultImage(const system::User& in_user, std::string& out_defaultImage) const
   {
      out_defaultImage = "";
      return Success();
   }

   /**
    * @brief Gets all RStudio jobs currently in the job scheduling system.
    *
    * @param out_jobs   All RStudio jobs currently in the job scheduling system.
    *
    * @return Success if all jobs could be retrieved; Error otherwise.
    */
   virtual Error getJobs(JobList& out_jobs) const = 0;

   /**
    * @brief Gets the custom placement constraints which may be set on jobs, if any.
    *
    * This function controls Cluster capabilities.
    *
    * @param in_user            The user who made the request to see the capabilities of the Cluster. This may be used
    *                           to return different capabilities based on the configured user profiles.
    * @param out_constraints    The list of custom placement constraints which may be set on jobs, if any.
    *
    * @return Success if the list of custom placement constraints could be populated; Error otherwise.
    */
   virtual Error getPlacementConstraints(
      const system::User& in_user,
      std::vector<PlacementConstraint>& out_constraints) const
   {
      out_constraints = {};
      return Success();
   }

   /**
    * @brief Gets the queues which are available to run jobs, if any.
    *
    * This function controls Cluster capabilities.
    *
    * @param in_user        The user who made the request to see the capabilities of the Cluster. This may be used to
    *                       return different capabilities based on the configured user profiles.
    * @param out_queues     The set of queues on which jobs may be run, if any.
    *
    * @return Success if the list of queues could be populated; Error otherwise.
    */
   virtual Error getQueues(const system::User& in_user, std::set<std::string>& out_queues) const
   {
      out_queues = {};
      return Success();
   }

   /**
    * @brief Gets the resource limit types which can be set for jobs, including default and maximum values, if any.
    *
    * This function controls Cluster capabilities.
    *
    * @param in_user        The user who made the request to see the capabilities of the Cluster. This may be used to
    *                       return different capabilities based on the configured user profiles.
    * @param out_limits     The resource limit types which can be set for jobs, including default and maximum values, if
    *                       any.
    *
    * @return Success if the list of resource limits could be populated; Error otherwise.
    */
   virtual Error getResourceLimits(const system::User& in_user, std::vector<ResourceLimit>& out_limits) const
   {
      out_limits = {};
      return Success();
   }

   /**
    * @brief Gets whether this job source supports containers.
    *
    * This function controls Cluster capabilities.
    *
    * @return True if the job source supports containers; false otherwise.
    */
   virtual bool supportsContainers() const
   {
      return false;
   }
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
