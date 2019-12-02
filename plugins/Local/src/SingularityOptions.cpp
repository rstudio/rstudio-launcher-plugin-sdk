/*
 * SingularityOptions.cpp
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

#include "SingularityOptions.hpp"

#include <options/Options.hpp>

using rstudio::launcher_plugins::system::FilePath;

namespace rstudio {
namespace launcher_plugins {
namespace singularity {

SingularityOptions& SingularityOptions::getInstance()
{
   static SingularityOptions options;
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
   // These are temporary and will be replaced with a list of available containers, probably using
   // UserProfiles later on.
   using namespace rstudio::launcher_plugins::options;
   Options& options = Options::getInstance();
   options.registerOptions()
       ("r-container", Value<FilePath>(m_rContainer), "the container to use for R jobs")
       ("r-session-container", Value<FilePath>(m_rSessionContainer), "the container to use for R sessions");
}

}
} // namespace launcher_plugins
} // namespace rstudio

