/*
 * AbstractJobRepository.hpp
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

#ifndef LAUNCHER_PLUGINS_JOB_REPOSITORY_HPP
#define LAUNCHER_PLUGINS_JOB_REPOSITORY_HPP

#include <Noncopyable.hpp>

#include <PImpl.hpp>
#include <api/Job.hpp>
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
namespace jobs {

/**
 * @brief Stores any jobs currently in the job scheduling system.
 */
class AbstractJobRepository : public Noncopyable, public std::enable_shared_from_this<AbstractJobRepository>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobStatusNotifier       The job status notifier. Used to add new jobs.
    */
   explicit AbstractJobRepository(JobStatusNotifierPtr in_jobStatusNotifier);

   /**
    * @brief Virtual Destructor, to allow for inheritance, if necessary.
    */
   virtual ~AbstractJobRepository() = default;

   /**
    * @brief Adds the job to the repository.
    *
    * If the job is already in the repository, nothing will happen.
    *
    * @param in_job     The job to add to the repository.
    */
   void addJob(const api::JobPtr& in_job);

   /**
    * @brief Gets the specified job for the specified user from the repository.
    *
    * If the job does not belong to the specified user and if the user does not represent "all users", no job will
    * be returned.
    *
    * @param in_jobId       The ID of the job to retrieve.
    * @param in_user        The user requesting the job. Default: All users.
    *
    * @return The Job, if it could be found; an empty pointer otherwise.
    */
   api::JobPtr getJob(const std::string& in_jobId, const system::User& in_user = system::User()) const;

   /**
    * @brief Gets all jobs belonging to the specified user.
    *
    * If the user object represents "all users", all jobs will be returned.
    *
    * @param in_user    The user for whom to retrieve all jobs. Default: All users.
    *
    * @return All of the jobs belonging to the specified user.
    */
   api::JobList getJobs(const system::User& in_use = system::User()) const;

   /**
    * @brief Initializes the AbstractJobRepository.
    *
    * @return Success if the repository could be initialized; Error otherwise.
    */
   Error initialize();

   /**
    * @brief Removes a job from the repository.
    *
    * If there is no job with the specified id, nothing will happen.
    *
    * @param in_jobId   The ID of the job to remove.
    */
   void removeJob(const std::string& in_jobId);

private:
   /**
    * @brief Allows inheriting classes to perform custom actions when a job is removed from the repository.
    *
    * @param in_job     The job that was removed from the repository.
    */
   virtual void onJobRemoved(const api::JobPtr& in_job);

   /**
    * @brief Allows inheriting classes to perform custom initialization actions when the repository is created.
    *
    * @return Success if the repository could be initialized; Error otherwise.
    */
   virtual Error onInitialize();

   // The private implementation of AbstractJobRepository.
   PRIVATE_IMPL(m_impl);
};

/** Convenience typedef. */
typedef std::shared_ptr<AbstractJobRepository> JobRepositoryPtr;

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio

#endif
