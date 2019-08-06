/*
 * SingularityOptions.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
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

#ifndef LAUNCHER_PLUGINS_SINGULARITY_OPTIONS_HPP
#define LAUNCHER_PLUGINS_SINGULARITY_OPTIONS_HPP

#include <boost/noncopyable.hpp>

#include <boost/thread.hpp>

#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

/**
 * @brief Class which stores options specific to the Singularity Container system.
 */
class SingularityOptions : public boost::noncopyable
{
public:
   /**
    * @brief Gets the single instance of SingularityOptions for the plugin.
    *
    * @return The single instance of SingularityOptions for the plugin.
    */
   static SingularityOptions& getInstance();

   /**
    * @brief Gets the Singularity container to use for R.
    *
    * NOTE: This option is a placeholder until I know what options I need. It will not end up in the release version of
    *       this plugin.
    *
    * @reurn The Singularity container to use for R.
    */
   const system::FilePath& getRContainer() const;
   /**
    * @brief Gets the Singularity container to use for R Sessions.
    *
    * NOTE: This option is a placeholder until I know what options I need. It will not end up in the release version of
    *       this plugin.
    *
    * @reurn The Singularity container to use for R Sessions.
    */
   const system::FilePath& getRSessionContainer() const;

   /**
    * @brief Method which initializes SingularityOptions. This method should be called exactly once, before the options
    *        file is read.
    *
    * This is where SingularityOptions are registered with the Options object.
    */
   void initialize();

private:
   /**
    * @brief Private constructor to prevent multiple instantiations of this singleton.
    */
   SingularityOptions() = default;

   // TODO: These options are placeholders.
   system::FilePath m_rContainer;
   system::FilePath m_rSessionContainer;
};

} // namespace singularity
} // namespace launcher_plugins
} // namespace rstudio

#endif
