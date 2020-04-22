/*
 * AbstractMain.cpp
 *
 * Copyright (C) 2019-20 by RStudio, PBC
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

#include <AbstractMain.hpp>

#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>

#include <Error.hpp>
#include <comms/StdIOLauncherCommunicator.hpp>
#include <logging/Logger.hpp>
#include <logging/FileLogDestination.hpp>
#include <logging/StderrLogDestination.hpp>
#include <logging/SyslogDestination.hpp>
#include <options/Options.hpp>
#include <system/PosixSystem.hpp>
#include <system/User.hpp>
#include <system/Asio.hpp>
#include <utils/ErrorUtils.hpp>
#include <utils/MutexUtils.hpp>

namespace rstudio {
namespace launcher_plugins {

#define CHECK_ERROR_1(in_error, in_msg)   \
if (in_error)                             \
{                                         \
   logging::logError(in_error);           \
   if (!std::string(in_msg).empty())      \
      logging::logErrorMessage(in_msg);   \
   return 1;                              \
}                                         \

#define CHECK_ERROR_0(in_error, dummy)  CHECK_ERROR_1(in_error, "")

#define CHECK_ERROR_GET_MACRO(_0, _1, NAME, ...) NAME
#define CHECK_ERROR(in_error, ...) CHECK_ERROR_GET_MACRO(_0, ##__VA_ARGS__, CHECK_ERROR_1, CHECK_ERROR_0) \
   (in_error, __VA_ARGS__)                                                                                \


namespace {

int configureScratchPath(const system::FilePath& in_scratchPath, const system::User& in_serverUser)
{
   std::string message;
   if (!in_scratchPath.exists())
      message = "please ensure that it exists.";
   else
      message = "please ensure that it is a directory.";

   Error error = in_scratchPath.ensureDirectory();
   CHECK_ERROR(error, "Invalid scratch path - " + message)

   // At this point the scratch path exists and is a directory. Make sure it belongs to the server user.
   // First, check if the real user is root.
   if (system::posix::realUserIsRoot())
   {
      // If we are, restore root privileges.
      error = system::posix::restoreRoot();
      CHECK_ERROR(error, "Could not restore root privilege.")

      // Change file ownership to the server user.
      error = in_scratchPath.changeOwnership(in_serverUser);
      CHECK_ERROR(
         error,
         "Could not change ownership of scratch path to server user: " +
            in_scratchPath.getAbsolutePath() +
            ".")

      // Drop privileges to the server  user.
      error = system::posix::temporarilyDropPrivileges(in_serverUser);
      CHECK_ERROR(error, "Could not lower privilege to server user: " + in_serverUser.getUsername() + ".")
   }

   // Change the file mode to rwxr-x-r-x so everyone can read the files in the scratch path, but only the server user
   // has full access.
   error = in_scratchPath.changeFileMode(system::FileMode::USER_READ_WRITE_EXECUTE_ALL_READ_EXECUTE);
   CHECK_ERROR(
      error,
      "Could not set permission on scratch path (" +
         in_scratchPath.getAbsolutePath() +
         ") - it is recommended to set them to rwxr-x-r-x.")

   return 0;
}

} // namespace


struct AbstractMain::Impl
{
   /**
    * @brief Constructor.
    */
   Impl() :
      m_exitProcess(false)
   {
   }

   /**
    * @brief Signals shutdown for the whole process.
    */
   void signalShutdown()
   {
      UNIQUE_LOCK_MUTEX(m_mutex)
      m_exitProcess = true;
      m_exitConditionVar.notify_all();
      END_LOCK_MUTEX
   }

   /**
    * @brief Signal handler to be invoked when the process recevies a signal such as SIGINT.
    *
    * @param in_sharedThis      A shared pointer to this.
    * @param in_signal          The signal which was received.
    */
   static void onSignal(std::shared_ptr<Impl> in_sharedThis, int in_signal)
   {
      logging::logInfoMessage("Received signal: " + std::to_string(in_signal));
      in_sharedThis->signalShutdown();
   }

   /**
    * @brief Callback function to be invoked when a communication error occurs with the RStudio Launcher, such as a
    *        malformed message.
    *
    * @param in_sharedThis      A shared pointer to this.
    * @param in_error           The error which occurred while communicating with the RStudio Launcher..
    */
   static void onCommunicationError(std::shared_ptr<Impl> in_sharedThis, const Error& in_error)
   {
      logging::logError(in_error);
      logging::logErrorMessage("Received fatal error while attempting to communicate with Job Launcher Framework.");

      in_sharedThis->signalShutdown();
   }

   /**
    * @brief Runs the process until a shutdown signal is receveid.
    */
   void waitForSignal()
   {
      UNIQUE_LOCK_MUTEX(m_mutex)
      if (!m_exitProcess)
         m_exitConditionVar.wait(uniqueLock, [&]{ return m_exitProcess; });
      END_LOCK_MUTEX
   }

