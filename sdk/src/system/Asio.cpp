/*
 * AsioService.cpp
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

#include <system/Asio.hpp>

#include <mutex>
#include <queue>
#include <thread>

#include <boost/asio.hpp>

#include <Error.hpp>
#include <system/DateTime.hpp>
#include <utils/ErrorUtils.hpp>
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
      IsRunning(true),
      IsSignalSetInit(false),
      SignalSet(IoService, SIGTERM, SIGINT) // These signals need to be passed in this order or it won't pick up SIGINTs
   {
   }

   /**
    * @brief Callback function which may be used to register a thread with the ASIO service and ensure it is available
    *        for ASIO work.
    */
   void startWorkerThread()
   {
      boost::asio::io_service::work work(IoService);
      IoService.run();
   }

   /** The underlying async IO service. */
   boost::asio::io_service& IoService;

   /** Whether the ASIO service is running */
   bool IsRunning;

   /** Whether the signal set is initialized or not. */
   bool IsSignalSetInit;

   /** The worker threads of the ASIO service. */
   std::vector<std::shared_ptr<std::thread> > Threads;

   /** The signal set which listens for singals and invokes the handler. */
   boost::asio::signal_set SignalSet;

   /** The mutex to protect the ASIO service. */
   std::mutex Mutex;
};

void AsioService::post(const AsioFunction& in_work)
{
   boost::asio::post(getAsioService().m_impl->IoService, in_work);
}

void AsioService::setSignalHandler(const OnSignal& in_onSignal)
{
   std::shared_ptr<Impl> sharedThis = getAsioService().m_impl;
   UNIQUE_LOCK_MUTEX(sharedThis->Mutex)
   {
      if (!sharedThis->IsSignalSetInit)
      {
         sharedThis->IsSignalSetInit = true;
         sharedThis->SignalSet.async_wait(std::bind(in_onSignal, std::placeholders::_2));
      }
   }
   END_LOCK_MUTEX
}

