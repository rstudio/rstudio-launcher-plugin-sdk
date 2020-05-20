/*
 * Process.hpp
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


#ifndef LAUNCHER_PLUGINS_PROCESS_HPP
#define LAUNCHER_PLUGINS_PROCESS_HPP

#include <Noncopyable.hpp>

#include <functional>
#include <string>
#include <vector>


#include <api/Job.hpp>
#include <system/FilePath.hpp>
#include <system/DateTime.hpp>
#include <system/User.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

typedef std::function<void(const Error&)> OnErrorCallback;
typedef std::function<void(int)> OnExitCallback;
typedef std::function<void(const std::string&)> OnOutputCallback;

/**
 * @brief Represents the result of a synchronous child process.
 */
struct ProcessResult
{
   /**
    * @brief Constructor.
    */
   ProcessResult() : ExitCode(0) {};

   /** The exit code of the process. */
   int ExitCode;

   /** Standard error output from the process. */
   std::string StdError;

   /** Standard output from the process. */
   std::string StdOut;
};

struct AsyncProcessCallbacks
{
   OnErrorCallback OnError;

   OnExitCallback OnExit;

   OnOutputCallback OnStandardError;

   OnOutputCallback OnStandardOutput;
};

struct ProcessOptions
{
   ProcessOptions() : IsShellCommand(false) {};

   std::vector<std::string> Arguments;

   api::EnvironmentList Environment;

   std::string Executable;

   bool IsShellCommand;

   api::MountList Mounts;

   std::string PamProfile;

   std::string Password;

   User RunAsUser;

   std::string StandardInput;

   FilePath StandardOutputFile;

   FilePath StandardErrorFile;

   FilePath WorkingDirectory;
};

class AbstractChildProcess : public Noncopyable
{
public:
   virtual ~AbstractChildProcess() = default;

   pid_t getPid() const;

   Error terminate();

protected:
   explicit AbstractChildProcess(const ProcessOptions& in_program);

   Error run();

   PRIVATE_IMPL(m_baseImpl);
};

class SyncChildProcess final : public AbstractChildProcess
{
public:
   explicit SyncChildProcess(const ProcessOptions& in_process);

   Error run(ProcessResult& out_result);
};

class AsyncChildProcess
{
public:
   AsyncChildProcess(const ProcessOptions& in_process);

   Error run(AsyncProcessCallbacks& out_result);

private:
   PRIVATE_IMPL(m_impl);
};

class ProcessSupervisor final : public Noncopyable
{
   static ProcessSupervisor& getInstance();

   bool hasRunningChildren() const;

   Error runProcess(
      const ProcessOptions& in_process,
      const AsyncProcessCallbacks& in_callbacks,
      AsyncChildProcess* out_childProcess);

   void terminateAllChildren(bool in_forceKill);

   bool waitForExit(const TimeDuration& in_maxWaitTime = TimeDuration::Infinity());
};

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
