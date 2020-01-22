/*
 * Request.cpp
 *
 * Copyright (C) 2019 by RStudio, Inc.
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

#include <api/Request.hpp>

#include <Error.hpp>
#include <json/Json.hpp>
#include <json/JsonUtils.hpp>
#include <logging/Logger.hpp>
#include <system/User.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

// Common fields for all requests.
constexpr char const * FIELD_MESSAGE_TYPE = "messageType";
constexpr char const * FIELD_REQUEST_ID = "requestId";

// Bootstrap request fields.
constexpr char const * FIELD_VERSION = "version";
constexpr char const * FIELD_VERSION_MAJOR = "major";
constexpr char const * FIELD_VERSION_MINOR = "minor";
constexpr char const * FIELD_VERSION_PATCH = "patch";

enum class RequestError
{
   SUCCESS = 0,
   INVALID_REQUEST_TYPE = 1,
};

Error requestError(
   RequestError in_errorCode,
   const std::string& in_details,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation)
{
   std::string message;

   switch (in_errorCode)
   {
      case RequestError::INVALID_REQUEST_TYPE:
      {
         message.append("Invalid request type received from launcher");
         break;
      }
      case RequestError::SUCCESS:
         return Success();
   }

   message.append(in_details.empty() ? "." : ": " + in_details + ".");

   if (in_cause == Success())
      return Error("RequestError", static_cast<int>(in_errorCode), message, in_errorLocation);

   return Error("RequestError", static_cast<int>(in_errorCode), message, in_cause, in_errorLocation);
}

Error requestError(RequestError in_errorCode, const std::string& in_details, const ErrorLocation& in_errorLocation)
{
   return requestError(in_errorCode, in_details, Success(), in_errorLocation);
}

}

// Request =============================================================================================================
struct Request::Impl
{
   /**
    * @brief Constructor.
    */
   Impl(Type in_requestType) :
      Id(0),
      RequestType(in_requestType),
      IsValid(true)
   {
   }

   /** The ID of the request. */
   uint64_t Id;

   /** The type of the request. */
   Type RequestType;

   /** Whether the request is valid. */
   bool IsValid;
};

PRIVATE_IMPL_DELETER_IMPL(Request)

Error Request::fromJson(const json::Object& in_requestJson, std::shared_ptr<Request>& out_request)
{
   int messageTypeVal = -1;
   Error error = json::readObject(in_requestJson, FIELD_MESSAGE_TYPE, messageTypeVal);
   if (error)
      return error;

   if ((messageTypeVal >= static_cast<int>(Type::INVALID)) || (messageTypeVal < 0))
      return requestError(
         RequestError::INVALID_REQUEST_TYPE,
         std::to_string(messageTypeVal),
         ERROR_LOCATION);

   Type messageType = static_cast<Type>(messageTypeVal);
   switch (messageType)
   {
      case Type::BOOTSTRAP:
      {
         out_request.reset(new BootstrapRequest(in_requestJson));
      }
      default:
      {
         return requestError(
            RequestError::INVALID_REQUEST_TYPE,
            std::to_string(messageTypeVal),
            ERROR_LOCATION);
      }
   }

   return Success();
}

uint64_t Request::getId() const
{
   return m_baseImpl->Id;
}

Request::Type Request::getType() const
{
   return m_baseImpl->RequestType;
}

Request::Request(Type in_requestType, const json::Object& in_requestJson) :
   m_baseImpl(new Impl(in_requestType))
{
   Error error = json::readObject(
      in_requestJson,
      FIELD_REQUEST_ID, m_baseImpl->Id);

   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
      return;
   }
}

// Bootstrap ===========================================================================================================
struct BootstrapRequest::Impl
{
   /**
    * @brief Constructor.
    */
   Impl() :
      Major(0),
      Minor(0),
      Patch(0)
   {
   }

   /** The major value of the version. */
   int Major;

   /** The minor value of the version. */
   int Minor;

   /** The patch value of the version. */
   int Patch;
};

BootstrapRequest::BootstrapRequest(const json::Object& in_requestJson) :
   Request(Type::BOOTSTRAP, in_requestJson),
   m_impl(new Impl())
{
   json::Object versionObject;
   Error error = json::readObject(in_requestJson, FIELD_VERSION, versionObject);

   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
      return;
   }

   error = json::readObject(
      versionObject,
      FIELD_VERSION_MAJOR, m_impl->Major,
      FIELD_VERSION_MINOR, m_impl->Minor,
      FIELD_VERSION_PATCH, m_impl->Patch);
   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
   }
}

int BootstrapRequest::getMajorVersion() const
{
   return m_impl->Major;
}

int BootstrapRequest::getMinorVersion() const
{
   return m_impl->Minor;
}

int BootstrapRequest::getPatchNumber() const
{
   return m_impl->Patch;
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
