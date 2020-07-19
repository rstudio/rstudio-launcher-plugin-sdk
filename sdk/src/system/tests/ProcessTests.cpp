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

#include <AsioRaii.hpp>

#include <options/Options.hpp>
#include <system/PosixSystem.hpp>
#include <system/Process.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

AsioRaii s_asioInit;

TEST_CASE("Get children")
{
   // Make sure default options are populated.
   REQUIRE_FALSE(options::Options::getInstance().readOptions(0, nullptr, system::FilePath()));

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

   AsyncProcessCallbacks cbs;
   bool failed = false;
   int exitCode = -1;
   std::string stdOut, stdErr;
   cbs.OnError = [&failed](const Error& in_error) { failed = true; };
   cbs.OnExit = [&exitCode](int in_exitCode) { exitCode = in_exitCode; };
   cbs.OnStandardError = [&stdErr](const std::string& in_err) { stdErr.append(in_err); };
   cbs.OnStandardOutput = [&stdOut](const std::string& in_out) { stdOut.append(in_out); };

   std::shared_ptr<AbstractChildProcess> child;
   Error error = ProcessSupervisor::runAsyncProcess(opts, cbs, &child);
   REQUIRE_FALSE(error);
   // Give a quarter of a second for the child process info to be populated in /proc.
   usleep(250000);

   std::vector<ProcessInfo> process;
   error = getChildProcesses(child->getPid(), process);
   CHECK_FALSE(error);

   // We are expecting 6 processes - one for the initial /bin/sh -c /bin/sh, one for the second /bin/sh, and one for each of the sleeps.
   CHECK(process.size() == 6);

   // Give the processes a chance to exit.
   CHECK_FALSE(ProcessSupervisor::waitForExit(TimeDuration::Seconds(2)));
   CHECK_FALSE(ProcessSupervisor::hasRunningChildren());

   // Ensure the process are definitely exited.
   ProcessSupervisor::terminateAll();
   ProcessSupervisor::waitForExit();

   CHECK(exitCode == 0);
   CHECK_FALSE(failed);
   CHECK(stdOut == "");
   CHECK(stdErr == "");
}

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio