/*
 * DateTime.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_DATE_TIME_HPP
#define LAUNCHER_PLUGINS_DATE_TIME_HPP

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace date_time {

template <typename TimeType>
std::string format(const TimeType& time,
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

} // namespace date_time
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
