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
   /** The capabilities of this Job Source, with respect to Containers. */
   ContainerConfiguration ContainerConfig;

   /** The customer configuration values supported by this Job Source. */
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
    * @brief Gets the capabilities of this Job Source for the specified user.
    *
    * This function controls Cluster capabilities.
    *
    * NOTE: Many of the values here should most likely be controllable by Launcher administrators when they configure
    *       the Launcher. For more details, see the RStudio Launcher Plugin SDK QuickStart Guide TODO #7.
    *
    * @param in_user                The user who made the request to see the capabilities of the Cluster. This may be
    *                               used to return different capabilities based on the configured user profiles. For
    *                               more information about user profiles, see the 'User Profiles' subsection of the
    *                               'Advanced Features' section of the RStudio Launcher Plugin SDK Developer's Guide.
    * @param out_configuration      The capabilities of this Job Source, for the specified user.
    *
    * @return Success if the capabilities for this Job Source could be populated; Error otherwise.
    */
   virtual Error getConfiguration(const system::User& in_user, JobSourceConfiguration& out_configuration) const = 0;
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
