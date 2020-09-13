/*
 * SmokeTest.hpp
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

#ifndef LAUNCHER_PLUGINS_SMOKETEST_HPP
#define LAUNCHER_PLUGINS_SMOKETEST_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <Error.hpp>
#include <api/Request.hpp>
#include <system/FilePath.hpp>
#include <system/Process.hpp>
#include <system/Asio.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace smoke_test {

/**
 * @brief Enables manual plugin testing.
 */
class SmokeTest : public std::enable_shared_from_this<SmokeTest>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_pluginPath      The path to the Plugin to be tested.
    * @param in_requestUser     The user to send requests for.
    */
   SmokeTest(system::FilePath in_pluginPath, system::User in_requestUser);

   /**
    * @brief Initializes the smoke tester, including starting threads and bootstrapping the plugin.
    *
    * @return Success if the smoke tester could be initialized; Error otherwise.
    */
   Error initialize();

   /**
    * @brief Prints the action menu and handles user input.
    *
    * @return False if the test application should exit; true if it should continue.
    */
   bool sendRequest();

   /**
    * @brief Stops the plugin and joins all threads.
    */
   void stop();

private:
   bool sendControlJobReqeust(api::ControlJobRequest::Operation in_operation);
   
   /**
    * @brief Sends a job output stream request and waits for the response(s).
    *
    * @param in_outputType      The type of output to stream.
    *
    * @return True if the plugin responded as expected; false otherwise.
    */
   bool sendJobOutputStreamRequest(api::OutputType in_outputType);

   /**
    * @brief Sends a job status stream request and waits for the response(s).
    *
    * @return True if the plugin responded as expected; false otherwise.
    */
   bool sendJobStatusStreamRequest();

   bool sendKillJobReqeust();
   bool sendSupsendResumeJobReqeust();

   /**
    * @brief Waits for the specified number of responses for the specified request.
    *
    * @param in_requestId               The ID of the request for which to wait for responses.
    * @param in_expectedResponses       The minimum number of responses to wait for.
    *
    * @return True if at least in_expected responses were received; false if the operation timed out.
    */
   bool waitForResponse(uint64_t in_requestId, uint64_t in_expectedResponses);

   /**
    * @brief Waits for the specified number of responses for the specified request.
    *
    * @param in_requestId               The ID of the request for which to wait for responses.
    * @param in_expectedResponses       The minimum number of responses to wait for.
    * @param in_lock                    The owned lock, if it is already held by this thread.
    *
    * @return True if at least in_expected responses were received; false if the operation timed out.
    */
   bool waitForResponse(
      uint64_t in_requestId,
      uint64_t in_expectedResponses,
      std::unique_lock<std::mutex>& in_lock);

   std::shared_ptr<system::process::AbstractChildProcess> m_plugin;
   system::FilePath m_pluginPath;
   bool m_exited;
   std::map<uint64_t, uint64_t> m_responseCount;
   system::User m_requestUser;
   std::mutex m_mutex;
   std::condition_variable m_condVar;
   std::vector<std::string> m_submittedJobIds;
   api::Request::Type m_lastRequestType;

   bool m_outputStreamFinished;
};

typedef std::shared_ptr<SmokeTest> SmokeTestPtr;

} // namespace smoke_test
} // namespace launcher_plugins
} // namespace rstudio

#endif
