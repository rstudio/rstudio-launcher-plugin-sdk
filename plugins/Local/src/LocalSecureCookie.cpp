/*
 * SecureCookie.cpp
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


#include <LocalSecureCookie.hpp>

#include <Error.hpp>
#include <options/Options.hpp>
#include <system/FilePath.hpp>
#include <system/User.hpp>
#include <utils/FileUtils.hpp>

#include <LocalOptions.hpp>

// Current dir is plugins/Local/src
// ONLY INCLUDING PRIVATE SDK HEADER BECAUSE ABSOLUTELY NECESSARY.
// PRIVATE HEADERS ARE NOT STABLE. THEY CANNOT BE RELIED ON.
#include "../../../sdk/src/system/PosixSystem.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace local {

Error LocalSecureCookie::initialize()
{
   // ------------------------------------------------------------------------------------------------------------------
   // MAINTENANCE NOTE:
   // We need to elevate privileges here to read the secure-cookie-key file since RSP installs it as owned-by-root.
   // This is not recommended - for similar use cases it would be better to create any necessary files with server-user
   // ownership.
   // DO NOT DO THIS UNLESS ABSOLUTELY NECESSARY.
   // ------------------------------------------------------------------------------------------------------------------
   if (!system::posix::realUserIsRoot())
      return systemError(EPERM, "Local Plugin must be run as the root user.", ERROR_LOCATION);

   Error error = system::posix::restoreRoot();
   if (error)
      return error;

   const system::FilePath& keyFilePath = LocalOptions::getInstance().getSecureCookieKeyFile();
   error = utils::readFileIntoString(keyFilePath, m_key);
   if (error)
      return error;

   system::User serverUser;
   error = options::Options::getInstance().getServerUser(serverUser);
   if (error)
      return error;

   error = system::posix::temporarilyDropPrivileges(serverUser);
   if (error)
      return error;

   // Ensure the key is at least 256 bits (32 bytes) in strength, for security purposes.
   if (m_key.size() < 32)
   {
      logging::logErrorMessage(
         "The specified 'secure-cookie-key' is not strong enough. It must be at least 32 bytes/characters long.");
      return systemError(EINVAL, ERROR_LOCATION);
   }

   return Success();
}

const std::string& LocalSecureCookie::getKey() const
{
   return m_key;
}

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
