/*
 * LocalJobStorage.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_JOB_STORAGE_HPP
#define LAUNCHER_PLUGINS_LOCAL_JOB_STORAGE_HPP

#include <api/Job.hpp>
#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace local {
namespace job_store {

/**
 * @brief Responsible for job persistence.
 */
class LocalJobStorage
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_hostname    The
    */
   explicit LocalJobStorage(std::string in_hostname);

   /**
    * @brief Initializes the local job storage.
    *
    * @return Success if all local job storage directories could be created; Error otherwise.
    */
   Error initialize() const;

   /**
    * @brief Loads all jobs from disk.
    *
    * @param out_jobs       The loaded jobs.
    *
    * @return Success if all the existing jobs could be loaded from disk; Error otherwise.
    */
   Error loadJobs(api::JobList& out_jobs) const;

private:
   /** The name of the host of this Local Pluign instance. */
   const std::string m_hostname;

   /** The scratch path configured by the system administrator. */
   const system::FilePath m_jobsRootPath;

   /** The scratch path configured by the system administrator. */
   const system::FilePath m_jobsPath;

   /** Whether to save job output when the output location is not specified by the user. */
   const bool m_saveUnspecifiedOutput;

   /** The scratch path configured by the system administrator. */
   const system::FilePath m_outputRootPath;
};

} // namespace job_store
} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
