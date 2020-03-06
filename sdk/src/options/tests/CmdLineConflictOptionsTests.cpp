/*
 * CmdLineConflictsOptionsTests.cpp
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

#include <TestMain.hpp>

#include <options/Options.hpp>
#include <system/FilePath.hpp>
#include <system/User.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace options {

TEST_CASE("command line options with conflicts")
{
   SECTION("read options")
   {
      constexpr const char* argv[] = {
         "options-test",               // The first element should be the process name. Make it up.
         "--log-level=WARNING",
         "--heartbeat-interval-seconds=8",
         "--thread-pool-size=2",
      };

      constexpr int argc = 4;

      Options& opts = Options::getInstance();
      Error error = opts.readOptions(argc, argv, system::FilePath("./conf-files/MissingOptional.conf"));
      REQUIRE(!error);
   }

   SECTION("check values")
   {
      Options& opts = Options::getInstance();
      system::User serverUser;
      Error error = opts.getServerUser(serverUser);

      REQUIRE(error);
      REQUIRE(error.getProperty("description") == "Failed to get user details.");
      REQUIRE(error.getProperty("user-value") == "aUser");
      REQUIRE(error.getCode() == 13);
      REQUIRE(error.getName() == systemError(1, ErrorLocation()).getName());

      REQUIRE(opts.getJobExpiryHours() == 24);
      REQUIRE(opts.getHeartbeatIntervalSeconds() == 8);
      REQUIRE(opts.getLogLevel() == logging::LogLevel::DEBUG);
      REQUIRE(opts.getScratchPath().getAbsolutePath() == "/home/aUser/temp/");
      REQUIRE(opts.getThreadPoolSize() == 6);
   }
}

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

