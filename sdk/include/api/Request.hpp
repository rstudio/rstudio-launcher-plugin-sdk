/*
 * Request.hpp
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

#ifndef LAUNCHER_PLUGINS_REQUEST_HPP
#define LAUNCHER_PLUGINS_REQUEST_HPP

#include <Noncopyable.hpp>

#include <set>
#include <vector>

#include <PImpl.hpp>
#include <Optional.hpp>
#include <api/Job.hpp>
#include <api/stream/AbstractOutputStream.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace json {

class Object;

} // namespace json

namespace system {

class User;

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Base class for all requests which may be received from the Launcher.
 */
class Request : public Noncopyable
{
public:
   /**
    * @enum Request::Type
    * @brief Enum which represents the type of a Request.
    *
    * The last enum value, INVALID, must always be the last value and is used to validate the received request.
    *
    * Types are defined as described in the RStudio Launcher API Documentation. See
    * https://docs.rstudio.com/job-launcher/latest/creating-plugins.html#plugin-messages for more details.
    */
   enum class Type
   {
      /** Heartbeat request */
      HEARTBEAT               = 0,

      /** Bootstrap request */
      BOOTSTRAP               = 1,

      /** Submit Job request */
      SUBMIT_JOB              = 2,

      /** Get Job request */
      GET_JOB                 = 3,

      /** Get Job Status request */
      GET_JOB_STATUS          = 4,

      /** Control Job request */
      CONTROL_JOB             = 5,

      /** Get Job Output request */
      GET_JOB_OUTPUT          = 6,

      /** Get Job Resource Utilization request */
      GET_JOB_RESOURCE_UTIL   = 7,

      /** Get Job Network information request */
      GET_JOB_NETWORK         = 8,

      /** Get Cluster Info request */
      GET_CLUSTER_INFO        = 9,

      /** Invalid request. Should not be received. Always the last element of this enum for comparison purposes. */
      INVALID
   };

   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~Request() = default;

   /**
    * @brief Converts a Json::Object into the appropriate Request object.
    *
    * @param in_requestJson     The json object which represents a request from the Launcher.
    * @param out_request        The converted request object.
    *
    * @return Success if the provided json Object was valid; Error otherwise.
    */
   static Error fromJson(const json::Object& in_requestJson, std::shared_ptr<Request>& out_request);

   /**
    * @brief Gets the ID of this request.
    *
    * @return The ID of this request.
    */
   uint64_t getId() const;

   /**
    * @brief Gets the request type.
    *
    * @return The type of the request.
    */
   Type getType() const;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_requestType     The type of the request.
    * @param in_requestJson     The JSON object representing the request.
    */
   explicit Request(Type in_requestType, const json::Object& in_requestJson);

   // The private implementation of Request
   PRIVATE_IMPL(m_baseImpl);
};

/**
 * @brief Base class which should be used by the class of requests which require a username.
 */
class UserRequest : public Request
{
public:
   /**
    * @brief Gets the user who initiated this request.
    *
    * If an admin user made this request, this object may represent all users (check by calling User::isAllUsers()). In
    * that case, information for all users should be returned.
    *
    * @return The user who initiated this request.
    */
   const system::User& getUser() const;

   /**
    * @brief Gets the actual username that was used when the request was submitted.
    *
    * This value is only useful for auditing purposes and should not be required by most plugins.
    *
    * @return The actual username that was used when the request was submitted.
    */
   const std::string& getRequestUsername() const;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_type            The type of the user request.
    * @param in_requestJson     The JSON Object which represents the user request.
    */
   explicit UserRequest(Request::Type in_type, const json::Object& in_requestJson);

private:
   // The private implementation of UserRequest.
   PRIVATE_IMPL(m_userImpl);

   friend class Request;
};

/**
 * @brief Base class which should be used for requests that require a Job ID.
 */
class JobIdRequest : public UserRequest
{
public:
   /**
    * @brief Gets the ID of the job for which this request was made.
    *
    * @return The ID of the job for which this request was made.
    */
   const std::string& getJobId() const;
   /**
    * @brief Gets the ID of the job for which this request was made.
    *
    * @return The ID of the job for which this request was made.
    */
   const std::string& getEncodedJobId() const;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_type            The type of the user request.
    * @param in_requestJson     The JSON Object which represents the job ID request.
    */
   JobIdRequest(Request::Type in_type, const json::Object& in_requestJson);

private:
   // The private implementation of JobIdRequest.
   PRIVATE_IMPL(m_jobIdImpl);
};

/**
 * @brief Represents a bootstrap request received from the Launcher.
 */
class BootstrapRequest final : public Request
{
public:
   /**
    * @brief Gets the major version of the RStudio Launcher that sent this bootstrap request.
    *
    * @return The major version of the RStudio Launcher.
    */
   int getMajorVersion() const;

   /**
    * @brief Gets the minor version of the RStudio Launcher that sent this bootstrap request.
    *
    * @return The minor version of the RStudio Launcher.
    */
   int getMinorVersion() const;

   /**
    * @brief Gets the patch number of the RStudio Launcher that sent this bootstrap request.
    *
    * @return The patch number of the RStudio Launcher.
    */
   int getPatchNumber() const;

private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the bootstrap request.
    */
   explicit BootstrapRequest(const json::Object& in_requestJson);

   // The private implementation of BootstrapRequest.
   PRIVATE_IMPL(m_impl);

   friend class Request;
};

/**
 * @brief Represents a submit job request from the Launcher.
 */
class SubmitJobRequest final : public UserRequest
{
public:
   /**
    * @brief Gets the job that should be submitted to the Job Scheduling System.
    *
    * @return The job that should be submitted to the Job Scheduling System.
    */
   JobPtr getJob();

private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the submit job request.
    */
   explicit SubmitJobRequest(const json::Object& in_requestJson);

