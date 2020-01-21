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
   /**
    * @enum Type
    * @brief Enum which represents the type of request.
    */
   enum class Type : unsigned int
   {
      HEARTBEAT               = 0,
      BOOTSTRAP               = 1,
      SUBMIT_JOB              = 2,
      GET_JOB                 = 3,
      GET_JOB_STATUS          = 4,
      CONTROL_JOB             = 5,
      GET_JOB_OUTPUT          = 6,
      GET_JOB_RESOURCE_UTIL   = 7,
      GET_JOB_NETWORK         = 8,
      GET_CLUSTER_INFO        = 9,
   };

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
    * @brief Gets the actual username used when the request was submitted.
    *
    * This value should be ignored by most plugins, and only used as additional information when needed.
    *
    * @return The actual username used when the request was submitted.
    */
   const std::string& getRequestUsername() const;

   /**
    * @brief Gets the request type.
    *
    * @return The type of the request.
    */
   Type getType() const;

   /**
    * @brief Gets the user who made this request.
    *
    * @return The user who made this request.
    */
   const system::User& getUser() const;

protected:
   /**
    * @brief Constructor.
    */
   Request();

private:
   // The private implementation of Request
   PRIVATE_IMPL(m_impl);

};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
