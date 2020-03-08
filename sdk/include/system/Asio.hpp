/*
 * AsioService.hpp
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

#ifndef LAUNCHER_PLUGINS_ASIO_HPP
#define LAUNCHER_PLUGINS_ASIO_HPP

#include <Noncopyable.hpp>

#include <functional>

#include <PImpl.hpp>
#include <utils/Functionals.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Callback function which will be invoked asynchronously by the AsioService.
 */
typedef std::function<void()> AsioFunction;

/**
 * @brief Callback function which will be invoked when bytes are read from an AsioStream.
 */
typedef std::function<void(const char*, size_t)> OnReadBytes;

/**
 * @brief Callback function which will be invoked when a signal is sent to this process.
 */
typedef std::function<void(int)> OnSignal;

/**
 * @brief Async input/output class which may be used to manage ASIO operations.
 */
class AsioService : public Noncopyable
{
public:

   /**
    * @brief Posts a job to be completed by this ASIO Service.
    *
    * @param in_work    The job to be posted to the ASIO Service.
    */
   static void post(const AsioFunction& in_work);

   /**
    * @brief Sets the signal handler on the ASIO service.
    *
    * The ASIO service will manage signals sent to the process. The signal handler provided here will be invoked when a
    * signal is received.
    *
    * @param in_onSignal    The function to invoke when a signal is received.
    */
   static void setSignalHandler(const OnSignal& in_onSignal);

   /**
    * @brief Creates and adds the specified number of worker threads to the ASIO service.
    *
    * @param in_numThreads  The number of worker threads to add to the ASIO service.
    */
   static void startThreads(size_t in_numThreads);

   /**
    * @brief Stops the ASIO Service.
    *
    * Calling this function will stop all async IO operations for the whole Plugin.
    * IMPORTANT: Only call this function from the main thread immediately prior to shutdown.
    */
   static void stop();

   /*
    * @brief Waits for the ASIO service to exit.
    */
   static void waitForExit();

private:
   /**
    * @brief Private default constructor.
    */
   AsioService();

   /**
    * @brief Gets the single ASIO service for this process.
    *
    * @return The single ASIO service for this process.
    */
   static AsioService& getAsioService();

   // Private implementation of AsioService.
   PRIVATE_IMPL_SHARED(m_impl);
};

/**
 * @brief Class which allows reading from or writing to streams asynchronously.
 */
class AsioStream
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_streamHandle    The handle of the stream for which to create this ASIO stream descriptor.
    */
   explicit AsioStream(int in_streamHandle);

   /**
    * @brief Closes the stream. Nothing may be read from or written to the stream after this is called.
    */
   void close();

   /**
    * @brief Attempts to read bytes from this ASIO stream.
    *
    * @param in_onReadBytes     Callback function which will be invoked on successful read.
    * @param in_onError         Callback function which will be invoked if an error occurs.
    */
   void readBytes(const OnReadBytes& in_onReadBytes, const OnError& in_onError);

   /**
    * @brief Writes the provided data to the stream asynchronously.
    *
    * This method is thread safe. Each provided block of data will be written to the stream in full before a new one
    * begins.
    */
   void writeBytes(const std::string& in_data, const OnError& in_onError);

private:
   // The private implementation of AsioStream.
   PRIVATE_IMPL_SHARED(m_impl);
};

/**
 * @brief Class which performs an action asynchronously every specified number of seconds.
 */
class AsyncTimedEvent
{
public:
   /**
    * @brief Default constructor.
    */
   AsyncTimedEvent();

   /**
    * @brief Starts performing the specified event every in_intervalSeconds seconds.
    *
    * This function may only be called once per instance. Restarting a canceled or otherwise stopped timed event will
    * not work. Instead, a new instance should be created and started.
    *
    * @param in_intervalSeconds     The number of seconds to wait between each event.
    * @param in_event               The action to perform every in_intervalSeconds seconds.
    */
   void start(uint64_t in_intervalSeconds, const AsioFunction& in_event);

   /**
    * @brief Cancels the timed event.
    */
   void cancel();

   /**
    * @brief Reports a fatal error to the AsyncTimedEvent instance. Invoking this will cause the timed event to stop
    *        running.
    *
    * This method should be invoked by the event passed to start if a fatal error occurs.
    *
    * @param in_error   The error which occurred.
    */
   void reportError(const Error& in_error);

private:

   // The private implementation of AsyncTimedEvent.
   PRIVATE_IMPL_SHARED(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
