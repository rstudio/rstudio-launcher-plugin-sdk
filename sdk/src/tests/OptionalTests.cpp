/*
 * OptionalTests.cpp
 *
 * Copyright (C) 2020 by RStudio, PBC
 *
 * Unless you have received this program directly from RStudio pursuant to the terms of a commercial license agreement
 * with RStudio, then this program is licensed to you under the following terms:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to doFAILURES=0

for test in ./*-tests;
do
  echo "Running ${test}..."
  ${test}
  FAILURES=$((FAILURES + $?))
done

exit $FAILURES so, subject to the following conditions:
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

#include <Optional.hpp>

namespace rstudio {
namespace launcher_plugins {

TEST_CASE("Default optional is empty")
{
   Optional<int> opt;

   CHECK(!opt);
   CHECK(!opt.hasValue());
   CHECK(opt.getValueOr(22) == 22);
}

TEST_CASE("Populated optional is not empty")
{
   Optional<int> opt(10);

   CHECK(opt);
   CHECK(opt.hasValue());
   CHECK(opt.getValueOr(22) == 10);
}

TEST_CASE("String optional works")
{
   Optional<std::string> opt("hello");

   CHECK(opt);
   CHECK(opt.hasValue());
   CHECK(opt.getValueOr("") == "hello");
}

TEST_CASE("Assignment works")
{
   Optional<float> opt;

   CHECK(!opt);
   CHECK(opt.getValueOr(0.2) == 0.2f);

   opt = 1.2;

   CHECK(opt);
   CHECK(opt.hasValue());
   CHECK(opt.getValueOr(0.2) == 1.2f);
}

TEST_CASE("Pointer construction works")
{
   Optional<std::string> opt(new std::string("Hello, world!"));

   CHECK(opt);
   CHECK(opt.getValueOr("") == "Hello, world!");
}

TEST_CASE("Pointer assignment works")
{
   Optional<std::string> opt;

   CHECK(!opt);
   CHECK(opt.getValueOr("").empty());

   opt = new std::string("Goodbye!");

   CHECK(opt);
   CHECK(opt.getValueOr("") == "Goodbye!");
}

} // namespace launcher_plugins
} // namespace rstudio
