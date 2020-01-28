/*
 * AsioService.hpp
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

#ifndef LAUNCHER_PLUGINS_ASIO_HPP
#define LAUNCHER_PLUGINS_ASIO_HPP

#include <Noncopyable.hpp>

#include <functional>

#include <PImpl.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Function which will be run asynchronously by the AsioService.
 */
typedef std::function<void()> AsioFunction;

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

private:
   // The private implementation of AsioStream.
   PRIVATE_IMPL(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
