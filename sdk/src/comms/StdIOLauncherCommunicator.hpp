/*
 * StdIOLauncherCommunicator.hpp
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

#ifndef LAUNCHER_PLUGINS_STD_IO_COMMUNICATOR_HPP
#define LAUNCHER_PLUGINS_STD_IO_COMMUNICATOR_HPP

#include "AbstractLauncherCommunicator.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace comms {

/**
 * @brief Communicator class which sends and receives data over standard input/output.
 */
class StdIOLauncherCommunicator : public AbstractLauncherCommunicator
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_maxMessageSize      The maximum allowable size of a message which can be sent to or received from the
    *                               RStudio Launcher.
    * @param in_onError             Function which can be used to handle communicator errors.
    */
   StdIOLauncherCommunicator(
      size_t in_maxMessageSize,
      const OnError& in_onError);

   /**
    * @brief Starts the STD IO communicator.
    *
    * @return Success if the communicator could be started; Error otherwise.
    */
   Error start() override;

   /**
    * @brief Stops the STD IO communicator.
    */
   void stop() override;

private:
   /**
    * @brief Begins reading from standard input.
    */
   void startReading();

   /**
    * @brief Writes the formatted response to the RStudio Launcher via standard output.
    *
    * @param in_responseMessage     The formatted response to send to the RStudio Launcher.
    */
   void writeResponse(const std::string& in_responseMessage) override;

   // The private implementation of StdIOLauncherCommunicator
   PRIVATE_IMPL(m_impl);
};

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio

#endif
