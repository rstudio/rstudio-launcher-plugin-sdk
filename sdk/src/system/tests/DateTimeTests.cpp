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

#include <unistd.h>

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

TEST_CASE("Equality and Inequality")
{
   SECTION("Two current times")
   {
      DateTime d1, d2(d1);
      CHECK(d1 == d2);
   }

   SECTION("Current time and other time")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));
      CHECK(d1 != d2);
   }

   SECTION("Two non-current times (same initial TZ)")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK(d1 == d2);
   }

   SECTION("Two non-current times (different initial TZ)")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T03:23:44.039876-8:00", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK(d1 == d2);
   }

   SECTION("Two different current times")
   {
      DateTime d1;

      // sleep for 1.2 milliseconds. (There should still be a difference)
      usleep(1200);
      DateTime d2;

      CHECK(d1 != d2);
   }

   SECTION("Two non-current times (inequal, different TZ)")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T03:23:44.039876-5:00", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK(d1 != d2);
   }

   SECTION("Same object")
   {
      DateTime d;
      CHECK(d == d);
   }
}

TEST_CASE("LT/LTE/GT/GTE Comparisons")
{
   SECTION("Two current times")
   {
      DateTime d1, d2(d1);

      CHECK_FALSE(d1 < d2);
      CHECK(d1 <= d2);
      CHECK_FALSE(d1 > d2);
      CHECK(d1 >= d2);
   }

   SECTION("Current time and other time")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK_FALSE(d1 < d2);
      CHECK_FALSE(d1 <= d2);
      CHECK(d1 > d2);
      CHECK(d1 >= d2);

      CHECK(d2 < d1);
      CHECK(d2 <= d1);
      CHECK_FALSE(d2 > d1);
      CHECK_FALSE(d2 >= d1);
   }

   SECTION("Two non-current times (same initial TZ)")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK_FALSE(d1 < d2);
      CHECK(d1 <= d2);
      CHECK_FALSE(d1 > d2);
      CHECK(d1 >= d2);
   }

   SECTION("Two non-current times (different initial TZ)")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T03:23:44.039876-8:00", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK_FALSE(d1 < d2);
      CHECK(d1 <= d2);
      CHECK_FALSE(d1 > d2);
      CHECK(d1 >= d2);
   }

   SECTION("Two different current times")
   {
      DateTime d1;

      // sleep for 1.2 milliseconds. (There should still be a difference)
      usleep(1200);
      DateTime d2;

      CHECK(d1 < d2);
      CHECK(d1 <= d2);
      CHECK_FALSE(d1 > d2);
      CHECK_FALSE(d1 >= d2);

      CHECK_FALSE(d2 < d1);
      CHECK_FALSE(d2 <= d1);
      CHECK(d2 > d1);
      CHECK(d2 >= d1);
   }

   SECTION("Two non-current times (inequal, different TZ)")
   {
      DateTime d1, d2;
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T03:23:44.039876-5:00", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));

      CHECK(d1 < d2);
      CHECK(d1 <= d2);
      CHECK_FALSE(d1 > d2);
      CHECK_FALSE(d1 >= d2);

      CHECK_FALSE(d2 < d1);
      CHECK_FALSE(d2 <= d1);
      CHECK(d2 > d1);
      CHECK(d2 >= d1);
   }

   SECTION("Same object")
   {
      DateTime d;

      CHECK_FALSE(d < d);
      CHECK(d <= d);
      CHECK_FALSE(d > d);
      CHECK(d >= d);
   }
}

TEST_CASE("Assignment")
{
   SECTION("Current into current")
   {
      DateTime d1;
      usleep(1200);
      DateTime d2 = d1; // These won't be equal unless assignment works.

      CHECK(d1 == d2);
   }

   SECTION("Current into non-current")
   {
      DateTime d1, d2;

      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));
      d2 = d1;

      CHECK(d1 == d2);
      CHECK(d1.toString() != "2019-02-15T11:23:44.039876Z");
      CHECK(d2.toString() != "2019-02-15T11:23:44.039876Z");
   }

   SECTION("Non-current into current")
   {
      DateTime d1, d2;

      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));
      d1 = d2;

      CHECK(d1 == d2);
      CHECK(d1.toString() == "2019-02-15T11:23:44.039876Z");
      CHECK(d2.toString() == "2019-02-15T11:23:44.039876Z");
   }

   SECTION("Non-current into non-current")
   {
      DateTime d1, d2;

      REQUIRE_FALSE(DateTime::fromString("2020-02-15T11:23:44.039876Z", d1));
      REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d2));
      d2 = d1;

      CHECK(d1 == d2);
      CHECK(d1.toString() == "2020-02-15T11:23:44.039876Z");
      CHECK(d2.toString() == "2020-02-15T11:23:44.039876Z");
   }
}

TEST_CASE("Add times")
{
   DateTime d;
   REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", d));

   SECTION("Hours")
   {
      DateTime res = const_cast<const DateTime*>(&d)->addHours(6);

      CHECK(res.toString() == "2019-02-15T17:23:44.039876Z");
      CHECK(d.addHours(28).toString() == "2019-02-16T15:23:44.039876Z");
   }

   SECTION("Microseconds")
   {
      DateTime res = const_cast<const DateTime*>(&d)->addMicroseconds(204);

      CHECK(res.toString() == "2019-02-15T11:23:44.040080Z");
      CHECK(d.addMicroseconds(300030).toString() == "2019-02-15T11:23:44.339906Z");
   }

   SECTION("Minutes")
   {
      DateTime res = const_cast<const DateTime*>(&d)->addMinutes(17);

      CHECK(res.toString() == "2019-02-15T11:40:44.039876Z");
      CHECK(d.addMinutes(1508).toString() == "2019-02-16T12:31:44.039876Z");
   }

   SECTION("Seconds")
   {
      DateTime res = const_cast<const DateTime*>(&d)->addSeconds(8);

      CHECK(res.toString() == "2019-02-15T11:23:52.039876Z");
      CHECK(d.addSeconds(10800).toString() == "2019-02-15T14:23:44.039876Z");
   }
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
