/*
 * Error.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "Error.hpp"

#include <ostream>
#include <sstream>

namespace rstudio {
namespace launcher_plugins {

// Error Location ======================================================================================================
struct ErrorLocation::Impl
{
   Impl() : Line(0) { };

   Impl(const char* in_function, const char* in_file, long in_line) :
      Function(in_function),
      File(in_file),
      Line(in_line)
   { }

   std::string Function;
   std::string File;
   long Line;
};

PRIVATE_IMPL_DELETER_IMPL(ErrorLocation)

std::ostream& operator<<(std::ostream& in_os, const ErrorLocation& in_location)
{
   in_os << in_location.getFunction() << " "
         << in_location.getFile() << ":"
         << in_location.getLine() ;

   return in_os;
}

ErrorLocation::ErrorLocation() :
   m_impl(new ErrorLocation::Impl())
{
}

ErrorLocation::ErrorLocation(ErrorLocation&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{
}

ErrorLocation::ErrorLocation(const char* in_function, const char* in_file, long in_line) :
   m_impl(new ErrorLocation::Impl(in_function, in_file, in_line))
{
}

bool ErrorLocation::hasLocation() const
{
   return m_impl->Line != 0;
}

const std::string& ErrorLocation::getFunction() const
{
   return m_impl->Function;
}

const std::string& ErrorLocation::getFile() const
{
   return m_impl->File;
}

long ErrorLocation::getLine() const
{
   return m_impl->Line;
}

std::string ErrorLocation::asString() const
{
   std::ostringstream oss;
   oss << *this;
   return oss.str();
}


// Error ==============================================================================================================
struct Error::Impl
{
   Impl() = default;

   Impl(int in_errorCode, std::string in_name, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Location(std::move(in_location))
   { };

   Impl(int in_errorCode, std::string in_name, Error in_cause, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Cause(std::move(in_cause)),
      Location(std::move(in_location))
   { };

   Impl(int in_errorCode, std::string in_name, std::string in_errorMessage, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Message(std::move(in_errorMessage)),
      Location(std::move(in_location))
   { };

   Impl(int in_errorCode, std::string in_name, std::string in_errorMessage, Error in_cause, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Message(std::move(in_errorMessage)),
      Cause(std::move(in_cause)),
      Location(std::move(in_location))
   { };

   int ErrorCode;
   std::string Name;
   std::string Message;
   Error Cause;
   ErrorLocation Location;
};

PRIVATE_IMPL_DELETER_IMPL(Error)

std::ostream& operator<<(std::ostream& in_os, const Error& in_error)
{
   in_os << in_error.getSummary() << " at " << in_error.getLocation();
   if (in_error.getCause())
   {
      in_os << std::endl << "   caused by: " << in_error.getCause();
   }

   return in_os;
}

Error::Error() :
   m_impl(new Impl())
{
}

Error::Error(Error&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{
}

Error::Error(const boost::system::error_code& in_ec, ErrorLocation in_errorLocation) :
   m_impl(new Impl(in_ec.value(), in_ec.category().name(), std::move(in_errorLocation)))
{
}

Error::Error(const boost::system::error_code& in_ec, Error in_cause, ErrorLocation in_errorLocation) :
   m_impl(new Impl(in_ec.value(), in_ec.category().name(), std::move(in_cause), std::move(in_errorLocation)))
{
}

Error::Error(const boost::system::error_code& in_ec, std::string in_errorMessage, ErrorLocation in_errorLocation) :
   m_impl(new Impl(in_ec.value(), in_ec.category().name(), std::move(in_errorMessage), std::move(in_errorLocation)))
{
}

Error::Error(
   const boost::system::error_code& in_ec,
   std::string in_errorMessage,
   Error in_cause,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_ec.value(),
            in_ec.category().name(),
            std::move(in_errorMessage),
            std::move(in_cause),
            std::move(in_errorLocation)))
{

}

Error::Error(
   int in_errorCode,
   std::string in_name,
   ErrorLocation in_errorLocation) :
      m_impl(new Impl(in_errorCode, std::move(in_name), std::move(in_errorLocation)))
{
}

Error::Error(
   int in_errorCode,
   std::string in_name,
   Error in_cause,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_errorCode,
            std::move(in_name),
            std::move(in_cause),
            std::move(in_errorLocation)))
{
}

Error::Error(
   int in_errorCode,
   std::string in_name,
   std::string in_errorMessage,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_errorCode,
            std::move(in_name),
            std::move(in_errorMessage),
            std::move(in_errorLocation)))
{
}

Error::Error(
   int in_errorCode,
   std::string in_name,
   std::string in_errorMessage,
   Error in_cause,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_errorCode,
            std::move(in_name),
            std::move(in_errorMessage),
            std::move(in_cause),
            std::move(in_errorLocation)))
{
}

Error::operator bool() const
{
   return m_impl->ErrorCode == 0;
}

std::string Error::asString() const
{
   std::ostringstream oss;
   oss << *this;
   return oss.str();
}

std::string Error::getSummary() const
{
   std::ostringstream oss;
   oss << m_impl->Name << " error " << m_impl->ErrorCode << " (" << m_impl->Message << ")";
   return oss.str();
}

int Error::getErrorCode() const
{
   return m_impl->ErrorCode;
}

const std::string& Error::getName() const
{
   return m_impl->Name;
}

const std::string& Error::getMessage() const
{
   return m_impl->Message;
}

const Error& Error::getCause() const
{
   return m_impl->Cause;
}

const ErrorLocation& Error::getLocation() const
{
   return m_impl->Location;
}

// System Error ========================================================================================================
Error systemError(int in_errorCode, ErrorLocation in_location)
{
   return std::move(
      Error(
         boost::system::error_code(
            in_errorCode,
            boost::system::system_category()),
         std::move(in_location)));
}

Error systemError(int in_errorCode, Error in_cause, ErrorLocation in_location)
{
   return std::move(
      Error(
         boost::system::error_code(
            in_errorCode,
            boost::system::system_category()),
         std::move(in_cause),
         std::move(in_location)));
}

Error systemError(int in_errorCode, std::string in_errorMessage, ErrorLocation in_location)
{
   return std::move(
      Error(
         boost::system::error_code(
            in_errorCode,
            boost::system::system_category()),
         std::move(in_errorMessage),
         std::move(in_location)));
}

Error systemError(int in_errorCode, std::string in_errorMessage, Error in_cause, ErrorLocation in_location)
{
   return std::move(
      Error(
         boost::system::error_code(
            in_errorCode,
            boost::system::system_category()),
         std::move(in_errorMessage),
         std::move(in_cause),
         std::move(in_location)));
}


} // namespace launcher_plugins
} // namespace rstudio

