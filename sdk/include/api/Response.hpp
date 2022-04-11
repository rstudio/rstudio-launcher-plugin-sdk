/*
 * Response.hpp
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


#ifndef LAUNCHER_PLUGINS_RESPONSE_HPP
#define LAUNCHER_PLUGINS_RESPONSE_HPP

#include <Noncopyable.hpp>

#include <vector>

#include <PImpl.hpp>
#include <api/Job.hpp>
#include <api/ResponseTypes.hpp>
#include <api/stream/AbstractOutputStream.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace json {

class Object;

} // namespace json
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

struct JobSourceConfiguration;

/**
 * @brief Represents the common components of all responses which can be sent the RStudio Launcher.
 */
class Response : public Noncopyable
{
public:
   /**
    * @brief Virtual destructor to allow for inheritance.
    */
   virtual ~Response() = default;

   /**
    * @brief Converts this response to a JSON object.
    *
    * @return The JSON object which represents this response.
    */
   virtual json::Object toJson() const;

protected:
   /**
    * @enum Response::Type
    * @brief Enum which represents the type of a Response.
    *
    * Types are defined as described in the RStudio Launcher API Documentation. See
    * https://docs.rstudio.com/job-launcher/latest/creating-plugins.html#plugin-messages for more details.
    */
   enum class Type
   {
      /** Error response */
      ERROR                = -1,

      /** Heartbeat response */
      HEARTBEAT            = 0,

      /** Bootstrap response */
      BOOTSTRAP            = 1,

      /** Job State response */
      JOB_STATE            = 2,

      /** Job Status response */
      JOB_STATUS           = 3,

      /** Control Job response */
      CONTROL_JOB          = 4,

      /** Control Job output response */
      JOB_OUTPUT           = 5,

      /** Job Resource utilization response */
      JOB_RESOURCE_UTIL    = 6,

      /** Job Network information response */
      JOB_NETWORK          = 7,

      /** Cluster Info response */
      CLUSTER_INFO         = 8,
   };

   /**
    * @brief Constructor.
    *
    * @param in_responseType    The type of response to be constructed.
    * @param in_requestId       The ID of the request for which this response is being sent.
    */
   Response(Type in_responseType, uint64_t in_requestId);

private:
   // The private implementation of Response.
   PRIVATE_IMPL(m_responseImpl);
};

/**
 * @brief Base class for responses which are returned to multiple stream requests.
 */
class MultiStreamResponse : public Response
{
public:
   /**
    * @brief Converts this MultiStreamResponse to a JSON object.
    *
    * @return The JSON object which represents this MultiStreamResponse.
    */
   json::Object toJson() const override;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_responseType    The type of the base class.
    * @param in_sequences       The sequence IDs for which this response should be sent.
    */
   MultiStreamResponse(Type in_responseType, StreamSequences in_sequences);

private:
   // The private implementation of MultiStreamResponse
   PRIVATE_IMPL(m_streamResponseImpl);
};

/**
 * @brief Class which represents an error response which can be sent to the Launcher in response to any request.
 */
class ErrorResponse final : public Response
{
public:
   /**
    * @enum ErrorResponse::Type
    * @brief Represents the type of error to send to the RStudio Launcher.
    *
    * Types are defined as described in the RStudio Launcher API Documentation. See
    * https://docs.rstudio.com/job-launcher/latest/creating-plugins.html#plugin-messages for more details.
    */
   enum class Type
   {
      INVALID_RESPONSE        = -1,
      UNKNOWN                 = 0,
      REQUEST_NOT_SUPPORTED   = 1,
      INVALID_REQUEST         = 2,
      JOB_NOT_FOUND           = 3,
      PLUGIN_RESTARTED        = 4,
      TIMEOUT                 = 5,
      JOB_NOT_RUNNING         = 6,
      JOB_OUTPUT_NOT_FOUND    = 7,
      INVALID_JOB_STATE       = 8,
      JOB_CONTROL_FAILURE     = 9,
      UNSUPPORTED_VERSION     = 10,
   };

   /**
    * @brief Constructor.
    *
    * @param in_requestId       The ID of the request for which this error is being returned.
    * @param in_errorType       The type of error which occurred while processing the request.
    * @param in_errorMessage    A message describing the details of the error.
    */
   ErrorResponse(uint64_t in_requestId, Type in_errorType, std::string in_errorMessage);

   /**
    * @brief Converts this error response to a JSON object.
    *
    * @return The JSON object which represents this error response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of ErrorResponse.
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents a heartbeat response which should be sent to the Launcher every configured
 * heartbeat-interval-seconds.
 */
class HeartbeatResponse final : public Response
{
public:
   /**
    * @brief Constructor.
    */
   HeartbeatResponse();
};

/**
 * @brief Class which represents a bootstrap response which can be sent to the Launcher in response to a bootstrap
 *        request.
 */
class BootstrapResponse final : public Response
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_requestId       The ID of the bootstrap request for which this bootstrap response is being sent.
    */
   explicit BootstrapResponse(uint64_t in_requestId);

