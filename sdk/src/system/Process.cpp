/*
 * Process.cpp
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

#include <system/Process.hpp>

#include <condition_variable>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <json/Json.hpp>
#include <options/Options.hpp>
#include <system/Asio.hpp>

#include "PosixSystem.hpp"
#include "../utils/ErrorUtils.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

namespace {

// Exit code for when a thread-safe spawn fails. Chosen to be something "unique" enough to identify since thread-safe
// forks cannot log until ::execve is invoked (because mutexes cannot be acquired before that point).
const int s_threadSafeExitError = 153;

// Pipe Constants
const int s_readPipe = 0;
const int s_writePipe = 1;

/**
 * @brief Structure that holds the FDs of the pipes that will be opened for parent/child communication.
 */
struct FileDescriptors
{
   /** The FDs of the pipe that will be used for standard input transfer (parent -> child). */
   int Input[2] = { 0, 0 };

   /** The FDs of the pipe that will be used for standard output transfer (parent <- child). */
   int Output[2] = { 0, 0 };

   /** The FDs of the pipe that will be used for standard error transfer (parent <- child). */
   int Error[2] = { 0, 0 };

   /** The FDs of the pipe that will be used for open FD transfer (parent -> child). */
   int CloseFd[2] = { 0, 0 };
};

/**
 * @brief A memory managed list of C-style strings.
 */
class CStringList final
{
public:
   /**
    * @brief Constructor.
    */
   CStringList() :
      m_length(0),
      m_data(nullptr)
   {
   }

   /**
    * @brief Constructor.
    *
    * @param in_vector      The C++-style vector of strings to be converted to a C-style list of strings.
    */
   explicit CStringList(const std::vector<std::string>& in_vector) :
      CStringList()
   {
      setData(in_vector);
   }

   /**
    * @brief Destructor.
    */
   ~CStringList() noexcept
   {
      try
      {
         free();
      }
      catch (...)
      {
         // Swallow exceptions for an exception-safe destructor.
      }
   }

   /**
    * @brief Gets a pointer to the list of strings.
    *
    * @return The list of strings.
    */
   char** getData() const
   {
      return !isEmpty() ? m_data : nullptr;
   }

   /**
    * @brief Gets the number of elements in the list of strings.
    *
    * @return The number of elements in the list of strings.
    */
   size_t getSize() const
   {
      return m_length;
   }

   /**
    * @brief Checks whether this list of strings is empty or not.
    *
    * @return True if this list of strings is empty; False otherwise.
    */
   bool isEmpty() const
   {
      return m_length == 0;
   }

private:
   /**
    * @brief Clears the buffer and populates it with the new vector of strings.
    *
    * @param in_vector      The vector of strings with which to populate the buffer.
    */
   void setData(const std::vector<std::string>& in_vector)
   {
      free();

      m_length = in_vector.size();
      m_data = new char*[m_length + 1];

      for (size_t i = 0; i < m_length; ++i)
      {
         const std::string& str = in_vector[i];
         m_data[i] = new char[str.size() + 1];
         str.copy(m_data[i], str.size());

         // Null terminate all strings.
         m_data[i][str.size()] = '\0';
      }

      // Null terminate the list of strings.
      m_data[m_length] = nullptr;
   }

   /**
    * @brief Frees the buffer.
    */
   void free()
   {
      if (m_data != nullptr)
      {
         for (size_t i = 0; i < m_length; ++i)
            delete[] m_data[i];
      }

      delete[] m_data;
   }

   /** The number of elements in m_data. */
   size_t m_length;

   /** The buffer to hold the list of strings. */
   char** m_data;
};

/**
 * @brief Clears the signal mask of the current process.
 *
 * @return 0 on success; an error number otherwise.
 */
int clearSignalMask()
{
   sigset_t emptyMask;
   sigemptyset(&emptyMask);
   return ::pthread_sigmask(SIG_SETMASK, &emptyMask, nullptr);
}

/**
 * @brief Closes the specified file descriptor.
 *
 * @param in_fd     The file descriptor to close.
 */
void closeFd(uint32_t in_fd)
{
   // Keep trying to close the file descriptor if the operation was interrupted. Otherwise, an error means the
   // FD isn't open so just ignore it and exit.
   while (::close(in_fd) == -1)
   {
      if (errno != EINTR)
         break;
   }
}

/**
 * @brief Closes the specified pipe file descriptor.
 *
 * @param in_pipeFd             The FD of the pipe to close.
 * @param in_errorLocation      The calling location.
 */
void closePipe(int in_pipeFd, const ErrorLocation& in_errorLocation)
{
   Error error = posix::posixCall<int>(std::bind(::close, in_pipeFd), in_errorLocation);
   if (error)
      logging::logError(error);
}

/**
 * @brief Closes both ends of the specified pie.
 *
 * @param in_pipeFds            Both of the FDs of the pipe to close.
 * @param in_errorLocation      The calling location.
 */
void closePipe(int* in_pipeFds, const ErrorLocation& in_errorLocation)
{
   closePipe(in_pipeFds[s_readPipe], in_errorLocation);
   closePipe(in_pipeFds[s_writePipe], in_errorLocation);
}

/**
 * @brief Closes the file descriptors that were inherited from the parent process.
 *
 * @param in_pipeFd     The FD of the read-end of the CloseFD pipe.
 * @param in_maxFd      The maximum possible FD. Used if there is a problem reading the list of open FDs from the
 *                      CloseFD pipe.
 */
