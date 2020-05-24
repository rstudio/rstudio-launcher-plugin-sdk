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

/**
 * @brief Callbacks that will be invoked when certain events happen in the asynchronous child process.
 */
struct AsyncProcessCallbacks
{
   /**
    * @brief Callback invoked if the asynchronous child process encounters an error.
    */
   OnErrorCallback OnError;

   /**
    * @brief Callback invoked when the asynchronous child process exits.
    */
   OnExitCallback OnExit;

   /** @brief Callback invoked when the asynchronous child process writes to standard error. */
   OnOutputCallback OnStandardError;

   /** @brief Callback invoked when the asynchronous child process writes to standard out. */
   OnOutputCallback OnStandardOutput;
};

/**
 * @brief Defines a process that can be run.
 */
struct ProcessOptions
{
   /**
    * @brief Constructor.
    */
   ProcessOptions() : IsShellCommand(false) {};

   /**
    * @brief The arguments of the process. Each argument will be escaped using single quotations so that the values are
    *        always interpreted literally. No expansion of environment variables or backslashes will be performed.
    */
   std::vector<std::string> Arguments;

   /**
    * @brief The environment variables which should available to the process. If PATH is not set, it will be added to
    *        the environment with the same value as the PATH of this process.
    */
   api::EnvironmentList Environment;

   /**
    * @brief The executable or shell command to run. This value should either be an absolute path, or it should be
    *        located in one of the locations in the PATH environment variable.
    */
   std::string Executable;

   /**
    * @brief True if the executable is a shell command; False otherwise. Default: false.
    */
   bool IsShellCommand;

   /**
    * @brief The set of mounts to be applied for the child process. Only mounts with a HostMountSource type will be
    *        applied. All other mounts will be ignored.
    */
   api::MountList Mounts;

   /**
    * @brief The PAM profile to load, if any.
    */
   std::string PamProfile;

   /**
    * @brief The password of the user running the job, if any.
    */
   std::string Password;

   /**
    * @brief The user who the job should be run as. The job may fail to run if the RunAsUser is "all users" or empty.
    */
   User RunAsUser;

   /**
    * @brief The standard input that should be sent to the process.
    */
   std::string StandardInput;

   /**
    * @brief The file to which to write standard output. If not set, standard output will not be redirected.
    */
   FilePath StandardOutputFile;

   /**
    * @brief The file to which to write standard error. If not set, standard error will not be redirected.
    */
   FilePath StandardErrorFile;

   /**
    * @brief The directory from which to run the process. Must exist and be accessible by the RunAsUser.
    */
   FilePath WorkingDirectory;
};

/**
 * @brief Base class for a child process, which will be launched via rsandbox.
 */
class AbstractChildProcess : public Noncopyable
{
public:
   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~AbstractChildProcess() = default;

   /**
    * @brief Gets the PID of this child process.
    *
    * @return The PID of this child process.
    */
   pid_t getPid() const;

   /**
    * @brief Terminates the child process.
    *
    * @return Success if the child process was terminated; Error otherwise.
    */
   Error terminate();

protected:
   /**
    * @brief Constructor.
    *
    * @param in_options     The options for the child process.
    */
   explicit AbstractChildProcess(const ProcessOptions& in_options);

   /**
    * @brief Forks and executes the child process, passing along arguments and environment variables.
    *
    * @return Success if the child process could be started; Error otherwise.
    */
   Error run();

   // The private implemenation of AbstractChildProcess.
   PRIVATE_IMPL(m_baseImpl);
};

/**
 * @brief A blocking child process.
 */
class SyncChildProcess final : public AbstractChildProcess
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_options     The options for the child process.
    */
   explicit SyncChildProcess(const ProcessOptions& in_options);

   /**
    * @brief Runs the child process, blocking until it completes.
    *
    * @param out_result     The process result.
    *
    * @return Success if the child process could be started; Error otherwise.
    */
   Error run(ProcessResult& out_result);
};

class AsyncChildProcess
{
public:
   explicit AsyncChildProcess(const ProcessOptions& in_process);

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
