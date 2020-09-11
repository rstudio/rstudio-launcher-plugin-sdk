/*
 * AsyncChildProcessTests.cpp
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

#include <TestMain.hpp>

#include <AsioRaii.hpp>

#include <options/Options.hpp>
#include <system/PosixSystem.hpp>
#include <system/Process.hpp>

#include "ProcessTestHelpers.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

AsioRaii s_asioInit;

struct TestCallbacks
{
   explicit TestCallbacks() :
      ExitCode(-12345)
   {
      int& exitCode = ExitCode;
      Callbacks.OnExit = [&exitCode](int in_exitCode)
      {
         exitCode = in_exitCode;
      };

      Callbacks.OnError = [](const Error& in_error)
      {
         FAIL(in_error);
      };

      std::string& stdErr = StdErr;
      Callbacks.OnStandardError = [&stdErr](const std::string& in_str)
      {
         stdErr.append(in_str);
      };

      std::string& stdOut = StdOut;
      Callbacks.OnStandardOutput = [&stdOut](const std::string& in_str)
      {
         stdOut.append(in_str);
      };
   }

   AsyncProcessCallbacks Callbacks;

   int ExitCode;
   std::string StdOut;
   std::string StdErr;
};

TEST_CASE("Create Async Processes")
{
   // Make sure default options are populated.
   REQUIRE_FALSE(loadOptions());

   // Get all the users for future user.
   system::User user1, user2, user3, user4, user5;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_ONE, user1));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_TWO, user2));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_THREE, user3));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FOUR, user4));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FIVE, user5));

   // Results/input used across multiple sections.
   static const std::string stdOutExpected = "multiple\nlines\nof\noutput\nwith a slash \\";
   static const std::string stdErrExpected = "/bin/sh: 1: fakecmd: not found\n";
   static const std::string stdErrAltExpected = "/bin/sh: fakecmd: command not found\n";


   const auto failOnError = [](const Error& in_error) { FAIL(in_error.getSummary()); };

   SECTION("Single process, No redirection, Success")
   {
      ProcessOptions opts;
      opts.Executable = "/bin/echo";
      opts.Arguments.emplace_back("-n");
      opts.Arguments.emplace_back("output");
      opts.IsShellCommand = false;
      opts.RunAsUser = user4;

      TestCallbacks cbs;

      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(opts, cbs.Callbacks));
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(30)));
      CHECK(cbs.StdErr == "");
      CHECK(cbs.StdOut == "output");
      CHECK(cbs.ExitCode == 0);
   }

   SECTION("Many processes")
   {
      // 1. No redirection, bad command
      ProcessOptions o1;
      o1.Executable = "grep";
      o1.Arguments.emplace_back("-x");
      o1.IsShellCommand = true;
      o1.RunAsUser = user3;

      TestCallbacks cb1;

      // 2. No redirection, missing user
      ProcessOptions o2;
      o2.Executable = "grep";
      o2.Arguments.emplace_back("-x");
      o2.IsShellCommand = true;
      o2.RunAsUser = system::User(true);

      TestCallbacks cb2;

      // 3. Stdout redirection
      ProcessOptions o3;
      o3.Executable = "/bin/echo";
      o3.Arguments.emplace_back("-ne");
      o3.Arguments.emplace_back(stdOutExpected);
      o3.IsShellCommand = true;
      o3.RunAsUser = user1;
      o3.StandardOutputFile = user1.getHomePath().completeChildPath("async-test-out.txt");

      TestCallbacks cb3;

      // 4. Stderr redirection
      ProcessOptions o4;
      o4.Executable = "fakecmd";
      o4.Arguments.emplace_back("-n");
      o4.Arguments.emplace_back("-e");
      o4.Arguments.emplace_back(stdOutExpected);
      o4.IsShellCommand = true;
      o4.RunAsUser = user1;
      o4.StandardErrorFile = user1.getHomePath().completeChildPath("async-test-err.txt");
      
      TestCallbacks cb4;

      // 5. Environment variables
      ProcessOptions o5;
      o5.Executable = "./test.sh";
      o5.IsShellCommand = false;
      o5.Environment.emplace_back("VAR", "Hello, world!");
      o5.RunAsUser = user3;
      o5.WorkingDirectory = user3.getHomePath();

      TestCallbacks cb5;

      // Run all the processes and wait for them all to exit.
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o1, cb1.Callbacks));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o2, cb2.Callbacks));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o3, cb3.Callbacks));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o4, cb4.Callbacks));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o5, cb5.Callbacks));
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(30)));

      // 1. No redirection, bad command
      CHECK(cb1.StdErr == "Usage: grep [OPTION]... PATTERN [FILE]...\n"
                       "Try 'grep --help' for more information.\n");
      CHECK(cb1.StdOut == "");
      CHECK(cb1.ExitCode == 2);

      // 2. No redirection, missing user
      CHECK((!cb2.StdErr.empty() && (cb2.StdErr.substr(21) ==
            "[rsandbox] ERROR Required option username not specified; LOGGED FROM: bool"
            " rstudio::core::program_options::{anonymous}::validateOptionsProvided(const"
            " rstudio_boost::program_options::variables_map&, const"
            " rstudio_boost::program_options::options_description&, const string&)"
            " src/cpp/core/ProgramOptions.cpp:46\n")));
      CHECK(cb2.StdOut == "");
      CHECK(cb2.ExitCode == 1);

      // 3. Stdout redirection
      CHECK(cb3.StdErr == "");
      CHECK(cb3.StdOut == "");
      CHECK(cb3.ExitCode == 0);

      // 4. Stderr redirection
      CHECK(cb4.StdErr == "");
      CHECK(cb4.StdOut == "");
      CHECK(cb4.ExitCode == 127);

      // 5. Environment variables
      CHECK(cb5.StdErr == "");
      CHECK(cb5.StdOut == "Hello, world!");
      CHECK(cb5.ExitCode == 0);
   }

   SECTION("Open file descriptors in parent, working dir")
   {
      int pipe[2];
      REQUIRE_FALSE(posix::posixCall<int>(std::bind(::pipe, pipe), ERROR_LOCATION));

      ProcessOptions opts;
      opts.Executable = "cat";
      opts.Arguments.emplace_back("async-test-out.txt");
      opts.Arguments.emplace_back("async-test-err.txt");
      opts.IsShellCommand = true;
      opts.RunAsUser = user1;
      opts.WorkingDirectory = user1.getHomePath();

      TestCallbacks cb;

      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(opts, cb.Callbacks));
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(30)));
      CHECK(cb.StdErr == "");

      std::string expected = stdOutExpected;
      if (cb.StdOut.find("command") != std::string::npos)
         expected.append(stdErrAltExpected);
      else
         expected.append(stdErrExpected);

      CHECK(cb.StdOut == expected);
      CHECK(cb.ExitCode == 0);

      ::close(pipe[0]);
      ::close(pipe[1]);
   }

   SECTION("Mount path")
   {
      const FilePath& mountedPath = user5.getHomePath();

      api::HostMountSource mountSource;
      mountSource.Path = FilePath::safeCurrentPath(FilePath()).getAbsolutePath();
      api::Mount mount;
      mount.DestinationPath = mountedPath.getAbsolutePath();
      mount.IsReadOnly = true;
      mount.HostSourcePath = mountSource;

      ProcessOptions opts;
      opts.Executable = "./test.sh";
      opts.IsShellCommand = false;
      opts.Environment.emplace_back("VAR", "Mount test passed!");
      opts.Mounts.push_back(mount);
      opts.RunAsUser = user5;
      opts.WorkingDirectory = mountedPath;

      TestCallbacks cbs;

      REQUIRE_FALSE(ProcessSupervisor::runAsyncProcess(opts, cbs.Callbacks));
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(5)));
      CHECK(cbs.ExitCode == 0);
      CHECK(cbs.StdErr == "");
      CHECK(cbs.StdOut == "Mount test passed!");
   }

   ProcessSupervisor::terminateAll();
   ProcessSupervisor::waitForExit();
}

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
