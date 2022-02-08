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

#include <string>

#include <PImpl.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <boost/date_time/local_time/local_time.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio


namespace rstudio {
namespace launcher_plugins {
namespace system {


// Forward Declaration
class DateTime;

/**
 * @brief Represents an duration of time (e.g. 5 hours, 43 minutes, and 21 seconds) as opposed to a point in time.
 */
class TimeDuration final
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_hours           The number of hours in this TimeDuration.
    * @param in_minutes         The number of minutes in this TimeDuration.
    * @param in_seconds         The number of seconds in this TimeDuration.
    * @param in_microseconds    The number of microseconds in this TimeDuration.
    */
   explicit TimeDuration(
      int64_t in_hours = 0,
      int64_t in_minutes = 0,
      int64_t in_seconds = 0,
      int64_t in_microseconds = 0);

   /**
    * @brief Copy constructor.
    *
    * @param in_other   The TimeDuration to copy into this TimeDuration.
    */
   TimeDuration(const TimeDuration& in_other);

   /**
    * @brief Move constructor.
    *
    * @param in_other   The TimeDuration to move into this TimeDuration.
    */
   TimeDuration(TimeDuration&& in_other) noexcept;

   /**
    * @brief Destructor.
    */
   ~TimeDuration() = default;

   /**
    * @brief Constructs a TimeDuration which represents "any amount of time". Use with caution.
    *
    * @return The new TimeDuration.
    */
   static TimeDuration Infinity();

   /**
    * @brief Constructs an TimeDuration which represents the specified number of hours.
    *
    * @param in_hours       The number of hours which should be represented by the TimeDuration.
    *
    * @return The new TimeDuration.
    */
   static TimeDuration Hours(int64_t in_hours);

   /**
    * @brief Constructs an TimeDuration which represents the specified number of minutes.
    *
    * @param in_minutes     The number of minutes which should be represented by the TimeDuration.
    *
    * @return The new TimeDuration.
    */
   static TimeDuration Minutes(int64_t in_minutes);

   /**
    * @brief Constructs an TimeDuration which represents the specified number of seconds.
    *
    * @param in_seconds     The number of seconds which should be represented by the TimeDuration.
    *
    * @return The new TimeDuration.
    */
   static TimeDuration Seconds(int64_t in_seconds);

   /**
    * @brief Constructs an TimeDuration which represents the specified number of microseconds.
    *
    * @param in_microseconds        The number of microseconds which should be represented by the TimeDuration.
    *
    * @return The new TimeDuration.
    */
   static TimeDuration Microseconds(int64_t in_microseconds);

   /**
    * @brief Assignment operator.
    *
    * @param in_other   The TimeDuration to copy into this TimeDuration.
    *
    * @return A reference to this TimeDuration.
    */
   TimeDuration& operator=(const TimeDuration& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other   The TimeDuration to move into this TimeDuration.
    *
    * @return A reference to this TimeDuration.
    */

   TimeDuration& operator=(TimeDuration&& in_other) noexcept;

   /**
    * @brief Equality comparison operator.
    *
    * @param in_other   The TimeDuration to compare against.
    *
    * @return True if in_other has the same values as this TimeDuration; false otherwise.
    */
   bool operator==(const TimeDuration& in_other) const;


   /**
    * @brief Inequality comparison operator.
    *
    * @param in_other   The TimeDuration to compare against.
    *
    * @return False if in_other has the same values as this TimeDuration; true otherwise.
    */
   bool operator!=(const TimeDuration& in_other) const;

   /**
    * @brief Less-than comparison operator.
    * 
    * @param in_other   The TimeDuration to compare against.
    * 
    * @return True if this TimeDuration is less than in_other; false otherwise.
    */
   bool operator<(const TimeDuration& in_other) const;

   /**
    * @brief Less-than-equal comparison operator.
    * 
    * @param in_other   The TimeDuration to compare against.
    * 
    * @return True if this TimeDuration is less than or equal to in_other; false otherwise.
    */
   bool operator<=(const TimeDuration& in_other) const;

   /**
    * @brief Greater-than comparison operator.
    * 
    * @param in_other   The TimeDuration to compare against.
    * 
    * @return True if this TimeDuration is greater than in_other; false otherwise.
    */
   bool operator>(const TimeDuration& in_other) const;

   /**
    * @brief Greater-than-equal comparison operator.
    * 
    * @param in_other   The TimeDuration to compare against.
    * 
    * @return True if this TimeDuration is greater than or equal to in_other; false otherwise.
    */
   bool operator>=(const TimeDuration& in_other) const;

   /**
    * @brief Checks whether this TimeDuration represents "any amount of time".
    *
    * @return True if this TimeDuration is "Infinity"; false otherwise.
    */
   bool isInfinity() const;

   /**
    * @brief Gets the number of hours in this TimeDuration.
    *
    * @return The number of hours in this TimeDuration.
    */
   int64_t getHours() const;

   /**
    * @brief Gets the number of minutes in this TimeDuration.
    *
    * @return The number of minutes in this TimeDuration.
    */
   int64_t getMinutes() const;

   /**
    * @brief Gets the number of seconds in this TimeDuration.
    *
    * @return The number of seconds in this TimeDuration.
    */
   int64_t getSeconds() const;

   /**
    * @brief Gets the number of days in this TimeDuration.
    *
    * @return The number of days in this TimeDuration.
    */
   int64_t getMicroseconds() const;

private:
   // The private implementation of interval time.
   PRIVATE_IMPL(m_impl);

   friend class DateTime;
};

/** @brief Class which represents a date and time in UTC. */
class DateTime final
{
public:
  static constexpr const char* kIso8601Format {"%Y-%m-%dT%H:%M:%S%FZ"};

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
    * @brief Constructor.
    *
    * @param in_time       The time to copy into DateTime.
    */
    DateTime(std::time_t& in_time) noexcept;

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
    
