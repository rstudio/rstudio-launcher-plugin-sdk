/*
 * TimedJobStatusWatcherTests.cpp
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

#include <TestMain.hpp>

#include <jobs/AbstractTimedJobStatusWatcher.hpp>

#include <AsioRaii.hpp>
#include <MockLogDestination.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

class MockJobStatusWatcher : public AbstractTimedJobStatusWatcher
{
public:
   MockJobStatusWatcher(uint64_t in_freqSeconds, JobRepositoryPtr in_jobRepo, JobStatusNotifierPtr in_notifier) :
      AbstractTimedJobStatusWatcher(
         system::TimeDuration::Seconds(in_freqSeconds),
         std::move(in_jobRepo),
         std::move(in_notifier)),
      PollCount(0)
   {
   }

   uint64_t PollCount;

private:
   Error pollJobStatus() override
   {
      ++PollCount;
      return Success();
   }

   Error getJobDetails(const std::string&, api::JobPtr&) const override
   {
      return Error("NotSupported", 1, "NotSupported", ERROR_LOCATION);
   }
};

class ErrorJobStatusWatcher : public AbstractTimedJobStatusWatcher
{
public:
   ErrorJobStatusWatcher(uint64_t in_freqSeconds, JobRepositoryPtr in_jobRepo, JobStatusNotifierPtr in_notifier) :
      AbstractTimedJobStatusWatcher(
         system::TimeDuration::Seconds(in_freqSeconds),
         std::move(in_jobRepo),
         std::move(in_notifier)),
      PollCount(0)
   {
   }

   uint64_t PollCount;

private:
   Error pollJobStatus() override
   {
      ++PollCount;
      return Error("WatcherError", 1, "An error message", ERROR_LOCATION);
   }

   Error getJobDetails(const std::string&, api::JobPtr&) const  override
   {
      return Error("NotSupported", 1, "NotSupported", ERROR_LOCATION);
   }
};

TEST_CASE("Timed Job Status Watcher Tests")
{
   system::AsioRaii init;

   // Job repo and status notifier.
   JobStatusNotifierPtr notifier(new JobStatusNotifier());
   JobRepositoryPtr repo(new JobRepository(notifier));

   SECTION("No polling errors")
   {
      std::shared_ptr<MockJobStatusWatcher> watcher(new MockJobStatusWatcher(2, repo, notifier));
      REQUIRE_FALSE(watcher->start());
      sleep(7); // 4 invocations - on start + 3 during the 7 second sleep.
      watcher->stop();
      sleep(2); // Validate it's really stopped by sleeping more.
      CHECK(watcher->PollCount == 4);
   }

   SECTION("Polling errors")
   {
      std::shared_ptr<ErrorJobStatusWatcher> watcher(new ErrorJobStatusWatcher(1, repo, notifier));
      CHECK(watcher->start());; // Error on first poll.
      sleep(2); // Shouldn't be anymore polls.
      CHECK(watcher->PollCount == 1);
   }
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