private:
   /** Indicates whether the process should exit (true) or continue running (false).*/
   bool m_exitProcess;

   /** Mutex to protect the exit process boolean value. */
   std::mutex m_mutex;

   /** Condition variable to use to wait for or send a shutdown signal. */
   std::condition_variable m_exitConditionVar;
};

int AbstractMain::run(int in_argc, char** in_argv)
{
   // Initialize Main. This should initialize the plugin-specific options, and any other plugin specific elements needed
   // (e.g it could add a custom logging destination). We need to do this before loggers are added in case the plugin
   // needs to initialize some things for its program ID.
   Error error = initialize();
   CHECK_ERROR(error)

   // Set up the logger.
   using namespace logging;
   setProgramId(getProgramId());

   // Add a syslog destination.
   addLogDestination(
      std::shared_ptr<ILogDestination>(
         new SyslogDestination(
            LogLevel::INFO,
            getProgramId())));

   // Turn on stderr logging while options are parsed.
   std::shared_ptr<ILogDestination> stderrLogDest(new StderrLogDestination(LogLevel::INFO));
   addLogDestination(stderrLogDest);

   // Initialize the default options. This must be done before the custom options are initialized.
   options::Options& options = options::Options::getInstance();

   // Read the options.
   error = options.readOptions(in_argc, in_argv, getConfigFile());
   CHECK_ERROR(error)

   // Ensure the server user exists.
   system::User serverUser;
   error = options.getServerUser(serverUser);
   CHECK_ERROR(error)

   // Ensure the scratch path exists and is configured correctly.
   int ret = configureScratchPath(options.getScratchPath(), serverUser);
   if (ret != 0)
      return ret;

   // Remove the stderr log destination.
   removeLogDestination(stderrLogDest->getId());

   if (options.getLogLevel() > LogLevel::INFO)
   {
      addLogDestination(
         std::unique_ptr<ILogDestination>(
            new FileLogDestination(
               3,
               options.getLogLevel(),
               getProgramId(),
               options.getScratchPath())));
   }

   // Create the launcher communicator. For now this is always an StdIO communicator. Later, it could be dependant on
   // the options.
   std::shared_ptr<comms::AbstractLauncherCommunicator> launcherCommunicator(
      new comms::StdIOLauncherCommunicator(
         options.getMaxMessageSize(),
         std::bind(&Impl::onCommunicationError, m_abstractMainImpl, std::placeholders::_1)));

   // Ignore SIGPIPE
   system::posix::ignoreSignal(SIGPIPE);

   // Configure the signal handler.
   system::AsioService::setSignalHandler(std::bind(&Impl::onSignal, m_abstractMainImpl, std::placeholders::_1));

   // Enable core dumps.
   error = system::posix::enableCoreDumps();
   CHECK_ERROR(error)

   // Create and initialize the LauncherPluginApi.
   std::shared_ptr<api::AbstractPluginApi> pluginApi = createLauncherPluginApi(launcherCommunicator);
   CHECK_ERROR(error);

   error = pluginApi->initialize();
   CHECK_ERROR(error)

   // Add the configured number of threads to the ASIO service.
   system::AsioService::startThreads(options.getThreadPoolSize());

   // Start the communicator.
   error = launcherCommunicator->start();
   CHECK_ERROR(error)

   // Run the process until the exit signal is received.
   m_abstractMainImpl->waitForSignal();

   // Stop the communicator and the threads.
   logInfoMessage("Stopping plugin...");
   launcherCommunicator->stop();
   system::AsioService::stop();
   launcherCommunicator->waitForExit();
   system::AsioService::waitForExit();

   return EXIT_SUCCESS;
}

AbstractMain::AbstractMain() :
   m_abstractMainImpl(new Impl())
{
}

system::FilePath AbstractMain::getConfigFile() const
{
   return system::FilePath("/etc/rstudio/launcher." + getPluginName() + ".conf");
}

std::string AbstractMain::getProgramId() const
{
   return "rstudio-" + getPluginName() + "-launcher";
}

} // namespace launcher_plugins
} // namespace rstudio
