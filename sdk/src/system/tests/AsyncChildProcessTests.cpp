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

#include <options/Options.hpp>
#include <system/Process.hpp>

#include "../PosixSystem.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

#define OUTPUT_CALLBACK(in_buffer) \
   [&in_buffer](const std::string& in_string) { in_buffer.append(in_string); }

#define EXIT_CALLBACK(in_expectedCode) \
   [](int in_exitCode) { CHECK(in_exitCode == in_expectedCode); }

TEST_CASE("Create Async Processes")
{
   // Make sure default options are populated.
   REQUIRE_FALSE(
      options::Options::getInstance().readOptions(
         0,
         nullptr,
         system::FilePath("conf-files/Empty.conf")));

   // Get all the users for future user.
   system::User user1, user2, user3, user4, user5;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_ONE, user1));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_TWO, user2));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_THREE, user3));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FOUR, user4));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FIVE, user5));

   // Results/input used across multiple sections.
   static const std::string stdOutExpected = "multiple\nlines\nof\nouptut\nwith a slash \\";
   static const std::string stdErrExpected = "/bin/sh: 1: fakecmd: not found\n";

   const auto failOnError = [](const Error& in_error) { FAIL(in_error.getSummary()); };

   SECTION("Single process, No redirection, Success")
   {
      ProcessOptions opts;
      opts.Executable = "/bin/echo";
      opts.Arguments.emplace_back("-n");
      opts.Arguments.emplace_back("output");
      opts.IsShellCommand = false;
      opts.RunAsUser = user4;

      std::string stdErr, stdOut;

      AsyncProcessCallbacks callbacks;
      callbacks.OnError = failOnError;
      callbacks.OnStandardError = OUTPUT_CALLBACK(stdErr);
      callbacks.OnStandardOutput = OUTPUT_CALLBACK(stdOut);
      callbacks.OnExit = EXIT_CALLBACK(0);

      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(opts, callbacks));
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(5))); // Wait at most 5 seconds for exit.
      CHECK(stdErr == "");
      CHECK(stdOut == "output");
   }

   SECTION("Many processes")
   {
      // 1. No redirection, bad command
      ProcessOptions o1;
      o1.Executable = "grep";
      o1.Arguments.emplace_back("-x");
      o1.IsShellCommand = true;
      o1.RunAsUser = user3;

      std::string stdErr1, stdOut1;
      AsyncProcessCallbacks cb1;
      cb1.OnError = failOnError;
      cb1.OnStandardError = OUTPUT_CALLBACK(stdErr1);
      cb1.OnStandardOutput = OUTPUT_CALLBACK(stdOut1);
      cb1.OnExit = EXIT_CALLBACK(2);

      // 2. No redirection, missing user
      ProcessOptions o2;
      o2.Executable = "grep";
      o2.Arguments.emplace_back("-x");
      o2.IsShellCommand = true;

      std::string stdErr2, stdOut2;
      AsyncProcessCallbacks cb2;
      cb2.OnError = failOnError;
      cb2.OnStandardError = OUTPUT_CALLBACK(stdErr2);
      cb2.OnStandardOutput = OUTPUT_CALLBACK(stdOut2);
      cb2.OnExit = EXIT_CALLBACK(1);

      // 3. Stdout redirection
      ProcessOptions o3;
      o3.Executable = "/bin/echo";
      o3.Arguments.emplace_back("-ne");
      o3.Arguments.emplace_back(stdOutExpected);
      o3.IsShellCommand = true;
      o3.RunAsUser = user1;
      o3.StandardOutputFile = user1.getHomePath().completeChildPath("async-test-out.txt");

      std::string stdErr3, stdOut3;
      AsyncProcessCallbacks cb3;
      cb3.OnError = failOnError;
      cb3.OnStandardError = OUTPUT_CALLBACK(stdErr3);
      cb3.OnStandardOutput = OUTPUT_CALLBACK(stdOut3);
      cb3.OnExit = EXIT_CALLBACK(0);

      // 4. Stderr redirection
      ProcessOptions o4;
      o4.Executable = "fakecmd";
      o4.Arguments.emplace_back("-n");
      o4.Arguments.emplace_back("-e");
      o4.Arguments.emplace_back(stdOutExpected);
      o4.IsShellCommand = true;
      o4.RunAsUser = user1;
      o4.StandardErrorFile = user1.getHomePath().completeChildPath("async-test-err.txt");

      std::string stdErr4, stdOut4;
      AsyncProcessCallbacks cb4;
      cb4.OnError = failOnError;
      cb4.OnStandardError = OUTPUT_CALLBACK(stdErr4);
      cb4.OnStandardOutput = OUTPUT_CALLBACK(stdOut4);
      cb4.OnExit = EXIT_CALLBACK(127);
      
      // 5. Environment variables
      ProcessOptions o5;
      o5.Executable = "./test.sh";
      o5.IsShellCommand = false;
      o5.Environment.emplace_back("VAR", "Hello, world!");
      o5.RunAsUser = user3;
      o5.WorkingDirectory = FilePath::safeCurrentPath(FilePath());
      
      std::string stdErr5, stdOut5;
      AsyncProcessCallbacks cb5;
      cb5.OnError = failOnError;
      cb5.OnStandardError = OUTPUT_CALLBACK(stdErr5);
      cb5.OnStandardOutput = OUTPUT_CALLBACK(stdOut5);
      cb5.OnExit = EXIT_CALLBACK(0);

      // Run all the processes and wait for them all to exit.
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o1, cb1));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o2, cb2));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o3, cb3));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o4, cb4));
      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(o5, cb5));
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(5))); // Wait at most 5 seconds for exit.

      // 1. No redirection, bad command
      CHECK(stdErr1 == "Usage: grep [OPTION]... PATTERN [FILE]...\n"
                       "Try 'grep --help' for more information.\n");
      CHECK(stdOut1 == "");

      // 2. No redirection, missing user
      CHECK(stdErr2.substr(21) ==
            "[rsandbox] ERROR Required option username not specified; LOGGED FROM: bool"
            " rstudio::core::program_options::{anonymous}::validateOptionsProvided(const"
            " rstudio_boost::program_options::variables_map&, const"
            " rstudio_boost::program_options::options_description&, const string&)"
            " src/cpp/core/ProgramOptions.cpp:46\n");
      CHECK(stdOut2 == "");

      // 3. Stdout redirection
      CHECK(stdErr3 == "");
      CHECK(stdOut3 == "");

      // 4. Stderr redirection
      CHECK(stdErr4 == "");
      CHECK(stdOut4 == "");

      // 5. Environment variables
      CHECK(stdErr4 == "");
      CHECK(stdOut4 == "Hello, world!");
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

      std::string stdErr, stdOut;
      AsyncProcessCallbacks callbacks;
      callbacks.OnError = failOnError;
      callbacks.OnStandardError = OUTPUT_CALLBACK(stdErr);
      callbacks.OnStandardOutput = OUTPUT_CALLBACK(stdOut);
      callbacks.OnExit = EXIT_CALLBACK(0);

      CHECK_FALSE(ProcessSupervisor::runAsyncProcess(opts, callbacks));
      CHECK(stdErr == "");
      CHECK(stdOut == stdOutExpected + stdErrExpected);

      ::close(pipe[0]);
      ::close(pipe[1]);
   }

   ProcessSupervisor::terminateAll();
   ProcessSupervisor::waitForExit();
}

} //namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
