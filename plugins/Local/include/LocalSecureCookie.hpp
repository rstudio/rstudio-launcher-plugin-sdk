/*
 * SecureCookie.hpp
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


#ifndef LAUNCHER_PLUGINS_LOCAL_SECURE_COOKIE_HPP
#define LAUNCHER_PLUGINS_LOCAL_SECURE_COOKIE_HPP

#include <string>

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Reads and makes available the secure-cookie-key-file specified in the launcher.local.conf file.
 */
class LocalSecureCookie
{
public:
   /**
    * @brief Constructor.
    */
   LocalSecureCookie() = default;

   /**
    * @brief Reads and validates the secure-cookie-key from the location specified in the options.
    *
    * @return Success if the secure-cookie-key exists and was valid; Error otherwise.
    */
   Error initialize();

   /**
    * @brief Gets the secure cookie key.
    *
    * @return The secure cookie key.
    */
   const std::string& getKey() const;

private:
   /** The secure cookie key. */
   std::string m_key;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
