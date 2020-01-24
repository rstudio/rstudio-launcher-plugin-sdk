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

namespace rstudio {
namespace launcher_plugins {
namespace system {

// Asio Service ========================================================================================================
struct AsioService::Impl
{
   /**
    * @brief Constructor.
    */
   Impl() :
      IsRunning(true)
   {
   }

   /** The underlying async IO service. */
   boost::asio::io_service IoService;

   /** Whether the ASIO service is running */
   bool IsRunning;
};

PRIVATE_IMPL_DELETER_IMPL(AsioService)

AsioService& AsioService::getAsioService()
{
   static AsioService asioService;
   return asioService;
}

void AsioService::post(const AsioFunction& in_work)
{
   boost::asio::post(m_impl->IoService, in_work);
}

void AsioService::stop()
{
   if (m_impl->IsRunning)
   {
      m_impl->IoService.stop();
      m_impl->IsRunning = false;
   }
}

AsioService::AsioService() :
   m_impl(new Impl())
{
}

// Asio Stream Descriptor ==============================================================================================
struct AsioStreamDescriptor::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_ioService       The IO service which manages asynchronous reading and writing to this stream.
    * @param in_streamHandle    The handle of the stream to open.
    */
   Impl(
      boost::asio::io_service& in_ioService,
      boost::asio::posix::stream_descriptor::native_handle_type in_streamHandle) :
         StreamDescriptor(in_ioService, in_streamHandle)
   {
   }

   /** The underlying stream descriptor. */
   boost::asio::posix::stream_descriptor StreamDescriptor;
};

PRIVATE_IMPL_DELETER_IMPL(AsioStreamDescriptor)

AsioStreamDescriptor::AsioStreamDescriptor(int in_streamHandle) :
   m_impl(new Impl(AsioService::getAsioService().m_impl->IoService, in_streamHandle))
{
}

// Asio Worker Thread ==================================================================================================
struct AsioWorkerThread::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_workFunction    The function which should be run on this thread.
    */
   explicit Impl(const AsioFunction& in_workFunction) :
      Thread(in_workFunction)
   {
   }

   /**
    * @brief Run function to use to register this thread as an ASIO worker thread.
    *
    * @param in_ioService   The ASIO service to which to register this thread.
    */
   static void run(boost::asio::io_service& in_ioService)
   {
      boost::asio::io_service::work work(in_ioService);
      in_ioService.run();
   }

   /** The underlying thread. */
   std::thread Thread;
};

PRIVATE_IMPL_DELETER_IMPL(AsioWorkerThread)

void AsioWorkerThread::join()
{
   if (m_impl)
      m_impl->Thread.join();
}

void AsioWorkerThread::run()
{
   // Ensure that this thread won't be deleted until the underlying thread is joined.
   std::shared_ptr<AsioWorkerThread> sharedThis = shared_from_this();
   AsioFunction runFunction = [sharedThis]()
   {
      sharedThis->m_impl->run(AsioService::getAsioService().m_impl->IoService);
   };

   m_impl.reset(new Impl(runFunction));
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
