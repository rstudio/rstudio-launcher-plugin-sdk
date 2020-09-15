/*
 * QuickStartJobSource.hpp
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

#ifndef LAUNCHER_PLUGINS_QUICK_START_JOB_SOURCE_HPP
#define LAUNCHER_PLUGINS_QUICK_START_JOB_SOURCE_HPP

#include <api/IJobSource.hpp>

#include <jobs/AbstractJobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>

#include "QuickStartJobStatusWatcher.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace quickstart {

/**
 * @brief Class which is responsible for running and retrieving information about jobs in the job scheduling system.
 */
class QuickStartJobSource : public api::IJobSource
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository           The job repository, from which to look up jobs.
    * @param in_jobStatusNotifier       The job status notifier to which to post or from which to receive job status
    *                                   updates.
    */
   QuickStartJobSource(
      const jobs::JobRepositoryPtr& in_jobRepository,
      const jobs::JobStatusNotifierPtr& in_jobStatusNotifier);

   /**
    * @brief Initializes the Job Source.
    *
    * This function should return an error if communication with the job source fails.
    *
    * @return Success if the job source could be initialized; Error otherwise.
    */
   Error initialize() override;

   /**
    * @brief Cancels a pending job.
    *
    * This method will not be invoked unless the job is currently pending.
    * The Job lock will be held when this method is invoked.
    *
    * @param in_job                 The job to be canceled.
    * @param out_isComplete         Whether the cancel operation completed successfully (true) or not (false).
    * @param out_statusMessage      The status message of the cancel operation, if any.
    *
    * @return False if the cancel operation is not supported; true otherwise. 
    */
   bool cancelJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage) override;

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
   Error getConfiguration(
      const system::User& in_user,
      api::JobSourceConfiguration& out_configuration) const override;

   /**
    * @brief Gets the network information for the specified job.
    *
    * @param in_job             The job for which to retrieve network information.
    * @param out_networkInfo    The network information of the specified job, if no error occurred.
    *
    * @return Success if the network information could be retrieved; Error otherwise.
    */
   Error getNetworkInfo(api::JobPtr in_job, api::NetworkInfo& out_networkInfo) const override;

   /**
    * @brief Forcibly kills a running job.
    *
    * This method should perform the equivalent of sending a SIGKILL to a process.
    * This method will not be invoked unless the job is currently running.
    * The Job lock will be held when this method is invoked.
    *
    * @param in_job                 The job to be killed.
    * @param out_isComplete         Whether the kill operation completed successfully (true) or not (false).
    * @param out_statusMessage      The status message of the kill operation, if any.
    *
    * @return False if the kill operation is not supported; true otherwise. 
    */
   bool killJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage) override;

   /**
    * @brief Resumes a suspended job.
    *
    * This method should perform the equivalent of sending a SIGCONT to a process.
    * This method will not be invoked unless the job is currently suspended.
    * The Job lock will be held when this method is invoked.
    *
    * @param in_job                 The job to be resumed.
    * @param out_isComplete         Whether the stop operation completed successfully (true) or not (false).
    * @param out_statusMessage      The status message of the stop operation, if any.
    *
    * @return False if the stop operation is not supported; true otherwise.  
    */
   bool resumeJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage) override;

   /**
    * @brief Stops a running job.
    *
    * This method should perform the equivalent of sending a SIGTERM to a process.
    * This method will not be invoked unless the job is currently running.
    * The Job lock will be held when this method is invoked.
    *
    * @param in_job                 The job to be canceled.
    * @param out_statusMessage      The status message of the cancel operation, if any.
    *
    * @return True if the job was stopped; false otherwise.
    */
   bool stopJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage) override;

   /**
    * @brief Suspends a running job.
    *
    * This method should perform the equivalent of sending a SIGSTOP to a process.
    * A suspended job should be able to be resumed at a later time.
    * This method will not be invoked unless the job is currently running.
    * The Job lock will be held when this method is invoked.
    *
    * @param in_job                 The job to be suspended.
    * @param out_isComplete         Whether the suspend operation completed successfully (true) or not (false).
    * @param out_statusMessage      The status message of the suspend operation, if any.
    *
    * @return False if the suspend operation is not supported; true otherwise. 
    */
   bool suspendJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage) override;

   /**
    * @brief Submits a job to the Job Scheduling System.
    *
    * @param io_job                     The Job to be submitted. On successful submission, the Job should be updated
    *                                   with relevant details, such as the ID of the job, the Submission time, the
    *                                   actual Job Queue (if applicable), and the current status.
    * @param out_wasInvalidRequest      Whether the requested Job was invalid, based on the features supported by the
    *                                   Job Scheduling System.
    *
    * @return Success if the job could be submitted to the Job Scheduling System; Error otherwise.
    */
   Error submitJob(api::JobPtr io_job, bool& out_wasInvalidRequest) const override;

   /**
    * @brief Creates an output stream for the specified job.
    *
    * @param in_outputType      The type of job output to stream.
    * @param in_job             The job for which output should be streamed.
    * @param in_onOutput        Callback function which will be invoked when data is reported.
    * @param in_onComplete      Callback function which will be invoked when the stream is complete.
    * @param in_onError         Callback function which will be invoked if an error occurs.
    * @param out_outputStream   The newly created output stream, on Success.
    *
    * @return Success if the output stream could be created; Error otherwise.
    */
   Error createOutputStream(
      api::OutputType in_outputType,
      api::JobPtr in_job,
      api::AbstractOutputStream::OnOutput in_onOutput,
      api::AbstractOutputStream::OnComplete in_onComplete,
      api::AbstractOutputStream::OnError in_onError,
      api::OutputStreamPtr& out_outputStream) override;

private:
   QuickStartJobStatusWatcherPtr m_jobStatusWatcher;
};

} // namespace quickstart
} // namespace launcher_plugins
} // namespace rstudio

#endif