   /**
    * @brief Converts this bootstrap response to a JSON object.
    *
    * @return The JSON object which represents this bootstrap response.
    */
   json::Object toJson() const override;
};

/**
 * @brief Class which represents a job state response which can be sent to the Launcher in response to a get or submit
 *        job request.
 */
class JobStateResponse final : public Response
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_requestId   The ID of the request for which this job state response is being sent.
    * @param in_jobs        The jobs to be returned to the Launcher.
    * @param in_jobFields   The optional set of job fields to include for each job. If this is not set, all fields will
    *                       be returned.
    */
   JobStateResponse(
      uint64_t in_requestId,
      JobList in_jobs,
      Optional<std::set<std::string> > in_jobFields = Optional<std::set<std::string> >());

   /**
    * @brief Converts this job state response to a JSON object.
    *
    * @return The JSON object which represents this job state response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of ClusterInfoResponse
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents a Job Status Stream, either for all jobs or for a specific job.
 */
class JobStatusResponse final : public MultiStreamResponse
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_sequences   The stream sequences for which this response will be sent.
    * @param in_job         The job that was updated.
    */
   JobStatusResponse(StreamSequences in_sequences, const JobPtr& in_job);

   /**
    * @brief Converts this job status response to a JSON object.
    *
    * @return The JSON object which represents this job status response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of JobStatusResponse.
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents the result of a control job operation.
 */
class ControlJobResponse final : public Response
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_requestId           The ID of the request for which this response is being sent.
    * @param in_statusMessage       A message describing the status of the control job operation that was requested.
    * @param in_isComplete          Whether the request control job operation has completed (true) or not (false).
    */
   ControlJobResponse(uint64_t in_requestId, std::string in_statusMessage, bool in_isComplete);


   /**
    * @brief Converts this control job response to a JSON object.
    *
    * @return The JSON object which represents this control job response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of ControlJobResponse
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents a Job Output Stream response for a specific job.
 */
class OutputStreamResponse final : public Response
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_requestId       The ID of the request for which this response is being sent.
    * @param in_sequenceId      The ID of this output in the sequence of responses for this request.
    * @param in_output          The output to send to the Launcher.
    * @param in_outputType      The type of output being sent.
    */
   OutputStreamResponse(
      uint64_t in_requestId,
      uint64_t in_sequenceId,
      std::string in_output,
      OutputType in_outputType);

   /**
    * @brief Constructor. Represents the last (complete notification) response of the output stream.
    *
    * @param in_requestId       The ID of the request for which this response is being sent.
    * @param in_sequenceId      The ID of this output in the sequence of responses for this request.
    */
   OutputStreamResponse(
      uint64_t in_requestId,
      uint64_t in_sequenceId);

   /**
    * @brief Converts this output stream response to a JSON object.
    *
    * @return The JSON object which represents this output stream response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of OutputStreamResponse
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents a Resource Utilization Stream response for a specific job.
 */
class ResourceUtilStreamResponse : public MultiStreamResponse
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_sequences        The stream sequences for which this response will be sent.
    * @param in_resourceData     The current resource utilization of the job for which the sequenced requests were made.
    * @param in_isComplete       Whether the stream is complete (true) or not (false).
    */
   ResourceUtilStreamResponse(
      StreamSequences in_sequences,
      const ResourceUtilData& in_resourceData,
      bool in_isComplete);

   /**
    * @brief Converts this resource utilization stream response to a JSON object.
    *
    * @return The JSON object which represents this resource utilization stream response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of ResourceUtilStreamResponse
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents a network information response which should be sent to the Launcher in response to a
 *        Job network information request.
 */
class NetworkResponse final : public Response
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_requestId       The ID of the request for which this response is being sent.
    * @param in_networkInfo     The network information for the requested Job.
    */
   NetworkResponse(uint64_t in_requestId, NetworkInfo in_networkInfo);
   /**
    * @brief Converts this cluster info response to a JSON object.
    *
    * @return The JSON object which represents this network info response.
    */
   json::Object toJson() const override;

private:
   // The private implementation of NetworkResponse
   PRIVATE_IMPL(m_impl);
};

/**
 * @brief Class which represents a cluster info response which should be sent to the Launcher in response to a cluster
 *        info request.
 */
 class ClusterInfoResponse final : public Response
 {
 public:
    /**
     * @brief Constructor.
     *
     * @param in_requestId          The ID of the request for which this response is being sent.
     * @param in_configuration      The configuration and capabilities of the cluster.
     */
    ClusterInfoResponse(uint64_t in_requestId, const JobSourceConfiguration& in_configuration);

    /**
     * @brief Converts this cluster info response to a JSON object.
     *
     * @return The JSON object which represents this cluster info response.
     */
    json::Object toJson() const override;

 private:
    // The private implementation of ClusterInfoResponse
    PRIVATE_IMPL(m_impl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
