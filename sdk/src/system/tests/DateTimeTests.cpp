/*
 * DateTimeTests.cpp
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

#include <Error.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

TEST_CASE("Construction and simple toString")
{
   // This is hard to validate because it will change each time - mostly just checks that we won't crash.
   SECTION("Current Time")
   {
      DateTime d;
      CHECK(true);
   }

   SECTION("From ISO 8601 str (UTC)")
   {
      std::string timeStr = "2019-02-15T11:23:44.039876Z";
      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == timeStr);
   }

   SECTION("From ISO 8601 str (+5:30)")
   {
      std::string expectedTime = "2019-02-15T05:53:44.039876Z";
      std::string timeStr =      "2019-02-15T11:23:44.039876+5:30";

      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == expectedTime);
   }

   SECTION("From ISO 8601 str (-5:00)")
   {
      std::string expectedTime = "2019-02-15T16:23:44.039876Z";
      std::string timeStr =      "2019-02-15T11:23:44.039876-5:00";

      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == expectedTime);
   }

   SECTION("From ISO 8601 str (Full posix time zone string)")
   {
      std::string expectedTime = "2019-02-15T19:23:44.039876Z";
      std::string timeStr =      "2019-02-15T11:23:44.039876PST-08PDT+01,M4.1.0/02:00,M10.5.0/02:00";

      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == expectedTime);
   }

   SECTION("Copy construction")
   {
      DateTime d1;
      DateTime d2(d1);

      CHECK(d1 == d2);
      CHECK(d1.toString() == d2.toString());
   }
}

TEST_CASE("Complex toString")
{
   SECTION("Alternate format (%y/%m/%d %H:%M:%S%Q")
   {
      std::string timeStr = "2019-02-15T11:23:44.039876Z";
      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == timeStr);
      CHECK(d.toString("%y/%m/%d %H:%M:%S%Q") == "19/02/15 11:23:44");
   }

   SECTION("Alternate format (%b %d, %Y)")
   {
      std::string timeStr = "2019-02-15T11:23:44.039876Z";
      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == timeStr);
      CHECK(d.toString("%b %d, %Y") == "Feb 15, 2019");
   }

   SECTION("Alternate format (%A, %B %d %I:%M:%S %p")
   {
      std::string timeStr = "2019-02-15T23:23:44.039876Z";
      DateTime d;

      REQUIRE_FALSE(DateTime::fromString(timeStr, d));
      CHECK(d.toString() == timeStr);
      CHECK(d.toString("%A, %B %d %I:%M:%S %p") == "Friday, February 15 11:23:44 PM");
   }
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
