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

TEST_CASE("TimeDuration Construction")
{
   SECTION("Standard Constructor")
   {
      TimeDuration i1(5, 24, 57, 109827),
                   i2(-14, -31, -16, -94821),
                   i3(50, 103, 72, 874680098),
                   i4(-36, -444, -91, -39827160),
                   i5(54, -71, -114, 100398764);

      // i1
      CHECK(!i1.isInfinity());
      CHECK(i1.getMicroseconds() == 109827);
      CHECK(i1.getSeconds() == 57);
      CHECK(i1.getMinutes() == 24);
      CHECK(i1.getHours() == 5);

      // i2
      CHECK(!i2.isInfinity());
      CHECK(i2.getMicroseconds() == -94821);
      CHECK(i2.getSeconds() == -16);
      CHECK(i2.getMinutes() == -31);
      CHECK(i2.getHours() == -14);

      // i3
      CHECK(!i3.isInfinity());
      CHECK(i3.getMicroseconds() == 680098);
      CHECK(i3.getSeconds() == 46);
      CHECK(i3.getMinutes() == 58);
      CHECK(i3.getHours() == 51);

      // i4
      CHECK(!i4.isInfinity());
      CHECK(i4.getMicroseconds() == -827160);
      CHECK(i4.getSeconds() == -10);
      CHECK(i4.getMinutes() == -26);
      CHECK(i4.getHours() == -43);

      // i5 - If one field is negative all are negative (apparently).
      CHECK(!i5.isInfinity());
      CHECK(i5.getMicroseconds() == -398764);
      CHECK(i5.getSeconds() == -34);
      CHECK(i5.getMinutes() == -14);
      CHECK(i5.getHours() == -55);
   }

   SECTION("Copy constructor and equality")
   {
      TimeDuration i1(5, 24, 57, 109827),
                   i2(i1),
                   i3(5, 24, 57, 109827);

      CHECK(!i1.isInfinity());
      CHECK(!i2.isInfinity());
      CHECK(!i3.isInfinity());
      CHECK(i1 == i2);
      CHECK(i1 == i3);
      CHECK(i2 == i3);

      i1 = TimeDuration(5, 10, 12, 49);

      CHECK(!i1.isInfinity());
      CHECK(!i2.isInfinity());
      CHECK(!i3.isInfinity());
      CHECK(i1 != i2);
      CHECK(i1 != i3);
      CHECK(i2 == i3);
   }

   SECTION("Helper Constructors")
   {
      TimeDuration i1a(1, 31, 26), i1b = TimeDuration::Seconds(5486),
         i2a(15), i2b = TimeDuration::Hours(15),
         i3a(0, 26), i3b = TimeDuration::Minutes(26),
         i4a(0, 0, 48), i4b = TimeDuration::Seconds(48),
         i5a(0, 0, 0, 150387), i5b = TimeDuration::Microseconds(150387),
         i6 = TimeDuration::Infinity();

      // i1
      CHECK(!i1a.isInfinity());
      CHECK(!i1b.isInfinity());
      CHECK(i1a == i1b);
      CHECK(i1b.getHours() == 1);
      CHECK(i1b.getMinutes() == 31);
      CHECK(i1b.getSeconds() == 26);
      CHECK(i1b.getMicroseconds() == 0);

      // i2
      CHECK(!i2a.isInfinity());
      CHECK(!i2b.isInfinity());
      CHECK(i2a == i2b);
      CHECK(i2b.getHours() == 15);
      CHECK(i2b.getMinutes() == 0);
      CHECK(i2b.getSeconds() == 0);
      CHECK(i2b.getMicroseconds() == 0);

      // i3
      CHECK(!i3a.isInfinity());
      CHECK(!i3b.isInfinity());
      CHECK(i3a == i3b);
      CHECK(i3b.getHours() == 0);
      CHECK(i3b.getMinutes() == 26);
      CHECK(i3b.getSeconds() == 0);
      CHECK(i3b.getMicroseconds() == 0);

      // i4
      CHECK(!i4a.isInfinity());
      CHECK(!i4b.isInfinity());
      CHECK(i4a == i4b);
      CHECK(i4b.getHours() == 0);
      CHECK(i4b.getMinutes() == 0);
      CHECK(i4b.getSeconds() == 48);
      CHECK(i4b.getMicroseconds() == 0);

      // i5
      CHECK(!i5a.isInfinity());
      CHECK(!i5b.isInfinity());
      CHECK(i5a == i5b);
      CHECK(i5b.getHours() == 0);
      CHECK(i5b.getMinutes() == 0);
      CHECK(i5b.getSeconds() == 0);
      CHECK(i5b.getMicroseconds() == 150387);

      // i6
      CHECK(i6.isInfinity());
      CHECK(i6 != i1a);
      CHECK(i6 != i1b);
      CHECK(i6 != i2a);
      CHECK(i6 != i2b);
      CHECK(i6 != i3a);
      CHECK(i6 != i3b);
      CHECK(i6 != i4a);
      CHECK(i6 != i4b);
      CHECK(i6 != i5a);
      CHECK(i6 != i5b);
      CHECK(i6 == i6);
      CHECK(i6 == TimeDuration::Infinity());
   }
}

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
      TimeDuration i1 = TimeDuration::Hours(6), i2 = TimeDuration::Hours(28);
      DateTime res = d + i1;
      d += i2;

      CHECK(res.toString() == "2019-02-15T17:23:44.039876Z");
      CHECK(d.toString() == "2019-02-16T15:23:44.039876Z");
   }

   SECTION("Microseconds")
   {
      TimeDuration i1 = TimeDuration::Microseconds(204), i2 = TimeDuration::Microseconds(300030);
      DateTime res = d + i1;
      d += i2;

      CHECK(res.toString() == "2019-02-15T11:23:44.040080Z");
      CHECK(d.toString() == "2019-02-15T11:23:44.339906Z");
   }

   SECTION("Minutes")
   {
      TimeDuration i1 = TimeDuration::Minutes(17), i2 = TimeDuration::Minutes(1508);
      DateTime res = d + i1;
      d += i2;

      CHECK(res.toString() == "2019-02-15T11:40:44.039876Z");
      CHECK(d.toString() == "2019-02-16T12:31:44.039876Z");
   }

   SECTION("Seconds")
   {
      TimeDuration i1 = TimeDuration::Seconds(8), i2 = TimeDuration::Seconds(10800);
      DateTime res = d + i1;
      d += i2;

      CHECK(res.toString() == "2019-02-15T11:23:52.039876Z");
      CHECK(d.toString() == "2019-02-15T14:23:44.039876Z");
   }

   SECTION("Composite")
   {
      TimeDuration i1(9, 0, 6, 60124),
                   i2(493, 65,34, 960124);
      DateTime res = d + i1;
      d += i2;

      CHECK(res.toString() == "2019-02-15T20:23:50.100000Z");
      CHECK(d.toString() == "2019-03-08T01:29:19Z");
   }
}

TEST_CASE("Add and Subtract Tests")
{
   DateTime later, earlier;
   REQUIRE_FALSE(DateTime::fromString("2019-02-15T18:43:49.040876Z", later));
   REQUIRE_FALSE(DateTime::fromString("2019-02-15T11:23:44.039876Z", earlier));

   TimeDuration difference(7, 20, 5, 1000),
      negDifference(-7, -20, -5, -1000);

   CHECK((later - earlier) == difference);
   CHECK((earlier - later) == negDifference);
   CHECK((earlier + difference) == later);
   CHECK((earlier - negDifference) == later);
   CHECK((later - difference) == earlier);
   CHECK((later + negDifference) == earlier);
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
