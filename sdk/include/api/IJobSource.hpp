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

#include <set>
#include "Job.hpp"

namespace rstudio {
namespace launcher_plugins {

class Error;

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
    * @return True if unknown images should be allowed; false otherwise.
    */
   virtual bool allowUnknownImages()
   {
      return false;
   }

   /**
    * @brief If this job source supports containers, returns the default container image to use when a job is launched,
    *        if any.
    *
    * This function controls Cluster capabilities.
    *
    * @return The default container image, if any.
    */
   virtual std::string getDefaultImage()
   {
      return "";
   }

   /**
    * @brief If this job source supports containers, gets the container images which are available to run jobs.
    *
    * This function controls Cluster capabilities.
    *
    * @return The container images on which jobs may be run, if any.
    */
   virtual std::set<std::string> getContainerImages()
   {
      return {};
   }

   /**
    * @brief Gets the custom configuration values which may be set on the jobs, if any.
    *
    * This function controls Cluster capabilities.
    *
    * @return The custom configuration values which may be set on the jobs, if any.
    */
   virtual std::vector<JobConfig> getCustomConfig()
   {
      return {};
   }

   /**
    * @brief Gets the custom placement constraints which may be set on jobs, if any.
    *
    * This function controls Cluster capabilities.
    *
    * @return The custom placement constraints which may be set on jobs, if any.
    */
   virtual std::vector<PlacementConstraint> getPlacementConstraints()
   {
      return {};
   }

   /**
    * @brief Gets the resource limit types which can be set for jobs, including default and maximum values, if any.
    *
    * This function controls Cluster capabilities.
    *
    * @return The resource limit types which can be set for jobs, including default and maximum values, if any.
    */
   virtual std::vector<ResourceLimit> getResourceLimits()
   {
      return {};
   }

   /**
    * @brief Gets whether this job source supports containers.
    *
    * This function controls Cluster capabilities.
    *
    * @return True if the job source supports containers; false otherwise.
    */
   virtual bool supportsContainers()
   {
      return false;
   }
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
