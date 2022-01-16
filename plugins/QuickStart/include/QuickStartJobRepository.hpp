/*
 * QuickStartJobRepository.hpp
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

#ifndef LAUNCHER_PLUGINS_QUICK_START_JOB_REPOSITORY_HPP
#define LAUNCHER_PLUGINS_QUICK_START_JOB_REPOSITORY_HPP

#include <jobs/AbstractJobRepository.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

class QuickStartJobRepository : public jobs::AbstractJobRepository
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobStatusNotifier       The job status notifier. Used to add new jobs.
    */
   explicit QuickStartJobRepository(jobs::JobStatusNotifierPtr in_notifier);

private:
   /**
    * @brief Responsible for loading any jobs which were in the system when the Plugin started.
    *
    * This method will be invoked once, when the Plugin is started.
    *
    * @param out_jobs       The jobs that were already in the job scheduling system on start up.
    *
    * @return Success if the jobs could be loaded; Error otherwise.
    */
   virtual Error loadJobs(api::JobList& out_jobs) const override;
};

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio

#endif
