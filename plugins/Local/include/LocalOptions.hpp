/*
 * LocalOptions.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_OPTIONS_HPP
#define LAUNCHER_PLUGINS_LOCAL_OPTIONS_HPP

#include <Noncopyable.hpp>

#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Class which stores options specific to the Local Container system.
 */
class LocalOptions : public Noncopyable
{
public:
   /**
    * @brief Gets the single instance of LocalOptions for the plugin.
    *
    * @return The single instance of LocalOptions for the plugin.
    */
   static LocalOptions& getInstance();

   /**
    * @brief Gets the number of seconds that can elapse before an attempted connection to another local node will be
    *        timed out.
    *
    * @return The timeout for connecting to other local nodes, in seconds.
    */
   size_t getNodeConnectionTimeoutSeconds() const;

   /**
    * @brief Gets the secure cookie key file to use for decrypting PAM passwords.
    *
    * @return The secure cookie key file to use for decrypting PAM passwords.
    */
   const system::FilePath& getSecureCookieKeyFile() const;

   /**
    * @brief Method which initializes LocalOptions. This method should be called exactly once, before the options
    *        file is read.
    *
    * This is where Local Options are registered with the Options object.
    */
   void initialize();

   /**
    * @brief Gets whether to save output for a job when the output path has not been specified.
    *
    * @return True if job output should be saved when no output path was specified; false otherwise.
    */
   bool shouldSaveUnspecifiedOutput() const;

private:
   /**
    * @brief Private constructor to prevent multiple instantiations of this singleton.
    */
   LocalOptions() = default;

   /**
    * The number of seconds that can elapse before an attempted connection to another local node will be timed out.
    */
   size_t m_nodeConnectionTimeoutSeconds;

   /**
    * Whether to save output for a job when the output path has not been specified.
    */
   bool m_saveUnspecifiedOutput;

   /**
    * The secure cookie key file to use for decrypting PAM passwords.
    */
   system::FilePath m_secureCookieKeyFile;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
