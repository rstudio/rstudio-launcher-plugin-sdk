/*
 * Options.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "options/Options.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/thread.hpp>

#include "logging/Logger.hpp"
#include "system/FilePath.hpp"
#include "system/User.hpp"

using namespace boost::program_options;

namespace rstudio {
namespace launcher_plugins {
namespace logging {

// Overload operator>> and operator<< for LogLevel for parsing the config file.
std::istream& operator>>(std::istream& in_stream, LogLevel& out_logLevel)
{
   std::string logLevelStr;
   in_stream >> logLevelStr;
   if (boost::iequals(logLevelStr, "OFF") || (logLevelStr == "0"))
   {
      out_logLevel = LogLevel::OFF;
   }
   else if (boost::iequals(logLevelStr, "ERROR") || (logLevelStr == "1"))
   {
      out_logLevel = LogLevel::ERROR;
   }
   else if (boost::iequals(logLevelStr, "WARNING") || (logLevelStr == "2"))
   {
      out_logLevel = LogLevel::WARNING;
   }
   else if (boost::iequals(logLevelStr, "INFO") || (logLevelStr == "3"))
   {
      out_logLevel = LogLevel::INFO;
   }
   else if (boost::iequals(logLevelStr, "DEBUG") || (logLevelStr == "4"))
   {
      out_logLevel = LogLevel::DEBUG;
   }
   else
   {
      in_stream.setstate(std::ios_base::failbit);
   }

   return in_stream;
}

std::ostream& operator<<(std::ostream& in_stream, const LogLevel& in_logLevel)
{
   using namespace logging;
   switch (in_logLevel)
   {
      case LogLevel::OFF:
      {
         in_stream << "OFF";
      }
      case LogLevel::ERROR:
      {
         in_stream << "ERROR";
      }
      case LogLevel::WARNING:
      {
         in_stream << "WARNING";
      }
      case LogLevel::INFO:
      {
         in_stream << "INFO";
      }
      case LogLevel::DEBUG:
      {
         in_stream << "DEBUG";
      }
      default:
      {
         assert(false);
         in_stream.setstate(std::ostream::failbit);
      }
   }

   return in_stream;
}

} // namespace logging

namespace system {

// Overload operator>> and operator<< for User for parsing the config file.
std::istream& operator>>(std::istream& in_stream, User& out_user)
{
   std::string username;
   in_stream >> username;
   out_user = User(username);
   return in_stream;
}

std::ostream& operator<<(std::ostream& in_stream, const User& in_user)
{
   // For these purposes, just the username is sufficient.
   return in_stream << in_user.getUsername();
}

// Overload operator>> and operator<< for FilePath for parsing the config file.
std::istream& operator>>(std::istream& in_stream, FilePath& out_filePath)
{
   std::string filePath;
   in_stream >> filePath;
   out_filePath = FilePath(out_filePath);
   return in_stream;
}

std::ostream& operator<<(std::ostream& in_stream, const FilePath& in_filePath)
{
   return in_stream << in_filePath.absolutePath();
}

} // namespace system

namespace options {

namespace {

enum class OptionsError
{
   SUCCESS = 0,
   NO_CONFIG_FILE = 1,
   PARSE_ERROR = 2,
   UNREGISTERED_OPTION = 3,
   INVALID_OPTION_VALUE = 4,
   READ_FAILURE = 5
};

Error optionsError(OptionsError in_errorCode, const std::string& in_message, ErrorLocation in_errorLocation)
{
   switch(in_errorCode)
   {
      case OptionsError::NO_CONFIG_FILE:
         return Error(static_cast<int>(in_errorCode), "NoConfigFile", in_message, std::move(in_errorLocation));
      case OptionsError::PARSE_ERROR:
         return Error(static_cast<int>(in_errorCode), "ParseError", in_message, std::move(in_errorLocation));
      case OptionsError::UNREGISTERED_OPTION:
         return Error(static_cast<int>(in_errorCode), "UnregisteredOption", in_message, std::move(in_errorLocation));
      case OptionsError::INVALID_OPTION_VALUE:
         return Error(static_cast<int>(in_errorCode), "InvalidOptionValue", in_message, std::move(in_errorLocation));
      case OptionsError::READ_FAILURE:
         return Error(static_cast<int>(in_errorCode), "OptionReadError", in_message, std::move(in_errorLocation));
      case OptionsError::SUCCESS:
         return Success();
      default:
      {
         assert(false);
         return Error(static_cast<int>(in_errorCode), "UnrecognizedError", in_message, std::move(in_errorLocation));
      }
   }
}

} // anonymous namespace

// Value ===============================================================================================================
template <class T>
struct Value<T>::Impl
{
   Impl() : ValueSemantic(boost::program_options::value<T>()) { };

   explicit Impl(boost::program_options::value_semantic* in_value) : ValueSemantic(in_value) { };

   // Using a boost shared ptr here so that reference counting will be done properly with the boost functions this will
   // be passed to.
   boost::shared_ptr<boost::program_options::value_semantic> ValueSemantic;
};

template <class T>
Value<T>::Value(T& io_storeTo) :
   m_impl(new Impl(boost::program_options::value<T>(&io_storeTo)))
{
}

template <class T>
Value<T>& Value<T>::setDefaultValue(const T& in_defaultValue)
{
   m_impl->ValueSemantic->default_value(in_defaultValue);
   return *this;
}

// Options =============================================================================================================
struct Options::Impl
{
   // The values for the option members (e.g. JobExpiryHours, ScratchPath, etc.) are just placeholders and don't matter.
   // The values for OptionsDescription and IsInitialzied are not placeholders.
   Impl() :
      OptionsDescription("program"),
      IsInitialized(false),
      EnableDebugLogging(false),
      JobExpiryHours(0),
      HeartbeatIntervalSeconds(0),
      MaxLogLevel(logging::LogLevel::OFF),
      ScratchPath(""),
      ServerUser(),
      ThreadPoolSize(0)
   { };

   void initialize()
   {
      // lock the mutex to ensure we don't initialize the options twice.
      boost::unique_lock<boost::mutex> lock(Mutex);
      if (!IsInitialized)
      {
         OptionsDescription.add_options()
            ("job-expiry-hours",
             value<unsigned int>(&JobExpiryHours)->default_value(24),
             "amount of hours before completed jobs are removed from the system")
            ("heartbeat-interval-seconds",
             value<unsigned int>(&HeartbeatIntervalSeconds)->default_value(5),
             "the amount of seconds between heartbeats - 0 to disable")
            ("enable-debug-logging",
             value<bool>(&EnableDebugLogging)->default_value(false),
             "whether to enable debug logging or not - if true, enforces a log-level of at least DEBUG")
            ("log-level",
             value<logging::LogLevel>(&MaxLogLevel)->default_value(logging::LogLevel::WARNING),
             "the maximum level of log messages to write")
            ("scratch-path",
             value<system::FilePath>(&ScratchPath)->default_value(system::FilePath("/var/lib/rstudio-launcher/")),
             "scratch path where logs and job state data are stored")
            ("server-user",
             value<system::User>(&ServerUser)->default_value(system::User("rstudio-server")),
             "user to run the plugin as")
            ("thread-pool-size",
             value<unsigned int>(&ThreadPoolSize)->default_value(
                std::max<unsigned int>(4, boost::thread::hardware_concurrency())),
             "the number of threads in the thread pool");

         IsInitialized = true;
      }
   }

   // Mutex to protect read and write.
   boost::mutex Mutex;

   // Boost program options.
   options_description OptionsDescription;

   // Whether the initialize method has been called yet.
   bool IsInitialized;

   // Option Members.
   bool EnableDebugLogging;
   unsigned int JobExpiryHours;
   unsigned int HeartbeatIntervalSeconds;
   logging::LogLevel MaxLogLevel;
   system::FilePath ScratchPath;
   system::User ServerUser;
   unsigned int ThreadPoolSize;

};

PRIVATE_IMPL_DELETER_IMPL(Options)

Options::Init::Init(Options& in_owner) :
   m_owner(in_owner)
{
}

template <class T>
Options::Init& Options::Init::operator()(const char* in_name, Value<T>& io_value, const char* in_description)
{
   m_owner.m_impl->OptionsDescription.add_options()(in_name, io_value->m_impl->ValueSematic.get(), in_description);
}

Options& Options::getInstance()
{
   static Options options;
   options.m_impl->initialize(); // This function is thread-safe and idempotent.
   return options;
}

Options::Init Options::registerOptions()
{
   return Options::Init(*this);
}

Error Options::readOptions(int in_argc, const char* const in_argv[], const system::FilePath& in_location)
{
   // This should be initialized in getInstance.
   assert(m_impl->IsInitialized);

   if (in_location.isEmpty())
   {
      return systemError(
         boost::system::errc::no_such_file_or_directory,
         "No configuration file specified",
         ERROR_LOCATION);
   }

   if (!in_location.exists())
   {
      return systemError(
         boost::system::errc::no_such_file_or_directory,
         "Configuration file does not exist: " + in_location.absolutePath(),
         ERROR_LOCATION);
   }

   try
   {
      // The configuration file overrides command line options, so parse the config file first.
      variables_map vm;
      std::shared_ptr<std::istream> inputStream;
      Error error = in_location.openForRead(inputStream);
      if (error)
         return error;

      try
      {
         store(parse_config_file(*inputStream, m_impl->OptionsDescription), vm);
         notify(vm);
      }
      catch (const std::exception& e)
      {
         return optionsError(
            OptionsError::READ_FAILURE,
            "Error reading " + in_location.absolutePath() + ": " + std::string(e.what()),
            ERROR_LOCATION);
      }

      // Now read the command line arguments.
      store(parse_command_line(in_argc, const_cast<char**>(in_argv), m_impl->OptionsDescription), vm);
      notify(vm);

      std::vector<std::string> unregisteredOptions;
   }
   catch (boost::program_options::error& e)
   {
      return optionsError(
         OptionsError::PARSE_ERROR,
         std::string(e.what()) + " in config file " + in_location.absolutePath(),
         ERROR_LOCATION);
   }
   catch (const std::exception& e)
   {
      return unknownError("Unexpected exception: " + std::string(e.what()), ERROR_LOCATION);
   }

   return Success();
}

unsigned int Options::getJobExpiryHours() const
{
   return m_impl->JobExpiryHours;
}

unsigned int Options::getHeartbeatIntervalSeconds() const
{
   return m_impl->HeartbeatIntervalSeconds;
}

logging::LogLevel Options::getLogLevel() const
{
   return (!m_impl->EnableDebugLogging || (m_impl->MaxLogLevel >= logging::LogLevel::DEBUG) ?
      m_impl->MaxLogLevel :
      logging::LogLevel::DEBUG);
}

const system::FilePath& Options::getScratchPath() const
{
   return m_impl->ScratchPath;
}

const system::User& Options::getServerUser() const
{
   return m_impl->ServerUser;
}

unsigned int Options::getThreadPoolSize() const
{
   return m_impl->ThreadPoolSize;
}

// Template Instantiations =============================================================================================
// We pre-define which classes can be used as the template parameter Value because it's implementation is private and
// they need to be compiled here to be used elsewhere.

//template <> class Value<bool>;
//template <> class Value<char>;
//template <> class Value<unsigned char>;
//template <> class Value<short>;
//template <> class Value<unsigned short>;
//template <> class Value<int>;
//template <> class Value<unsigned int>;
//template <> class Value<long>;
//template <> class Value<unsigned long>;
//template <> class Value<long long>;
//template <> class Value<unsigned long long>;
//template <> class Value<float>;
//template <> class Value<double>;
//template <> class Value<long double>;
//template <> class Value<std::string>;
//template <> class Value<logging::LogLevel>;
//template <> class Value<system::FilePath>;
//template <> class Value<system::User>;

#define DECLARE_INIT_OPERATOR_TEMPLATE(in_type)                                                                      \
template<>                                                                                                           \
Options::Init& Options::Init::operator()(const char* in_name, Value<in_type>& io_value, const char* in_description); \

// These should instantiate both Init::operator() and the Value class with these types.
DECLARE_INIT_OPERATOR_TEMPLATE(bool)
DECLARE_INIT_OPERATOR_TEMPLATE(char)
DECLARE_INIT_OPERATOR_TEMPLATE(unsigned char)
DECLARE_INIT_OPERATOR_TEMPLATE(short)
DECLARE_INIT_OPERATOR_TEMPLATE(unsigned short)
DECLARE_INIT_OPERATOR_TEMPLATE(int)
DECLARE_INIT_OPERATOR_TEMPLATE(unsigned int)
DECLARE_INIT_OPERATOR_TEMPLATE(long)
DECLARE_INIT_OPERATOR_TEMPLATE(unsigned long)
DECLARE_INIT_OPERATOR_TEMPLATE(long long)
DECLARE_INIT_OPERATOR_TEMPLATE(unsigned long long)
DECLARE_INIT_OPERATOR_TEMPLATE(float)
DECLARE_INIT_OPERATOR_TEMPLATE(double)
DECLARE_INIT_OPERATOR_TEMPLATE(long double)
DECLARE_INIT_OPERATOR_TEMPLATE(std::string)
DECLARE_INIT_OPERATOR_TEMPLATE(logging::LogLevel)
DECLARE_INIT_OPERATOR_TEMPLATE(system::FilePath)
DECLARE_INIT_OPERATOR_TEMPLATE(system::User)

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

