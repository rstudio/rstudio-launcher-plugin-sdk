/*
 * Request.cpp
 *
 * Copyright (C) 2019-20 by RStudio, PBC
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
#include <logging/Logger.hpp>
#include <system/User.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>

#include "Constants.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace api {

namespace {

enum class RequestError
{
   SUCCESS = 0,
   INVALID_REQUEST_TYPE = 1,
   INVALID_REQUEST = 2,
   INVALID_USER = 3,
   INVALID_INPUT = 4,
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
      case RequestError::INVALID_REQUEST:
      {
         message.append("Invalid request received from launcher");
         break;
      }
      case RequestError::INVALID_USER:
      {
         message.append("Details of request user could not be found");
         break;
      }
      case RequestError::INVALID_INPUT:
      {
         message.append("Invalid input received");
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

} // anonymous namespace

// Request =============================================================================================================
struct Request::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_requestType     The type of this request.
    */
   explicit Impl(Type in_requestType) :
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
      case Type::HEARTBEAT:
      {
         out_request.reset(new Request(Request::Type::HEARTBEAT, in_requestJson));
         break;
      }
      case Type::BOOTSTRAP:
      {
         out_request.reset(new BootstrapRequest(in_requestJson));
         break;
      }
      case Type::GET_CLUSTER_INFO:
      {
         std::shared_ptr<UserRequest> userRequest(new UserRequest(Type::GET_CLUSTER_INFO, in_requestJson));
         if (userRequest->getUser().isEmpty())
            return requestError(RequestError::INVALID_USER, in_requestJson.writeFormatted(), ERROR_LOCATION);

         out_request = userRequest;
         break;
      }
      case Type::GET_JOB:
      {
         std::shared_ptr<JobStateRequest> jobStateRequest(new JobStateRequest(in_requestJson));

         out_request = jobStateRequest;
         break;
      }
      default:
      {
         std::ostringstream osstream;
         osstream << messageType;
         return requestError(
            RequestError::INVALID_REQUEST_TYPE,
            osstream.str(),
            ERROR_LOCATION);
      }
   }

   if (!out_request->m_baseImpl->IsValid)
      return requestError(RequestError::INVALID_REQUEST,
         in_requestJson.writeFormatted(),
         ERROR_LOCATION);

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

// User ================================================================================================================
struct UserRequest::Impl
{
   /** The real user for whom the request should be performed. */
   system::User RealUser;

   /** The actual user who submitted the request. */
   std::string RequestUsername;
};

PRIVATE_IMPL_DELETER_IMPL(UserRequest);

const system::User & UserRequest::getUser() const
{
   return m_userImpl->RealUser;
}

const std::string& UserRequest::getRequestUsername() const
{
   return m_userImpl->RequestUsername;
}

UserRequest::UserRequest(Request::Type in_type, const json::Object& in_requestJson) :
   Request(in_type, in_requestJson),
   m_userImpl(new Impl())
{
   std::string realUsername;
   Optional<std::string> requestUsername;
   Error error = json::readObject(in_requestJson,
                                  FIELD_REAL_USER, realUsername,
                                  FIELD_REQUEST_USERNAME, requestUsername);

   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
      return;
   }

   boost::trim(realUsername);
   if (realUsername != "*")
   {
      error = system::User::getUserFromIdentifier(realUsername, m_userImpl->RealUser);

      if (error)
      {
         logging::logError(error);
         m_userImpl->RealUser = system::User(true);
         m_baseImpl->IsValid = false;
         return;
      }
   }

   m_userImpl->RequestUsername = requestUsername.getValueOr("");
}

// JobId ===============================================================================================================
struct JobIdRequest::Impl
{
   std::string JobId;
   std::string EncodedJobId;
};

PRIVATE_IMPL_DELETER_IMPL(JobIdRequest)

const std::string& JobIdRequest::getJobId() const
{
   return m_jobIdImpl->JobId;
}

const std::string& JobIdRequest::getEncodedJobId() const
{
   return m_jobIdImpl->EncodedJobId;
}

