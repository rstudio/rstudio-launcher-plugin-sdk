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

typedef boost::program_options::value_semantic ValueType;

class IOptionsHolder;

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
       *
       * @param in_owner
       */
      explicit Init(Options& in_owner);

      Init& operator()(const char* in_name, const ValueType* in_value, const char* in_description);

   private:
      Options& m_owner;
   };

   static Options& getInstance();

   Init registerOptions();

   Error readOptions(int in_argc, const char* const in_argv[], const system::FilePath& in_location);

   unsigned int getJobExpiryHours() const;
   unsigned int getHeartbeatIntervalSeconds() const;
   logging::LogLevel getLogLevel() const;
   system::FilePath getScratchPath() const;
   const system::User& getServerUser() const;
   unsigned int getThreadPoolSize() const;

private:
   PRIVATE_IMPL(m_impl);

   void initialize();

   friend Init;
};

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

#endif
