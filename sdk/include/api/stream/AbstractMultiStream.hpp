/*
 * AbstractMultiStream.hpp
 *
 * Copyright (C) 2020 by RStudio, PBC
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_MULTI_STREAM_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_MULTI_STREAM_HPP

#include <Noncopyable.hpp>

#include <string>
#include <set>
#include <mutex>

#include <Error.hpp>
#include <PImpl.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

class User;

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Manages the sending of streamed responses.
 *
 * @tparam R        The Response type which should be sent.
 * @param Args      The additional constructor parameters of R, besides the request and sequence IDs.
 */
template <typename R, typename ... Args>
class AbstractMultiStream : public Noncopyable
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_launcherCommunicator    The launcher communicator which will send the responses.
    */
   explicit AbstractMultiStream(comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Adds a request to the stream.
    * 
    * @param in_requestId     The ID of the request.
    * @param in_requestUser   The user who made the request.
    */
   virtual void addRequest(uint64_t in_requestId, const system::User& in_requestUser) = 0;

   /**
    * @brief Initializes the response stream.
    *
    * @return Success if the response stream could be initialized; Error otherwise.
    */
   virtual Error initialize() = 0;

   /**
    * @brief Checks whether there are any requests listening to this stream.
    *
    * @return True if this stream has any requests; false otherwise.
    */
   bool isEmpty() const;

   /**
    * @brief Removes a request from the multi-stream response.
    *
    * @param in_requestId       The request ID which has stopped listening to this response stream.
    */
   virtual void removeRequest(uint64_t in_requestId);

protected:
   /**
    * @brief Adds a new request ID to the multi-stream response.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_requestId       The request ID which is listening to this response stream.
    */
   void onAddRequest(uint64_t in_requestId);

   /**
    * @brief Removes a request from the multi-stream response.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_requestId       The request ID which has stopped listening to this response stream.
    */
   void onRemoveRequest(uint64_t in_requestId);

   /**
    * @brief Sends a response to the Launcher for all requests.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_responseArgs    The details of the response, if any.
    */
   void sendResponse(Args... in_responseArgs);

   /**
    * @brief Sends a response to the Launcher for the specified requests.
    *
    * NOTE: The mutex must be held when this is called.
    *
    * @param in_requestIds      Sends the response only for the specified request IDs.
    * @param in_responseArgs    The details of the response, if any.
    */
   void sendResponse(const std::set<uint64_t>& in_requestIds, Args... in_responseArgs);

   /** Mutex to protect shared state of the stream. */
   mutable std::mutex m_mutex;

private:
   PRIVATE_IMPL(m_baseImpl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
