/*
 * SmokeTest.cpp
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

#include <SmokeTest.hpp>

#include <iomanip>
#include <iostream>
#include <thread>

#include <api/Request.hpp>
#include <json/Json.hpp>
#include <system/Asio.hpp>

// Private SDK Includes - These are not reliable!
#include <api/Constants.hpp>
#include <comms/MessageHandler.hpp>
#include <logging/StderrLogDestination.hpp>
#include <system/PosixSystem.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace smoke_test {

typedef SmokeTestPtr SharedThis;
typedef std::weak_ptr<SmokeTest> WeakThis;

namespace {

// Force arithmetic overflow
std::atomic_uint64_t s_requestId { 0 };

constexpr char const* CLUSTER_INFO_REQ = "Get cluster info";
constexpr char const* GET_JOBS_REQ = "Get all jobs";
constexpr char const* GET_FILTERED_JOBS_REQ = "Get filtered jobs";
constexpr char const* GET_RUNNING_JOBS_REQ = "Get running jobs";
constexpr char const* GET_FINISHED_JOBS_REQ = "Get finished jobs";
constexpr char const* GET_JOB_STATUSES_REQ = "Get job statuses";
constexpr char const* SUB_JOB_1_REQ = "Submit quick job (matches filter)";
constexpr char const* SUB_JOB_2_REQ = "Submit quick job 2 (doesn't match filter)";
constexpr char const* SUB_JOB_3_REQ = "Submit long job (matches filter)";
constexpr char const* EXIT_REQ = "Exit";

typedef std::vector<std::string> Requests;

comms::MessageHandler& getMessageHandler()
{
   static comms::MessageHandler msgHandler;
   return msgHandler;
}

const Requests& getRequests()
{
   static Requests requests =
      {
         CLUSTER_INFO_REQ,
         GET_JOBS_REQ,
         GET_FILTERED_JOBS_REQ,
         GET_RUNNING_JOBS_REQ,
         GET_FINISHED_JOBS_REQ,
         GET_JOB_STATUSES_REQ,
         SUB_JOB_1_REQ,
         SUB_JOB_2_REQ,
         SUB_JOB_3_REQ,
         EXIT_REQ
      };

   return requests;
}

std::string getBootstrap()
{
   json::Object version;
   version[api::FIELD_VERSION_MAJOR] = api::API_VERSION_MAJOR;
   version[api::FIELD_VERSION_MINOR] = api::API_VERSION_MINOR;
   version[api::FIELD_VERSION_PATCH] = api::API_VERSION_PATCH;

   json::Object bootstrap;
   bootstrap[api::FIELD_REQUEST_ID] = 0;
   bootstrap[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::BOOTSTRAP);
   bootstrap[api::FIELD_VERSION] = version;

   return getMessageHandler().formatMessage(bootstrap.write());
}

std::string getClusterInfo(const system::User& in_user)
{
   json::Object clusterInfo;
   clusterInfo[api::FIELD_REQUEST_ID] = ++s_requestId;
   clusterInfo[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::GET_CLUSTER_INFO);
   clusterInfo[api::FIELD_REQUEST_USERNAME] = in_user.getUsername();
   clusterInfo[api::FIELD_REAL_USER] = in_user.getUsername();

   return getMessageHandler().formatMessage(clusterInfo.write());
}

std::string getAllJobs(const system::User& in_user)
{
   json::Object jobsReq;
   jobsReq[api::FIELD_REQUEST_ID] = ++s_requestId;
   jobsReq[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::GET_JOB);
   jobsReq[api::FIELD_REQUEST_USERNAME] = in_user.getUsername();
   jobsReq[api::FIELD_REAL_USER] = in_user.getUsername();
   jobsReq[api::FIELD_JOB_ID] = "*";

   return getMessageHandler().formatMessage(jobsReq.write());
}

std::string getFilteredJobs(const system::User& in_user)
{
   json::Array tags;
   tags.push_back("filter job");

   json::Object jobsReq;
   jobsReq[api::FIELD_REQUEST_ID] = ++s_requestId;
   jobsReq[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::GET_JOB);
   jobsReq[api::FIELD_REQUEST_USERNAME] = in_user.getUsername();
   jobsReq[api::FIELD_REAL_USER] = in_user.getUsername();
   jobsReq[api::FIELD_JOB_ID] = "*";
   jobsReq[api::FIELD_JOB_TAGS] = tags;

   return getMessageHandler().formatMessage(jobsReq.write());
}

std::string getStatusJobs(const system::User& in_user, api::Job::State in_state)
{
   json::Array status;
   status.push_back(api::Job::stateToString(in_state));

   json::Object jobsReq;
   jobsReq[api::FIELD_REQUEST_ID] = ++s_requestId;
   jobsReq[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::GET_JOB);
   jobsReq[api::FIELD_REQUEST_USERNAME] = in_user.getUsername();
   jobsReq[api::FIELD_REAL_USER] = in_user.getUsername();
   jobsReq[api::FIELD_JOB_ID] = "*";
   jobsReq[api::FIELD_JOB_STATUSES] = status;

   return getMessageHandler().formatMessage(jobsReq.write());
}

std::string streamJobStatuses(const system::User& in_user)
{
   json::Object statusReq;
   statusReq[api::FIELD_REQUEST_ID] = ++s_requestId;
   statusReq[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::GET_JOB_STATUS);
   statusReq[api::FIELD_REQUEST_USERNAME] = in_user.getUsername();
   statusReq[api::FIELD_REAL_USER] = in_user.getUsername();
   statusReq[api::FIELD_JOB_ID] = "*";

   return getMessageHandler().formatMessage(statusReq.write());
}

std::string cancelJobStream(const system::User& in_user)
{
   json::Object statusReq;
   statusReq[api::FIELD_REQUEST_ID] = s_requestId;
   statusReq[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::GET_JOB_STATUS);
   statusReq[api::FIELD_REQUEST_USERNAME] = in_user.getUsername();
   statusReq[api::FIELD_REAL_USER] = in_user.getUsername();
   statusReq[api::FIELD_JOB_ID] = "*";
   statusReq[api::FIELD_CANCEL_STREAM] = true;

   return getMessageHandler().formatMessage(statusReq.write());
}

std::string submitJobReq(const api::Job& in_job)
{
   json::Object submitJob;
   submitJob[api::FIELD_REQUEST_ID] = ++s_requestId;
   submitJob[api::FIELD_MESSAGE_TYPE] = static_cast<int>(api::Request::Type::SUBMIT_JOB);
   submitJob[api::FIELD_REQUEST_USERNAME] = in_job.User.getUsername();
   submitJob[api::FIELD_REAL_USER] = in_job.User.getUsername();
   submitJob[api::FIELD_JOB] = in_job.toJson();

   return getMessageHandler().formatMessage(submitJob.write());
}

std::string submitJob1Req(const system::User& in_user)
{
   api::Job job;
   job.User = in_user;
   job.Exe = "/bin/sh";
   job.Environment = { {"ENV_VAR", "This is an environment variable!"} };
   job.StandardIn = "#!/bin/sh\necho $ENV_VAR";
   job.Name = "Quick Job 1";
   job.Tags = { "filter job" };

   return submitJobReq(job);
}

std::string submitJob2Req(const system::User& in_user)
{
   api::Job job;
   job.User = in_user;
   job.Command = "echo";
   job.Arguments = { "This is a shell command." };
   job.Environment = { {"ENV_VAR", "This is not used!"} };
   job.Name = "Quick Job 2";
   job.Tags = { "other tag" };

   return submitJobReq(job);
}

std::string submitJob3Req(const system::User& in_user)
{
   api::Job job;
   job.User = in_user;
   job.Exe = "/bin/bash";
   job.StandardIn = "#!/bin/bash\nset -e\nfor I in 1 2 3 4 5 6 7 8 9 10 11; do\n  echo \"$I...\"\n  sleep $I\ndone";
   job.Name = "Slow job";
   job.Tags = { "filter job" };

   return submitJobReq(job);
}

} // anonymous namespace

SmokeTest::SmokeTest(system::FilePath in_pluginPath, system::User in_requestUser) :
   m_pluginPath(std::move(in_pluginPath)),
   m_exited(false),
   m_requestUser(std::move(in_requestUser))
{
}

Error SmokeTest::initialize()
{
   // Add an stderr logger.
   logging::addLogDestination(
      std::shared_ptr<logging::ILogDestination>(
         new logging::StderrLogDestination(logging::LogLevel::DEBUG)));

   // There must be at least 2 threads.
   system::AsioService::startThreads(2);

   system::process::ProcessOptions pluginOpts;
   pluginOpts.Executable = m_pluginPath.getAbsolutePath();
   pluginOpts.IsShellCommand = false;
   pluginOpts.CloseStdIn = false;
   pluginOpts.UseSandbox = false;
   pluginOpts.Arguments = { "--heartbeat-interval-seconds=0", "--enable-debug-logging=1" };
   pluginOpts.RunAsUser = system::User(true); // Don't change users - run as whoever launched this.

   if (!system::posix::realUserIsRoot())
      pluginOpts.Arguments.emplace_back("--unprivileged=1");

   system::process::AsyncProcessCallbacks callbacks;
   callbacks.OnError = [](const Error& in_error)
   {
      std::cerr << "Error occurred while communicating with plugin: " << std::endl
                << in_error.asString() << std::endl;
   };

   WeakThis weakThis = weak_from_this();
   callbacks.OnExit = [weakThis](int exitCode)
   {
      if (exitCode == 0)
         std::cout << "Plugin exited normally" << std::endl;
      else
         std::cerr << "Plugin exited with code " << exitCode << std::endl;

      if (SharedThis sharedThis = weakThis.lock())
      {
         UNIQUE_LOCK_MUTEX(sharedThis->m_mutex)
         {
            sharedThis->m_exited = true;
         }
         END_LOCK_MUTEX

         // In case anyone is waiting on the cond var, notify that exit occurred.
         sharedThis->m_condVar.notify_all();
      }
   };

   callbacks.OnStandardError = [](const std::string& in_string)
   {
      std::cerr << in_string << std::endl;
   };


   callbacks.OnStandardOutput = [weakThis](const std::string& in_string)
   {
      std::vector<std::string> messages;
      getMessageHandler().processBytes(in_string.c_str(), in_string.size(), messages);

      if (messages.empty())
      {
         std::cerr << "No messages received" << std::endl;
      }

      if (SharedThis sharedThis = weakThis.lock())
      {
         UNIQUE_LOCK_MUTEX(sharedThis->m_mutex)
         {
            for (const std::string& msg: messages)
            {
               json::Value jsonVal;
               Error error = jsonVal.parse(msg);
               if (error)
                  std::cerr << "Error parsing response from plugin: " << std::endl
                            << error.asString() << std::endl
                            << "Response: " << std::endl
                            << in_string << std::endl;
               else
                  std::cout << jsonVal.writeFormatted() << std::endl;

               uint64_t requestId = jsonVal.getObject()[api::FIELD_REQUEST_ID].getUInt64();
               if (sharedThis->m_responseCount.find(requestId) == sharedThis->m_responseCount.end())
                  sharedThis->m_responseCount[requestId] = 0;
               sharedThis->m_responseCount[requestId] += 1;
            }
         }
         END_LOCK_MUTEX

         sharedThis->m_condVar.notify_all();
      }
   };

   Error error = system::process::ProcessSupervisor::runAsyncProcess(pluginOpts, callbacks, &m_plugin);
   if (error)
      return error;

   std::cout << "Bootstrapping..." << std::endl;
   m_responseCount[0] = 0;
   error = m_plugin->writeToStdin(getBootstrap(), false);
   if (error)
      return error;

   // Wait for the response.
   if (!waitForResponse(0, 1))
   {
      error = systemError(ETIME, "Failed to bootstrap plugin", ERROR_LOCATION);
   }

   return error;
}

bool SmokeTest::sendRequest()
{
   if (m_exited)
      return false;

   std::cout << std::endl << "Actions:" << std::endl;

   const auto& requests = getRequests();
   for (int i = 0; i < requests.size(); ++i)
      std::cout << "  " << std::setw(2) << (i + 1) << std::setw(1) << ". " << requests[i] << std::endl;


   std::cout << std::endl << "Enter a number: ";
   std::string line;
   std::getline(std::cin, line);

   if (std::cin.bad())
   {
      std::cout << std::endl;
      logging::logError(systemError(EIO, "Received bad bit on std::cin", ERROR_LOCATION));
      return false;
   }
   else if (std::cin.eof()) // Operation canceled by user.
   {
      std::cout << std::endl;
      return false;
   }

   if (m_exited)
   {
      std::cerr << "Plugin exited unexpectedly. Shutting down..." << std::endl;
      return false;
   }

   int choice = -1;
   try
   {
       choice = std::stoi(line);
   }
   catch (...)
   {
      std::cout << "Invalid choice (" << line << "). Please enter a positive integer." << std::endl;
   }

   bool success = true;
   if (choice > 0)
   {
      const std::string& request = requests[choice - 1];
      if (request == EXIT_REQ)
      {
         m_plugin->writeToStdin("", true);
         return false;
      }
      else
      {
         bool jobStreamRequest = false;
         std::string message;
         uint64_t targetResponses = 1;
         if (request == CLUSTER_INFO_REQ)
            message = getClusterInfo(m_requestUser);
         if (request == GET_JOBS_REQ)
            message = getAllJobs(m_requestUser);
         if (request == GET_FILTERED_JOBS_REQ)
            message = getFilteredJobs(m_requestUser);
         if (request == GET_RUNNING_JOBS_REQ)
            message = getStatusJobs(m_requestUser, api::Job::State::RUNNING);
         if (request == GET_FINISHED_JOBS_REQ)
            message = getStatusJobs(m_requestUser, api::Job::State::FINISHED);
         if (request == GET_JOB_STATUSES_REQ)
         {
            message = streamJobStatuses(m_requestUser);
            jobStreamRequest = true;
         }
         if (request == SUB_JOB_1_REQ)
            message = submitJob1Req(m_requestUser);
         if (request == SUB_JOB_2_REQ)
            message = submitJob2Req(m_requestUser);
         if (request == SUB_JOB_3_REQ)
            message = submitJob3Req(m_requestUser);

         UNIQUE_LOCK_MUTEX(m_mutex)
         {
            if (!jobStreamRequest)
               m_responseCount[s_requestId] = 0;
            else
               m_responseCount[0] = 0;
         }
         END_LOCK_MUTEX

         Error error = m_plugin->writeToStdin(message, false);
         if (error)
         {
            std::cerr << "Error communicating with plugin." << std::endl;
            logging::logError(error);
            return false;
         }

         if (jobStreamRequest)
            success = waitForResponse(0, targetResponses);
         else
            success = waitForResponse(s_requestId, targetResponses);

         // Job status stream request might not return anything. Just keep trying.
         if (!success && jobStreamRequest)
         {
            std::cout << "No job status stream response returned. Are there any jobs?" << std::endl;
            success = true;
         }

         if (!success)
            return success;

         if (jobStreamRequest)
         {
            message = cancelJobStream(m_requestUser);
            error = m_plugin->writeToStdin(message, false);
            if (error)
            {
               std::cerr << "Error communicating with plugin." << std::endl;
               logging::logError(error);
               return false;
            }
            // Wait for half a second to ensure the stream has time to finish.
            std::this_thread::sleep_for(std::chrono::microseconds(500));
         }
      }
   }

   return !m_exited && success;
}

void SmokeTest::stop()
{
   m_exited = true;
   system::process::ProcessSupervisor::terminateAll();
   system::process::ProcessSupervisor::waitForExit(system::TimeDuration::Seconds(30));
   system::AsioService::stop();
   system::AsioService::waitForExit();
}

bool SmokeTest::waitForResponse(uint64_t in_requestId, uint64_t in_expectedResponses)
{
   // Wait up to 30 seconds for a response
   UNIQUE_LOCK_MUTEX(m_mutex)
   {
      std::cv_status stat = std::cv_status::no_timeout;
      uint64_t responseCount = m_responseCount[in_requestId];
      while ((stat != std::cv_status::timeout) && (responseCount < in_expectedResponses) && !m_exited)
      {
         stat = m_condVar.wait_for(uniqueLock, std::chrono::seconds(30));

         // If we're waiting for multiple responses and we've received at least one in the last 30 seconds, keep
         // waiting.
         if (m_responseCount[in_requestId] > responseCount)
            stat = std::cv_status::no_timeout;

         responseCount = m_responseCount[in_requestId];
      }

      if (stat == std::cv_status::timeout)
      {
         std::cerr << "Timed out waiting for response." << std::endl;
         return false;
      }
   }
   END_LOCK_MUTEX

   return true;
}

} // namespace smoke_test
} // namespace launcher_plugins
} // namespace rstudio
