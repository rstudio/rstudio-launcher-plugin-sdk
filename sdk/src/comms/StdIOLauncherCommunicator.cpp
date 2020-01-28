/*
 * StdIOLauncherCommunicator.cpp
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

#include "StdIOLauncherCommunicator.hpp"

#include <boost/asio/posix/stream_descriptor.hpp>

#include <system/Asio.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace comms {

/** Convenience typedef for shared pointer to this. */
typedef std::shared_ptr<StdIOLauncherCommunicator> SharedThis;

/** Convenience typedef for weak pointer to this. */
typedef std::weak_ptr<StdIOLauncherCommunicator> WeakThis;

struct StdIOLauncherCommunicator::Impl
{
   Impl() :
      StdInStream(STDIN_FILENO),
      StdOutStream(STDOUT_FILENO)
   {

   }

   system::AsioStream StdInStream;
   system::AsioStream StdOutStream;
};

StdIOLauncherCommunicator::StdIOLauncherCommunicator(size_t in_maxMessageSize, const OnError& in_onError) :
      AbstractLauncherCommunicator(in_maxMessageSize, in_onError),
      m_impl(new Impl())
{
}

Error StdIOLauncherCommunicator::start()
{
   Error error = AbstractLauncherCommunicator::start();
   if (error)
      return error;

   startReading();
}

void StdIOLauncherCommunicator::stop()
{
   AbstractLauncherCommunicator::stop();

   m_impl->StdOutStream.close();
   m_impl->StdInStream.close();
}

void StdIOLauncherCommunicator::startReading()
{
   WeakThis weakThis = std::static_pointer_cast<StdIOLauncherCommunicator>(
      shared_from_derived<StdIOLauncherCommunicator>());

   system::OnReadBytes onRead = [weakThis](const char* in_data, size_t in_length)
   {
      if (SharedThis instance = weakThis.lock())
      {
         instance->onDataReceived(in_data, in_length);
         instance->startReading();
      }
   };

   OnError onError = [weakThis](const Error& in_error)
   {
      if (SharedThis instance = weakThis.lock())
      {
         instance->reportError(in_error);
      }
   };

   m_impl->StdInStream.readBytes(onRead, onError);
}

void StdIOLauncherCommunicator::writeResponse(const std::string& in_responseMessage)
{
   WeakThis weakThis = std::static_pointer_cast<StdIOLauncherCommunicator>(
      shared_from_derived<StdIOLauncherCommunicator>());

   OnError onError = [weakThis](const Error& in_error)
   {
      if (SharedThis instance = weakThis.lock())
      {
         instance->reportError(in_error);
      }
   };

   m_impl->StdOutStream.writeBytes(in_responseMessage, onError);
}

} // namespace comms
} // namespace launcher_plugins
} // namespace rstudio
