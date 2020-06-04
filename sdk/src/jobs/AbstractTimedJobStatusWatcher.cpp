/*
 * AbstractTimedJobStatusWatcher.cpp
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


#include <jobs/AbstractTimedJobStatusWatcher.hpp>

#include <system/Asio.hpp>
#include <utils/MutexUtils.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

typedef std::shared_ptr<AbstractTimedJobStatusWatcher> SharedThis;
typedef std::weak_ptr<AbstractTimedJobStatusWatcher> WeakThis;

struct AbstractTimedJobStatusWatcher::Impl
{
   explicit Impl(system::TimeDuration in_frequency) :
      Frequency(std::move(in_frequency))
   {

   }

   system::TimeDuration Frequency;

   system::AsyncTimedEvent TimedEvent;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractTimedJobStatusWatcher)

AbstractTimedJobStatusWatcher::~AbstractTimedJobStatusWatcher() noexcept
{
   try
   {
      stop();
   }
   catch (...)
   {
      // Swallow any possible exceptions to prevent throwing in the destructor.
   }
}

Error AbstractTimedJobStatusWatcher::start()
{
   // First poll is immediately, and then every Frequency time.
   Error error = pollJobStatus();
   if (error)
      return error;

   // Success full first poll, set up the timed event.
   WeakThis weakThis = shared_from_this();
   system::AsioFunction onTimer = [weakThis]()
   {
      if (SharedThis sharedThis = weakThis.lock())
      {
         Error error = sharedThis->pollJobStatus();
         if (error)
            sharedThis->m_timedBaseImpl->TimedEvent.reportError(error);
      }
   };
   m_timedBaseImpl->TimedEvent.start(m_timedBaseImpl->Frequency, onTimer);

   return Success();
}

void AbstractTimedJobStatusWatcher::stop()
{
   m_timedBaseImpl->TimedEvent.cancel();
}

AbstractTimedJobStatusWatcher::AbstractTimedJobStatusWatcher(
   system::TimeDuration in_frequency,
   JobRepositoryPtr in_jobRepository,
   JobStatusNotifierPtr in_jobStatusNotifier) :
      AbstractJobStatusWatcher(std::move(in_jobRepository), std::move(in_jobStatusNotifier)),
      m_timedBaseImpl(new Impl(std::move(in_frequency)))
{
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