void closeParentFds(int in_pipeFd, rlim_t in_maxFd)
{
   // The parent process will send its open FDs on in_pipeFd. Read them and close them (except the pipe).
   static const uint32_t startFd = STDERR_FILENO + 1;
   bool error = false;
   bool fdsRead = false;

   int32_t buffer = -2;
   while (true)
   {
      ssize_t bytesRead = ::read(in_pipeFd, &buffer, 4);

      if (bytesRead <= 0)
      {
         if ((errno != EINTR) && (errno != EAGAIN))
         {
            error = true;
            break;
         }

         continue;
      }

      // The parent sends -1 after all other FDs have been sent.
      if (buffer == -1)
         break;

      fdsRead = true;
      uint32_t fd = static_cast<uint32_t>(buffer);

      if ((fd >= startFd) && (fd < in_maxFd) && (buffer != in_pipeFd))
         closeFd(fd);
   }

   // If we didn't manage to read the file descriptors, loop through all the FDs (besides the current pipe, STDIN,
   // STDOUT, and STDERR) and close them all. This is much slower,
   if (error || !fdsRead)
   {
      // Get the max fd, or a default of 1024 (the default maximum on linux) if the maximum is "infinity".
      uint32_t maxFd = (in_maxFd == RLIM_INFINITY) ? 1024 : in_maxFd;
      for (uint32_t fd = startFd; fd < maxFd; ++fd)
         if (fd != in_pipeFd)
            closeFd(fd);
   }
}

/**
 * @brief Creates the pipes that will be needed for parent/child communications.
 *
 * @param out_fds   The created FDs.
 *
 * @return Success if all of the FDs could be created; Error otherwise.
 */
Error createPipes(FileDescriptors& out_fds)
{
   Error error = posix::posixCall<int>(std::bind(::pipe, out_fds.Input), ERROR_LOCATION);
   if (error)
      return error;

   error = posix::posixCall<int>(std::bind(::pipe, out_fds.Output), ERROR_LOCATION);
   if (error)
   {
      closePipe(out_fds.Input, ERROR_LOCATION);
      return error;
   }

   error = posix::posixCall<int>(std::bind(::pipe, out_fds.Error), ERROR_LOCATION);
   if (error)
   {
      closePipe(out_fds.Input, ERROR_LOCATION);
      closePipe(out_fds.Output, ERROR_LOCATION);
      return error;
   }

   error = posix::posixCall<int>(std::bind(::pipe, out_fds.CloseFd), ERROR_LOCATION);
   if (error)
   {
      closePipe(out_fds.Input, ERROR_LOCATION);
      closePipe(out_fds.Output, ERROR_LOCATION);
      closePipe(out_fds.Error, ERROR_LOCATION);
      return error;
   }

   return error;
}

/**
 * @brief Shell escapes a string.
 *
 * @param in_string     The string to escape.
 *
 * @return The escaped string.
 */
std::string escape(const std::string& in_string)
{
   boost::regex pattern("'");
   return "'" + boost::regex_replace(in_string, pattern, R"('"'"')") + "'";
}

/**
 * @brief Shell escapes a FilePath.
 *
 * @param in_filePath   The FilePath to escape.
 *
 * @return The escaped FilePath, as a string.
 */
std::string escape(const FilePath& in_filePath)
{
   return escape(in_filePath.getAbsolutePath());
}

/**
 * @brief Gets the process exit code from its exit status.
 *
 * @param in_status     The status returned by the process on exit.
 *
 * @return The appropriate exit code, based on the status.
 */
int getExitCodeFromStatus(int in_status)
{
   return WIFEXITED(in_status) ? WEXITSTATUS(in_status) : in_status;
}

/**
 * @brief Gets the system limits for FDs.
 *
 * @param out_softLimit     The soft FD limit.
 * @param out_hardLimit     The hard FD limit.
 *
 * @return Success if the limits could be retrieved; Error otherwise.
 */
Error getFilesLimit(rlim_t& out_softLimit, rlim_t& out_hardLimit)
{
   struct rlimit fileLimit;
   if (::getrlimit(RLIMIT_NOFILE, &fileLimit))
      return systemError(errno, ERROR_LOCATION);

   out_softLimit = fileLimit.rlim_cur;
   out_hardLimit = fileLimit.rlim_max;
   return Success();
}

/**
 * @brief Get the list of open FDs for the specified process.
 *
 * @param in_pid        The ID of the process for which to retrieve its FDs.
 * @param out_fds       The list of open FDs for the specified process.
 *
 * @return Success if the FDs could be read; Error otherwise.
 */
Error getOpenFds(pid_t in_pid, std::vector<uint32_t>& out_fds)
{
   std::string pidStr = std::to_string(in_pid);
   FilePath filePath("/proc/" + pidStr + "/fd");

   // note: we use a FileScanner to list the pids instead of using boost
   // (FilePath class), because there is a bug in boost filesystem where
   // directory iterators can segfault under heavy load while reading the /proc filesystem
   // there aren't many details on this, but see https://svn.boost.org/trac10/ticket/10450

   // Scan the directory non-recursively.
   struct dirent** nameList;
   int childCount = ::scandir(
      filePath.getAbsolutePath().c_str(),
      &nameList,
      [](const struct dirent* in_entry)
      {
         return ((::strcmp(in_entry->d_name, ".") == 0) || (::strcmp(in_entry->d_name, "..") == 0)) ? 0 : 1;
      },
      [](const dirent** in_lhs, const dirent** in_rhs)
      {
         return ::strcmp((*in_lhs)->d_name, (*in_rhs)->d_name);
      });

   if (childCount < 0)
   {
      Error error = systemError(errno, ERROR_LOCATION);
      error.addProperty("path", filePath.getAbsolutePath());
      return error;
   }

   // Iterate over the names, converting them to uint32_t values. If they can't be converted to an int, just skip them.
   for (int i = 0; i < childCount; ++i)
   {
      try
      {
         uint32_t fd = std::strtoul(nameList[i]->d_name, nullptr, 10);
         out_fds.push_back(fd);
      }
      catch (...)
      {
         // Swallow exceptions - they don't matter here.
      }

      // Free the value.
      ::free(nameList[i]);
   }

   // Free the whole name list.
   ::free(nameList);

   return Success();
}

