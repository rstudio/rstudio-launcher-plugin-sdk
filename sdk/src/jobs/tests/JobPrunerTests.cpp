/*
 * JobPrunerTests.cpp
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

#include <atomic>

#include <AsioRaii.hpp>
#include <system/Asio.hpp>
#include <jobs/JobRepository.hpp>

#include "../JobPruner.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

std::atomic<uint32_t> s_count = { 0 };

class MockRepo : public JobRepository
{
private:
   void onJobRemoved(api::JobPtr in_job) override
   {
      s_count.fetch_add(1);
      // Only the second and third job will be removed.
      if (in_job->Id == "2")
      {
         CHECK(in_job->Status == api::Job::State::FAILED);
         CHECK(in_job->StatusMessage == "No such device.");
      }
      else if (in_job->Id == "3")
      {
         CHECK(in_job->Status == api::Job::State::FINISHED);
         CHECK(in_job->StatusMessage.empty());
      }
      else
         CHECK(false);
   }
};

TEST_CASE("Prune job")
{
   system::AsioRaii init;

   JobRepositoryPtr jobRepo(new MockRepo());
   JobStatusNotifierPtr notifier(new JobStatusNotifier());
   JobPruner jobPruner(jobRepo, notifier);

   api::JobPtr job1(new api::Job()), job2(new api::Job()), job3(new api::Job());

   job1->Id = "1";
   job1->Status = api::Job::State::PENDING;
   job1->SubmissionTime = system::DateTime();
   job1->LastUpdateTime = system::DateTime();

   system::DateTime sdt, ldt1, ldt2, idt;
   REQUIRE_FALSE(system::DateTime::fromString("2019-12-30T11:34:09.210984", sdt));
   REQUIRE_FALSE(system::DateTime::fromString("2019-12-30T15:34:09.210984", ldt1));
   REQUIRE_FALSE(system::DateTime::fromString("2019-12-30T15:34:09.210984", ldt2));
   REQUIRE_FALSE(system::DateTime::fromString("2019-12-30T17:34:09.210984", idt));

   job2->Id = "2";
   job2->Status = api::Job::State::PENDING;
   job2->SubmissionTime = sdt;
   job2->LastUpdateTime = ldt1;

   job3->Id = "3";
   job3->Status = api::Job::State::PENDING;
   job3->SubmissionTime = sdt;
   job3->LastUpdateTime = ldt1;

   jobRepo->addJob(job1);
   jobRepo->addJob(job2);
   jobRepo->addJob(job3);

   // Updates:
   // job 3 -> running
   notifier->updateJob(job3, api::Job::State::RUNNING, "", ldt2);
   // job 1 -> running
   notifier->updateJob(job1, api::Job::State::RUNNING);
   // job 2 -> failed
   notifier->updateJob(job2, api::Job::State::FAILED, "No such device.", idt);
   // job3 -> finished
   notifier->updateJob(job3, api::Job::State::FINISHED, "", idt);

   sleep(2);
   CHECK(s_count == 2);

   api::JobList expected, actual = jobRepo->getJobs(system::User());
   expected.push_back(job1);

   CHECK(std::equal(expected.begin(), expected.end(), actual.begin()));
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