    template <typename TimeType>
     static std::string format(const TimeType& time,
                   const std::string& format)
      {
      using namespace boost::posix_time;

      // facet for http date (construct w/ a_ref == 1 so we manage memory)
      time_facet httpDateFacet(1);
      httpDateFacet.format(format.c_str());

       // output and return the date
         std::ostringstream dateStream;
         dateStream.imbue(std::locale(dateStream.getloc(), &httpDateFacet));
         dateStream << time;
         return dateStream.str();
      }
static inline boost::posix_time::ptime timeFromStdTime(std::time_t t)
{
       return boost::posix_time::ptime(boost::gregorian::date(1970,1,1)) +
         boost::posix_time::seconds(static_cast<long>(t));
}
static inline bool parseUtcTimeFromFormatString(const std::string& timeStr,
                                         const std::string& formatStr,
                                         boost::posix_time::ptime *pOutTime)
{
   using namespace boost::local_time;

   std::stringstream ss(timeStr);
   local_time_input_facet* ifc = new local_time_input_facet(formatStr);

   ss.imbue(std::locale(ss.getloc(), ifc));

   local_date_time ldt(not_a_date_time);

   if (ss >> ldt)
   {
      *pOutTime = ldt.utc_time();
      return true;
   }

   return false;
}
static inline bool parseUtcTimeFromIsoString(const std::string& timeStr,
                                      boost::posix_time::ptime *pOutTime)
{
   return parseUtcTimeFromFormatString(timeStr,
                                       "%Y-%m-%d %H:%M:%S %ZP",
                                       pOutTime);
}
static inline bool parseUtcTimeFromIso8601String(const std::string& timeStr,
                                          boost::posix_time::ptime *pOutTime)
{
   return parseUtcTimeFromFormatString(timeStr,
                                       kIso8601Format,
                                       pOutTime);
}

static Error fromStdTime(const std::string& in_timeStr, DateTime& out_dateTime);
static Error fromString(const std::string& in_timeStr, const std::string& in_format, DateTime& out_dateTime);


   /**
    * @brief Assignment operator.
    *
    * @param in_other   The DateTime to assign to this.
    *
    * @return A reference to this DateTime.
    */
   DateTime& operator=(const DateTime& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other   The DateTime to move to this.
    *
    * @return A reference to this DateTime.
    */
   DateTime& operator=(DateTime&& in_other) noexcept;

   /**
    * @brief Subtracts two DateTimes to produce an TimeDuration.
    *
    * @param in_other       The date time to subtract from this.
    *
    * @return An interval time representing the difference between this DateTime and in_other.
    */
   TimeDuration operator-(const DateTime& in_other) const;

   /**
    * @brief Subtracts the given TimeDuration from a copy of this DateTime.
    *
    * @param in_intervalTime    The interval time to subtract from this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime operator-(const TimeDuration& in_intervalTime) const;

   /**
    * @brief Subtracts the given TimeDuration from this DateTime.
    *
    * @param in_intervalTime    The interval time to subtract from this DateTime.
    *
    * @return The new DateTime, which is this value of DateTime minus the specified TimeDuration.
    */
   DateTime& operator-=(const TimeDuration& in_intervalTime);

   /**
    * @brief Adds the given TimeDuration to a copy of this DateTime.
    *
    * @param in_intervalTime    The interval time to add to this DateTime.
    *
    * @return The new DateTime, which is this value of DateTime plus the specified TimeDuration.
    */
   DateTime operator+(const TimeDuration& in_intervalTime) const;

   /**
    * @brief Adds the given TimeDuration to this DateTime.
    *
    * @param in_intervalTime    The interval time to add to this DateTime.
    *
    * @return A reference to this DateTime.
    */
   DateTime& operator+=(const TimeDuration& in_intervalTime);

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
