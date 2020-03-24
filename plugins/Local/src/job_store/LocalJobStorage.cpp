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
#include <json/Json.hpp>

using namespace rstudio::launcher_plugins::system;

namespace rstudio {
namespace launcher_plugins {
namespace local {
namespace job_store {

namespace {

constexpr const char* JOB_FILE_EXT     = "job";
constexpr const char* ROOT_JOBS_DIR    = "jobs";
constexpr const char* ROOT_OUTPUT_DIR  = "output";

inline Error createDirectory(const FilePath& in_directory, FileMode in_fileMode = FileMode::USER_READ_WRITE_EXECUTE)
{
   Error error = in_directory.ensureDirectory();
   if (error)
      return error;

   return in_directory.changeFileMode(in_fileMode);
}

inline Error readJobFromFile(const FilePath& in_jobFile, api::JobPtr& out_job)
{
   // Programmer error if the out_job is a nullptr.
   assert(out_job != nullptr);

   std::string jobJsonStr;
   Error error = utils::readFileToString(in_jobFile, jobJsonStr);
   if (error)
      return error;

   json::Object jobObj;
   error = jobObj.parse(jobJsonStr);
   if (error)
      return error;

   return api::Job::fromJson(jobObj, *out_job);
}

} // anonymous namespace

LocalJobStorage::LocalJobStorage(std::string in_hostname) :
   m_hostname(std::move(in_hostname)),
   m_jobsRootPath(options::Options::getInstance().getScratchPath().completeChildPath(ROOT_JOBS_DIR)),
   m_jobsPath(m_jobsRootPath.completeChildPath(m_hostname)),
   m_saveUnspecifiedOutput(LocalOptions::getInstance().shouldSaveUnspecifiedOutput()),
   m_outputRootPath(options::Options::getInstance().getScratchPath().completeChildPath(ROOT_OUTPUT_DIR))
{
}

Error LocalJobStorage::initialize() const
{
   Error error = createDirectory(m_jobsRootPath);
   if (error)
      return error;

   return createDirectory(m_jobsPath);
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

} // namespace job_store
} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
