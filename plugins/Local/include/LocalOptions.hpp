/*
 * LocalOptions.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_OPTIONS_HPP
#define LAUNCHER_PLUGINS_LOCAL_OPTIONS_HPP

#include <boost/noncopyable.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

/**
 * @brief Class which stores options specific to the Local Container system.
 */
class LocalOptions : public boost::noncopyable
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
   boost::posix_time::time_duration getNodeConnectionTimeoutSeconds() const;

   /**
    * @brief Gets the path to the rsandbox executable provided by the RStudio Server Pro installation.
    *
    * If RStudio Server Pro is installed to the default location, this value does not need to be set.
    *
    * @return The path to the rsandbox executable.
    */
   const system::FilePath& getRsandboxPath() const;

   /**
    * @brief Gets the secure cookie key file to use for decrypting PAM passwords.
    *
    * @return The secure cookie key file to use for decrypting PAM passwords.
    */
   const system::FilePath& getSecureCookieKeyFile() const;

   /**
    * @brief Gets whether to save output for a job when the output path has not been specified.
    *
    * @return True if job output should be saved when no output path was specified; false otherwise.
    */
   bool shouldSaveUnspecifiedOutput() const;


   /**
    * @brief Gets whether jobs will be run in an unprivileged environment or not.
    *
    * Most environments will not require this value to be set to true. It only needs to be set if the job will be run in
    * an environment where the run-as-user cannot take privileged actions, such as within a docker container.
    *
    * If this value is set to true the user will not be changed, and the job will be run without root and impersonation.
    *
    * @return True if jobs will be run in an unprivileged environment; false otherwise.
    */
   bool useUnprivilegedMode() const;

   /**
    * @brief Method which initializes LocalOptions. This method should be called exactly once, before the options
    *        file is read.
    *
    * This is where LocalOptions are registered with the Options object.
    */
   void initialize();

private:
   /**
    * @brief Private constructor to prevent multiple instantiations of this singleton.
    */
   LocalOptions() = default;

   /**
    * The number of seconds that can elapse before an attempted connection to another local node will be timed out.
    */
   int m_nodeConnectionTimeoutSeconds;

   /**
    * Whether to save output for a job when the output path has not been specified.
    */
   bool m_saveUnspecifiedOutput;

   /**
    * Whether jobs will be run in an unprivileged environment or not.
    */
   bool m_useUnprivilegedMode;

   /**
    * The path to the rsandbox executable provided by the RStudio Server Pro installation.
    */
   system::FilePath m_rsandboxPath;

   /**
    * The secure cookie key file to use for decrypting PAM passwords.
    */
   system::FilePath m_secureCookieKeyFile;

};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
