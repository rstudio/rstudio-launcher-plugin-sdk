/*
 * LocalPluginApi.cpp
 * 
 * Copyright (C) 2019-20 by RStudio, PBC
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

#include <LocalPluginApi.hpp>

#include <LocalJobSource.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

LocalPluginApi::LocalPluginApi(
   std::string in_hostname,
   std::shared_ptr<comms::AbstractLauncherCommunicator> in_launcherCommunicator) :
   AbstractPluginApi(std::move(in_launcherCommunicator)),
   m_hostname(std::move(in_hostname))
{
}

std::shared_ptr<api::IJobSource> LocalPluginApi::createJobSource() const
{
   return std::shared_ptr<api::IJobSource>(new LocalJobSource(m_hostname));
}

Error LocalPluginApi::doInitialize()
{
   return Success();
}

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

