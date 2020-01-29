/*
 * PosixSystemUtils.hpp
 *
 * Copyright (C) 2020 by RStudio, Inc.
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


#ifndef LAUNCHER_PLUGINS_POSIXSYSTEMUTILS_HPP
#define LAUNCHER_PLUGINS_POSIXSYSTEMUTILS_HPP

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace system {

class User;

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace system {
namespace posix {

/**
 * @brief Enables core dumps for this process.
 *
 * @return Success if core dumps could be enabled; Error otherwise.
 */
Error enableCoreDumps();

/**
 * @brief Ignores SIGPIPE for this process.
 *
 * @return Success if SIGPIPE could be ignored; Error otherwise.
 */
Error ignoreSigPipe();

/**
 * @brief Checks whether the real user (not the effective user) running this process is root.
 *
 * @return True if the real user is root; false otherwise.
 */
bool realUserIsRoot();

/**
 * @briefs Restores root privileges.
 *
 * @return Success if root privileges could be restored; Error otherwise.
 */
Error restoreRoot();

/**
 * @brief Temporarily drops privileges from root to the requested user.
 *
 * @param in_user   The user to which to drop privileges.
 *
 * @return Success if privileges could be dropped to the requested user; Error otherwise.
 */
Error temporarilyDropPriv(const User& in_user);

} // namespace posix
} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
