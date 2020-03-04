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
   DateTime(DateTime&& in_other);

   /**
    * @brief Constructs a DateTime from an ISO 8601 string representation.
    *
    * @param in_iso8601TimeStr      The string representation of the DateTime to construct.
    * @param out_dateTime           The newly constructed DateTime, if no error occurs.
    *
    * @return Success if in_iso8601TimeStr is a valid ISO 8601 representation of a date and time; Error otherwise.
    */
   static Error fromString(const std::string& in_iso8601TimeStr, DateTime& out_dateTime);

   /**
    * @brief Constructs a DateTime from an ISO 8601 string representation.
    *
    * @param in_iso8601TimeStr      The string representation of the DateTime to construct.
    * @param in_timeZone            The time zone of the time string, in the form [+/-]H:MM (e.g. -4:30). This should
    *                               only be provided if the time string is not already in UTC time and the time zone is
    *                               not included in the time string.
    * @param out_dateTime           The newly constructed DateTime, if no error occurs.
    *
    * @return Success if in_iso8601TimeStr is a valid ISO 8601 representation of a date and time; Error otherwise.
    */
   static Error fromString(const std::string& in_iso8601TimeStr, const std::string& in_timeZone, DateTime& out_dateTime);

   /**
    * @brief Constructs a DateTime object at the current time.
    *
    * @return A DateTime object representing the time at which it was created.
    */
   static DateTime getCurrentTime();

   /**
    * @brief Equality operator.
    *
    * @param in_other   The DateTime to compare against this.
    *
    * @return True if this DateTime and in_other represent the exact same moment in time; false otherwise.
    */
   bool operator==(const DateTime& in_other) const;

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
    * @brief Converts this DateTime to an ISO 8601 time string.
    *
    * @return This DateTime as an ISO 8601 string representation.
    */
   std::string toString() const;

private:
   /**
    * @brief Private constructor.
    */
   DateTime();

   // The private implemenation of DateTime.
   PRIVATE_IMPL(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
