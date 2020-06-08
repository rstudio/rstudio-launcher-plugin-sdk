/*
 * LocalOptions.cpp
 * 
 * Copyright (C) 2019-20 by RStudio, PBC
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

#include "LocalOptions.hpp"

#include <options/Options.hpp>

using rstudio::launcher_plugins::system::FilePath;

namespace rstudio {
namespace launcher_plugins {
namespace local {

LocalOptions& LocalOptions::getInstance()
{
   static LocalOptions options;
   return options;
}

size_t LocalOptions::getNodeConnectionTimeoutSeconds() const
{
   return m_nodeConnectionTimeoutSeconds;
}

const system::FilePath& LocalOptions::getSecureCookieKeyFile() const
{
   return m_secureCookieKeyFile;
}

void LocalOptions::initialize()
{
   // These are temporary and will be replaced with a list of available containers, probably using
   // UserProfiles later on.
   using namespace rstudio::launcher_plugins::options;
   Options& options = Options::getInstance();
   options.registerOptions()
      ("node-connection-timeout-seconds",
       Value<size_t>(m_nodeConnectionTimeoutSeconds).setDefaultValue(3),
       "amount of seconds to allow for outgoing connections to other nodes in a load balanced cluster or 0 to use "
       "the system default")
      ("save-unspecified-output",
       Value<bool>(m_saveUnspecifiedOutput).setDefaultValue(true),
       "whether or not to save output for jobs that don't specify an output path - saved in scratch path")
      ("secure-cookie-key-file",
       Value<FilePath>(m_secureCookieKeyFile).setDefaultValue(FilePath("/var/lib/rstudio-server/secure-cookie-key")),
       "amount of seconds to allow for outgoing connections to other nodes in a load balanced cluster or 0 to use "
       "the system default");
}

bool LocalOptions::shouldSaveUnspecifiedOutput() const
{
   return m_saveUnspecifiedOutput;
}

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

