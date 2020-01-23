/*
 * AbstractCommunicator.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_COMMUNICATOR_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_COMMUNICATOR_HPP

#include <Noncopyable.hpp>

#include <functional>

#include <api/Request.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace api {

class Response;

}
}
}

namespace rstudio {
namespace launcher_plugins {
namespace comms {

/**
 * @brief Function which handles a request from the RStudio Launcher.
 */
typedef std::function<void(std::shared_ptr<api::Request>)> RequestHandler;

/**
 * @brief Base class responsible for communicating the the launcher. The type of communication is implementation
 *        dependent.
 */
class AbstractCommunicator : public Noncopyable
{
public:
   /**
    * @brief Virtual destructor to allow for inheritance.
    */
   virtual ~AbstractCommunicator() = default;

   /**
    * @brief Registers a request handler for the specified type. Only one handler may be registered per type.
    *
    * @param in_requestType         The type of the request.
    * @param in_requestHandler      The handler for the request.
    */
   void registerRequestHandler(api::Request::Type in_requestType, const RequestHandler& in_requestHandler);

   /**
    * @brief Sends the response to the RStudio Launcher.
    *
    * @param in_response    The response to be sent to the RStudio Launcher.
    */
   void sendResponse(const api::Response& in_response);

   /**
    * @brief Starts the communicator.
    *
    * Child classes which override this method should also invoke the base method.
    *
    * @return Success if the communicator could be started; Error otherwise.
    */
   virtual Error start();

   /**
    * @brief Stops the communicator.
    *
    * Child classes which override this method should also invoke the base method.
    */
   virtual void stop();

   /**
    * @brief Blocks until the communicator has successfully stopped.
    */
   virtual void waitForExit() = 0;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_maxMessageSize      The maximum allowable size of a message which can be received from or sent to the
    *                               RStudio Launcher.
    */
   explicit AbstractCommunicator(size_t in_maxMessageSize);

   /**
    * @brief Handles data that is received from the RStudio Launcher.
    *
    * If this method returns an error, the plugin should shut down.
    *
    * @param in_data        The data received from the RStudio Launcher.
    * @param in_length      The length of the data received from the RStudio Launcher.
    *
    * @return Success if the data was valid; Error otherwise.
    */
   Error onDataReceived(const char* in_data, size_t in_length);

private:
   /**
    * @brief Sends the formatted response to the RStudio Launcher via implementation specific communication method.
    *
    * @param in_responseMessage     The formatted response to send to the RStudio Launcher.
    */
   virtual void sendResponse(const std::string& in_responseMessage) = 0;

   // The private implementation of AbstractCommunicator.
   PRIVATE_IMPL(m_baseImpl);
};

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio

#endif
