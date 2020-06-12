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
 * @brief Represents the details of a process that is running on this machine.
 */
struct ProcessInfo
{
   /**
    * @brief Constructor.
    */
   ProcessInfo();

   /**
    * @brief Gets the process information for the process with the specified PID.
    *
    * @param in_pid         The PID of the process for which to retrieve the details.
    * @param out_info       The details of the specified process, if no error occurs.
    *
    * @return Success if the process information could be retrieved; Error otherwise.
    */
   static Error getProcessInfo(pid_t in_pid, ProcessInfo& out_info);

   /** The arguments that were passed to the process. */
   std::vector<std::string> Arguments;

   /** The executable that was run. */
   std::string Executable;

   /** The process' owner. */
   User Owner;

   /** The process group ID of the process. */
   pid_t PGrp;

   /** The PID of the process. */
   pid_t Pid;

   /** The PID of the process' parent process. */
   pid_t PPid;

   /** The current state of the process. */
   std::string State;
};

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
   ProcessOptions() :
      CloseStdIn(true),
      IsShellCommand(false),
      UseRSandbox(true)
   {};

   /**
    * @brief The arguments of the process. Each argument will be escaped using single quotations so that the values are
    *        always interpreted literally. No expansion of environment variables or backslashes will be performed.
    */
   std::vector<std::string> Arguments;

   /**
    * @brief Whether to close write end of the standard input stream after the specified StandardInput is written.
    *        Default: true.
    *
    * If UseRSandbox is true, this value will be ignored and treated as true.
    */
   bool CloseStdIn;

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
    *
    * Mounts will be ignored if UseRSandbox is false.
    */
   api::MountList Mounts;

   /**
    * @brief The PAM profile to load, if any.
    *
    * PamProfile will be ignored if UseRSandbox is false.
    */
   std::string PamProfile;

   /**
    * @brief The password of the user running the job, if any.
    *
    * Password will be ignored if UseRSandbox is false.
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
    * @brief Whether to use RSandbox or launch the child process directly.
    *
    * If this value is true, CloseStdIn will be ignored and treated as true.
    *
    * The following values will be ignored if UseRSandbox is false:
    *       - Mounts
    *       - PamProfile
    *       - Password
    */
   bool UseRSandbox;

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
   virtual Error terminate();

   /**
    * @brief Writes the specified string to stdin.
    *
    * @param in_string      The data to write to stdin.
    * @param in_eof         True if this is the last data to write to stdin.
    *
    * @return Success if the data could be written; Error otherwise.
    */
   virtual Error writeToStdin(const std::string& in_string, bool in_eof) = 0;

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

   // The private implementation of AbstractChildProcess.
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

   /**
    * @brief Writes the specified string to stdin.
    *
    * @param in_string      The data to write to stdin.
    * @param in_eof         True if this is the last data to write to stdin.
    *
    * @return Success if the data could be written; Error otherwise.
    */
   Error writeToStdin(const std::string& in_string, bool in_eof) override;
};

/**
 * @brief Creates and manages non-blocking child processes.
 */
class ProcessSupervisor final : public Noncopyable
{
public:

   /**
    * @brief Checks whether the supervisor is tracking any processes which have not exited yet.
    *
    * @return True if there are any children running; false otherwise.
    */
   static bool hasRunningChildren();

   /**
    * @brief Runs a child process asynchronously.
    *
    * @param in_options         The options for the child process.
    * @param in_callbacks       The callbacks to invoke when output is written, an error occurs, or the process exits.
    * @param out_childProcess   The child process, if no error occurs on startup.
    *
    * @return Success if the child process could be started; Error otherwise.
    */
   static Error runAsyncProcess(
      const ProcessOptions& in_options,
      const AsyncProcessCallbacks& in_callbacks,
      std::shared_ptr<AbstractChildProcess>* out_childProcess = nullptr);

   /**
    * @brief Terminates all running children forcefully.
    */
   static void terminateAll();

   /**
    * @brief Waits for all child processes to exit.
    *
    * @param in_maxWaitTime     The maximum amount of time to wait for the child processes to exit. Default: no limit.
    *
    * @return True if the function exited because the timeout was reached; false if the function exited because all
    *         child processes exited.
    */
   static bool waitForExit(const TimeDuration& in_maxWaitTime = TimeDuration::Infinity());

private:
   /**
    * @brief Gets the single Process Supervisor for this process.
    *
    * @return The single Process Supervisor for this process.
    */
   static ProcessSupervisor& getInstance();

   /**
    * @brief Constructor.
    */
   ProcessSupervisor();

   // The private implementation of ProcessSupervisor.
   PRIVATE_IMPL_SHARED(m_impl);
};

/**
 * @brief Shell escapes a string.
 *
 * @param in_toEscape   The string to escape.
 *
 * @return The escaped string.
 */
std::string shellEscape(const std::string& in_toEscape);

/**
 * @brief Shell escapes a FilePath.
 *
 * @param in_filePath   The FilePath to escape.
 *
 * @return The escaped FilePath, as a string.
 */
std::string shellEscape(const FilePath& in_filePath);

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
