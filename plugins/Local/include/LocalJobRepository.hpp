/*
 * LocalJobRepository.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_JOB_REPOSITORY_HPP
#define LAUNCHER_PLUGINS_LOCAL_JOB_REPOSITORY_HPP

#include <jobs/AbstractJobRepository.hpp>

#include <memory>

#include <api/Job.hpp>
#include <jobs/JobStatusNotifier.hpp>
#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Responsible for job persistence.
 */
class LocalJobRepository : public jobs::AbstractJobRepository
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_hostname    The hostname of machine which is hosting this instance of the Local Plugin.
    * @param in_notifier    The job status notifier from which to receive job status update notifications.
    */
   LocalJobRepository(const std::string& in_hostname, jobs::JobStatusNotifierPtr in_notifier);

   /**
    * @brief Saves a job to disk.
    *
    * @param in_job     The job to be saved.
    */
   void saveJob(api::JobPtr in_job) const;

   /**
    * @brief Sets the default output paths for the specified job.
    *
    * @param io_job     The job to modify.
    */
   Error setJobOutputPaths(api::JobPtr io_job) const;

private:
   /**
    * @brief Loads all jobs from disk.
    *
    * @param out_jobs       The loaded jobs.
    *
    * @return Success if all the existing jobs could be loaded from disk; Error otherwise.
    */
   Error loadJobs(api::JobList& out_jobs) const override;

   /**
    * @brief Saves newly added jobs to disk.
    *
    * @param in_job     The job that was added to the repository.
    */
   void onJobAdded(const api::JobPtr& in_job) override;

   /**
    * @brief Removes expired jobs from disk, including all output data.
    *
    * @param in_job     The job that was removed from the repository.
    */
   virtual void onJobRemoved(const api::JobPtr& in_job) override;

   /**
    * @brief Initializes the local job repository.
    *
    * @return Success if all local job repository directories could be created; Error otherwise.
    */
   Error onInitialize() override;
   
   /** The name of the host of this Local Plugin instance. */
   const std::string& m_hostname;

   /** The scratch path configured by the system administrator. */
   const system::FilePath m_jobsRootPath;

   /** The scratch path configured by the system administrator. */
   const system::FilePath m_jobsPath;

   /** Whether to save job output when the output location is not specified by the user. */
   const bool m_saveUnspecifiedOutput;

   /** The scratch path configured by the system administrator. */
   const system::FilePath m_outputRootPath;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
