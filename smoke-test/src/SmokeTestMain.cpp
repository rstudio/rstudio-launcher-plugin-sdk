/*
 * SmokeTestMain.cpp
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

#include <SmokeTest.hpp>

#include <iostream>

#include <system/FilePath.hpp>

using namespace rstudio::launcher_plugins;
using namespace rstudio::launcher_plugins::smoke_test;

/**
 * @brief The main function.
 *
 * @param in_argc      The number of arguments supplied to the program.
 * @param in_argv      The list of arguments supplied to the program.
 *
 * @return 0 on success; non-zero exit code otherwise.
 */
int main(int in_argc, char** in_argv)
{
   if (in_argc != 2)
   {
      std::cerr << "Unexpected number of arguments: " << in_argc << std::endl
                << "Usage: ./rlps-smoke-test <path/to/plugin/exe>" << std::endl;
      return 1;
   }

   SmokeTest tester((system::FilePath(in_argv[1])));
   Error error = tester.initialize();
   if (error)
   {
      std::cerr << "An error occurred while initializing: " << std::endl
                << error.asString() << std::endl;
      return 1;
   }

   while(tester.sendRequest());

   tester.stop();
   return 0;
}