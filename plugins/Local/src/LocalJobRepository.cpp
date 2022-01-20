/*
 * LocalJobRepository.cpp
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

#include <LocalJobRepository.hpp>

#include <Error.hpp>
#include <json/Json.hpp>
#include <options/Options.hpp>
#include <system/FilePath.hpp>
#include <system/Process.hpp>
#include <utils/FileUtils.hpp>

#include <LocalOptions.hpp>

using namespace rstudio::launcher_plugins::system;

namespace rstudio {
namespace launcher_plugins {
namespace local {

typedef std::shared_ptr<LocalJobRepository> SharedThis;
typedef std::weak_ptr<LocalJobRepository> WeakThis;

namespace {

constexpr const char* JOB_FILE_EXT = ".job";
constexpr const char* ERR_FILE_EXT = ".stderr";
constexpr const char* OUT_FILE_EXT = ".stdout";
constexpr const char* ROOT_JOBS_DIR = "jobs";
constexpr const char* ROOT_OUTPUT_DIR = "output";

inline void deleteFileAsUser(const system::User& in_user, const FilePath& in_file)
{
   if (!in_file.isEmpty())
   {
      logging::logDebugMessage("Deleting job file: " + in_file.getAbsolutePath());

      system::process::ProcessOptions opts;
      opts.Executable = "rm";
      opts.Arguments = { "-f", in_file.getAbsolutePath() };
      opts.RunAsUser = in_user;
      opts.IsShellCommand = true;

      system::process::ProcessResult result;
      system::process::SyncChildProcess rmProc(opts);
      Error error = rmProc.run(result);
      if (error)
      {
         logging::logErrorMessage("Could not delete output file: " + in_file.getAbsolutePath(), ERROR_LOCATION);
         logging::logError(error, ERROR_LOCATION);
      }
      else if (result.ExitCode != 0)
      {
         logging::logErrorMessage(
            "Deleting output file " +
               in_file.getAbsolutePath() +
               " exited with non-zero exit code: " +
               std::to_string(result.ExitCode),
            ERROR_LOCATION);

         logging::logDebugMessage(
            "Delete output file stdout: " + result.StdOut + "\nDelete output file stderr:" + result.StdError);
      }

      // If the file couldn't be deleted, treat it as a permissions issue.
      if (in_file.exists())
      {
         logging::logError(
            systemError(EPERM, "Could not delete output file: " + in_file.getAbsolutePath(), ERROR_LOCATION));
      }
   }
}

inline Error ensureDirectory(const FilePath& in_directory, FileMode in_fileMode = FileMode::USER_READ_WRITE_EXECUTE)
{
   Error error = in_directory.ensureDirectory();
   if (error)
      return error;

   return in_directory.changeFileMode(in_fileMode);
}

inline Error ensureUserDirectory( const system::FilePath& in_userDirectory, const system::User& in_user)
{
   if (!in_userDirectory.exists())
   {
      const std::string& userDir = in_userDirectory.getAbsolutePath();

      system::process::ProcessOptions procOpts;
      procOpts.Executable = "mkdir " + userDir + " && chmod 700 " + userDir;
      procOpts.IsShellCommand = true;
      procOpts.RunAsUser = in_user;

      system::process::ProcessResult result;
      system::process::SyncChildProcess child(procOpts);
      Error error = child.run(result);

      const std::string errMsg =
         "Could not create output directory " +
            userDir +
            " for user " +
            in_user.getUsername();

      if (error)
      {
         logging::logErrorMessage(errMsg);
         return error;
      }

      if (result.ExitCode != 0)
      {
         logging::logErrorMessage(
            "Creating output directory " +
               userDir +
               " for user " +
               in_user.getUsername() +
               " exited with non-zero exit code " +
               std::to_string(result.ExitCode));

         logging::logDebugMessage(
            "Create directory for user " +
               in_user.getUsername() +
               "\n    stdout: \"" +
               result.StdOut +
               "\"\n    stderr: \"" +
               result.StdError +
               "\"");
      }

      if (!in_userDirectory.exists())
         return fileNotFoundError(errMsg, ERROR_LOCATION);
   }

   return Success();
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

LocalJobRepository::LocalJobRepository(const std::string& in_hostname, jobs::JobStatusNotifierPtr in_notifier) :
   AbstractJobRepository(std::move(in_notifier)),
   m_hostname(in_hostname),
   m_jobsRootPath(options::Options::getInstance().getScratchPath().completeChildPath(ROOT_JOBS_DIR)),
   m_jobsPath(m_jobsRootPath.completeChildPath(m_hostname)),
   m_saveUnspecifiedOutput(LocalOptions::getInstance().shouldSaveUnspecifiedOutput()),
   m_outputRootPath(options::Options::getInstance().getScratchPath().completeChildPath(ROOT_OUTPUT_DIR))
{
}

void LocalJobRepository::saveJob(api::JobPtr in_job) const
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

Error LocalJobRepository::setJobOutputPaths(api::JobPtr io_job) const
{
   bool outputEmpty = io_job->StandardOutFile.empty(),
        errorEmpty = io_job->StandardErrFile.empty();
   if (m_saveUnspecifiedOutput && (outputEmpty || errorEmpty))
   {
      system::FilePath outputDir = m_outputRootPath.completeChildPath(io_job->User.getUsername());
      Error error = ensureUserDirectory(outputDir, io_job->User);
      if (error)
         return error;

      if (outputEmpty)
         io_job->StandardOutFile = outputDir.completeChildPath(io_job->Id + OUT_FILE_EXT).getAbsolutePath();
      if (errorEmpty)
         io_job->StandardErrFile = outputDir.completeChildPath(io_job->Id + ERR_FILE_EXT).getAbsolutePath();
   }

   return Success();
}

Error LocalJobRepository::loadJobs(api::JobList& out_jobs) const
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

      // Update the status of the job on load.
      if (!job->isCompleted())
      {
         bool jobModified = false;
         system::process::ProcessInfo procInfo;
         Error error = system::process::ProcessInfo::getProcessInfo(job->Pid.getValueOr(0), procInfo);
         if (isFileNotFoundError(error))
         {
            // If we couldn't find details about the job, it finished between the time the last instance of the Local 
            // Plugin exited and this instance started. Update the job state to the best of our knowledge to avoid jobs
            // stuck in their states.
            job->Status = api::Job::State::FINISHED;
            job->LastUpdateTime = system::DateTime();
            jobModified = true;
         }
         else if (!error && (job->Status ==  api::Job::State::PENDING) && (procInfo.Executable != "rsandbox"))
         {
            job->Status = api::Job::State::RUNNING;
            job->LastUpdateTime = system::DateTime();
            jobModified = true;
         }
         else if (error)
         {
            job->Status = api::Job::State::FAILED;
            job->LastUpdateTime = system::DateTime();
            jobModified = true;
         }
         

         if (jobModified)
            saveJob(job);
      }

      out_jobs.push_back(job);
   }

   logging::logInfoMessage("Loaded " + std::to_string(out_jobs.size())  + " jobs from file");

   return Success();
}

void LocalJobRepository::onJobAdded(const api::JobPtr& in_job)
{
   saveJob(in_job);
}

void LocalJobRepository::onJobRemoved(const api::JobPtr& in_job)
{
   LOCK_JOB(in_job)
   {
      if (in_job->Host != m_hostname)
      {
         logging::logDebugMessage("Not deleting job files for job " + in_job->Id + " owned by host " + in_job->Host);
         return;
      }

      logging::logDebugMessage("Deleting job files for job: " + in_job->Id);

      FilePath jobFile = getJobFilePath(in_job->Id, m_jobsPath);
      Error error = jobFile.removeIfExists();
      if (error)
         logging::logError(error, ERROR_LOCATION);

      FilePath stdoutFile(in_job->StandardOutFile);
      FilePath stderrFile(in_job->StandardErrFile);

      if (stdoutFile.isWithin(m_outputRootPath))
         deleteFileAsUser(in_job->User, stdoutFile);
      if (stderrFile.isWithin(m_outputRootPath))
         deleteFileAsUser(in_job->User, stderrFile);
   }
   END_LOCK_JOB
}

Error LocalJobRepository::onInitialize()
{
   Error error = ensureDirectory(m_jobsRootPath);
   if (error)
      return error;

   error = ensureDirectory(m_jobsPath);
   if (error)
      return error;

   error = ensureDirectory(m_outputRootPath, FileMode::ALL_READ_WRITE_EXECUTE);
   if (error)
      return error;

   return Success();
}


} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