   // The private implementation of SubmitJobRequest
   PRIVATE_IMPL(m_impl);

   friend class Request;
};

/**
 * @brief Represents a job state request received from the Launcher.
 */
class JobStateRequest final : public JobIdRequest
{
public:
   /**
    * @brief Gets the end of the date range for this request.
    *
    * If this value is set, only jobs which were submitted before this DateTime should be returned in the response.
    *
    * @param out_endTime    The end time, if it was set and the string value could be parsed as a DateTime correctly.
    *
    * @return Success if the value was set and could be parsed correctly, or if the value was not set; Error otherwise.
    */
   Error getEndTime(Optional<system::DateTime>& out_endTime) const;

   /**
    * @brief Gets the set of Job fields which should be included in the response.
    *
    * If this value is set, only the fields which are included in this set should be returned in the response. ID will
    * always be returned, as it is required.
    *
    * @return The optional set of Job fields to include in the response.
    */
   const Optional<std::set<std::string> >& getFieldSet() const;

   /**
    * @brief Gets the start of the date range for this request.
    *
    * If this value is set, only jobs which were submitted after this DateTime should be returned in the response.
    *
    * @param out_startTime      The start time, if it was set and the string value could be parsed as a DateTime
    *                           correctly.
    *
    * @return Success if the value was set and could be parsed correctly, or if the value was not set; Error otherwise.
    */
   Error getStartTime(Optional<system::DateTime>& out_endTime) const;

   /**
    * @brief Gets the set of Job statuses by which to filter the returned list of jobs.
    *
    * If this value is set, only the jobs which have one of the specified states should be returned in the response.
    *
    * @param out_statuses       The set of statuses to filter by, if any were set and they could all be parsed as
    *                           Job::State values correctly.
    *
    * @return Success if the value was set and could be parsed correctly, or if the value was not set; Error otherwise.
    */
   Error getStatusSet(Optional<std::set<Job::State> >& out_statuses) const;

   /**
    * @brief Gets the set of Job tags by which to filter the returned list of jobs.
    *
    * If this value is set, only the jobs which have one of the specified states should be returned in the response.
    *
    * @return The optional set of Job statuses by which to filter the returned list of jobs.
    */
   const Optional<std::set<std::string> >& getTagSet() const;

private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the job state request.
    */
   explicit JobStateRequest(const json::Object& in_requestJson);

   // The private implementation of JobStateRequest
   PRIVATE_IMPL(m_impl);

   friend class Request;
};

/**
 * @brief Request from the launcher to begin or end a Job Status Stream.
 */
class JobStatusRequest final : public JobIdRequest
{
public:
   /**
    * @brief Gets whether the Job Status Stream should be started (false) or ended (true).
    *
    * @return True if the stream should be canceled; false if it should be started.
    */
   bool isCancelRequest() const;

private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the job status request.
    */
   explicit JobStatusRequest(const json::Object& in_requestJson);

   // The private implementation of JobStatusRequest
   PRIVATE_IMPL(m_impl);

   friend class Request;
};

/**
 * @brief Request from the launcher to control the state of a Job.
 */
class ControlJobRequest final : public JobIdRequest
{
public:
   /**
    * @enum ControlJobRequest::Operation
    * @brief Enum which represents the operations that can be performed on Jobs.
    */
   enum class Operation
   {
      /** Indicates that the job should be suspended. This operation should be equivalent to sending SIGSTOP. */
      SUSPEND = 0,

      /** Indicates that the job should be resumed. This operation should be equivalent to sending SIGCONT. */
      RESUME = 1,

      /** Indicates that the job should be stopped. This operation should be equivalent to sending SIGTERM. */
      STOP = 2,

      /** Indicates that the job should be killed. This operation should be equivalent to sending SIGKILL. */
      KILL = 3,

      /** Indicates that the job should be canceled through the job scheduling system, if possible. */
      CANCEL = 4
   };

   /**
    * @brief Gets the control job action which should be taken.
    *
    * @return The control job action which should be taken.
    */
   Operation getOperation() const;

private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the control job request.
    */
   explicit ControlJobRequest(const json::Object& in_requestJson);

   // The private implementation of ControlJobRequest
   PRIVATE_IMPL(m_impl);

   friend class Request;
};

/**
 * @brief Request from the launcher to begin or end a Job Output Stream.
 */
class OutputStreamRequest final : public JobIdRequest
{
public:

   /**
    * @brief Gets the type of Output that should be streamed.
    *
    * @return The type of Output that should be streamed.
    */
   OutputType getStreamType() const;

   /**
    * @brief Gets whether the Job Output Stream should be started (false) or ended (true).
    *
    * @return True if the stream should be canceled; false if it should be started.
    */
   bool isCancelRequest() const;

private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the output stream request.
    */
   explicit OutputStreamRequest(const json::Object& in_requestJson);

   // The private implementation of OutputStreamRequest
   PRIVATE_IMPL(m_impl);

   friend class Request;

};

/**
 * @brief Request from the Launcher to get the network information for a job.
 */
class NetworkRequest final : public JobIdRequest
{
private:
   /**
    * @brief Constructor.
    *
    * @param in_requestJson     The JSON Object which represents the network request.
    */
   explicit NetworkRequest(const json::Object& in_requestJson);

   friend class Request;
};

/**
 * @brief Converts a Request::Type to string and adds it to the specified stream.
 *
 * @param in_ostream    The stream to which to add the string version of the type.
 * @param in_type       The type to convert to string.
 *
 * @return The provided stream.
 */
std::ostream& operator<<(std::ostream& in_ostream, Request::Type in_type);

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
