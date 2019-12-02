/*
 * LocalOptions.cpp
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

#include "LocalOptions.hpp"

#include <options/Options.hpp>

using rstudio::launcher_plugins::system::FilePath;

namespace rstudio {
namespace launcher_plugins {
namespace local {

namespace {
constexpr char const* s_defaultSandboxPath = "/usr/lib/rstudio-server/bin/rsandbox";
}

LocalOptions& LocalOptions::getInstance()
{
   static LocalOptions options;
   return options;
}

boost::posix_time::time_duration LocalOptions::getNodeConnectionTimeoutSeconds() const
{
   return boost::posix_time::seconds(m_nodeConnectionTimeoutSeconds);
}

const system::FilePath& LocalOptions::getRsandboxPath() const
{
   return m_rsandboxPath;
}

const system::FilePath& LocalOptions::getSecureCookieKeyFile() const
{
   return m_secureCookieKeyFile;
}

bool LocalOptions::shouldSaveUnspecifiedOutput() const
{
   return m_saveUnspecifiedOutput;
}

bool LocalOptions::useUnprivilegedMode() const
{
   return m_useUnprivilegedMode;
}

void LocalOptions::initialize()
{
   // These are temporary and will be replaced with a list of available containers, probably using
   // UserProfiles later on.
   using namespace rstudio::launcher_plugins::options;
   Options& options = Options::getInstance();
   options.registerOptions()
       ("node-connection-timeout-seconds",
          Value<int>(m_nodeConnectionTimeoutSeconds).setDefaultValue(3),
          "amount of seconds to allow for outgoing connections to other nodes in a load balanced cluster or 0 to use "
          "the system default")
       ("save-unspecified-output",
          Value<bool>(m_saveUnspecifiedOutput).setDefaultValue(true),
          "whether or not to save output for jobs that don't specify an output path - saved in scratch path")
       ("unprivileged-mode",
          Value<bool>(m_useUnprivilegedMode).setDefaultValue(false),
          "special unprivileged mode - does not change user, runs without root, no impersonation, single user")
       ("rsandbox-path",
          Value<FilePath>(m_rsandboxPath).setDefaultValue(FilePath(s_defaultSandboxPath)),
          "path to rsandbox executable")
       ("secure-cookie-key-file",
          Value<FilePath>(m_secureCookieKeyFile).setDefaultValue(FilePath()),
          "amount of seconds to allow for outgoing connections to other nodes in a load balanced cluster or 0 to use "
          "the system default");
}

}
} // namespace launcher_plugins
} // namespace rstudio
