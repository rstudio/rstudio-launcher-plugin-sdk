/*
 * Options.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_OPTIONS_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_OPTIONS_HPP

#include <boost/noncopyable.hpp>

#include <boost/program_options/value_semantic.hpp>

#include <memory>

#include "Error.hpp"
#include "logging/Logger.hpp"
#include "system/DateTime.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {

class FilePath;
class User;

}
}
}

namespace rstudio {
namespace launcher_plugins {
namespace options {

template <class T>
class Value;
class Options;
/**
 * @brief Concrete class which represents an option Value.
 *
 * This class supports the following template types:
 *    - bool
 *    - char
 *    - unsigned char
 *    - short
 *    - unsigned short
 *    - int
 *    - unsigned int
 *    - long
 *    - unsigned long
 *    - long long
 *    - unsigned long long
 *    - float
 *    - double
 *    - long double
 *    - std::string
 *    - rstudio::launcher_plugins::logging::LogLevel
 *    - rstudio::launcher_plugins::system::FilePath
 *    - rstudio::launcher_plugins::system::User
 *
 * If a custom type is needed, treat the option value as a string and do the parsing and conversion from the string
 * value.
 *
 * @tparam T    The type of the option value.
 */
template<class T>
class Value
{
public:
   /**
    * @brief Default Constructor.
    */
   Value();

   /**
    * @brief Constructor which takes an object to store the value to.
    *
    * @param io_storeTo     The object to store the option value to. The caller is responsible for ensuring that this
    *                       object is alive when the option file is parsed.
    */
   explicit Value(T& io_storeTo);

   /**
    * @brief Sets the default value of the option.
    *
    * @param in_defaultValue    The default value of the option.
    *
    * @return A reference to this value.
    */
   Value& setDefaultValue(const T& in_defaultValue);

private:
   /**
    * @brief The private implementation of Value.
    */
   PRIVATE_IMPL_SHARED(m_impl);

   friend class Options;
};

/**
 * @brief Options for the plugin.
 */
class Options : boost::noncopyable
{
public:
   /**
    * @brief Class for initializing Options.
    */
   class Init
   {
   public:
      /**
       * @brief Helper class which initializes Options.
       *
       * @param in_owner    The Options object which owns this init object.
       */
      explicit Init(Options& in_owner);

      /**
       * @brief Operator which initializes a specific option value.
       *
       * @tparam T                  The type of the option.
       *
       * @param in_name             The name of the option.
       * @param in_value            The value object, which holds the default and the storage object. The Value object
       *                            is not usable after this call.
       * @param in_description      The description of the option.
       *
       * @return A reference to this Init object.
       */
      template <class T>
      Init& operator()(const char* in_name, Value<T>& in_value, const char* in_description);

      /**
       * @brief Operator which initializes a specific option value.
       *
       * @tparam T                  The type of the option.
       *
       * @param in_name             The name of the option.
       * @param in_value            The value object, which holds the default and the storage object. The Value object
       *                            is not usable after this call.
       * @param in_description      The description of the option.
       *
       * @return A reference to this Init object.
       */
      template <class T>
      Init& operator()(const char* in_name, Value<T>&& in_value, const char* in_description);

   private:
      /**
       * @brief The owner of this init object.
       */
      Options& m_owner;
   };

   /**
    * @brief Gets the single instance of Options for the plugin.
    *
    * @return The single instance of Options for the plugin.
    */
   static Options& getInstance();

   /**
    * @brief Allows the caller to register their options using the Init helper object.
    *
    * @return The Init helper object with which options can be registered.
    */
   Init registerOptions();

   /**
    * @brief Reads the option file, loading all registered options.
    *
    * registerOptions() must be called before this is called in order to include additional options.
    *
    * @param in_argc        The count of command line arguments.
    * @param in_argv        The command line arguments.
    * @param in_location    The location of the configuration file. Must exist.
    *
    * @return Success if all required options were read and no parsing errors occurred; Error otherwise.
    */
   Error readOptions(int in_argc, const char* const in_argv[], const system::FilePath& in_location);

   /**
    * @brief Gets the number of hours after which finished jobs expire and should be pruned from the plugin.
    *
    * @return The number of hours after which finished jobs expire and should be pruned from the plugin.
    */
   system::TimeDuration getJobExpiryHours() const;

   /**
    * @brief Gets the number of seconds between heartbeats.
    *
    * @return The number of seconds between heartbeats.
    */
   system::TimeDuration getHeartbeatIntervalSeconds() const;

   /**
    * @brief Gets the maximum level of log messages to write.
    *
    * @return The maximum level of log messages to write.
    */
   logging::LogLevel getLogLevel() const;

   /**
    * @brief Gets the location of the configuration file for the RStudio Job Launcher.
    *
    * This is useful if the plugin implementation requires knowledge of the Job Launcher's configuration. Most plugin
    * implementations will not need this value.
    *
    * @return The location of the configuration file for the RStudio Job Launcher.
    */
   const system::FilePath& getLauncherConfigFile() const;

   /**
    * @brief Gets the maximum allowable size of messages which can be used in communications with the RStudio Launcher.
    *
    * It is not recommended to change this value directly in the Plugin's configuration file. Changes to this value will
    * be propagated from the RStudio Launcher to all Plugins.
    *
    * @return The maximum allowable size of messages which can be used in communications with the RStudio Launcher.
    */
   size_t getMaxMessageSize() const;

   /**
    * @brief Gets the name the administrator gave to this instance of the Plugin in the launcher.conf file.
    *
    * This value may be useful if a plugin implementation needs to be aware of other instances of the same Plugin in a
    * load balanced scenario.
    *
    * @return The name the administrator gave to this instance of the Plugin in the launcher.conf file.
    */
   const std::string& getPluginName() const;

   /**
    * @brief Gets the path to the rsandbox executable provided by the RStudio Server Pro installation.
    *
    * If RStudio Server Pro is installed to the default location, this value does not need to be set.
    *
    * @return The path to the rsandbox executable.
    */
   const system::FilePath& getRSandboxPath() const;

  /**
   * @brief Gets the scratch path to which log files and other plugin data may be written.
   *
   * Note that this does not include job output. Job output should be written in the location specified by the user
   * when the job is run.
   *
   * @return The scratch path to which log files and other plugin data may be written.
   */
  const system::FilePath& getScratchPath() const;

   /**
   * @brief Gets path where debug logs should be written.
   *
   * @return The path where debug logs should be written.
   */
   const system::FilePath& getLoggingDir() const;

   /**
    * @brief Gets the user to run as when root privileges are dropped.
    *
    * @param out_serverUser       The server user, if it exists.
    *
    * @return Success if the server user exists; error otherwise.
    */
   Error getServerUser(system::User& out_serverUser) const;

   /**
    * @brief Gets the size of the thread pool.
    *
    * @return The size of the thread pool.
    */
   size_t getThreadPoolSize() const;

   /**
    * @brief Gets whether the plugin should run in single-user unprivileged mode.
    *
    * @return True if the plugin should run in uprivileged mode; false otherwise.
    */
   bool useUnprivilegedMode() const;
   
   /**
    * @brief Gets whether debug logging is activated.
    *
    * @return True if the enableDebugLogging is true; false otherwise.
    */
   bool enableDebugLogging() const;

private:
   /**
    * @brief Private constructor.
    */
   Options();

   // The private implementation of Options.
   PRIVATE_IMPL(m_impl);

   friend Init;
};

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

#endif
