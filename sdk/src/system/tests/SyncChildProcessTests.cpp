/*
 * SyncChildProcessTests.cpp
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

#include "TestMain.hpp"

#include <options/Options.hpp>
#include <system/FilePath.hpp>
#include <system/PosixSystem.hpp>
#include <system/Process.hpp>

#include "../../tests/MockLogDestination.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

TEST_CASE("Create Processes")
{
   // Make sure default options are populated.
   REQUIRE_FALSE(options::Options::getInstance().readOptions(0, nullptr, system::FilePath()));

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

   SECTION("No redirection, Success")
   {
      ProcessOptions opts;
      opts.Executable = "/bin/echo";
      opts.Arguments.emplace_back("-n");
      opts.Arguments.emplace_back("output");
      opts.IsShellCommand = false;
      opts.RunAsUser = user4;

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "output");
   }

   SECTION("No redirection, Bad Command")
   {
      ProcessOptions opts;
      opts.Executable = "grep";
      opts.Arguments.emplace_back("-x");
      opts.IsShellCommand = true;
      opts.RunAsUser = user3;

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 2);
      CHECK(result.StdError == "Usage: grep [OPTION]... PATTERN [FILE]...\n"
                               "Try 'grep --help' for more information.\n");
      CHECK(result.StdOut == "");
   }

   SECTION("No redirection, missing user")
   {
      ProcessOptions opts;
      opts.Executable = "grep";
      opts.Arguments.emplace_back("-x");
      opts.IsShellCommand = true;
      opts.RunAsUser = system::User(true);

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 1);
      CHECK((!result.StdError.empty() && (result.StdError.substr(21) ==
         "[rsandbox] ERROR Required option username not specified; LOGGED FROM: bool"
         " rstudio::core::program_options::{anonymous}::validateOptionsProvided(const"
         " rstudio_boost::program_options::variables_map&, const"
         " rstudio_boost::program_options::options_description&, const string&)"
         " src/cpp/core/ProgramOptions.cpp:46\n")));
      CHECK(result.StdOut == "");
   }

   SECTION("Stdout redirection")
   {
      ProcessOptions opts;
      opts.Executable = "/bin/echo";
      opts.Arguments.emplace_back("-ne");
      opts.Arguments.emplace_back(stdOutExpected);
      opts.IsShellCommand = true;
      opts.RunAsUser = user1;
      opts.StandardOutputFile = user1.getHomePath().completeChildPath("test-out.txt");

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "");
   }

   SECTION("Stderr redirection")
   {
      ProcessOptions opts;
      opts.Executable = "fakecmd";
      opts.Arguments.emplace_back("-n");
      opts.Arguments.emplace_back("-e");
      opts.Arguments.emplace_back(stdOutExpected);
      opts.IsShellCommand = true;
      opts.RunAsUser = user1;
      opts.StandardErrorFile = user1.getHomePath().completeChildPath("test-err.txt");

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 127);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "");
   }

   SECTION("Open file descriptors in parent, working dir")
   {
      int pipe[2];
      REQUIRE_FALSE(posix::posixCall<int>(std::bind(::pipe, pipe), ERROR_LOCATION));

      ProcessOptions opts;
      opts.Executable = "cat";
      opts.Arguments.emplace_back("test-out.txt");
      opts.Arguments.emplace_back("test-err.txt");
      opts.IsShellCommand = true;
      opts.RunAsUser = user1;
      opts.WorkingDirectory = user1.getHomePath();

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");

      std::string expected = stdOutExpected;
      if (result.StdOut.find("command") != std::string::npos)
         expected.append(stdErrAltExpected);
      else
         expected.append(stdErrExpected);

      CHECK(result.StdOut == expected);

      ::close(pipe[0]);
      ::close(pipe[1]);
   }

   SECTION("Env variables")
   {
      ProcessOptions opts;
      opts.Executable = "./test.sh";
      opts.IsShellCommand = false;
      opts.Environment.emplace_back("VAR", "Hello, world!");
      opts.RunAsUser = user3;
      opts.WorkingDirectory = FilePath::safeCurrentPath(FilePath());

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "Hello, world!");
   }

   SECTION("Password logging")
   {
      logging::MockLogPtr mockLog = logging::getMockLogDest();

      ProcessOptions opts;
      opts.Executable = "./test.sh";
      opts.IsShellCommand = false;
      opts.Environment.emplace_back("VAR", "Hello, world!");
      opts.Environment.emplace_back("VAR2", "Something else!");
      opts.RunAsUser = user2;
      opts.Password = "test-pwd";
      opts.WorkingDirectory = FilePath::safeCurrentPath(FilePath());

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "Hello, world!");

      // Check the log.
      REQUIRE(mockLog->getSize() == 1);
      CHECK(mockLog->peek().Level == logging::LogLevel::DEBUG);
      CHECK(mockLog->peek().Message.find("test-pwd") == std::string::npos);
      CHECK(mockLog->pop().Message.find(R"("password":"<redacted>")") != std::string::npos);
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

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "Mount test passed!");
   }
}

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