/**
 * @brief Reads a string from the specified pipe.
 *
 * @param in_fd         The FD of the pipe to read from.
 * @param out_data      The data that was read.
 * @param out_eof       Denotes whether there is more data to read (false) or not (true). Stands of end of file.
 *
 * @return Success if the pipe could be read from; Error otherwise.
 */
Error readFromPipe(int in_fd, std::string& out_data, bool* out_eof = nullptr)
{
   if (out_eof != nullptr)
      *out_eof = false;

   static const size_t bufferSize = 512;
   char buffer[bufferSize];
   size_t bytesRead = posix::posixCall<size_t>(std::bind(::read, in_fd, buffer, bufferSize));

   while (true)
   {
      // check for error
      if (bytesRead == -1)
      {
         if (errno == EAGAIN) // carve-out for O_NONBLOCK pipes
            return Success();
         else
            return systemError(errno, ERROR_LOCATION);
      }
      // check for eof
      else if (bytesRead == 0)
      {
         if (out_eof)
            *out_eof = true;

         return Success();
      }

      // Otherwise, we read some data.
      out_data.append(buffer, bytesRead);
      bytesRead = posix::posixCall<size_t>(std::bind(::read, in_fd, buffer, bufferSize));
   }

   return Success();
}

/**
 * @brief Sends a list of file descriptors to the specified pipe FD.
 *
 * @param in_fd     The FD of the pipe to which to write the list of open FDs.
 * @param in_pid    The PID of the process for which to list FDs.
 *
 * @return Success if the FDs could be read and written to the pipe; Error otherwise.
 */
Error sendFileDescriptors(int in_fd, pid_t in_pid)
{
   size_t bytesWritten = 0;

   // Get list of FileDescriptors open in the child process.
   std::vector<uint32_t> openFds;
   Error error = getOpenFds(in_pid, openFds);
   if (error)
      logging::logError(error);
   else
   {
      // Write them to the closeFd.
      for (uint32_t openFd : openFds)
      {
         error = posix::posixCall<size_t>(
            std::bind(::write, in_fd, &openFd, 4),
            ERROR_LOCATION,
            &bytesWritten);

         if (error)
            return error;
      }
   }

   int streamEnd = -1;
   Error lastWriteError = posix::posixCall<size_t>(
      std::bind(::write, in_fd, &streamEnd, 4),
      ERROR_LOCATION,
      &bytesWritten);

   if (lastWriteError)
      return error;

   return Success();
}

/**
 * @brief Sets the non-blocking flag on the specified pipe FD.
 *
 * @param in_pipeFd     The FD of the pipe to make non-blocking.
 */
