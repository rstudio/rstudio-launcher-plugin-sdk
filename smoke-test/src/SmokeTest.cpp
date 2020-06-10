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

#include <iostream>

#include <json/Json.hpp>
#include <options/Options.hpp>
#include <system/Asio.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace smoke_test {

namespace {

Error readOptions()
{
   std::vector<char> buffer;
   buffer.resize(2048);
   ssize_t len = ::readlink("/proc/self/exe", &buffer[0], buffer.size());
   
   if (len < 0)
      return systemError(errno, "Failed to read path to self.", ERROR_LOCATION);
   
   std::string selfPathStr(&buffer[0], len);
   if (len == buffer.size())
      return Error(
         "TruncationError",
         1,
         "Self path was truncated: " + selfPathStr,
         ERROR_LOCATION);
   
   system::FilePath selfPath(selfPathStr);
   return options::Options::getInstance().readOptions(
      0,
      {},
      selfPath.getParent().completeChildPath("smoke-test.conf"));
}

} // anonymous namespace

SmokeTest::SmokeTest(system::FilePath in_pluginPath) :
   m_pluginPath(std::move(in_pluginPath)),
   m_isRunning(false)
{
}

Error SmokeTest::initialize()
{
   // There must be at least 2 threads.
   system::AsioService::startThreads(2);

   // Read options.
   Error error = readOptions();
   if (error)
      return error;

   system::process::ProcessOptions pluginOpts;
   error = options::Options::getInstance().getServerUser(pluginOpts.RunAsUser);
   if (error)
      return error;

   pluginOpts.Executable = m_pluginPath.getAbsolutePath();
   pluginOpts.IsShellCommand = false;
   pluginOpts.CloseStdin = false;
   pluginOpts.Arguments = { "--heartbeat-interval-seconds=0", "--enable-debug-logging=1" };

   system::process::AsyncProcessCallbacks callbacks;
   callbacks.OnError = [](const Error& in_error)
   {
      std::cerr << "Error occurred while communicating with plugin: " << std::endl
                << in_error.asString() << std::endl;
   };

   std::atomic_bool& isRunning = m_isRunning;
   callbacks.OnExit = [&isRunning](int exitCode)
   {
      if (isRunning.load())
      {
         if (exitCode == 0)
            std::cout << "Plugin exited normally" << std::endl;
         else
            std::cerr << "Plugin exited with code " << exitCode << std::endl;

         isRunning.exchange(false);
      }
   };

   callbacks.OnStandardError = [](const std::string& in_string)
   {
      std::cerr << in_string << std::endl;
   };

   std::atomic_bool& responseReceived = m_responseReceived;
   callbacks.OnStandardOutput = [&responseReceived](const std::string& in_string)
   {
      json::Value jsonVal;
      Error error = jsonVal.parse(in_string);
      if (error)
         std::cerr << "Error parsing response from plugin: " << std::endl
                   << error.asString() << std::endl
                   << "Response: " << std::endl
                   << in_string << std::endl;
      else
         std::cout << jsonVal.writeFormatted() << std::endl;

      responseReceived.exchange(true);
   };

   error = system::process::ProcessSupervisor::runAsyncProcess(pluginOpts, callbacks, &m_plugin);
   return error;
}

bool SmokeTest::sendRequest()
{
   return false;
}

void SmokeTest::stop()
{
   m_isRunning.exchange(false);
   system::process::ProcessSupervisor::terminateAll();
   system::process::ProcessSupervisor::waitForExit(system::TimeDuration::Seconds(30));
   system::AsioService::stop();
   system::AsioService::waitForExit();
}

} // namespace smoke_test
} // namespace launcher_plugins
} // namespace rstudio
