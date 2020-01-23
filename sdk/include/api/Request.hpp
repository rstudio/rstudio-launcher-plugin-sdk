/*
 * Request.hpp
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

#ifndef LAUNCHER_PLUGINS_REQUEST_HPP
#define LAUNCHER_PLUGINS_REQUEST_HPP

#include <PImpl.hpp>

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
 * @brief Base class for all requests which may be received from the RStudio Launcher.
 */
class Request
{
public:
   /**
    * @enum Request::Type
    * @brief Enum which represents the type of a Request.
    *
    * The last enum value, INVALID, must always be the last value and is used to validate the received request.
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
 * @brief Represents a bootstrap request received from the RStudio Launcher.
 */
class BootstrapRequest: public Request
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

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif