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
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <Error.hpp>
#include <utils/MutexUtils.hpp>

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
struct AsioStream::Impl : public std::enable_shared_from_this<AsioStream::Impl>
{
   typedef std::shared_ptr<Impl> SharedThis;
   typedef std::weak_ptr<Impl> WeakThis;
   /**
    * @brief Constructor.
    *
    * @param in_streamHandle    The handle of the stream to open.
    */
   explicit Impl(boost::asio::posix::stream_descriptor::native_handle_type in_streamHandle) :
         StreamDescriptor(getIoService(), in_streamHandle)
   {
   }


   void startWriting(const boost::unique_lock<boost::mutex>& in_lock, const OnError& in_onError)
   {
      BOOST_ASSERT(in_lock.owns_lock());

      if (WriteBuffer.empty())
         return;

      const std::string& nextData = WriteBuffer.front();

      std::vector<boost::asio::const_buffer> buffers;
      buffers.emplace_back(nextData.c_str(), nextData.size());

      WeakThis weakThis = std::static_pointer_cast<Impl>(shared_from_this());
      auto onWrite =
         [weakThis, in_onError](const boost::system::error_code& in_ec, size_t in_writtenLength)
         {
            if (SharedThis instance = weakThis.lock())
            {
               // If the operation was aborted, just stop.
               if (in_ec == boost::asio::error::operation_aborted)
                  return;

               if (in_ec)
                  in_onError(
                     systemError(
                        in_ec.value(),
                        "Could not write to stream with descriptor " + std::to_string(instance->StreamDescriptor.native_handle()) + ".",
                        ERROR_LOCATION));

               try
               {
                  boost::unique_lock<boost::mutex> lock(instance->Mutex);
                  if (instance->WriteBuffer.front().size() > in_writtenLength)
                  {
                     // We failed to write all of the bytes.
                     in_onError(
                        Error(
                           "StreamWriteError",
                           1,
                           "Failed to write " +
                              std::to_string(instance->WriteBuffer.front().size()) +
                              " bytes to the stream. Wrote " +
                              std::to_string(in_writtenLength) +
                              " bytes.",
                           ERROR_LOCATION));
                     return;
                  }

                  // If there was no error, pop the first message and start writing over again.
                  instance->WriteBuffer.pop();
                  instance->startWriting(lock, in_onError);
               END_LOCK_MUTEX
            }
         };

      boost::asio::async_write(StreamDescriptor, buffers, onWrite);
   }

   /** The size of the buffer for reading data. */
   static const size_t READ_BUFFER_SIZE = 1024;

   /** The buffer into which to read data. */
   char ReadBuffer[READ_BUFFER_SIZE];

   /** The underlying stream descriptor. */
   boost::asio::posix::stream_descriptor StreamDescriptor;

   /** The buffer of data to write to the stream. */
   std::queue<std::string> WriteBuffer;

   /** Mutex which ensures only one block of data will be written at a time. */
   boost::mutex Mutex;
};

AsioStream::AsioStream(int in_streamHandle) :
   m_impl(new Impl(in_streamHandle))
{
}

void AsioStream::close()
{
   m_impl->StreamDescriptor.close();
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

         in_onReadBytes(implPtr->ReadBuffer, in_bytesRead);
      };

   m_impl->StreamDescriptor.async_read_some(boost::asio::buffer(m_impl->ReadBuffer, Impl::READ_BUFFER_SIZE), onRead);
}

void AsioStream::writeBytes(const std::string& in_data, const OnError& in_onError)
{
   try
   {
      boost::unique_lock<boost::mutex> lock(m_impl->Mutex);

      m_impl->WriteBuffer.push(in_data);
      if (m_impl->WriteBuffer.size() == 1)
         m_impl->startWriting(lock, in_onError);

   END_LOCK_MUTEX
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
