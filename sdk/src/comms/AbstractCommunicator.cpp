/*
 * AbstractCommunicator.cpp
 *
 * Copyright (C) 2020 by RStudio, Inc.
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

#include "AbstractCommunicator.hpp"

#include <map>

#include <Error.hpp>
#include <api/Response.hpp>
#include <logging/Logger.hpp>
#include <json/Json.hpp>

#include "MessageHandler.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace comms {

typedef std::map<api::Request::Type, RequestHandler> RequestHandlerMap;

struct AbstractCommunicator::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_maxMessageSize      The maximum allowable size of a message which can be received from or sent to the
    *                               RStudio Launcher.
    * @param in_onError             Error handler to allow the creator of this communicator to receive communications
    *                               errors.
    */
   explicit Impl(size_t in_maxMessageSize, const ErrorHandler& in_onError) :
      MsgHandler(in_maxMessageSize),
      OnError(in_onError)
   {
   }

   /** The map of registered request handlers */
   RequestHandlerMap RequestHandlers;

   /** The message handler object which parses and formats messages. */
   MessageHandler MsgHandler;

   /** The error handler function, provided by the communicator creator. */
   const ErrorHandler OnError;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractCommunicator)

void AbstractCommunicator::registerRequestHandler(
   api::Request::Type in_requestType,
   const RequestHandler& in_requestHandler)
{
   if (m_baseImpl->RequestHandlers.find(in_requestType) != m_baseImpl->RequestHandlers.end())
   {
      std::string message = "Overwriting existing request handler for request type " +
         std::to_string(static_cast<int>(in_requestType)) + ".";
      logging::logDebugMessage(message, ERROR_LOCATION);
   }

   m_baseImpl->RequestHandlers[in_requestType] = in_requestHandler;
}

void AbstractCommunicator::sendResponse(const api::Response& in_response)
{
   std::string message = m_baseImpl->MsgHandler.formatMessage(in_response.asJson().write());
   writeResponse(message);
}

Error AbstractCommunicator::start()
{
   // Eventually this will start the heartbeat timer.
   return Success();
}

void AbstractCommunicator::stop()
{
   // Eventually this will stop the heartbeat timer.
}

AbstractCommunicator::AbstractCommunicator(size_t in_maxMessageSize, const ErrorHandler& in_onError) :
   m_baseImpl(new Impl(in_maxMessageSize, in_onError))
{
}

void AbstractCommunicator::reportError(const Error& in_error)
{
   stop();

   if (m_baseImpl->OnError)
      m_baseImpl->OnError(in_error);
}

Error AbstractCommunicator::onDataReceived(const char* in_data, size_t in_length)
{
   std::vector<std::string> messages;
   Error error = m_baseImpl->MsgHandler.parseMessages(in_data, in_length, messages);
   if (error)
      return error;

   for (const std::string& message: messages)
   {
      // Parse the JSON object.
      json::Object jsonRequest;
      error = jsonRequest.parse(message);
      if (error)
         return error;

      // Try to construct a request object.
      std::shared_ptr<api::Request> request;
      error = api::Request::fromJson(jsonRequest, request);
      if (error)
         return error;

      // Send the object to the appropriate handle.
      auto itr = m_baseImpl->RequestHandlers.find(request->getType());
      if (itr == m_baseImpl->RequestHandlers.end())
      {
         std::string logMessage = "No request handler found for request type " +
            std::to_string(static_cast<int>(request->getType())) + ".";
         logging::logDebugMessage(logMessage, ERROR_LOCATION);
         // TODO: return error?
      }
      else
      {
         itr->second(request);
      }
   }

   return Success();
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
