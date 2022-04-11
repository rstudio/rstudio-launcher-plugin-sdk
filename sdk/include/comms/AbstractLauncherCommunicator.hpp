/*
 * AbstractLauncherCommunicator.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_COMMUNICATOR_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_COMMUNICATOR_HPP

#include <Noncopyable.hpp>

#include <utils/Functionals.hpp>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace api {

class Request;
class Response;

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace comms {

/**
 * @brief Callback function which will be invoked a particular request type is received from the RStudio Launcher.
 */
typedef std::function<void(const std::shared_ptr<api::Request>&)> RequestHandler;

/**
 * @brief Base class responsible for communicating the the launcher. The type of communication is implementation
 *        dependent.
 */
class AbstractLauncherCommunicator : public Noncopyable,
                                     public std::enable_shared_from_this<AbstractLauncherCommunicator>
{
public:
   /**
    * @brief Virtual destructor to allow for inheritance.
    */
   virtual ~AbstractLauncherCommunicator() = default;

   /**
    * @brief Registers a request handler for all requests.
    *
    * @param in_requestHandler      The handler for the request.
    */
   void registerRequestHandler(std::unique_ptr<RequestHandler>&& in_requestHandler);

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
    *
    * Child classes which override this method should also invoke the base method.
    */
   virtual void waitForExit();

protected:
   /**
    * @brief Constructor.
    *
    * @param in_maxMessageSize      The maximum allowable size of a message which can be received from or sent to the
    *                               RStudio Launcher.
    * @param in_onError             Error handler to allow the creator of this communicator to receive communications
    *                               errors.
    */
   AbstractLauncherCommunicator(size_t in_maxMessageSize, const OnError& in_onError);

   /**
    * @brief Reports an error and stops the communicator.
    *
    * @param in_error       The error to report.
    */
    void reportError(const Error& in_error);

   /**
    * @brief Handles data that is received from the RStudio Launcher.
    *
    * @param in_data        The data received from the RStudio Launcher.
    * @param in_length      The length of the data received from the RStudio Launcher.
    */
   void onDataReceived(const char* in_data, size_t in_length);

protected:
   /**
    * @breif Template method which allows classes which inherit AbstractLauncherCommunicator to get a shared_ptr to
    *        themselves.
    * @tparam Derived       Type of the class which inherits from AbstractLauncherCommunicator.
    *
    * @return A shared_ptr to this derived class.
    */
   template <typename Derived>
   std::shared_ptr<Derived> shared_from_derived()
   {
      static_assert(
         std::is_base_of<AbstractLauncherCommunicator, Derived>::value,
         "Derived must inherit from AbstractLauncherCommunicator");

      return std::static_pointer_cast<Derived>(shared_from_this());
   }


private:
   /**
    * @brief Writes the formatted response to the RStudio Launcher via implementation specific communication method.
    *
    * This function must be thread safe. Each response must be fully written before the next begins.
    *
    * @param in_responseMessage     The formatted response to send to the RStudio Launcher.
    */
   virtual void writeResponse(const std::string& in_responseMessage) = 0;

   // The private implementation of AbstractLauncherCommunicator.
   PRIVATE_IMPL(m_baseImpl);
};

typedef std::shared_ptr<AbstractLauncherCommunicator> AbstractLauncherCommunicatorPtr;

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio

#endif
