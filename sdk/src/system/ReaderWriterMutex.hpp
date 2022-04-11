/*
 * ReaderWriterMutex.hpp
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

#ifndef LAUNCHER_PLUGINS_READER_WRITER_MUTEX_HPP
#define LAUNCHER_PLUGINS_READER_WRITER_MUTEX_HPP

#include <PImpl.hpp>

#include <system_error>

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Reentrant reader-writer mutex implementation. This implementation is write-preferring.
 */
class ReaderWriterMutex
{
public:
   /**
    * @brief Constructor.
    */
   ReaderWriterMutex();

   /**
    * @brief Move constructor.
    *
    * @param in_other   The mutex to move to this mutex.
    */
   ReaderWriterMutex(ReaderWriterMutex&& in_other) noexcept;

   /**
    * @brief Deleted copy constructor to prevent copy.
    */
   ReaderWriterMutex(const ReaderWriterMutex&) = delete;

   /**
    * @brief Deleted assignment operator to prevent copy.
    */
   ReaderWriterMutex& operator=(const ReaderWriterMutex&) = delete;

   /**
    * @brief Locks the mutex for read.
    *
    * Note: unlockRead() must be called once for each time lockRead() was called.
    */
   void lockRead();

   /**
    * @brief Locks the mutex for write.
    *
    * Note: unlockWrite() must be called once for each time lockWrite() was called.
    */
   void lockWrite();

   /**
    * @brief Unlocks the mutex after a read operation.
    *
    * Note: unlockRead() must be called once for each time lockRead() was called.
    */
   void unlockRead();

   /**
    * @brief Unlocks the mutex after a write operation.
    *
    * Note: unlockWrite() must be called once for each time lockWrite() was called.
    */
   void unlockWrite();

private:

   PRIVATE_IMPL(m_impl);
};

/**
 * @brief RAII class which allows you to lock a ReaderWriterMutex for read.
 */
class ReaderLock
{
public:
   /**
    * @brief Constructor. Locks the specified mutex for read.
    *
    * @param in_mutex    The mutex to lock for read.
    */
   explicit ReaderLock(ReaderWriterMutex& in_mutex);

   /**
    * @brief Destructor.
    */
   ~ReaderLock();

private:
   // The mutex this lock manages.
   ReaderWriterMutex& m_mutex;
};

/**
 * @brief RAII class which allows you to lock a ReaderWriterMutex for write.
 */
class WriterLock
{
public:
   /**
    * @brief Constructor. Locks the specified mutex for write.
    *
    * @param in_mutex    The mutex to lock for write.
    */
   explicit WriterLock(ReaderWriterMutex& in_mutex);

   /**
    * @brief Destructor.
    */
   ~WriterLock();

private:
   // The mutex this lock manages.
   ReaderWriterMutex& m_mutex;
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#define READ_LOCK_BEGIN(mutex)                                    \
   try                                                            \
   {                                                              \
      rstudio::launcher_plugins::system::ReaderLock lock(mutex);  \

#define WRITE_LOCK_BEGIN(mutex)                                   \
   try                                                            \
   {                                                              \
      rstudio::launcher_plugins::system::WriterLock lock(mutex);  \

#define RW_LOCK_END(tryLog)                                                                  \
   }                                                                                         \
   catch (const std::system_error& e)                                                        \
   {                                                                                         \
      if (tryLog)                                                                            \
         logging::logError(systemError(e, ERROR_LOCATION));                                  \
   }                                                                                         \
   catch (const std::exception& e)                                                           \
   {                                                                                         \
      if (tryLog)                                                                            \
         logging::logErrorMessage(std::string("Unexpected exception: ") + e.what(),          \
                                  ERROR_LOCATION);                                           \
   }                                                                                         \
   catch (...)                                                                               \
   {                                                                                         \
      if (tryLog)                                                                            \
         logging::logErrorMessage("Unknown exception while trying to acquire lock.",         \
                                  ERROR_LOCATION);                                           \
   }                                                                                         \

#endif //LAUNCHER_PLUGINS_READER_WRITER_MUTEX_HPP
