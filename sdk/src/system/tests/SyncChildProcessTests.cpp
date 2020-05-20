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
#include <system/Process.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace process {

TEST_CASE("Create Processes")
{
   // Make sure default options are populated.
   REQUIRE_FALSE(
      options::Options::getInstance().readOptions(
         0,
         nullptr,
         system::FilePath("conf-files/Empty.conf")));

   SECTION("No redirection")
   {
      ProcessOptions opts;
      opts.Executable = "echo";
      opts.Arguments.emplace_back("-n");
      opts.Arguments.emplace_back("output");
      opts.IsShellCommand = true;

      REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_FOUR, opts.RunAsUser));

      ProcessResult result;
      SyncChildProcess child(opts);
      REQUIRE_FALSE(child.run(result));
      CHECK(result.ExitCode == 0);
      CHECK(result.StdError == "");
      CHECK(result.StdOut == "output");
   }
}

} // namespace process
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
