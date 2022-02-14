/*
 * DateTime.cpp
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

#include <system/DateTime.hpp>

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/gregorian/greg_duration.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

namespace {

constexpr char const* ISO_8601_INPUT_FORMAT  = "%Y-%m-%dT%H:%M:%S%F%ZP";
constexpr char const* ISO_8601_OUTPUT_FORMAT = "%Y-%m-%dT%H:%M:%S%FZ";

} // anonymous namespace

// TimeDuration ========================================================================================================
struct TimeDuration::Impl
{
   explicit Impl(
      int64_t in_hours = 0,
      int64_t in_minutes = 0,
      int64_t in_seconds = 0,
      int64_t in_microseconds = 0) :
      Time(in_hours, in_minutes, in_seconds, in_microseconds)
   {
   }

   boost::posix_time::time_duration Time;
};

PRIVATE_IMPL_DELETER_IMPL(TimeDuration)

TimeDuration::TimeDuration(
   int64_t in_hours,
   int64_t in_minutes,
   int64_t in_seconds,
   int64_t in_microseconds) :
   m_impl(new Impl(in_hours, in_minutes, in_seconds, in_microseconds))
{
}

TimeDuration::TimeDuration(const TimeDuration& in_other) :
   m_impl(new Impl(*in_other.m_impl))
{
}

TimeDuration::TimeDuration(TimeDuration&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{
}

TimeDuration TimeDuration::Infinity()
{
   TimeDuration inf;
   inf.m_impl->Time = boost::posix_time::not_a_date_time;
   return inf;
}

TimeDuration TimeDuration::Hours(int64_t in_hours)
{
   return TimeDuration(in_hours);
}

TimeDuration TimeDuration::Minutes(int64_t in_minutes)
{
   return TimeDuration(0, in_minutes);
}

TimeDuration TimeDuration::Seconds(int64_t in_seconds)
{
   return TimeDuration(0, 0, in_seconds);
}

TimeDuration TimeDuration::Microseconds(int64_t in_microseconds)
{
   return TimeDuration(0, 0, 0, in_microseconds);
}

TimeDuration& TimeDuration::operator=(const TimeDuration& in_other)
{
   if (this != &in_other)
   {
      if (in_other.m_impl == nullptr)
         m_impl.reset();
      else if (m_impl == nullptr)
         m_impl.reset(new Impl(*in_other.m_impl));
      else
         *m_impl = *in_other.m_impl;
   }

   return *this;
}

TimeDuration& TimeDuration::operator=(TimeDuration&& in_other) noexcept
{
   if (this != &in_other)
   {
      m_impl.swap(in_other.m_impl);
      in_other.m_impl.reset();
   }

   return *this;
}

bool TimeDuration::operator==(const TimeDuration& in_other) const
{
   if ((this == &in_other) || (isInfinity() && in_other.isInfinity()))
      return true;
   else if (isInfinity() || in_other.isInfinity())
      return false;

   return m_impl->Time == in_other.m_impl->Time;
}

bool TimeDuration::operator!=(const TimeDuration& in_other) const
{
   return !(*this == in_other);
}

bool TimeDuration::operator<(const TimeDuration& in_other) const
{
   if (isInfinity() && in_other.isInfinity())
      return false;
   else if (isInfinity())
      return false;
   else if (in_other.isInfinity())
      return true;
   
   return m_impl->Time < in_other.m_impl->Time;
}

bool TimeDuration::operator<=(const TimeDuration& in_other) const
{
   return !(*this > in_other);
}

bool TimeDuration::operator>(const TimeDuration& in_other) const
{
   if (isInfinity() && in_other.isInfinity())
      return false;
   else if (isInfinity())
      return true;
   else if (in_other.isInfinity())
      return false;
   
   return m_impl->Time > in_other.m_impl->Time;
}

bool TimeDuration::operator>=(const TimeDuration& in_other) const
{
   return !(*this < in_other);
}

bool TimeDuration::isInfinity() const
{
   return (m_impl == nullptr) || (m_impl->Time.is_not_a_date_time());
}

int64_t TimeDuration::getHours() const
{
   if (isInfinity())
      return 0;

   return m_impl->Time.hours();
}

int64_t TimeDuration::getMinutes() const
{
   if (isInfinity())
      return 0;

   return m_impl->Time.minutes();
}

int64_t TimeDuration::getSeconds() const
{
   if (isInfinity())
      return 0;

   return m_impl->Time.seconds();
}

int64_t TimeDuration::getMicroseconds() const
{
   if (isInfinity())
      return 0;

   return m_impl->Time.fractional_seconds();
}

// DateTime ============================================================================================================
struct DateTime::Impl
{
   Impl() :
      Time(boost::posix_time::microsec_clock::universal_time())
   {
   }

   Impl(const Impl& in_other) = default;
   boost::posix_time::ptime Time;
};

PRIVATE_IMPL_DELETER_IMPL(DateTime)

DateTime::DateTime() :
   m_impl(new Impl())
{
}

DateTime::DateTime(std::time_t& in_time) noexcept :
   m_impl(new Impl())
{
   m_impl->Time = boost::posix_time::ptime(boost::gregorian::date(1970,1,1)) +
      boost::posix_time::seconds(static_cast<long>(in_time));
}

 DateTime::DateTime(boost::posix_time::ptime Time) noexcept :
   m_impl(new Impl())
{
   m_impl->Time = Time;
}

DateTime::DateTime(const DateTime& in_other) :
   m_impl(new Impl(*in_other.m_impl))
{
}

DateTime::DateTime(DateTime&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{

}
Error DateTime::fromString(const std::string& in_timeStr,DateTime& out_dateTime)
{
   return fromString(in_timeStr, ISO_8601_INPUT_FORMAT, out_dateTime);
}
Error DateTime::fromString(const std::string& in_timeStr,const std::string& in_format, DateTime& out_dateTime)
{
   // Invalidate the DateTime so it won't act as the current time if this function fails.
   out_dateTime.m_impl->Time = boost::posix_time::not_a_date_time;

   using namespace boost::local_time;

   std::stringstream ss(in_timeStr);
   std::unique_ptr<local_time_input_facet> facet(new local_time_input_facet(in_format));

   // Locale takes ownership of facet.
   ss.imbue(std::locale(ss.getloc(), facet.release()));

   local_date_time time(boost::posix_time::not_a_date_time);
   try
   {
      ss >> time;
   }
   catch(const std::exception& e)
   {
      return Error("TimeParseError", 1, "Failed to parse time: " + in_timeStr + ". Reason: " + e.what(), ERROR_LOCATION);
   }
   catch (...)
   {
      return Error("TimeParseError", 1, "Failed to parse time: " + in_timeStr, ERROR_LOCATION);
   }

   if (time.is_not_a_date_time())
      return Error("TimeParseError", 1, "Failed to parse time: " + in_timeStr, ERROR_LOCATION);

   out_dateTime.m_impl->Time = time.utc_time();

   return Success();
}

DateTime& DateTime::operator=(const DateTime& in_other)
{
   if (this != &in_other)
   {
      if (in_other.m_impl == nullptr)
         m_impl.reset();
      else if (m_impl == nullptr)
         m_impl.reset(new Impl(*in_other.m_impl));
      else
         m_impl->Time = in_other.m_impl->Time;
   }
   return *this;
}

DateTime& DateTime::operator=(DateTime&& in_other) noexcept
{
   if (this != &in_other)
   {
      m_impl.swap(in_other.m_impl);
      in_other.m_impl.reset();
   }

   return *this;
}

TimeDuration DateTime::operator-(const DateTime& in_other) const
{
   TimeDuration result;

   if ((this != &in_other) && (m_impl != nullptr) && (in_other.m_impl != nullptr))
      result.m_impl->Time = m_impl->Time - in_other.m_impl->Time;

   return result;
}

DateTime DateTime::operator-(const TimeDuration& in_intervalTime) const
{
   DateTime result = *this;
   result -= in_intervalTime;
   return result;
}

DateTime& DateTime::operator-=(const TimeDuration& in_intervalTime)
{
   if ((m_impl != nullptr) && (in_intervalTime.m_impl != nullptr))
      m_impl->Time -= in_intervalTime.m_impl->Time;

   return *this;
}

DateTime DateTime::operator+(const TimeDuration& in_intervalTime) const
{
   DateTime result = *this;
   result += in_intervalTime;
   return result;
}

DateTime& DateTime::operator+=(const TimeDuration& in_intervalTime)
{
   if ((m_impl != nullptr) && (in_intervalTime.m_impl != nullptr))
      m_impl->Time += in_intervalTime.m_impl->Time;

   return *this;
}

bool DateTime::operator==(const DateTime& in_other) const
{
   if (this == &in_other)
      return true;

   if (((m_impl == nullptr) && (in_other.m_impl == nullptr)) ||
      ((m_impl == nullptr) && (in_other.m_impl->Time.is_not_a_date_time())) ||
      ((in_other.m_impl == nullptr) && (m_impl->Time.is_not_a_date_time())))
      return true;
   else if ((m_impl == nullptr) || (in_other.m_impl == nullptr))
      return false;

   if (m_impl->Time.is_not_a_date_time() && in_other.m_impl->Time.is_not_a_date_time())
      return true;

   return m_impl->Time == in_other.m_impl->Time;
}

bool DateTime::operator!=(const DateTime& in_other) const
{
   return !(*this == in_other);
}

bool DateTime::operator<(const DateTime& in_other) const
{
   // If either are NULL or not a time, LT comparison is invalid. LT is also false  if the two objects are the same.
   if ((m_impl == nullptr) ||
      (in_other.m_impl == nullptr) ||
      (m_impl->Time.is_not_a_date_time()) ||
      (in_other.m_impl->Time.is_not_a_date_time()) ||
      (this == &in_other))
      return false;

   return m_impl->Time < in_other.m_impl->Time;
}

bool DateTime::operator<=(const DateTime& in_other) const
{
   // This should return true whenever `operator==` would. Don't rely on operator< + operator== to avoid checking for
   // NULL and not_a_date_time more than necessary.
   if (this == &in_other)
      return true;

   if (((m_impl == nullptr) && (in_other.m_impl == nullptr)) ||
       ((m_impl == nullptr) && (in_other.m_impl->Time.is_not_a_date_time())) ||
       ((in_other.m_impl == nullptr) && (m_impl->Time.is_not_a_date_time())))
      return true;
   else if ((m_impl == nullptr) || (in_other.m_impl == nullptr))
         return false;

   if (m_impl->Time.is_not_a_date_time() && in_other.m_impl->Time.is_not_a_date_time())
      return true;

   return m_impl->Time <= in_other.m_impl->Time;
}

bool DateTime::operator>(const DateTime& in_other) const
{
   return in_other < *this;
}

bool DateTime::operator>=(const DateTime& in_other) const
{
   return in_other <= *this;
}

std::string DateTime::toString() const
{
   return toString(ISO_8601_OUTPUT_FORMAT);
}

std::string DateTime::toString(const char* in_format) const
{
   // No string representation if the DateTime has been gutted or isn't valid.
   if ((m_impl == nullptr) || m_impl->Time.is_not_a_date_time())
      return "";

   using namespace boost::posix_time;

   std::unique_ptr<time_facet> facet(new time_facet(in_format));

   // The locale takes ownership of the facet.
   std::ostringstream ss;
   ss.imbue(std::locale(ss.getloc(), facet.release()));
   ss << m_impl->Time;

   return ss.str();
}

std::string DateTime::toString(const std::string& in_format) const
{
   return toString(in_format.c_str());
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
