/*
 * AbstractResourceStream.hpp
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

#include <api/stream/AbstractResourceStream.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

typedef std::shared_ptr<AbstractResourceStream> SharedThis;
typedef std::weak_ptr<AbstractResourceStream> WeakThis;

struct AbstractResourceStream::Impl
{
   bool hasData() const
   {
      return LastData.CpuPercent || LastData.CpuSeconds || LastData.ResidentMem || LastData.VirtualMem;
   }

   bool IsComplete = false;

   ResourceUtilData LastData;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractResourceStream);

AbstractResourceStream::AbstractResourceStream(
   const ConstJobPtr& in_job,
   comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
   AbstractMultiStream(in_launcherCommunicator),
   m_job(in_job),
   m_resBaseImpl(new Impl())
{
}

void AbstractResourceStream::addRequest(uint64_t in_requestId, const system::User&)
{
   LOCK_MUTEX(m_mutex)
   {
      onAddRequest(in_requestId);

      if (m_resBaseImpl->hasData() || m_resBaseImpl->IsComplete)
         sendResponse({ in_requestId } , m_resBaseImpl->LastData, m_resBaseImpl->IsComplete);
   }
   END_LOCK_MUTEX
}

void AbstractResourceStream::setStreamComplete()
{
   LOCK_MUTEX(m_mutex)
   {
      if (!m_resBaseImpl->IsComplete)
         sendResponse(ResourceUtilData(), m_resBaseImpl->IsComplete = true);
   }
   END_LOCK_MUTEX
}

void AbstractResourceStream::reportData(const ResourceUtilData& in_data)
{
   LOCK_MUTEX(m_mutex)
   {
      if (!m_resBaseImpl->IsComplete)
         sendResponse(in_data, m_resBaseImpl->IsComplete);
   }
   END_LOCK_MUTEX
}

void AbstractResourceStream::reportError(const Error& in_error)
{
   LOCK_MUTEX(m_mutex)
   {
      if (!m_resBaseImpl->IsComplete)
      {
         logging::logErrorMessage(
            "An error occurred while streaming resource utilization metrics for Job " + m_job->Id);
         logging::logError(in_error);
         sendResponse(ResourceUtilData(), m_resBaseImpl->IsComplete = true);
      }
   }
   END_LOCK_MUTEX
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio