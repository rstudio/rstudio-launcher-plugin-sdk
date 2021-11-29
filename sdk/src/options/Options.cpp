/*
 * Options.cpp
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

#include <options/Options.hpp>

#include <mutex>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <logging/Logger.hpp>
#include <system/FilePath.hpp>
#include <system/User.hpp>

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
      out_logLevel = LogLevel::ERR;
   }
   else if (boost::iequals(logLevelStr, "WARNING") || (logLevelStr == "2"))
   {
      out_logLevel = LogLevel::WARN;
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
         break;
      }
      case LogLevel::ERR:
      {
         in_stream << "ERROR";
         break;
      }
      case LogLevel::WARN:
      {
         in_stream << "WARNING";
         break;
      }
      case LogLevel::INFO:
      {
         in_stream << "INFO";
         break;
      }
      case LogLevel::DEBUG:
      {
         in_stream << "DEBUG";
         break;
      }
      default:
      {
         assert(false);
         in_stream.setstate(std::ostream::failbit);
         break;
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

   Error error = User::getUserFromIdentifier(username, out_user);
   if (error)
      logging::logError(error, ERROR_LOCATION);

   return in_stream;
}

std::ostream& operator<<(std::ostream& in_stream, const User& in_user)
{
   // For these purposes, just the username is sufficient.
   return in_stream << in_user.getUsername();
}

// Overload operator>> for FilePath for parsing the config file.
std::istream& operator>>(std::istream& in_stream, FilePath& out_filePath)
{
   std::string filePath;
   in_stream >> filePath;
   out_filePath = FilePath(filePath);
   return in_stream;
}

} // namespace system

namespace options {

namespace {

constexpr char const* s_defaultSandboxPath = "/usr/lib/rstudio-server/bin/rsandbox";
constexpr char const* s_defaultScratchPath = "/var/lib/rstudio-launcher/";
constexpr char const* s_defaultLoggingDir = "var/log/rstudio/launcher";

enum class OptionsError
{
   SUCCESS = 0,
   PARSE_ERROR = 1,
   UNREGISTERED_OPTION = 2,
   READ_FAILURE = 3,
   MISSING_REQUIRED_OPTION=4,
};

Error optionsError(OptionsError in_errorCode, const std::string& in_message, ErrorLocation in_errorLocation)
{
   switch(in_errorCode)
   {
      case OptionsError::PARSE_ERROR:
         return Error("ParseError", static_cast<int>(in_errorCode), in_message, std::move(in_errorLocation));
      case OptionsError::UNREGISTERED_OPTION:
         return Error("UnregisteredOption", static_cast<int>(in_errorCode), in_message, std::move(in_errorLocation));
      case OptionsError::READ_FAILURE:
         return Error("OptionReadError", static_cast<int>(in_errorCode), in_message, std::move(in_errorLocation));
      case OptionsError::MISSING_REQUIRED_OPTION:
         return Error("MissingRequiredOption", static_cast<int>(in_errorCode), in_message, std::move(in_errorLocation));
      case OptionsError::SUCCESS:
         return Success();
      default:
      {
         assert(false);
         return Error("UnrecognizedError", static_cast<int>(in_errorCode), in_message, std::move(in_errorLocation));
      }
   }
}

void collectUnrecognizedOptions(
      const variables_map& in_vm,
      const parsed_options& in_parsedOptions,
      std::vector<std::string>& out_unrecognized)
{
   for (const auto& opt: in_parsedOptions.options)
   {
      if (in_vm.find(opt.string_key) == in_vm.end())
      {
         std::string value;
         for (std::string v: opt.value)
            value += v + ", ";

         // Cut the last ", " from the value list string.
         out_unrecognized.push_back(opt.string_key + "=" + value.substr(0, value.size() - 2));
      }
   }
}

Error validateOptions(
   const variables_map& in_vm,
   const options_description& in_optionsDescription,
   const std::string& in_configFile)
{
   for (const auto& optionDesc: in_optionsDescription.options())
   {
      const std::string& optionName = optionDesc->long_name();
      if (in_vm.count(optionName) < 1)
      {
         std::string message = "Required option (";
         message += optionName + ") not specified in config file ";
         message += in_configFile;
         return optionsError(OptionsError::MISSING_REQUIRED_OPTION, message, ERROR_LOCATION);
      }
   }

   return Success();
}

} // anonymous namespace

// Value ===============================================================================================================
template <class T>
struct Value<T>::Impl
{

   Impl() : ValueSemantic(boost::program_options::value<T>()) { };

   explicit Impl(boost::program_options::typed_value<T>* in_value) :
      ValueSemantic(in_value)
   { };

   // Ownership of this ptr should be dropped if we're passing it to boost::program_options::options_description::easy+io
   std::unique_ptr<boost::program_options::typed_value<T> > ValueSemantic;
};

template <class T>
Value<T>::Value() :
   m_impl(new Impl())
{

}

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
   // The values for OptionsDescription and IsInitialized are not placeholders.
   Impl() :
      OptionsDescription("program"),
      IsInitialized(false),
      EnableDebugLogging(false),
      JobExpiryHours(0),
      HeartbeatIntervalSeconds(0),
      LauncherConfigFile(""),
      MaxLogLevel(logging::LogLevel::OFF),
      ScratchPath(""),
      ServerUser(),
      LoggingDir(""),
      ThreadPoolSize(0)
   { };

   void initialize()
   {
      // lock the mutex to ensure we don't initialize the options twice.
      std::unique_lock<std::mutex> lock(Mutex);
      if (!IsInitialized)
      {
         OptionsDescription.add_options()
            ("enable-debug-logging",
               value<bool>(&EnableDebugLogging)->default_value(false),
               "whether to enable debug logging or not - if true, enforces a log-level of at least DEBUG")
            ("job-expiry-hours",
               value<unsigned int>(&JobExpiryHours)->default_value(24),
               "amount of hours before completed jobs are removed from the system")
            ("heartbeat-interval-seconds",
               value<unsigned int>(&HeartbeatIntervalSeconds)->default_value(5),
               "the amount of seconds between heartbeats - 0 to disable")
            ("launcher-config-file",
               value<system::FilePath>(&LauncherConfigFile)->default_value(system::FilePath()),
               "path to launcher config file")
            ("log-level",
               value<logging::LogLevel>(&MaxLogLevel)->default_value(logging::LogLevel::WARN),
               "the maximum level of log messages to write")
            ("max-message-size",
               value<size_t>(&MaxMessageSize)->default_value(5242880),
               "the maximum size of a message which can be sent to or received from the RStudio Launcher")
            ("plugin-name",
               value<std::string>(&PluginName)->default_value(""),
               "the name of this plugin")
            ("rsandbox-path",
               value<system::FilePath>(&RSandboxPath)->default_value(system::FilePath(s_defaultSandboxPath)),
               "path to rsandbox executable")
            ("scratch-path",
               value<system::FilePath>(&ScratchPath)->default_value(system::FilePath(s_defaultScratchPath)),
               "scratch path where logs and job state data are stored")
            ("server-user",
               value<std::string>(&ServerUser)->default_value("rstudio-server"),
               "user to run the plugin as")
            ("thread-pool-size",
               value<size_t>(&ThreadPoolSize)->default_value(std::max<size_t>(4, std::thread::hardware_concurrency())),
               "the number of threads in the thread pool")
            ("unprivileged",
               value<bool>(&UseUnprivilegedMode)->default_value(false),
               "special unprivileged mode - does not change user, runs without root, no impersonation, single user")
             ("logging-dir",
               value<system::FilePath>(&LoggingDir)->default_value(system::FilePath(s_defaultLoggingDir)),
               "specifies path where debug logs should be written");
         IsInitialized = true;
      }
   }

   // Mutex to protect read and write.
   std::mutex Mutex;

   // Boost program options.
   options_description OptionsDescription;

   // Whether the initialize method has been called yet.
   bool IsInitialized;

   // Option Members.
   bool EnableDebugLogging;
   unsigned int JobExpiryHours;
   unsigned int HeartbeatIntervalSeconds;
   system::FilePath LauncherConfigFile;
   logging::LogLevel MaxLogLevel;
   size_t MaxMessageSize;
   std::string PluginName;
   system::FilePath RSandboxPath;
   system::FilePath ScratchPath;
   system::FilePath LoggingDir;
   std::string ServerUser;
   size_t ThreadPoolSize;
   bool UseUnprivilegedMode;
};

PRIVATE_IMPL_DELETER_IMPL(Options)

Options::Init::Init(Options& in_owner) :
   m_owner(in_owner)
{
}

template <class T>
Options::Init& Options::Init::operator()(const char* in_name, Value<T>& in_value, const char* in_description)
{
   // Boost takes ownership of the semantic value here.
   m_owner.m_impl->OptionsDescription.add_options()(in_name, in_value.m_impl->ValueSemantic.release(), in_description);
   return *this;
}

template <class T>
Options::Init& Options::Init::operator()(const char* in_name, Value<T>&& in_value, const char* in_description)
{
   // Boost takes ownership of the semantic value here.
   m_owner.m_impl->OptionsDescription.add_options()(in_name, in_value.m_impl->ValueSemantic.release(), in_description);
   return *this;
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

   try
   {
      variables_map vm;
      std::vector<std::string> unrecognizedFileOpts;

      // The configuration file overrides command line options, so parse the config file first.
      try
      {
         std::shared_ptr<std::istream> inputStream;
         if (!in_location.isEmpty() && in_location.exists())
         {
            Error error = in_location.openForRead(inputStream);
            if (error)
               return error;
         }
         else
            inputStream.reset(new std::istringstream());

         parsed_options parsed = parse_config_file(*inputStream, m_impl->OptionsDescription, true);
         store(parsed, vm);
         notify(vm);

         collectUnrecognizedOptions(vm, parsed, unrecognizedFileOpts);
      }
      catch (const std::exception& e)
      {
         return optionsError(
            OptionsError::READ_FAILURE,
            "Error reading " + in_location.getAbsolutePath() + ": " + std::string(e.what()),
            ERROR_LOCATION);
      }

      // Now read the command line arguments.
      std::vector<std::string> unrecognizedCmdOpts;
      if (in_argc > 0)
      {
         // Set up the parser so that command line options will override code-defaults.
         command_line_parser parser = command_line_parser(in_argc, const_cast<char**>(in_argv));
         parser.options(m_impl->OptionsDescription);
         parser.allow_unregistered();

         // Run the parser.
         parsed_options parsed = parser.run();
         store(parsed, vm);
         notify(vm);
         collectUnrecognizedOptions(vm, parsed, unrecognizedCmdOpts);
      }

      // Handle unrecognized options
      if (!unrecognizedFileOpts.empty() || !unrecognizedCmdOpts.empty())
      {
         std::string message = "The following options were unrecognized:";
         if (!unrecognizedFileOpts.empty())
            message += "\n    in config file " + in_location.getAbsolutePath() + ":";
         for (const std::string& opt: unrecognizedFileOpts)
            message += "\n        " + opt;

         if (!unrecognizedCmdOpts.empty())
            message += "\n    on the command line:";
         for (const std::string& opt: unrecognizedCmdOpts)
            message += "\n        " + opt;

         return optionsError(OptionsError::UNREGISTERED_OPTION, message, ERROR_LOCATION);
      }

      // Now validate the provided options.
      return validateOptions(vm, m_impl->OptionsDescription, in_location.getAbsolutePath());
   }
   catch (boost::program_options::error& e)
   {
      return optionsError(
         OptionsError::PARSE_ERROR,
         std::string(e.what()) + " in config file " + in_location.getAbsolutePath(),
         ERROR_LOCATION);
   }
   catch (const std::exception& e)
   {
      return unknownError("Unexpected exception: " + std::string(e.what()), ERROR_LOCATION);
   }
}

system::TimeDuration Options::getJobExpiryHours() const
{
   return system::TimeDuration::Hours(m_impl->JobExpiryHours);
}

system::TimeDuration Options::getHeartbeatIntervalSeconds() const
{
   return system::TimeDuration::Seconds(m_impl->HeartbeatIntervalSeconds);
}

const system::FilePath& Options::getLauncherConfigFile() const
{
   return m_impl->LauncherConfigFile;
}

logging::LogLevel Options::getLogLevel() const
{
   return (!m_impl->EnableDebugLogging || (m_impl->MaxLogLevel >= logging::LogLevel::DEBUG) ?
      m_impl->MaxLogLevel :
      logging::LogLevel::DEBUG);
}

size_t Options::getMaxMessageSize() const
{
   return m_impl->MaxMessageSize;
}

const system::FilePath& Options::getRSandboxPath() const
{
   return m_impl->RSandboxPath;
}
const system::FilePath& Options::getLoggingDir() const
{
   return m_impl->LoggingDir;
}
const system::FilePath& Options::getScratchPath() const
{
   return m_impl->ScratchPath;
}

Error Options::getServerUser(system::User& out_serverUser) const
{
   return system::User::getUserFromIdentifier(m_impl->ServerUser, out_serverUser);
}

size_t Options::getThreadPoolSize() const
{
   return m_impl->ThreadPoolSize;
}

bool Options::useUnprivilegedMode() const
{
   return m_impl->UseUnprivilegedMode;
}

bool Options::getEnableDebugLogging() const
{
   return m_impl->EnableDebugLogging;
}

Options::Options() :
   m_impl(new Options::Impl())
{
}

// Template Instantiations =============================================================================================
// We pre-define which classes can be used as the template parameter Value because it's implementation is private and
// they need to be compiled here to be used elsewhere.
#define INSTANTIATE_VALUE_TEMPLATES(in_type)                                                        \
template                                                                                            \
Options::Init& Options::Init::operator()<in_type>(const char*, Value<in_type>&, const char*);       \
template                                                                                            \
Options::Init& Options::Init::operator()<in_type>(const char*, Value<in_type>&&, const char*);      \
template class Value<in_type>;                                                                      \

// These should instantiate both Init::operator() and the Value class with these types.
INSTANTIATE_VALUE_TEMPLATES(bool)
INSTANTIATE_VALUE_TEMPLATES(char)
INSTANTIATE_VALUE_TEMPLATES(unsigned char)
INSTANTIATE_VALUE_TEMPLATES(short)
INSTANTIATE_VALUE_TEMPLATES(unsigned short)
INSTANTIATE_VALUE_TEMPLATES(int)
INSTANTIATE_VALUE_TEMPLATES(unsigned int)
INSTANTIATE_VALUE_TEMPLATES(long)
INSTANTIATE_VALUE_TEMPLATES(unsigned long)
INSTANTIATE_VALUE_TEMPLATES(long long)
INSTANTIATE_VALUE_TEMPLATES(unsigned long long)
INSTANTIATE_VALUE_TEMPLATES(float)
INSTANTIATE_VALUE_TEMPLATES(double)
INSTANTIATE_VALUE_TEMPLATES(long double)
INSTANTIATE_VALUE_TEMPLATES(std::string)
INSTANTIATE_VALUE_TEMPLATES(logging::LogLevel)
INSTANTIATE_VALUE_TEMPLATES(system::FilePath)
INSTANTIATE_VALUE_TEMPLATES(system::User)

} // namespace options
} // namespace launcher_plugins
} // namespace rstudio

