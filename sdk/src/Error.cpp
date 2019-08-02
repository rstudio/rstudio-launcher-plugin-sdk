/*
 * Error.cpp
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

#include "Error.hpp"

#include <ostream>
#include <sstream>

namespace rstudio {
namespace launcher_plugins {

namespace
{

// Static empty values so we can avoid creating an Impl for the default Error constructor.
static std::string kEmptyString = "";
static Error kNoError = Error();
static ErrorLocation kNoErrorLocation = ErrorLocation();

} // anonymous namespace

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
   Impl() : Impl(0, "", ErrorLocation())
   { };

   Impl(int in_errorCode, std::string in_name, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Location(std::move(in_location))
   { };

   Impl(int in_errorCode, std::string in_name, const Error& in_cause, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Cause(in_cause),
      Location(std::move(in_location))
   { };

   Impl(int in_errorCode, std::string in_name, std::string in_errorMessage, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Message(std::move(in_errorMessage)),
      Location(std::move(in_location))
   { };

   Impl(int in_errorCode, std::string in_name, std::string in_errorMessage, const Error& in_cause, ErrorLocation in_location) :
      ErrorCode(in_errorCode),
      Name(std::move(in_name)),
      Message(std::move(in_errorMessage)),
      Cause(in_cause),
      Location(std::move(in_location))
   { };

   int ErrorCode;
   std::string Name;
   std::string Message;
   Error Cause;
   ErrorLocation Location;
};

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
   m_impl(nullptr)
{
}

Error::Error(const boost::system::error_code& in_ec, ErrorLocation in_errorLocation) :
   m_impl(new Impl(in_ec.value(), in_ec.category().name(), std::move(in_errorLocation)))
{
}

Error::Error(const boost::system::error_code& in_ec, const Error& in_cause, ErrorLocation in_errorLocation) :
   m_impl(new Impl(in_ec.value(), in_ec.category().name(), in_cause, std::move(in_errorLocation)))
{
}

Error::Error(const boost::system::error_code& in_ec, std::string in_errorMessage, ErrorLocation in_errorLocation) :
   m_impl(new Impl(in_ec.value(), in_ec.category().name(), std::move(in_errorMessage), std::move(in_errorLocation)))
{
}

Error::Error(
   const boost::system::error_code& in_ec,
   std::string in_errorMessage,
   const Error& in_cause,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_ec.value(),
            in_ec.category().name(),
            std::move(in_errorMessage),
            in_cause,
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
   const Error& in_cause,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_errorCode,
            std::move(in_name),
            in_cause,
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
   const Error& in_cause,
   ErrorLocation in_errorLocation) :
      m_impl(
         new Impl(
            in_errorCode,
            std::move(in_name),
            std::move(in_errorMessage),
            in_cause,
            std::move(in_errorLocation)))
{
}

Error::operator bool() const
{
   if (m_impl)
   {
      return m_impl->ErrorCode != 0;
   }

   return false;
}

std::string Error::asString() const
{
   if (m_impl)
   {
      std::ostringstream oss;
      oss << *this;
      return oss.str();
   }

   return "";
}

std::string Error::getSummary() const
{
   if (m_impl)
   {
      std::ostringstream oss;
      oss << m_impl->Name << " error " << m_impl->ErrorCode << " (" << m_impl->Message << ")";
      return oss.str();
   }

   return "";
}

int Error::getErrorCode() const
{
   if (m_impl)
   {
      return m_impl->ErrorCode;
   }

   return 0;
}

const std::string& Error::getName() const
{
   if (m_impl)
   {
      return m_impl->Name;
   }

   return kEmptyString;
}

const std::string& Error::getMessage() const
{
   if (m_impl)
   {
      return m_impl->Message;
   }

   return kEmptyString;
}

const Error& Error::getCause() const
{
   if (m_impl)
   {
      return m_impl->Cause;
   }

   return kNoError;
}

const ErrorLocation& Error::getLocation() const
{
   if (m_impl)
   {
      return m_impl->Location;
   }

   return kNoErrorLocation;
}

// System Error ========================================================================================================
Error systemError(int in_errorCode, ErrorLocation in_location)
{
   return Error(
      boost::system::error_code(
         in_errorCode,
         boost::system::system_category()),
      std::move(in_location));
}

Error systemError(int in_errorCode, const Error& in_cause, ErrorLocation in_location)
{
   return Error(
      boost::system::error_code(
         in_errorCode,
         boost::system::system_category()),
      in_cause,
      std::move(in_location));
}

Error systemError(int in_errorCode, std::string in_errorMessage, ErrorLocation in_location)
{
   return Error(
      boost::system::error_code(
         in_errorCode,
         boost::system::system_category()),
      std::move(in_errorMessage),
      std::move(in_location));
}

Error systemError(int in_errorCode, std::string in_errorMessage, const Error& in_cause, ErrorLocation in_location)
{
   return Error(
      boost::system::error_code(
         in_errorCode,
         boost::system::system_category()),
      std::move(in_errorMessage),
      in_cause,
      std::move(in_location));
}

Error unknownError(std::string in_errorMessage, ErrorLocation in_location)
{
   return Error(
      1,
      "UnknownError",
      std::move(in_errorMessage),
      std::move(in_location));
}

Error unknownError(std::string in_errorMessage, const Error& in_cause, ErrorLocation in_location)
{
   return Error(
      1,
      "UnknownError",
      std::move(in_errorMessage),
      in_cause,
      std::move(in_location));
}

} // namespace launcher_plugins
} // namespace rstudio

