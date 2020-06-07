/*
 * LocalJobStorage.cpp
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

#include <job_store/LocalJobStorage.hpp>

#include <Error.hpp>
#include <json/Json.hpp>
#include <options/Options.hpp>
#include <system/FilePath.hpp>
#include <utils/FileUtils.hpp>

#include <LocalOptions.hpp>

using namespace rstudio::launcher_plugins::system;

namespace rstudio {
namespace launcher_plugins {
namespace local {
namespace job_store {

typedef std::shared_ptr<LocalJobStorage> SharedThis;
typedef std::weak_ptr<LocalJobStorage> WeakThis;

namespace {

constexpr const char* JOB_FILE_EXT = ".job";
constexpr const char* ERR_FILE_EXT = ".stderr";
constexpr const char* OUT_FILE_EXT = ".stdout";
constexpr const char* ROOT_JOBS_DIR = "jobs";
constexpr const char* ROOT_OUTPUT_DIR = "output";

inline Error createDirectory(const FilePath& in_directory, FileMode in_fileMode = FileMode::USER_READ_WRITE_EXECUTE)
{
   Error error = in_directory.ensureDirectory();
   if (error)
      return error;

   return in_directory.changeFileMode(in_fileMode);
}

inline FilePath getJobFilePath(const std::string& in_id, const system::FilePath& in_jobsPath)
{
   return in_jobsPath.completeChildPath(in_id + JOB_FILE_EXT);
}

inline Error readJobFromFile(const FilePath& in_jobFile, api::JobPtr& out_job)
{
   // Programmer error if the out_job is a nullptr.
   assert(out_job != nullptr);

   std::string jobJsonStr;
   Error error = utils::readFileIntoString(in_jobFile, jobJsonStr);
   if (error)
      return error;

   json::Object jobObj;
   error = jobObj.parse(jobJsonStr);
   if (error)
      return error;

   return api::Job::fromJson(jobObj, *out_job);
}

} // anonymous namespace

LocalJobStorage::LocalJobStorage(const std::string& in_hostname, jobs::JobStatusNotifierPtr in_notifier) :
   m_hostname(in_hostname),
   m_jobsRootPath(options::Options::getInstance().getScratchPath().completeChildPath(ROOT_JOBS_DIR)),
   m_jobsPath(m_jobsRootPath.completeChildPath(m_hostname)),
   m_notifier(std::move(in_notifier)),
   m_saveUnspecifiedOutput(LocalOptions::getInstance().shouldSaveUnspecifiedOutput()),
   m_outputRootPath(options::Options::getInstance().getScratchPath().completeChildPath(ROOT_OUTPUT_DIR))
{
}

Error LocalJobStorage::initialize()
{
   Error error = createDirectory(m_jobsRootPath);
   if (error)
      return error;

   error = createDirectory(m_jobsPath);
   if (error)
      return error;

   WeakThis weakThis = weak_from_this();
   m_subscriptionHandle = m_notifier->subscribe(
      [weakThis](const api::JobPtr& in_job)
      {
         if (SharedThis sharedThis = weakThis.lock())
         {
            sharedThis->saveJob(in_job);
         }
      });

   return Success();
}

Error LocalJobStorage::loadJobs(api::JobList& out_jobs) const
{
   std::vector<FilePath> jobFiles;
   Error error = m_jobsPath.getChildren(jobFiles);
   if (error)
      return error;

   for (const FilePath& jobFile: jobFiles)
   {
      if (jobFile.getExtension() != JOB_FILE_EXT)
         continue;

      api::JobPtr job(new api::Job());
      error = readJobFromFile(jobFile, job);
      if (error)
      {
         // If there's a problem loading a job, just log the error and skip the job.
         logging::logError(error);
         continue;
      }

      out_jobs.push_back(job);
   }

   logging::logInfoMessage("Loaded " + std::to_string(out_jobs.size())  + " jobs from file");

   return Success();
}

void LocalJobStorage::saveJob(api::JobPtr in_job) const
{
   LOCK_JOB(in_job)
   {
      if (m_hostname == in_job->Host)
      {
         Error error = utils::writeStringToFile(in_job->toJson().write(), getJobFilePath(in_job->Id, m_jobsPath));
         if (error)
            logging::logError(error, ERROR_LOCATION);
      }
   }
   END_LOCK_JOB
}

Error LocalJobStorage::setJobOutputPaths(api::JobPtr io_job) const
{
   bool outputEmpty = io_job->StandardOutFile.empty(),
        errorEmpty = io_job->StandardErrFile.empty();
   if (m_saveUnspecifiedOutput && (outputEmpty || errorEmpty))
   {
      const system::FilePath outputDir = m_outputRootPath.completeChildPath(io_job->User.getUsername());
      Error error = createDirectory(outputDir);
      if (error)
         return error;

      if (outputEmpty)
         io_job->StandardOutFile = outputDir.completeChildPath(io_job->Id + OUT_FILE_EXT).getAbsolutePath();
      if (errorEmpty)
         io_job->StandardErrFile = outputDir.completeChildPath(io_job->Id + ERR_FILE_EXT).getAbsolutePath();
   }

   return Success();
}

} // namespace job_store
} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
