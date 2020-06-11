/*
 * SmokeTest.hpp
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

#ifndef LAUNCHER_PLUGINS_SMOKETEST_HPP
#define LAUNCHER_PLUGINS_SMOKETEST_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <Error.hpp>
#include <system/FilePath.hpp>
#include <system/Process.hpp>
#include <system/Asio.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace smoke_test {

class SmokeTest : public std::enable_shared_from_this<SmokeTest>
{
public:
   explicit SmokeTest(system::FilePath in_pluginPath);

   Error initialize();

   bool sendRequest();

   void stop();

private:
   std::shared_ptr<system::process::AbstractChildProcess> m_plugin;
   system::FilePath m_pluginPath;
   bool m_exited;
   std::map<uint64_t, uint64_t> m_responseCount;

   std::mutex m_mutex;
   std::condition_variable m_condVar;
};

typedef std::shared_ptr<SmokeTest> SmokeTestPtr;

} // namespace smoke_test
} // namespace launcher_plugins
} // namespace rstudio

#endif
