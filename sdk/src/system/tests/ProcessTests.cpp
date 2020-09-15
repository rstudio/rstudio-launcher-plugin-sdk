/*
 * ProcessTests.cpp
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

#include <csignal>

#include <AsioRaii.hpp>

#include <system/PosixSystem.hpp>
#include <system/Process.hpp>

#include "ProcessTestHelpers.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

AsioRaii s_asioInit;

TEST_CASE("General tests")
{
   // Make sure default options are populated.
   REQUIRE_FALSE(loadOptions());

   AsyncProcessCallbacks cbs;
   bool failed = false;
   int exitCode = -1;
   std::string stdOut, stdErr;
   cbs.OnError = [&failed](const Error& in_error) { failed = true; };
   cbs.OnExit = [&exitCode](int in_exitCode) { exitCode = in_exitCode; };
   cbs.OnStandardError = [&stdErr](const std::string& in_err) { stdErr.append(in_err); };
   cbs.OnStandardOutput = [&stdOut](const std::string& in_out) { stdOut.append(in_out); };

   SECTION("Get children")
   {
      ProcessOptions opts;
      REQUIRE_FALSE(User::getUserFromIdentifier(USER_ONE, opts.RunAsUser));

      opts.IsShellCommand = false;
      opts.UseSandbox = false;
      opts.Executable ="/bin/sh";
      opts.StandardInput = "#!/bin/sh \n"
                           "sleep 2& \n"
                           "sleep 2& \n"
                           "sleep 2& \n"
                           "sleep 2";


      std::shared_ptr<AbstractChildProcess> child;
      REQUIRE_FALSE(ProcessSupervisor::runAsyncProcess(opts, cbs, &child));
      
      // Give a quarter of a second for the child process info to be populated in /proc.
      usleep(250000);

      std::vector<ProcessInfo> process;
      CHECK_FALSE(getChildProcesses(child->getPid(), process));

      // On some OS's are expecting 6 processes - one for the initial /bin/sh -c /bin/sh, one for the second /bin/sh, and
      // one for each of the sleeps. On others we're expecting only five - one for the /bin/sh and one for each of the sleeps.
      CHECK(((process.size() == 6) || (process.size() == 5)));

#ifdef NDEBUG
      // Give the process a chance to exit. A couple seconds should be more than enough.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(2)));
#else
      // Give the process a chance to exit. Be more generous in debug mode, sometimes stuff is slower.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(10)));
#endif

      if (ProcessSupervisor::hasRunningChildren())
      {
         // Ensure the processes are definitely exited.
         ProcessSupervisor::terminateAll();
         ProcessSupervisor::waitForExit();
      }

      CHECK(exitCode == 0);
      CHECK_FALSE(failed);
      CHECK(stdOut == "");
      CHECK(stdErr == "");
   }

   SECTION("Send kill signal, process group only")
   {
      int sig = SIGTERM;

      ProcessOptions opts;
      REQUIRE_FALSE(User::getUserFromIdentifier(USER_TWO, opts.RunAsUser));
      opts.IsShellCommand = false;
      opts.UseSandbox = false;
      opts.Executable ="/bin/sh";
      opts.StandardInput = "#!/bin/sh \n"
                           "sleep 20 \n"
                           "echo \"Failed\"";

      std::shared_ptr<AbstractChildProcess> child;
      REQUIRE_FALSE(ProcessSupervisor::runAsyncProcess(opts, cbs, &child));
      
      CHECK_FALSE(signalProcess(child.get()->getPid(), sig));

#ifdef NDEBUG
      // Give the process a chance to exit. A second should be more than enough.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(1)));
#else
      // Give the process a chance to exit. Be more generous in debug mode, sometimes stuff is slower.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(5)));
#endif

      if (ProcessSupervisor::hasRunningChildren())
      {
         // Ensure the processes are definitely exited.
         ProcessSupervisor::terminateAll();
         ProcessSupervisor::waitForExit();
      }

      CHECK(exitCode == sig);
      CHECK_FALSE(failed);
      CHECK(stdOut == "");
      CHECK(stdErr == "");
   }

   SECTION("Send term signal, all children not just group")
   {
      int sig = SIGTERM;

      ProcessOptions opts;
      REQUIRE_FALSE(User::getUserFromIdentifier(USER_THREE, opts.RunAsUser));
      opts.IsShellCommand = false;
      opts.UseSandbox = false;
      opts.Executable ="/bin/bash";
      opts.StandardInput = "#!/bin/bash \n"
                           "set -m \n"
                           "sleep 500& \n"
                           "sleep 500& \n"
                           "sleep 500& \n"
                           "sleep 500& \n"
                           "sleep 500 \n"
                           "echo \"Failed\"";

      std::shared_ptr<AbstractChildProcess> child;
      REQUIRE_FALSE(ProcessSupervisor::runAsyncProcess(opts, cbs, &child));

      // Sleep for half a second to give the script a chance to launch its children - it seems to be slower with set -m.
      usleep(500000);
      CHECK_FALSE(signalProcess(child.get()->getPid(), sig, false));

#ifdef NDEBUG
      // Give the process a chance to exit. A half a second should be more than enough.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Microseconds(5000000)));
#else
      // Give the process a chance to exit. Be more generous in debug mode, sometimes stuff is slower.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(5)));
#endif
      
      if (ProcessSupervisor::hasRunningChildren());
      {
         // Ensure the processes are definitely exited.
         ProcessSupervisor::terminateAll();
         ProcessSupervisor::waitForExit();
      }

      CHECK(exitCode == sig);
      CHECK_FALSE(failed);
      CHECK(stdOut == "");
      CHECK(stdErr == "");
   }

   SECTION("Send sigstop and resume, with sandbox")
   {
      ProcessOptions opts;
      REQUIRE_FALSE(User::getUserFromIdentifier(USER_FOUR, opts.RunAsUser));
      opts.IsShellCommand = true;
      opts.UseSandbox = true;
      opts.Executable ="sleep 1 && echo Success";

      std::shared_ptr<AbstractChildProcess> child;
      REQUIRE_FALSE(ProcessSupervisor::runAsyncProcess(opts, cbs, &child));

      CHECK_FALSE(signalProcess(child.get()->getPid(), SIGSTOP));
      CHECK(ProcessSupervisor::hasRunningChildren());
      CHECK(stdOut == "");
      CHECK_FALSE(signalProcess(child.get()->getPid(), SIGCONT));

#ifdef NDEBUG
      // Give the process a chance to exit. A couple seconds should be more than enough.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(2)));
#else
      // Give the process a chance to exit. Be more generous in debug mode, sometimes stuff is slower.
      CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(10)));
#endif
      
      if (ProcessSupervisor::hasRunningChildren());
      {
         // Ensure the processes are definitely exited.
         ProcessSupervisor::terminateAll();
         ProcessSupervisor::waitForExit();
      }

      CHECK(exitCode == 0);
      CHECK_FALSE(failed);
      CHECK(stdOut == "Success\n");
      CHECK(stdErr == "");
   }
}


} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio