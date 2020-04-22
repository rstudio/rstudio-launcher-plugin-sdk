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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

constexpr char const* ISO_8601_INPUT_FORMAT  = "%Y-%m-%dT%H:%M:%S%F%ZP";
constexpr char const* ISO_8601_OUTPUT_FORMAT = "%Y-%m-%dT%H:%M:%S%FZ";

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

DateTime::DateTime(const DateTime& in_other) :
   m_impl(new Impl(*in_other.m_impl))
{
}

DateTime::DateTime(DateTime&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{
}

Error DateTime::fromString(const std::string& in_timeStr, DateTime& out_dateTime)
{
   // Invalidate the DateTime so it won't act as the current time if this function fails.
   out_dateTime.m_impl->Time = boost::posix_time::not_a_date_time;

   using namespace boost::local_time;

   std::stringstream ss(in_timeStr);
   std::unique_ptr<local_time_input_facet> facet(new local_time_input_facet(ISO_8601_INPUT_FORMAT));

   // Locale takes ownership of facet.
   ss.imbue(std::locale(ss.getloc(), facet.release()));

   local_date_time time(boost::posix_time::not_a_date_time);
   try
   {
      ss >> time;
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
   if (in_other.m_impl == nullptr)
      m_impl.reset();
   else
      m_impl->Time = in_other.m_impl->Time;

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

DateTime& DateTime::addHours(uint64_t in_hours)
{
   return *this;
}

DateTime DateTime::addHours(uint64_t in_hours) const
{
   system::DateTime copy(*this);
   copy.addHours(in_hours);
   return copy;
}

DateTime& DateTime::addMicroseconds(uint64_t in_microseconds)
{
   return *this;
}

DateTime DateTime::addMicroseconds(uint64_t in_microseconds) const
{
   system::DateTime copy(*this);
   copy.addMicroseconds(in_microseconds);
   return copy;
}

DateTime& DateTime::addMinutes(uint64_t in_minutes)
{
   return *this;
}

DateTime DateTime::addMinutes(uint64_t in_minutes) const
{
   system::DateTime copy(*this);
   copy.addMinutes(in_minutes);
   return copy;
}

DateTime& DateTime::addSeconds(uint64_t in_seconds)
{
   return *this;
}

DateTime DateTime::addSeconds(uint64_t in_seconds) const
{
   system::DateTime copy(*this);
   copy.addSeconds(in_seconds);
   return copy;
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
