/*
 * SingularityOptions.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "SingularityOptions.hpp"

#include <options/Options.hpp>

using rstudio::launcher_plugins::system::FilePath;

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

SingularityOptions& SingularityOptions::getInstance()
{
   static SingularityOptions options;
   options.initialize();
   return options;
}

const FilePath& SingularityOptions::getRContainer() const
{
   return m_rContainer;
}

const FilePath& SingularityOptions::getRSessionContainer() const
{
   return m_rSessionContainer;
}

void SingularityOptions::initialize()
{
   boost::unique_lock<boost::mutex> lock(m_mutex);
   if (!m_isInitilazied)
   {
      // These are probably temporary and will be replaced with a list of available containers, probably using
      // UserProfiles later on.
      using namespace rstudio::launcher_plugins::options;
      Options& options = Options::getInstance();
      options.registerOptions()
          ("r-container", Value<FilePath>(m_rContainer), "the container to use for R jobs")
          ("r-session-container", Value<FilePath>(m_rSessionContainer), "the container to use for R sessions");

      m_isInitilazied = true;
   }
}

}
} // namespace launcher_plugins
} // namespace rstudio