void setPipeNonBlocking(int in_pipeFd)
{
   int flags = ::fcntl(in_pipeFd, F_GETFL);
   if ( (flags != -1) && !(flags & O_NONBLOCK) )
      ::fcntl(in_pipeFd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * @brief Writes the specified data to the given FD.
 *
 * @param in_fd         The FD of the pipe to which to write.
 * @param in_data       The data to write.
 * @param in_eof        Indicates whether this is the last data to write (true) or not (false). Stands for end of file.
 *
 * @return Success if all of the data could be written. Error otherwise.
 */
Error writeToPipe(int in_fd, const std::string& in_data, bool in_eof)
{
   size_t bytesWritten = 0;

   Error error = posix::posixCall<size_t>(
      std::bind(::write, in_fd, in_data.c_str(), in_data.length()),
      ERROR_LOCATION,
      &bytesWritten);
   if (error)
      return error;

   if (in_eof)
      closePipe(in_fd, ERROR_LOCATION);

   if (bytesWritten != in_data.length())
      return utils::createErrorFromBoostError(boost::system::errc::io_error, ERROR_LOCATION);

   return Success();
}

} // anonymous namespace


// AbstractChildProcess ================================================================================================
struct AbstractChildProcess::Impl
{
   /**
    * @brief Constructor.
    * @param in_options     The options for this child process.
    */
   explicit Impl(const ProcessOptions& in_options) :
      Pid(-1),
      StdErrFd(-1),
      StdInFd(-1),
      StdOutFd(-1)
   {
      RSandbox = options::Options::getInstance().getRSandboxPath().getAbsolutePath();
      Arguments.push_back(RSandbox);
      createEnvironmentVars(in_options);
      createLaunchProfile(in_options);
      createSandboxArguments(in_options);
   }

   /**
    * @brief Creates a C-style list of environment variables from the process options.
    *
    * If PATH is not present, the current PATH will be added.
    *
    * @param in_options     The options for this child process.
    */
   void createEnvironmentVars(const ProcessOptions& in_options)
   {
      bool pathFound = false;
      for (const api::EnvVariable& envVar: in_options.Environment)
      {
         if (envVar.first == "PATH")
            pathFound = true;

         Environment.emplace_back(envVar.first + "=" + envVar.second);
      }

      if (!pathFound)
         Environment.emplace_back("PATH=" + posix::getEnvironmentVariable("PATH"));
   }

   /**
    * @brief Creates an RSandbox launch profile.
    *
    * Also creates a sterilized version for logging.
    *
    * @param in_options     The options for this child process.
    */
   void createLaunchProfile(const ProcessOptions& in_options)
   {
      json::Object contextObj;
      contextObj["username"] = (in_options.RunAsUser.isAllUsers() || in_options.RunAsUser.isEmpty()) ?
                               "" : in_options.RunAsUser.getUsername();
      contextObj["project"] = "";
      contextObj["id"] = "";

      json::Object configObj;
      configObj["args"] = json::Array();
      configObj["environment"] = json::Object();
      configObj["stdInput"] = in_options.StandardInput;
      configObj["stdStreamBehavior"] = 2; // Inherit
      configObj["priority"] = 0;
      configObj["memoryLimitBytes"] = 0;
      configObj["stackLimitBytes"] = 0;
      configObj["userProcessesLimit"] = 0;
      configObj["cpuLimit"] = 0;
      configObj["niceLimit"] = 0;
      configObj["filesLimit"] = 0;
      configObj["cpuAffinity"] = json::Array();

      json::Object profileObj;
      profileObj["context"] = contextObj;
      profileObj["password"] = in_options.Password;
      profileObj["executablePath"] = in_options.Executable;
      profileObj["config"] = configObj;

      StandardInput = profileObj.write();

      if (!in_options.Password.empty())
      {
         profileObj["password"] = "<redacted>";
         SafeStdin = profileObj.write();
      }
   }

   /**
    * @brief Creates the RSandbox arguments from the given process options.
    *
    * @param in_options     The options for this child process.
    */
   void createSandboxArguments(const ProcessOptions& in_options)
   {
      std::string shellCommand = in_options.Executable;
      for (const std::string& arg: in_options.Arguments)
         shellCommand.append(" ").append(escape(arg));

      bool redirectStdout = !in_options.StandardOutputFile.isEmpty(),
         redirectStderr = !in_options.StandardErrorFile.isEmpty(),
         redirectToSame = redirectStdout &&
                          redirectStderr &&
                          (in_options.StandardOutputFile == in_options.StandardErrorFile);

      if (in_options.IsShellCommand && (redirectStdout || redirectStderr))
         shellCommand = "(" + shellCommand.append(")");

      if (redirectStdout)
         shellCommand.append(" > ").append(escape(in_options.StandardOutputFile));

      if (redirectToSame)
         shellCommand.append(" 2>&1");
      else if (redirectStderr)
         shellCommand.append(" 2> ").append(escape(in_options.StandardErrorFile));

      if (!in_options.RunAsUser.isAllUsers() && !in_options.RunAsUser.isAllUsers())
      {
         Arguments.emplace_back("--username");
         Arguments.push_back(in_options.RunAsUser.getUsername());
      }

      if (!in_options.WorkingDirectory.isEmpty())
      {
         Arguments.emplace_back("--workingdir");
         Arguments.push_back(in_options.WorkingDirectory.getAbsolutePath());
      }

      if (!in_options.PamProfile.empty())
      {
         Arguments.emplace_back("--pam-profile");
         Arguments.push_back(in_options.PamProfile);
      }

      for (const api::Mount& mount: in_options.Mounts)
      {
         if (mount.HostSourcePath)
         {
            Arguments.emplace_back("--mount");
            Arguments.emplace_back(
               escape(
                  mount.HostSourcePath.getValueOr(api::HostMountSource()).Path +
                  ":" +
                  mount.DestinationPath +
                  (mount.IsReadOnly ? ":ro" : "")));
         }
      }

      Arguments.emplace_back("/bin/sh");
      Arguments.emplace_back("-c");
      Arguments.emplace_back(shellCommand);
   }

   /**
    * @brief Prepares the child process by closing uncessary FDs and then invokes ::execv[e].
    *
    * This method should only be invoked from the child process.
    *
    * @param in_fds             The currently open FDs for parent-child communication.
    * @param in_maxFd           The maximum possible FD for the system.
    * @param in_arguments       The process arguments.
    * @param in_environment     The process environment.
    *
    * @return Error if a failure occurred before or during the call to ::execv[e]. This method should not return on
    *         successful ::execv[e].
    */
   Error execChild(
      const FileDescriptors& in_fds,
      rlim_t in_maxFd,
      const CStringList& in_arguments,
      const CStringList& in_environment) const
   {
      // Set up the parent group id to ensure all children of this child process will belong to its process group, and
      // as such can be cleaned up by the parent.
      if (::setpgid(0, 0) == -1)
         ::exit(s_threadSafeExitError);

      if (clearSignalMask() != 0)
         ::exit(s_threadSafeExitError);

      // Close the side of the pipe that won't be used in the child.
      ::close(in_fds.Input[s_writePipe]);
      ::close(in_fds.Output[s_readPipe]);
      ::close(in_fds.Error[s_readPipe]);
      ::close(in_fds.CloseFd[s_writePipe]);

      // Connect the pipes to the appropriate stream.
      int result = ::dup2(in_fds.Input[s_readPipe], STDIN_FILENO);
      if (result == -1)
         ::_exit(s_threadSafeExitError);

      result = ::dup2(in_fds.Output[s_writePipe], STDOUT_FILENO);
      if (result == -1)
         ::_exit(s_threadSafeExitError);

      result = ::dup2(in_fds.Error[s_writePipe], STDERR_FILENO);
      if (result == -1)
         ::_exit(s_threadSafeExitError);

      // Close any file descriptors that were already open in the parent process. If these FDs are left open, it's
      // possible that this child will clobber the parent's FDs and make it miss notifications that children have
      // exit if the clobbered FDs were being used in epoll calls.
      closeParentFds(in_fds.CloseFd[s_readPipe], in_maxFd);
      ::close(in_fds.CloseFd[s_readPipe]);

      if (in_environment.isEmpty())
         ::execv(RSandbox.c_str(), in_arguments.getData());
      else
         ::execve(RSandbox.c_str(), in_arguments.getData(), in_environment.getData());

      // If we get here the execv(e) call failed.
      ::_exit(s_threadSafeExitError);
   }

   /**
    * @brief Logs a process spawn at the debug level.
    */
   void logProcessSpawn()
   {
      // If SafeStdin is empty, that means there was no sensitive data to sterilize, so just log the regular
      // rdrStandardInput.
      logging::logDebugMessage(
         "Launching rsandbox. \nArgs " +
            boost::algorithm::join(Arguments, " ") +
            "\nLaunch Profile: " +
            (SafeStdin.empty() ? StandardInput : SafeStdin),
         ERROR_LOCATION);
   }

   /** The list of RSandbox arguments. */
   std::vector<std::string> Arguments;

   /** The environment to send to the RSandbox process. */
   std::vector<std::string> Environment;

   /** A sterilized version of StandardInput for logging purposes. */
   std::string SafeStdin;

   /** The launch profile to send to the RSandbox process via standard input. */
   std::string StandardInput;

   /** The read end of the Standard Error FD for the parent process. */
   int StdErrFd;

   /** The write end of the Standard Input FD for the parent process. */
   int StdInFd;

   /** The read end of the Standard Output FD for the parent process. */
   int StdOutFd;

   /** The PID of the child process. */
   pid_t Pid;

   /** The RSandbox executable path. */
   std::string RSandbox;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractChildProcess)

pid_t AbstractChildProcess::getPid() const
{
   return m_baseImpl->Pid;
}

Error AbstractChildProcess::terminate()
{
   // Don't send a signal if the child isn't running.
   if (m_baseImpl->Pid == -1)
      return systemError(ESRCH, ERROR_LOCATION);

   // Try sending SIGTERM for the whole process group of the child.
   if (::kill(-m_baseImpl->Pid, SIGTERM) == -1)
   {
      // When killing an entire process group EPERM can be returned if even a single one of the subprocesses couldn't be
      // killed. In this case the signal is still delivered and other subprocesses may have been killed so we don't log
      // an error.
      // We also don't consider it an error if the process couldn't be killed because it had already exited (ESRCH).
      if ((errno == EPERM) || (errno == ESRCH))
         return Success();

      return systemError(errno, ERROR_LOCATION);
   }

   return Success();
}

AbstractChildProcess::AbstractChildProcess(const ProcessOptions& in_options) :
   m_baseImpl(new Impl(in_options))
{
}

Error AbstractChildProcess::run()
{
   m_baseImpl->logProcessSpawn();

   // Get the system resource limit for open files. This is needed to allow the child to properly close its files in
   // an async-safe way.
   rlim_t softLimit, hardLimit;
   Error error = getFilesLimit(softLimit, hardLimit);
   if (error)
      return error;

   // Set up process communication pipes.
   FileDescriptors fds;
   error = createPipes(fds);
   if (error)
      return error;

   // Now fork the process.
   pid_t pid;
   error = posix::posixCall<pid_t>(::fork, ERROR_LOCATION, &pid);

   // If this is the child process, execute the requested process.
   if (pid == 0)
      m_baseImpl->execChild(
         fds,
         hardLimit,
         CStringList(m_baseImpl->Arguments),
         CStringList(m_baseImpl->Environment));
   // Otherwise, this is still the parent.
   else
   {
      // Close the unused pipes from the parent's perspective.
      closePipe(fds.Input[s_readPipe], ERROR_LOCATION);
      closePipe(fds.Output[s_writePipe], ERROR_LOCATION);
      closePipe(fds.Error[s_writePipe], ERROR_LOCATION);
      closePipe(fds.CloseFd[s_readPipe], ERROR_LOCATION);

      // Save the relevant StdIn, StdErr, and StdOut pipes for future use.
      m_baseImpl->StdInFd = fds.Input[s_writePipe];
      m_baseImpl->StdOutFd = fds.Output[s_readPipe];
      m_baseImpl->StdErrFd = fds.Error[s_readPipe];

      // Send the list of the child's open pipes to it.
      error = sendFileDescriptors(fds.CloseFd[s_writePipe], pid);
      closePipe(fds.CloseFd[s_writePipe], ERROR_LOCATION);

      return error;
   }

   return Success();
}

// SyncChildProcess ====================================================================================================
SyncChildProcess::SyncChildProcess(const ProcessOptions& in_options) :
   AbstractChildProcess(in_options)
{
}

Error SyncChildProcess::run(ProcessResult& out_result)
{
   // Start the child process and exec as requested.
   Error error = AbstractChildProcess::run();
   if (error)
      return error;

   // Send the requested StdIn, if any.
   if (!m_baseImpl->StandardInput.empty())
   {
      error = writeToPipe(m_baseImpl->StdInFd, m_baseImpl->StandardInput, true);
      if (error)
      {
         Error terminateError = terminate();
         if (terminateError)
            logging::logError(terminateError);
      }
   }

   // Don't return on previous errors because we need to wait for the child process to exit before returning, but don't
   // keep attempting to perform operations on the child process either.

   // If no errors, read standard output.
   if (!error)
      error = readFromPipe(m_baseImpl->StdOutFd, out_result.StdOut);

   // If no errors, read standard error.
   if (!error)
      error = readFromPipe(m_baseImpl->StdErrFd, out_result.StdError);

   // Wait for the process to exit and record the exit code.
   int status = 0;
   pid_t result = posix::posixCall<pid_t>(std::bind(::waitpid, m_baseImpl->Pid, &status, 0));

   if (result == -1)
   {
      out_result.ExitCode = -1;

      // If the child had already exited return success.
      if (errno == ECHILD)
         return Success();

      // Otherwise return an appropriate system error.
      return systemError(errno, ERROR_LOCATION);
   }
   else
      out_result.ExitCode = getExitCodeFromStatus(status);

   return Success();
}

// AsyncChildProcess ===================================================================================================
/**
 * @brief An asynchronous rsandbox process.
 */
class AsyncChildProcess final : public AbstractChildProcess, public std::enable_shared_from_this<AsyncChildProcess>
{
   typedef std::shared_ptr<AsyncChildProcess> SharedThis;
   typedef std::weak_ptr<AsyncChildProcess> WeakThis;

public:
   /**
    * @brief Constructor.
    *
    * @param in_options     The options for the child process.
    */
   explicit AsyncChildProcess(const ProcessOptions& in_options);

   /**
    * @brief Checks whether this child process has exited.
    *
    * @return True if this child process has exited; false otherwise.
    */
   bool hasExited() const;

   /**
    * @brief Runs the process. May only be called once.
    *
    * @param in_callbacks   Optional callbacks to be invoked on certain events.
    *
    * @return Success if the process could be run; Error otherwise.
    */
   Error run(const AsyncProcessCallbacks& in_callbacks);

   /**
    * @brief Terminates the process.
    *
    * @return Success if the process could be terminated; Error otherwise.
    */
   Error terminate() override;

private:
   /**
    * @brief Starts a timed event to check for process exit every 20 milliseconds until time time limit is reached.
    *
    * @param in_sharedThis      A shared pointer to this.
    * @param in_waitTime        The maximum amount of time to wait for the process to exit.
    * @param in_forceExit       True if the exit should be forced; false otherwise.
    * @param in_error           The error that initiated this check for exit, if any.
    * @param in_startTime       The time at which this check started. Default: invocation time.
    */
   static void startCheckingExit(
      const SharedThis& in_sharedThis,
      const system::TimeDuration& in_waitTime = system::TimeDuration::Infinity(),
      bool in_forceExit = false,
      const Error& in_error = Success(),
      const system::DateTime& in_startTime = system::DateTime());

   /**
    * @brief Checks whether this process has exited, canceling the timed event if the time limit is reached.
    *
    * @param in_sharedThis      A shared pointer to this.
    * @param in_waitTime        The maximum amount of time to wait for the process to exit.
    * @param in_forceExit       True if the exit should be forced; false otherwise.
    * @param in_error           The error that initiated this check for exit, if any.
    * @param in_startTime       The time at which this check started. Default: invocation time.
    */
   void checkExited(
      const system::TimeDuration& in_waitTime = system::TimeDuration::Infinity(),
      bool in_forceExit = false,
      const Error& in_error = Success(),
      const system::DateTime& in_startTime = system::DateTime());

   /**
    * @brief Waits for the other output stream to fail, if one of them has failed, for up to 5 seconds.
    *
    * @param in_error           The error that occrued when the stream failed.
    * @param in_startTime       The time at which waiting has started. Default: invocation time.
    */
   void waitForOtherStreamFailure(
      const Error& in_error,
      const system::DateTime& in_startTime = system::DateTime());

   /** The callbacks to be invoked when certain events occur (such as process exit or stdout output). */
   AsyncProcessCallbacks m_callbacks;

   /** Whether the stream has exited. */
   bool m_hasExited;

   /** Whether the stderr stream has failed. */
   bool m_stdErrFailure;

   /** Whether the stdout stream has failed. */
   bool m_stdOutFailure;

   /** The stderr stream. */
   std::unique_ptr<AsioStream> m_stdErrStream;

   /** The stdin stream. */
   std::unique_ptr<AsioStream> m_stdInStream;

   /** The stdout stream. */
   std::unique_ptr<AsioStream> m_stdOutStream;

   /** Timer that watches for process exit or stream failure. */
   std::unique_ptr<AsyncTimedEvent> m_exitWatcher;

   /** Mutex to protect shared state. */
   std::mutex m_mutex;
};

AsyncChildProcess::AsyncChildProcess(const ProcessOptions& in_options) :
   AbstractChildProcess(in_options),
   m_hasExited(false),
   m_stdErrFailure(false),
   m_stdOutFailure(false)
{
}

bool AsyncChildProcess::hasExited() const
{
   return m_hasExited;
}

Error AsyncChildProcess::run(const AsyncProcessCallbacks& in_callbacks)
{
   m_callbacks = in_callbacks;

   Error error = AbstractChildProcess::run();
   if (error)
      return error;

   // Make the pipes non-blocking.
   setPipeNonBlocking(m_baseImpl->StdErrFd);
   setPipeNonBlocking(m_baseImpl->StdInFd);
   setPipeNonBlocking(m_baseImpl->StdOutFd);

   WeakThis weakThis = weak_from_this();
   if (!m_baseImpl->StandardInput.empty())
   {
      m_stdInStream.reset(new AsioStream(m_baseImpl->StdInFd));

      AsioFunction onFinishedWriting = [weakThis]()
      {
         if (SharedThis sharedThis = weakThis.lock())
            sharedThis->m_stdInStream->close();
      };

      m_stdInStream->writeBytes(m_baseImpl->StandardInput, in_callbacks.OnError, onFinishedWriting);
   }

   auto onRead = [](const OnOutputCallback& in_onOutput, const char* in_data, size_t in_length)
   {
      std::string strData(in_data, in_length);
      in_onOutput(strData);
   };

   auto streamFailureWatch = [weakThis](const Error& in_error, const system::DateTime& in_startTime)
   {
      if (SharedThis sharedThis = weakThis.lock())
      {
         sharedThis->waitForOtherStreamFailure(in_error, in_startTime);
      }
   };

   auto onReadError = [weakThis, streamFailureWatch](bool is_stdOut, const Error& in_error)
   {
      if (SharedThis sharedThis = weakThis.lock())
      {
         if (is_stdOut)
            sharedThis->m_stdOutFailure = true;
         else
            sharedThis->m_stdErrFailure = true;

         // Start watching for the other stream to fail in 20 millisecond intervals, for at most 5 seconds, only if
         // there isn't another timer already running (we don't want to interrupt an actual exit watcher from terminate).
         LOCK_MUTEX(sharedThis->m_mutex)
         {
            if (sharedThis->m_exitWatcher == nullptr)
            {
               sharedThis->m_exitWatcher.reset(new AsyncTimedEvent());
               sharedThis->m_exitWatcher->start(
                  system::TimeDuration::Microseconds(20000),
                  std::bind(streamFailureWatch, in_error, system::DateTime()));
            }
         }
         END_LOCK_MUTEX
      }
   };

   m_stdOutStream.reset(new AsioStream(m_baseImpl->StdOutFd));
   m_stdErrStream.reset(new AsioStream(m_baseImpl->StdErrFd));

   {
      using namespace std::placeholders;
      m_stdOutStream->readBytes(
         std::bind(onRead, in_callbacks.OnStandardOutput, _1, _2),
         std::bind(onReadError, true, _1));
      m_stdErrStream->readBytes(
         std::bind(onRead, in_callbacks.OnStandardError, _1, _2),
         std::bind(onReadError, false, _1));
   }

   return Success();
}

Error AsyncChildProcess::terminate()
{
   if (m_hasExited)
      return Success();

   Error error = AsyncChildProcess::terminate();
   if (!error)
   {
      // Wait up to 30 seconds for the process to exit. If it fails to exit within this time, there's likely something
      // wrong. At this point, it's best not to let the child process impact the parent, so invoke the onExit callback
      // and continue as if it had exited.
      startCheckingExit(shared_from_this(), system::TimeDuration::Seconds(30), true);
   }

   return error;
}

void AsyncChildProcess::startCheckingExit(
   const SharedThis& in_sharedThis,
   const system::TimeDuration& in_waitTime,
   bool in_forceExit,
   const Error& in_error,
   const system::DateTime& in_startTime)
{
   WeakThis weakThis = in_sharedThis;
   auto onTimer = [weakThis, in_waitTime, in_forceExit, in_error, in_startTime]()
   {
      if (SharedThis sharedThis = weakThis.lock())
      {
         sharedThis->checkExited(in_waitTime, in_forceExit, in_error, in_startTime);
      }
   };

   LOCK_MUTEX(in_sharedThis->m_mutex)
   {
      if (in_sharedThis->m_exitWatcher != nullptr)
         in_sharedThis->m_exitWatcher->cancel();

      in_sharedThis->m_exitWatcher.reset(new AsyncTimedEvent());
      in_sharedThis->m_exitWatcher->start(system::TimeDuration::Microseconds(20000), onTimer);
   }
   END_LOCK_MUTEX
}

void AsyncChildProcess::checkExited(
   const system::TimeDuration& in_waitTime,
   bool in_forceExit,
   const Error& in_error,
   const system::DateTime& in_startTime)
{
   // Don't bother checking for exit if one of the output/error streams still hasn't exited, unless exit is being
   // forced.
   if (!in_forceExit && (!m_stdOutFailure || !m_stdErrFailure))
      return;

   int exitCode = -1;

   LOCK_MUTEX(m_mutex)
   {
      if (m_hasExited)
         return;

      // Perform a non-blocking wait for the process to fully exit. Check back periodically until the maximum wait time
      // is reached.
      int status = 0;
      pid_t result = posix::posixCall<pid_t>(std::bind(::waitpid, m_baseImpl->Pid, &status, WNOHANG));

      // If the result is 0, the process hasn't exited. Otherwise, set the exit code appropriately.
      if (result != 0)
      {
         m_hasExited = true;

         if (result > 0)
            exitCode = getExitCodeFromStatus(status);
      }
      else if (!in_waitTime.isInfinity() && (system::DateTime() < (in_startTime + in_waitTime)))
      {
         // If we haven't hit the maximum wait time, keep waiting.
         return;
      }

      // If we should be forcing exit, act like the process has exited anyway.
      if (in_forceExit && !m_hasExited)
         m_hasExited = true;

      if (m_hasExited ||
         (!in_waitTime.isInfinity() &&
            (system::DateTime() > (in_startTime + in_waitTime)) &&
            (m_exitWatcher != nullptr)))
      {
         // If we have hit the maximum wait time or if we've already exited, cancel the timer.
         m_exitWatcher->cancel();
         return;
      }
   }
   END_LOCK_MUTEX

   // Here we make explicit copies of the bound functions to ensure they won't be cleaned up off the stack if this
   // object is destroyed before the function is allocated a thread to run on.
   if (m_hasExited && m_callbacks.OnExit)
   {
      AsioFunction onExitHandler = std::bind(m_callbacks.OnExit, exitCode);
      AsioService::post(onExitHandler);
   }
   else if (in_error)
   {
      if (m_callbacks.OnError)
      {
         AsioFunction onErrorHandler = std::bind(m_callbacks.OnError, in_error);
         AsioService::post(onErrorHandler);
      }
      else
         logging::logError(in_error, ERROR_LOCATION);

      // At this point, the output streams have both failed but the child process hasn't exited. Force terminate it.
      // This can't be a recursive call because we shouldn't get to this block if in_forceExit is true.
      assert(!in_forceExit);
      Error error = terminate();
      if (error)
         logging::logError(error, ERROR_LOCATION);
   }
}

void AsyncChildProcess::waitForOtherStreamFailure(const Error& in_error, const system::DateTime& in_startTime)
{
   // If both have failed, start checking for exit.
   system::TimeDuration fiveSeconds = system::TimeDuration::Seconds(5);
   if (m_stdOutFailure && m_stdErrFailure)
   {
      startCheckingExit(
         shared_from_this(),
         fiveSeconds,
         false,
         in_error,
         in_startTime);
   }

   // One of the streams has failed
   system::DateTime now;
   if (now >= (in_startTime + fiveSeconds))
   {
      if (m_callbacks.OnError)
         m_callbacks.OnError(in_error);
      else
         logging::logError(in_error, ERROR_LOCATION);

      terminate();
   }
}

// ProcessSupervisor ===================================================================================================
struct ProcessSupervisor::Impl : public std::enable_shared_from_this<Impl>
{
   typedef std::weak_ptr<Impl> WeakThis;
   typedef std::shared_ptr<Impl> SharedThis;

   /**
    * @brief Checks whether any children has exited and removes from the list of children if they have.
    */
   void cleanExitedChildren()
   {
      LOCK_MUTEX(Mutex)
      {
         for (auto itr = Children.begin(); itr != Children.end();)
         {
            if ((*itr)->hasExited())
               itr = Children.erase(itr);
            else
               ++itr;
         }

         if (Children.empty())
            ExitCondition.notify_all();
      }
      END_LOCK_MUTEX
   }

   /**
    * @brief Wrapper for the caller's onExit callback that triggers a cleanup call.
    *
    * @param in_onExit      The caller's onExit callback.
    * @param in_exitCode    The exit code of the process.
    */
   void exitHandler(const OnExitCallback& in_onExit, int in_exitCode)
   {
      WeakThis weakThis = weak_from_this();
      auto cleanup = [weakThis]()
      {
         if (SharedThis sharedThis = weakThis.lock())
            sharedThis->cleanExitedChildren();
      };

      LOCK_MUTEX(Mutex)
      {
         AsioService::post(cleanup);
      }
      END_LOCK_MUTEX

      if (in_onExit)
         in_onExit(in_exitCode);
   }

   /** The running child processes. */
   std::vector<std::shared_ptr<AsyncChildProcess> > Children;

   /** Condition variable that will be notified if Children becomes empty. */
   std::condition_variable ExitCondition;

   /** Mutex to protect Children. */
   std::mutex Mutex;
};

ProcessSupervisor& ProcessSupervisor::getInstance()
{
   static ProcessSupervisor supervisor;
   return supervisor;
}

bool ProcessSupervisor::hasRunningChildren()
{
   // First purge any exited children.
   getInstance().m_impl->cleanExitedChildren();
   return !getInstance().m_impl->Children.empty();
}

Error ProcessSupervisor::runAsyncProcess(
   const ProcessOptions& in_options,
   const AsyncProcessCallbacks& in_callbacks,
   std::shared_ptr<AbstractChildProcess>& out_childProcess)
{
   ProcessSupervisor& instance = getInstance();

   // Wrap the exit call back so we can keep track of when a child exits.
   OnExitCallback onExit = in_callbacks.OnExit;
   Impl::WeakThis weakThis = instance.m_impl;
   auto onExitWrapper = [weakThis, onExit](int in_exitCode)
   {
      if (Impl::SharedThis sharedThis = weakThis.lock())
         sharedThis->exitHandler(onExit, in_exitCode);
   };

   AsyncProcessCallbacks wrappedCallbacks = in_callbacks;
   wrappedCallbacks.OnExit = onExitWrapper;

   // Run the process.
   std::shared_ptr<AsyncChildProcess> child(new AsyncChildProcess(in_options));
   Error error = child->run(in_callbacks);
   if (error)
      return error;

   LOCK_MUTEX(instance.m_impl->Mutex)
   {
      instance.m_impl->Children.push_back(child);
   }
   END_LOCK_MUTEX

   out_childProcess = child;
   return Success();
}

void ProcessSupervisor::terminateAll()
{
   ProcessSupervisor& instance = getInstance();
   LOCK_MUTEX(instance.m_impl->Mutex)
   {
      for (const auto& child: instance.m_impl->Children)
      {
         Error error = child->terminate();
         if (error)
            logging::logError(error, ERROR_LOCATION);
      }
   }
   END_LOCK_MUTEX
}

bool ProcessSupervisor::waitForExit(const TimeDuration& in_maxWaitTime)
{
   ProcessSupervisor& instance = getInstance();

   // Set up the callback that checks whether we've reached the time limit.
   system::DateTime startTime;
   auto isPastTimeout = [in_maxWaitTime, startTime]()
   {
      if (in_maxWaitTime.isInfinity())
         return false;

      return system::DateTime() > (startTime + in_maxWaitTime);
   };

   // Wait on the condition variable, up to the timeout length.
   UNIQUE_LOCK_MUTEX(instance.m_impl->Mutex)
   {
      if (hasRunningChildren())
      {
         instance.m_impl->ExitCondition.wait(uniqueLock, isPastTimeout);
      }
   }
   END_LOCK_MUTEX

   // If there are still running children when we get here, we timed out.
   return hasRunningChildren();
}

ProcessSupervisor::ProcessSupervisor() :
   m_impl(new Impl())
{
}

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
