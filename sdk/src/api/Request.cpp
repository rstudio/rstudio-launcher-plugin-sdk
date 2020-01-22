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

constexpr char const * FIELD_MESSAGE_TYPE = "messageType";
constexpr char const * FIELD_REQUEST_ID = "requestId";
constexpr char const * FIELD_USERNAME = "username";
constexpr char const * FIELD_REQUEST_USERNAME = "requestUsername";

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
   Impl() :
      Id(0),
      RequestType(Type::HEARTBEAT),
      IsValid(false)
   {
   }

   uint64_t Id;
   std::string RequestUsername;
   Type RequestType;
   system::User User;
   bool IsValid;
};

PRIVATE_IMPL_DELETER_IMPL(Request)

Error Request::fromJson(const json::Object& in_requestJson, std::shared_ptr<Request>& out_request)
{
   return Success();
}

uint64_t Request::getId() const
{
   return m_baseImpl->Id;
}

const std::string& Request::getRequestUsername() const
{
   return m_baseImpl->RequestUsername;
}

Request::Type Request::getType() const
{
   return m_baseImpl->RequestType;
}

const system::User& Request::getUser() const
{
   return m_baseImpl->User;
}

Request::Request(const json::Object& in_requestJson) :
   m_baseImpl(new Impl())
{
   int messageType;
   Error error = json::readObject(
      in_requestJson,
      FIELD_MESSAGE_TYPE,
      messageType,
      FIELD_REQUEST_ID,
      m_baseImpl->Id);

   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
      return;
   }

   if (messageType >= static_cast<int>(Type::INVALID))
   {
      logging::logError(
         requestError(
            RequestError::INVALID_REQUEST_TYPE,
            std::to_string(messageType), ERROR_LOCATION));
      m_baseImpl->IsValid = false;
      return;
   }

   // Username is not required for all request types.
   std::string username;
   error = json::readObject(in_requestJson, FIELD_USERNAME, username);
   if (error && !json::isMissingMemberError(error))
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
      return;
   }
   else if (!error)
   {
      error = system::User::getUserFromIdentifier(username, m_baseImpl->User);
      if (error)
      {
         logging::logError(error);
         m_baseImpl->IsValid = false;
         return;
      }
   }

   // Request username is not required for all request types.
   error = json::readObject(in_requestJson, FIELD_REQUEST_USERNAME, m_baseImpl->RequestUsername);
   if (error && !json::isMissingMemberError(error))
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
   }
}

Request::Request() :
   m_impl(new Impl())
{
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
