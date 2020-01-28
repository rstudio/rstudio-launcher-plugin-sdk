/*
 * AsioService.cpp
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

#include <system/Asio.hpp>

#include <thread>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

namespace {

boost::asio::io_service& getIoService()
{
   static boost::asio::io_service ioService;
   return ioService;
}

}

// Asio Service ========================================================================================================
struct AsioService::Impl
{
   /**
    * @brief Constructor.
    */
   Impl() :
      IoService(getIoService()),
      IsRunning(true)
   {
   }

   void startWorkerThread()
   {
      boost::asio::io_service::work work(IoService);
      IoService.run();
   }

   /** The underlying async IO service. */
   boost::asio::io_service& IoService;

   /** Whether the ASIO service is running */
   bool IsRunning;

   /** The worker threads of the ASIO service. */
   std::vector<std::shared_ptr<std::thread> > Threads;

   /** The mutex to protect the ASIO service. */
   boost::mutex Mutex;
};

void AsioService::post(const AsioFunction& in_work)
{
   boost::asio::post(getAsioService().m_impl->IoService, in_work);
}

void AsioService::startThreads(size_t in_numThreads)
{
   std::shared_ptr<Impl> sharedThis = getAsioService().m_impl;
   boost::unique_lock<boost::mutex> lock(sharedThis->Mutex);
   if (sharedThis->IsRunning)
   {
      for (size_t i = 0; i < in_numThreads; ++i)
      {
         sharedThis->Threads.emplace_back(new std::thread(
            [sharedThis]()
            {
               sharedThis->startWorkerThread();
            }));
      }
   }
}

void AsioService::stop()
{
   std::shared_ptr<Impl> sharedThis = getAsioService().m_impl;
   boost::lock_guard<boost::mutex> lock(sharedThis->Mutex);
   if (sharedThis->IsRunning)
   {
      sharedThis->IoService.stop();
      sharedThis->IsRunning = false;
   }
}

void AsioService::waitForExit()
{
   std::shared_ptr<Impl> sharedThis = getAsioService().m_impl;
   for (const std::shared_ptr<std::thread>& thread: sharedThis->Threads)
   {
      thread->join();
   }
}

AsioService& AsioService::getAsioService()
{
   static AsioService asioService;
   return asioService;
}

AsioService::AsioService() :
   m_impl(new Impl())
{
}

// Asio Stream Descriptor ==============================================================================================
struct AsioStream::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_streamHandle    The handle of the stream to open.
    */
   explicit Impl(boost::asio::posix::stream_descriptor::native_handle_type in_streamHandle) :
         StreamDescriptor(getIoService(), in_streamHandle)
   {
   }

   /** The size of the buffer for reading data. */
   static const size_t BUFFER_SIZE = 1024;

   /** The underlying stream descriptor. */
   boost::asio::posix::stream_descriptor StreamDescriptor;

   /** The buffer into which to read data. */
   char Buffer[BUFFER_SIZE];
};

AsioStream::AsioStream(int in_streamHandle) :
   m_impl(new Impl(in_streamHandle))
{
}

void AsioStream::readBytes(const OnReadBytes& in_onReadBytes, const OnError& in_onError)
{
   std::shared_ptr<Impl> implPtr = m_impl;
   auto onRead =
      [=](const boost::system::error_code& in_ec, size_t in_bytesRead)
      {
         // If the read was aborted, there's nothing else to do.
         if (in_ec == boost::asio::error::operation_aborted)
            return;

         if (in_ec)
         {
            in_onError(
               systemError(
                  in_ec.value(),
                  "Could not read from stream with descriptor " + std::to_string(implPtr->StreamDescriptor.native_handle()) + ".",
                  ERROR_LOCATION));
            return;
         }

         in_onReadBytes(implPtr->Buffer, in_bytesRead);
      };

   m_impl->StreamDescriptor.async_read_some(boost::asio::buffer(m_impl->Buffer, Impl::BUFFER_SIZE), onRead);
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
