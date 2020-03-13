/*
 * LocalJobSource.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_JOB_SOURCE_HPP
#define LAUNCHER_PLUGINS_LOCAL_JOB_SOURCE_HPP

#include <api/IJobSource.hpp>

#include <vector>

#include <api/Job.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Class which is responsible for running and retrieving information about jobs on the Local system.
 */
class LocalJobSource : public api::IJobSource
{
public:
   /**
    * @brief Initializes the Local Job Source.
    *
    * This function initializes the file-based job storage and communications with other Local plugins which are part of
    * this Launcher cluster.
    *
    * @return Success if the job source could be initialized; Error otherwise.
    */
   Error initialize() override;

   /**
    * @brief Gets the custom configuration values for the Local Plugin.
    *
    * This function controls Cluster capabilities.
    *
    * The Local Plugin supports two custom configuration values: The PAM Profile, and an encrypted password.
    *
    * The system::User parameter is not used. The Local Plugin does not support user profiles, and returns the same
    * custom configuration values regardless of the request user.
    *
    * @param out_customConfig       The custom configuration settings available to set on jobs.
    *
    * @return The PAM Profile and encrypted password custom configuration values.
    */
    Error getCustomConfig(const system::User&, std::vector<api::JobConfig>& out_customConfig) const override;

   /**
    * @brief Gets all RStudio jobs currently in the job scheduling system.
    *
    * @param out_jobs   All RStudio jobs currently in the job scheduling system.
    *
    * @return Success if all jobs could be retrieved; Error otherwise.
    */
    Error getJobs(api::JobList& out_jobs) const override;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
