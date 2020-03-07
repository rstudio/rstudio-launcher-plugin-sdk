/*
 * Response.hpp
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


#ifndef LAUNCHER_PLUGINS_RESPONSE_HPP
#define LAUNCHER_PLUGINS_RESPONSE_HPP

#include <cstdint>

#include <vector>

#include <PImpl.hpp>
#include <api/Job.hpp>

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

/**
 * @brief Represents the common components of all responses which can be sent the RStudio Launcher.
 */
class Response
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
 * @brief Class which represents a bootstrap response which can be sent to the Launcher in response to a bootstrap
 *        request.
 */
class BootstrapResponse : public Response
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
 * @brief Class which represents an error response which can be sent to the Launcher in response to any request.
 */
class ErrorResponse : public Response
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
class HeartbeatResponse : public Response
{
public:
   /**
    * @brief Constructor.
    */
   HeartbeatResponse();
};

/**
 * @brief Class which represents a cluster info response which should be sent to the Launcher in response to a cluster
 *        info request.
 */
 class ClusterInfoResponse : public Response
 {
 public:
    /**
     * @brief
     *
     * @param in_requestId                  The ID of the request for which this response is being sent.
     * @param in_queues                     The set of available queues on which to run jobs, if any. Default: none.
     * @param in_resourceLimits             The set of resource limits which may be set on jobs, including default and
     *                                      maximum values. Default: none.
     * @param in_supportsContainers         Whether or not the cluster supports containers. Default: false.
     * @param in_placementConstraints       The set of custom placement constraints which may be set on jobs. Default:
     *                                      none.
     * @param in_config                     The set of custom job configuration settings and their possible values.
     *                                      Default: none.
     */
    explicit ClusterInfoResponse(
       uint64_t in_requestId,
       std::vector<std::string> in_queues = std::vector<std::string>(),
       std::vector<ResourceLimit> in_resourceLimits = std::vector<ResourceLimit>(),
       bool in_supportsContainers = false,
       std::vector<PlacementConstraint> in_placementConstraints = std::vector<PlacementConstraint>(),
       std::vector<JobConfig> in_config = std::vector<JobConfig>());

    /**
     * @brief Converts this cluster info response to a JSON object.
     *
     * @return The JSON object which represents this cluster info response.
     */
    json::Object toJson() const override;

 private:
    PRIVATE_IMPL(m_impl);
 };

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