JobIdRequest::JobIdRequest(Request::Type in_type, const json::Object& in_requestJson) :
   UserRequest(in_type, in_requestJson),
   m_jobIdImpl(new Impl())
{
   Optional<std::string> encodedId;
   Error error = json::readObject(in_requestJson,
      FIELD_JOB_ID, m_jobIdImpl->JobId,
      FIELD_ENCODED_JOB_ID, encodedId);

   m_jobIdImpl->EncodedJobId = encodedId.getValueOr("");

   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
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

PRIVATE_IMPL_DELETER_IMPL(BootstrapRequest)

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

// Job State ===========================================================================================================
struct JobStateRequest::Impl
{
   /** The end of the range of submission times by which to filter the jobs. */
   Optional<std::string> EndTime;

   /** The set of fields to be returned for each job. */
   Optional<std::set<std::string> > FieldSet;

   /** The start of the range of submission times by which to filter the jobs. */
   Optional<std::string> StartTime;

   /** The set of statuses by which to filter the returned jobs. */
   Optional<std::set<std::string> > StatusSet;

   /** The set of tags tby which to filter the returned jobs. */
   Optional<std::set<std::string> > TagSet;
};

PRIVATE_IMPL_DELETER_IMPL(JobStateRequest)

Error JobStateRequest::getEndTime(Optional<system::DateTime>& out_endTime) const
{
   if (m_impl->EndTime)
   {
      system::DateTime endTime;
      Error error = system::DateTime::fromString(m_impl->EndTime.getValueOr(""), endTime);
      if (error)
         return error;

      out_endTime = endTime;
   }

   return Success();
}

const Optional<std::set<std::string> >& JobStateRequest::getFieldSet() const
{
   return m_impl->FieldSet;
}

Error JobStateRequest::getStartTime(Optional<system::DateTime>& out_startTime) const
{
   if (m_impl->StartTime)
   {
      system::DateTime startTime;
      Error error = system::DateTime::fromString(m_impl->StartTime.getValueOr(""), startTime);
      if (error)
         return error;

      out_startTime = startTime;
   }

   return Success();
}

Error JobStateRequest::getStatusSet(Optional<std::set<Job::State> >& out_statuses) const
{
   if (m_impl->StatusSet)
   {
      std::set<Job::State> statuses;
      std::set<std::string> invalidStatuses;
      for (const std::string& status: m_impl->StatusSet.getValueOr({}))
      {
         Job::State state;
         Error error = Job::stateFromString(status, state);
         if (error)
            invalidStatuses.insert(status);
         else
            statuses.insert(state);
      }

      if (!invalidStatuses.empty())
         return requestError(
            RequestError::INVALID_INPUT,
            boost::algorithm::join(invalidStatuses, ","),
            ERROR_LOCATION);

      out_statuses = statuses;
   }

   return Success();
}

const Optional<std::set<std::string> >& JobStateRequest::getTagSet() const
{
   return m_impl->TagSet;
}

JobStateRequest::JobStateRequest(const json::Object& in_requestJson) :
   JobIdRequest(Type::GET_JOB, in_requestJson),
   m_impl(new Impl())
{
   Error error = json::readObject(in_requestJson,
      FIELD_JOB_END_TIME, m_impl->EndTime,
      FIELD_JOB_FIELDS, m_impl->FieldSet,
      FIELD_JOB_START_TIME, m_impl->StartTime,
      FIELD_JOB_STATUSES, m_impl->StatusSet,
      FIELD_JOB_TAGS, m_impl->TagSet);

   // Test
   if (error)
   {
      logging::logError(error);
      m_baseImpl->IsValid = false;
      return;
   }

   // ID is required, ensure it is in the set of fields.
   std::set<std::string> tmp;
   m_impl->FieldSet.getValueOr(tmp).insert("id");
}

// Helpers =============================================================================================================
std::ostream& operator<<(std::ostream& in_ostream, Request::Type in_type)
{
   switch(in_type)
   {
      case Request::Type::HEARTBEAT:
      {
         in_ostream << "Heartbeat";
         break;
      }
      case Request::Type::BOOTSTRAP:
      {
         in_ostream << "Bootstrap";
         break;
      }
      case Request::Type::SUBMIT_JOB:
      {
         in_ostream << "SubmitJob";
         break;
      }
      case Request::Type::GET_JOB:
      {
         in_ostream << "GetJob";
         break;
      }
      case Request::Type::GET_JOB_STATUS:
      {
         in_ostream << "GetJobStatus";
         break;
      }
      case Request::Type::CONTROL_JOB:
      {
         in_ostream << "ControlJob";
         break;
      }
      case Request::Type::GET_JOB_OUTPUT:
      {
         in_ostream << "GetJobOutput";
         break;
      }
      case Request::Type::GET_JOB_RESOURCE_UTIL:
      {
         in_ostream << "GetJobResourceUtil";
         break;
      }
      case Request::Type::GET_JOB_NETWORK:
      {
         in_ostream << "GetJobNetwork";
         break;
      }
      case Request::Type::GET_CLUSTER_INFO:
      {
         in_ostream << "GetClusterInfo";
         break;
      }
      default:
         in_ostream << "Invalid";
   }

   return in_ostream;
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