void AsioService::startThreads(size_t in_numThreads)
{
   std::shared_ptr<Impl> sharedThis = getAsioService().m_impl;

   UNIQUE_LOCK_MUTEX(sharedThis->Mutex)
   {
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
   END_LOCK_MUTEX
}

void AsioService::stop()
{
   std::shared_ptr<Impl> sharedThis = getAsioService().m_impl;

   UNIQUE_LOCK_MUTEX(sharedThis->Mutex)
   {
      if (sharedThis->IsRunning)
      {
         sharedThis->IoService.stop();
         sharedThis->IsRunning = false;
      }
   }
   END_LOCK_MUTEX
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
      StreamDescriptor(getIoService())
   {
      try
      {
         StreamDescriptor.assign(in_streamHandle);
      }
      catch (const boost::system::system_error& ec)
      {
         CreationError = utils::createErrorFromBoostError(ec.code(), ec.what(), ERROR_LOCATION);
      }
      catch (const std::exception& e)
      {
         CreationError = unknownError(e.what(), ERROR_LOCATION);
      }
   }

   void startReading(
      const std::unique_lock<std::mutex>& in_lock,
      const OnReadBytes& in_onReadBytes,
      const OnError& in_onError)
   {
      BOOST_ASSERT(in_lock.owns_lock());

      if (CreationError)
      {
         in_onError(CreationError);
         return;
      }

      WeakThis weakThis = weak_from_this();
      auto onRead =
         [=](const boost::system::error_code& in_ec, size_t in_bytesRead)
         {
            // If the read was aborted or this object has been destroyed, there's nothing else to do.
            SharedThis sharedThis = weakThis.lock();
            if ((in_ec == boost::asio::error::operation_aborted) || !sharedThis)
               return;

            if (in_ec || (in_bytesRead == 0))
            {
               in_onError(
                  systemError(
                     in_ec.value(),
                     "Could not read from stream with descriptor " + std::to_string(sharedThis->StreamDescriptor.native_handle()) + ".",
                     ERROR_LOCATION));
               return;
            }

            in_onReadBytes(sharedThis->ReadBuffer, in_bytesRead);
            UNIQUE_LOCK_MUTEX(sharedThis->ReadMutex)
            {
               sharedThis->startReading(uniqueLock, in_onReadBytes, in_onError);
            }
            END_LOCK_MUTEX
         };

      StreamDescriptor.async_read_some(boost::asio::buffer(ReadBuffer, Impl::READ_BUFFER_SIZE), onRead);
   }

   void startWriting(
      const std::unique_lock<std::mutex>& in_lock,
      const OnError& in_onError,
      const AsioFunction& in_onFinishedWriting)
   {
      BOOST_ASSERT(in_lock.owns_lock());

      if (CreationError)
      {
         in_onError(CreationError);
         return;
      }

      // Clear empty data, so we can treat writtenLength == 0 as an error
      while (!WriteBuffer.empty() && WriteBuffer.front().empty())
      {
         WriteBuffer.pop();
      }

      if (WriteBuffer.empty())
      {
         in_onFinishedWriting();
         return;
      }

      const std::string& nextData = WriteBuffer.front();

      std::vector<boost::asio::const_buffer> buffers;
      buffers.emplace_back(nextData.c_str(), nextData.size());

      WeakThis weakThis = weak_from_this();
      auto onWrite =
         [weakThis, in_onError, in_onFinishedWriting](const boost::system::error_code& in_ec, size_t in_writtenLength)
         {
            if (SharedThis instance = weakThis.lock())
            {
               // If the operation was aborted, just stop.
               if (in_ec == boost::asio::error::operation_aborted)
               {
                  in_onFinishedWriting();
                  return;
               }

               if (in_ec || (in_writtenLength == 0))
               {
                  in_onError(
                     systemError(
                        (in_ec.value() != 0) ? in_ec.value() : EIO,
                        "Could not write to stream with descriptor " +
                        std::to_string(instance->StreamDescriptor.native_handle()) + ".",
                        ERROR_LOCATION));

                  in_onFinishedWriting();
               }

               UNIQUE_LOCK_MUTEX(instance->WriteMutex)
               {
                  // If we didn't write all of the data, push the remainder onto the buffer queue and start over.
                  if (instance->WriteBuffer.front().size() > in_writtenLength)
                     instance->WriteBuffer.push(instance->WriteBuffer.front().substr(in_writtenLength));

                  // Pop the first message and start writing over again.
                  instance->WriteBuffer.pop();
                  instance->startWriting(uniqueLock, in_onError, in_onFinishedWriting);
               }
               END_LOCK_MUTEX
            }
         };

      boost::asio::async_write(StreamDescriptor, buffers, onWrite);
   }

   Error CreationError;

   /** The size of the buffer for reading data. */
   static const size_t READ_BUFFER_SIZE = 1024;

   /** The buffer into which to read data. */
   char ReadBuffer[READ_BUFFER_SIZE];

   /** The underlying stream descriptor. */
   boost::asio::posix::stream_descriptor StreamDescriptor;

   /** The buffer of data to write to the stream. */
   std::queue<std::string> WriteBuffer;

   /** Mutex which ensures only one block of data will be read at a time. */
   std::mutex ReadMutex;

   /** Mutex which ensures only one block of data will be written at a time. */
   std::mutex WriteMutex;
};

AsioStream::AsioStream(int in_streamHandle) :
   m_impl(new Impl(in_streamHandle))
{
}

AsioStream::~AsioStream() noexcept
{
   close();
}

void AsioStream::close() noexcept
{
   // We pass the ec variable to invoke the version of close that won't throw an exception on error - either
   if (m_impl->StreamDescriptor.is_open())
   {
      boost::system::error_code ec;
      m_impl->StreamDescriptor.close(ec);
   }
}

void AsioStream::readBytes(const OnReadBytes& in_onReadBytes, const OnError& in_onError)
{
   UNIQUE_LOCK_MUTEX(m_impl->ReadMutex)
   {
      m_impl->startReading(uniqueLock, in_onReadBytes, in_onError);
   }
   END_LOCK_MUTEX
}

void AsioStream::writeBytes(
   const std::string& in_data,
   const OnError& in_onError,
   const AsioFunction& in_onFinishedWriting)
{
   UNIQUE_LOCK_MUTEX(m_impl->WriteMutex)
   {
      m_impl->WriteBuffer.push(in_data);
      if (m_impl->WriteBuffer.size() == 1)
         m_impl->startWriting(
            uniqueLock,
            [in_onError](const Error& in_error)
            {
               if (in_onError)
                  in_onError(in_error);
            },
            [in_onFinishedWriting]()
            {
               if (in_onFinishedWriting)
                  in_onFinishedWriting();
            });
   }
   END_LOCK_MUTEX
}

// AsyncTimedEvent =====================================================================================================
struct AsyncTimedEvent::Impl
{
   typedef std::shared_ptr<AsyncTimedEvent::Impl> SharedImpl;
   typedef std::weak_ptr<AsyncTimedEvent::Impl> WeakImpl;

   Impl() :
      Running(true)
   {
   }

   static void runEvent(
      const WeakImpl& in_weakThis,
      const boost::posix_time::time_duration& in_intervalSeconds,
      const AsioFunction& in_event,
      const boost::system::error_code& in_ec)
   {
      // If the operation was canceled, there's nothing to do.
      if (in_ec == boost::asio::error::operation_aborted)
         return;
      // If another error occurred, log it and exit.
      else if (in_ec.value() != 0)
         return logging::logError(utils::createErrorFromBoostError(in_ec, ERROR_LOCATION));

      // If this has been deleted, there's nothing to do.
      SharedImpl sharedThis = in_weakThis.lock();
      if (!sharedThis)
         return;

      UNIQUE_LOCK_MUTEX(sharedThis->Mutex)
      {
         // If this is no longer running, there's nothing to do.
         if (!sharedThis->Running)
            return;

         // Otherwise, perform the action and restart the timer.
         in_event();
         sharedThis->Timer->expires_from_now(in_intervalSeconds);
         sharedThis->Timer->async_wait(
            std::bind(runEvent, in_weakThis, in_intervalSeconds, in_event, std::placeholders::_1));
      }
      END_LOCK_MUTEX
   }

   /** Mutex to protect the running status. */
   std::mutex Mutex;

   /** Whether the timed event is currently running. */
   bool Running;

   /** The timer that will invoke the callback function. */
   std::shared_ptr<boost::asio::deadline_timer> Timer;
};

AsyncTimedEvent::AsyncTimedEvent() :
   m_impl(new Impl())
{
}

void AsyncTimedEvent::start(const TimeDuration& in_timeDuration, const AsioFunction& in_event)
{
   // If the interval is 0 there is nothing to do.
   if (in_timeDuration == TimeDuration())
      return;

   boost::posix_time::time_duration timeDuration(
      in_timeDuration.getHours(),
      in_timeDuration.getMinutes(),
      in_timeDuration.getSeconds(),
      in_timeDuration.getMicroseconds());

   m_impl->Timer.reset(
      new boost::asio::deadline_timer(getIoService(), timeDuration));

   Impl::WeakImpl weakImpl = m_impl;
   m_impl->Timer->async_wait(std::bind(&Impl::runEvent, m_impl, timeDuration, in_event, std::placeholders::_1));
}

void AsyncTimedEvent::cancel()
{
   UNIQUE_LOCK_MUTEX(m_impl->Mutex)
   {
      m_impl->Running = false;
      if (m_impl->Timer)
         m_impl->Timer->cancel();
   }
   END_LOCK_MUTEX
}

void AsyncTimedEvent::reportError(const Error& in_error)
{
   logging::logError(in_error);
   cancel();
}

// AsyncDeadlineEvent ==================================================================================================
struct AsyncDeadlineEvent::Impl
{
   Impl(AsioFunction in_work, DateTime in_deadline) :
      Deadline(std::move(in_deadline)),
      Work(std::move(in_work))
   {

   }

   DateTime Deadline;
   std::shared_ptr<boost::asio::deadline_timer> Timer;
   AsioFunction Work;
};

AsyncDeadlineEvent::AsyncDeadlineEvent(const AsioFunction& in_work, const DateTime& in_deadlineTime) :
   m_impl(new Impl(in_work, in_deadlineTime))
{
}

AsyncDeadlineEvent::AsyncDeadlineEvent(const AsioFunction& in_work, const TimeDuration& in_waitTime) :
   m_impl(new Impl(in_work, DateTime() + in_waitTime))
{
}

AsyncDeadlineEvent::~AsyncDeadlineEvent()
{
   cancel();
}

void AsyncDeadlineEvent::cancel()
{
   if (m_impl->Timer != nullptr)
      m_impl->Timer->cancel();
}

void AsyncDeadlineEvent::start()
{
   DateTime now;
   if (now >= m_impl->Deadline)
      AsioService::post(m_impl->Work);
   else
   {
      TimeDuration diff = m_impl->Deadline - now;
      m_impl->Timer.reset(
         new boost::asio::deadline_timer(
            getIoService(),
            boost::posix_time::time_duration(
               diff.getHours(),
               diff.getMinutes(),
               diff.getSeconds(),
               diff.getMicroseconds())));

      std::weak_ptr<Impl> weakThis = m_impl;
      m_impl->Timer->async_wait(
         [weakThis](boost::system::error_code in_ec)
         {
            // Don't do any work if this was canceled or m_impl has been destroyed.
            std::shared_ptr<Impl> sharedThis = weakThis.lock();
            if ((in_ec != boost::asio::error::operation_aborted) && sharedThis)
               sharedThis->Work();
         });
   }
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio
