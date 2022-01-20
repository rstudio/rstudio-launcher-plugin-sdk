/*
 * AbstractLauncherCommunicator.cpp
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

#include <comms/AbstractLauncherCommunicator.hpp>

#include <map>
#include <sstream>

#include <boost/system/error_code.hpp>

#include <Error.hpp>
#include <api/Request.hpp>
#include <api/Response.hpp>
#include <logging/Logger.hpp>
#include <json/Json.hpp>
#include <system/Asio.hpp>
#include <utils/MutexUtils.hpp>

#include "MessageHandler.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace comms {

typedef std::shared_ptr<AbstractLauncherCommunicator> SharedThis;
typedef std::weak_ptr<AbstractLauncherCommunicator> WeakThis;

struct AbstractLauncherCommunicator::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_maxMessageSize      The maximum allowable size of a message which can be received from or sent to the
    *                               RStudio Launcher.
    * @param in_onError             Error handler to allow the creator of this communicator to receive communications
    *                               errors.
    */
   Impl(size_t in_maxMessageSize, OnError in_onError) :
      MsgHandler(in_maxMessageSize),
      OnErrorFunc(std::move(in_onError))
   {
   }

   static void defaultRequestHandler(const SharedThis& in_sharedThis, const std::shared_ptr<api::Request>& in_request)
   {
      std::ostringstream msgStream;
      msgStream << "No request handler found for request type " << in_request->getType() << ".";
      logging::logDebugMessage(msgStream.str(), ERROR_LOCATION);

      // Send an error response to the launcher.
      in_sharedThis->sendResponse(api::ErrorResponse(
         in_request->getId(),
         api::ErrorResponse::Type::REQUEST_NOT_SUPPORTED,
         "Request not supported"));
   }

   /** The map of registered request handlers */
   std::unique_ptr<RequestHandler> RequestHandlerPtr;

   /** The message handler object which parses and formats messages. */
   MessageHandler MsgHandler;

   /** The error handler function, provided by the communicator creator. */
   const OnError OnErrorFunc;

   /** Mutex to protect members during threaded operations. */
   std::mutex Mutex;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractLauncherCommunicator)

void AbstractLauncherCommunicator::registerRequestHandler(std::unique_ptr<RequestHandler>&& in_requestHandler)
{
   if (m_baseImpl->RequestHandlerPtr != nullptr)
      logging::logDebugMessage("Overwriting request handler", ERROR_LOCATION);
   m_baseImpl->RequestHandlerPtr = std::move(in_requestHandler);
}

void AbstractLauncherCommunicator::sendResponse(const api::Response& in_response)
{
   std::string jsonStr = in_response.toJson().write();
   std::string message = m_baseImpl->MsgHandler.formatMessage(jsonStr);

   logging::logDebugMessage("Sending message to the Launcher: " + jsonStr);
   writeResponse(message);
}

Error AbstractLauncherCommunicator::start()
{
   // Nothing to explicitly start.
   return Success();
}

void AbstractLauncherCommunicator::stop()
{
   // Nothing to explicitly stop.
}

void AbstractLauncherCommunicator::waitForExit()
{
   // Nothing to wait for, for now.
}

AbstractLauncherCommunicator::AbstractLauncherCommunicator(size_t in_maxMessageSize, const OnError& in_onError) :
   m_baseImpl(new Impl(in_maxMessageSize, in_onError))
{
}

void AbstractLauncherCommunicator::reportError(const Error& in_error)
{
   stop();

   if (m_baseImpl->OnErrorFunc)
      m_baseImpl->OnErrorFunc(in_error);
}

void AbstractLauncherCommunicator::onDataReceived(const char* in_data, size_t in_length)
{
   WeakThis weakThis = shared_from_this();
   std::function<void(const std::string&)> parseMessage =
      [weakThis](const std::string& in_message)
      {
         // If we can't lock the weak pointer, the communicator has been destroyed and there's nothing left to do.
         SharedThis sharedThis = weakThis.lock();
         if (!sharedThis)
            return;

         // Parse the JSON object.
         json::Object jsonRequest;
         Error error = jsonRequest.parse(in_message);
         if (error)
         {
            sharedThis->reportError(
               systemError(boost::system::errc::protocol_error,
                  "Received malformed launcher message: " + in_message,
                  error,
                  ERROR_LOCATION));
            return;
         }

         // Try to construct a request object.
         std::shared_ptr<api::Request> request;
         error = api::Request::fromJson(jsonRequest, request);
         if (error)
         {
            Error specificError;
            if (!request)
               specificError = systemError(boost::system::errc::protocol_error,
                                           "Could not deserialize launcher message: " + in_message,
                                           error,
                                           ERROR_LOCATION);
            else
               specificError = systemError(boost::system::errc::protocol_error,
                                           "Received invalid launcher message: " + in_message,
                                           error,
                                           ERROR_LOCATION);

            sharedThis->reportError(specificError);
            return;
         }

         // Send the object to the request handler, or send an error to the launcher;
         if (sharedThis->m_baseImpl->RequestHandlerPtr == nullptr)
            Impl::defaultRequestHandler(sharedThis, request);
         else
            (*sharedThis->m_baseImpl->RequestHandlerPtr)(request);
      };

   std::vector<std::string> messages;
   Error error;

   LOCK_MUTEX(m_baseImpl->Mutex)
      // Lock here to protect the message handler member, which is not thread-safe.
      error = m_baseImpl->MsgHandler.processBytes(in_data, in_length, messages);
   END_LOCK_MUTEX

   if (error)
   {
      reportError(error);
      return;
   }

   for (const std::string& message: messages)
   {
      system::AsioService::post(std::bind(parseMessage, message));
   }
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
