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
#include <jobs/JobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>

#include "job_store/LocalJobStorage.hpp"
#include "LocalSecureCookie.hpp"
#include "LocalJobRunner.hpp"

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
    * @brief Constructor.
    *
    * @param in_hostname                The name of the host running this instance of the Local Plugin.
    * @param in_jobRepository           The job repository, from which to look up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post or from which to receive job status
    *                                   updates.
    */
   LocalJobSource(
      std::string in_hostname,
      jobs::JobRepositoryPtr in_jobRepository,
      jobs::JobStatusNotifierPtr in_jobStatusNotifier);

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
    * @brief Gets the configuration and capabilities of the Local Job Source.
    *
    * The Local Job Source only has two custom configuration values. It does not support resource limits, placement
    * constraints, queues, or containers.
    *
    * @param out_configuration       The configuration and capabilities of this Job Source, for the specified user.
    *
    * @return Success if the configuration and capabilities for this Job Source could be populated; Error otherwise.
    */
   Error getConfiguration(const system::User&, api::JobSourceConfiguration& out_configuration) const override;

   /**
    * @brief Gets all RStudio jobs currently in the job scheduling system.
    *
    * @param out_jobs   All RStudio jobs currently in the job scheduling system.
    *
    * @return Success if all jobs could be retrieved; Error otherwise.
    */
    Error getJobs(api::JobList& out_jobs) const override;

   /**
    * @brief Runs a job on the local instance.
    *
    * @param io_job                     The Job to be submitted.
    * @param out_wasInvalidRequest      Whether the requested Job was invalid, based on the features supported by the
    *                                   Job Scheduling System.
    *
    * @return Success if the job could be submitted to the Job Scheduling System; Error otherwise.
    */
   Error submitJob(api::JobPtr io_job, bool& out_wasInvalidRequest) const override;

   /**
    * @brief Creates a file output stream for the specified job.
    *
    * @param in_requestId                   The ID of the request for which job output should be streamed.
    * @param in_outputType                  The type of job output to stream.
    * @param in_job                         The job for which output should be streamed.
    * @param in_launcherCommunicator        The launcher communicator for sending responses to the Launcher.
    * @param out_outputStream               The newly created output stream, on Success.
    *
    * @return Success if the output stream could be created; Error otherwise.
    */
   Error createOutputStream(
      uint64_t in_requestId,
      api::OutputType in_outputType,
      api::JobPtr in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator,
      api::OutputStreamPtr& out_outputStream) override;

private:
   /** The hostname of the machine running this instance of the Local Launcher Plugin. */
   const std::string m_hostname;

   /** The job runner. */
   std::shared_ptr<LocalJobRunner> m_jobRunner;

   /** The job storage. */
   std::shared_ptr<job_store::LocalJobStorage> m_jobStorage;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
