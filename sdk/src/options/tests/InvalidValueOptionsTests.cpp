/*
 * InvalidValueOptionsTests.cpp
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

TEST_CASE("option with invalid value")
{
   SECTION("read options")
   {
      const system::FilePath configFile("./conf-files/InvalidValue.conf");
      const std::string expectedMessage = "Error reading " + configFile.getAbsolutePath() + ": the argument ('7') for"
                                          " option 'log-level' is invalid";
      constexpr const char* argv[] = {};
      constexpr int argc = 0;

      Options& opts = Options::getInstance();
      Error error = opts.readOptions(argc, argv, configFile);
      CHECK(error);
      CHECK(error.getName() == "OptionReadError");
      CHECK(error.getMessage() == expectedMessage);
   }
}

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

