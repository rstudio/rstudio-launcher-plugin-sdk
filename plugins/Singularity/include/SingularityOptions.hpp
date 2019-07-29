/*
 * SingularityOptions.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_SINGULARITY_OPTIONS_HPP
#define LAUNCHER_PLUGINS_SINGULARITY_OPTIONS_HPP

#include <boost/noncopyable.hpp>

#include <boost/thread.hpp>

#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

class SingularityOptions : public boost::noncopyable
{
public:
   static SingularityOptions& getInstance();

   const system::FilePath& getRContainer() const;
   const system::FilePath& getRSessionContainer() const;

private:
   SingularityOptions() = default;

   void initialize();

   boost::mutex m_mutex;
   bool m_isInitilazied;

   system::FilePath m_rContainer;
   system::FilePath m_rSessionContainer;
};

} // namespace singularity
} // namespace launcher_plugins
} // namespace rstudio

#endif
