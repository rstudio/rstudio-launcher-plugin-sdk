/*
 * MutexUtils.hpp
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


#ifndef LAUNCHER_PLUGINS_MUTEX_UTILS_HPP
#define LAUNCHER_PLUGINS_MUTEX_UTILS_HPP

#include <mutex>
#include <system_error>

#include <Error.hpp>
#include <logging/Logger.hpp>

#define LOCK_MUTEX(in_mutex)                          \
try {                                                 \
   std::lock_guard<std::mutex> lockGuard(in_mutex);   \
   
#define RECURSIVE_LOCK_MUTEX(in_mutex)                         \
try {                                                          \
   std::lock_guard<std::recursive_mutex> uniqueLock(in_mutex); \

#define UNIQUE_LOCK_MUTEX(in_mutex)                   \
try {                                                 \
   std::unique_lock<std::mutex> uniqueLock(in_mutex); \

#define UNIQUE_RECURSIVE_LOCK_MUTEX(in_mutex)                     \
try {                                                             \
   std::unique_lock<std::recursive_mutex> uniqueLock(in_mutex);   \

#define END_LOCK_MUTEX                             \
}                                                  \
catch (const std::system_error& e)                 \
{                                                  \
   using namespace rstudio::launcher_plugins;      \
   logging::logError(                              \
      systemError(e, ERROR_LOCATION));             \
}                                                  \
CATCH_UNEXPECTED_EXCEPTION;                        \

#endif
