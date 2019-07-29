/*
 * Options.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_ABSTRACT_OPTIONS_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_OPTIONS_HPP

#include <boost/noncopyable.hpp>

#include <boost/program_options/value_semantic.hpp>

#include <memory>

#include "Error.hpp"
#include "logging/Logger.hpp"

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
   Value() = default;

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
   // The private implementation of Value.
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
       * @param io_value            The value object, which holds the default and the storage object.
       * @param in_description      The description of the option.
       *
       * @return A reference to this Init object.
       */
      template <class T>
      Init& operator()(const char* in_name, Value<T>& io_value, const char* in_description);

   private:
      // The owner of this init object.
      Options& m_owner;
   };

   /**
    * @brief Gets the single instance of Object for the plugin.
    *
    * @return The single instance of Object for the plugin.
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
   unsigned int getJobExpiryHours() const;

   /**
    * @brief Gets the number of seconds between heartbeats.
    *
    * @return The number of seconds between heartbeats.
    */
   unsigned int getHeartbeatIntervalSeconds() const;

   /**
    * @brief Gets the maximum level of log messages to write.
    *
    * @return The maximum level of log messages to write.
    */
   logging::LogLevel getLogLevel() const;

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
    * @brief Gets the user to run as when root privileges are dropped.
    *
    * @return The user to run as when root privileges are dropped.
    */
   const system::User& getServerUser() const;

   /**
    * @brief Gets the size of the thread pool.
    *
    * @return The size of the thread pool.
    */
   unsigned int getThreadPoolSize() const;

private:
   // The private implementation of Options.
   PRIVATE_IMPL(m_impl);

   friend Init;
};

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

#endif

