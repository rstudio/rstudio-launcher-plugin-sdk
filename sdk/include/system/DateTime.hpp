/*
 * DateTime.hpp
 * 
 * Copyright (C) 2019-20 by RStudio, PBC
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

#ifndef LAUNCHER_PLUGINS_DATE_TIME_HPP
#define LAUNCHER_PLUGINS_DATE_TIME_HPP

#include <PImpl.hpp>

#include <string>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio


namespace rstudio {
namespace launcher_plugins {
namespace system {

/** @brief Class which represents a date and time in UTC. */
class DateTime
{
public:
   /**
    * @brief Constructor.
    *
    * Creates a date time which represents the time at which it was created.
    */
   DateTime();

   /**
    * @brief Copy constructor.
    *
    * @param in_other       The DateTime to copied into this.
    */
   DateTime(const DateTime& in_other);

   /**
    * @brief Move constructor.
    *
    * @param in_other       The DateTime to moved into this.
    */
   DateTime(DateTime&& in_other) noexcept;

   /**
    * @brief Constructs a DateTime from an ISO 8601 string representation. The string must be in UTC time.
    *
    * Valid format:
    *       "%Y-%m-%dT%H:%M:%S%F%ZP"
    *       e.g. "2020-03-05T14:33:15.008765Z"
    *       e.g. "1995-10-31T02:06:22+8:00" (fractional seconds are 0)
    *       e.g. "1988-12-25T23:23:23.054321MST-06"
    *       e.g. "1972-04-18T00:01:51PST-08PDT+01,M4.1.0/02:00,M10.5.0/02:00" (Full Posix Time Zone String)
    *
    * @param in_timeStr             The string representation of the DateTime to construct.
    * @param out_dateTime           The newly constructed DateTime, if no error occurs.
    *
    * @return Success if in_timeStr is a valid ISO 8601 representation of a date and time; Error otherwise.
    */
   static Error fromString(const std::string& in_timeStr, DateTime& out_dateTime);

   /**
    * @brief Assignment operator.
    *
    * @param in_other   The DateTime to assign to this.
    *
    * @return A reference to this DateTime.
    */
   DateTime& operator=(const DateTime& in_other);

   /**
    * @brief Equality operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime and in_other represent the exact same moment in time; false otherwise.
    */
   bool operator==(const DateTime& in_other) const;

   /**
    * @brief Inequality operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime and in_other represent the different times; false otherwise.
    */
   bool operator!=(const DateTime& in_other) const;

   /**
    * @brief Less than operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime is an earlier time than in_other; false otherwise.
    */
   bool operator<(const DateTime& in_other) const;

   /**
    * @brief Less than operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime is an earlier time or the same time as in_other; false otherwise.
    */
   bool operator<=(const DateTime& in_other) const;

   /**
    * @brief Less than operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime is a later time than in_other; false otherwise.
    */
   bool operator>(const DateTime& in_other) const;

   /**
    * @brief Less than operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime is the same time or a later time than in_other; false otherwise.
    */
   bool operator>=(const DateTime& in_other) const;

   /**
    * @brief Adds the specified number of days to this DateTime.
    *
    * @param in_days    The number of days to add to this DateTime.
    *
    * @return A refernce to this DateTime.
    */
   DateTime& addDays(int in_days);

   /**
    * @brief Adds the specified number of days to a copy of this DateTime.
    *
    * @param in_days    The number of days to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addDays(int in_days) const;

   /**
    * @brief Adds the specified number of hours to this DateTime.
    *
    * @param in_hours   The number of hours to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& addHours(int in_hours);

   /**
    * @brief Adds the specified number of hours to a copy of this DateTime.
    *
    * @param in_hours   The number of hours to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addHours(int in_hours) const;

   /**
    * @brief Adds the specified number of microseconds to this DateTime.
    *
    * @param in_microseconds    The number of microseconds to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& addMicroseconds(int in_microseconds);

   /**
    * @brief Adds the specified number of microseconds to a copy of this DateTime.
    *
    * @param in_microseconds    The number of microseconds to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addMicroseconds(int in_microseconds) const;

   /**
    * @brief Adds the specified number of minutes to this DateTime.
    *
    * @param in_minutes     The number of minutes to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& addMinutes(int in_minutes);

   /**
    * @brief Adds the specified number of minutes to a copy of this DateTime.
    *
    * @param in_minutes     The number of minutes to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addMinutes(int in_minutes) const;

   /**
    * @brief Adds the specified number of months to this DateTime.
    *
    * @param in_months      The number of months to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& addMonths(int in_months);

   /**
    * @brief Adds the specified number of months to a copy this DateTime.
    *
    * @param in_months      The number of months to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addMonths(int in_months) const;

   /**
    * @brief Adds the specified number of seconds to this DateTime.
    *
    * @param in_seconds     The number of seconds to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& addSeconds(int in_seconds);

   /**
    * @brief Adds the specified number of seconds to a copy of this DateTime.
    *
    * @param in_seconds     The number of seconds to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addSeconds(int in_seconds) const;

   /**
    * @brief Adds the specified number of years to this DateTime.
    *
    * @param in_years       The number of years to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& addYears(int in_years);

   /**
    * @brief Adds the specified number of years to a copy of this DateTime.
    *
    * @param in_years       The number of years to add to this DateTime.
    *
    * @return The new DateTime.
    */
   DateTime addYears(int in_years) const;

   /**
    * @brief Converts this DateTime to an ISO 8601 time string.
    *
    * @return This DateTime as an ISO 8601 string representation.
    */
   std::string toString() const;

   /**
    * @brief Converts this DateTime to a string representation defined by the provided format.
    *
    * @param in_format      The time format string, as documented in the 'Date-Time Support' section of the
    *                       'Advanced Features' chapter of the RStudio Launcher Plugin SDK Developer's Guide.
    *
    * @return This DateTime, as a string with the specified format.
    */
   std::string toString(const char* in_format) const;

   /**
    * @brief Converts this DateTime to a string representation defined by the provided format.
    *
    * @param in_format      The time format string, as documented in the 'Date-Time Support' section of the
    *                       'Advanced Features' chapter of the RStudio Launcher Plugin SDK Developer's Guide.
    *
    * @return This DateTime, as a string with the specified format.
    */
   std::string toString(const std::string& in_format) const;

private:
   // The private implementation of DateTime.
   PRIVATE_IMPL(m_impl);
};



} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
