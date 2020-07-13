/*
 * JobStatusNotifierTests.cpp
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

#include <queue>

#include <jobs/AbstractJobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

TEST_CASE("Job Status Notifier")
{

   JobStatusNotifierPtr notifier(new JobStatusNotifier());
   JobRepositoryPtr jobRepo(new AbstractJobRepository(notifier));

   api::JobPtr job1(new api::Job()),
               job2(new api::Job()),
               job3(new api::Job()),
               job4(new api::Job());

   job1->Id = "1";
   job2->Id = "2";
   job3->Id = "3";
   job4->Id = "4";

   job1->Name = "Job 1";
   job2->Name = "Job 2";
   job3->Name = "Job 3";
   job4->Name = "Job 4";

   job1->Status = api::Job::State::SUSPENDED;
   job2->Status = api::Job::State::PENDING;
   job3->Status = api::Job::State::RUNNING;
   job4->Status = api::Job::State::PENDING;

   job1->StatusMessage = "Suspended by user.";
   job2->StatusMessage = "Resources.";

   system::DateTime sd1, sd2, sd3, sd4;
   REQUIRE_FALSE(system::DateTime::fromString("2020-03-16T15:43:21.123456", sd1));
   REQUIRE_FALSE(system::DateTime::fromString("2020-03-13T21:10:11.002244", sd3));
   REQUIRE_FALSE(system::DateTime::fromString("2020-03-18T05:38:19.997755", sd4));
   job1->SubmissionTime = sd1;
   job2->SubmissionTime = sd2;
   job3->SubmissionTime = sd3;
   job4->SubmissionTime = sd4;

   system::DateTime ld1, ld3, ld4;
   REQUIRE_FALSE(system::DateTime::fromString("2020-03-20T09:57:26.030409", ld1));
   REQUIRE_FALSE(system::DateTime::fromString("2020-03-14T15:07:08.665544", ld3));
   job1->LastUpdateTime = ld1;
   job2->LastUpdateTime = sd2;
   job3->LastUpdateTime = ld3;
   job4->LastUpdateTime = ld4;

   jobRepo->addJob(job1);
   jobRepo->addJob(job2);
   jobRepo->addJob(job3);
   jobRepo->addJob(job4);

   SECTION("Subscribe to one")
   {
      system::DateTime ut;

      OnJobStatusUpdate onJobStatusUpdate = [&](const api::JobPtr& in_jobPtr)
      {
         CHECK(in_jobPtr->Id == "1");
         CHECK(in_jobPtr->LastUpdateTime.getValueOr(system::DateTime()) == ut);
         CHECK(in_jobPtr->Name == "Job 1");
         CHECK(in_jobPtr->Status == api::Job::State::RUNNING);
         CHECK(in_jobPtr->StatusMessage.empty());
         CHECK(in_jobPtr->SubmissionTime == sd1);
      };

      // Ensure ut != system::DateTime() when update is called.
      sleep(1);

      // Subscribe.
      SubscriptionHandle subHandle = notifier->subscribe("1", onJobStatusUpdate);

      // Update the subscribed job.
      notifier->updateJob(job1, api::Job::State::RUNNING, "", ut);

      // Update another job.
      notifier->updateJob(job2, api::Job::State::SUSPENDED);
   }

   SECTION("Subscribe to one (old update)")
   {
      system::DateTime ut;
      OnJobStatusUpdate onJobStatusUpdate = [&](const api::JobPtr& in_job)
      {
         CHECK(in_job->Id == "3");
         CHECK(in_job->LastUpdateTime.getValueOr(system::DateTime()) == ut);
         CHECK(in_job->Name == "Job 3");
         CHECK(in_job->Status == api::Job::State::FINISHED);
         CHECK(in_job->StatusMessage == "Exited with non-zero exit code (2)");
         CHECK(in_job->SubmissionTime == sd3);
      };

      // Subscribe.
      SubscriptionHandle subHandle = notifier->subscribe("3", onJobStatusUpdate);

      // Update the subscribed job with an old status.
      notifier->updateJob(job3, api::Job::State::PENDING, "", sd3);

      // Update the subscribed job with a new status.
      notifier->updateJob(job3, api::Job::State::FINISHED, "Exited with non-zero exit code (2)", ut);
   }

   SECTION("Subscribe to all")
   {
      struct Expected
      {
         std::string Id;
         std::string Name;
         api::Job::State Status;
         std::string StatusMessage;
         system::DateTime SubmissionTime;
         system::DateTime UpdateTime;
      };

      system::DateTime ut1;
      sleep(1);
      system::DateTime ut2;
      sleep(1);
      system::DateTime ut3;
      sleep(1);

      // Update 2, 3, 4, 1, 4, 1, 2, 2, 4
      // For readers of this test: It shouldn't be possible for a job to transition from Canceled to Running, this is
      // just a set of non-sensical sample data to ensure the notifier works.
      std::queue<Expected> results;
      results.push({ "2", "Job 2", api::Job::State::CANCELED, "Canceled by user.", sd2, ut1 });
      results.push({ "3", "Job 3", api::Job::State::FINISHED, "", sd3, ut1 });
      results.push({ "4", "Job 4", api::Job::State::PENDING, "Waiting for resources...", sd4, ut1 });
      results.push({ "1", "Job 1", api::Job::State::RUNNING, "Resumed", sd1, ut1 });
      results.push({ "4", "Job 4", api::Job::State::RUNNING, "", sd4, ut2 });
      results.push({ "1", "Job 1", api::Job::State::FINISHED, "Non-zero exit code (127)", sd1, ut2 });
      results.push({ "2", "Job 2", api::Job::State::RUNNING, "", sd2, ut2 });
      results.push({ "2", "Job 2", api::Job::State::FINISHED, "", sd2, ut3 });
      results.push({ "4", "Job 4", api::Job::State::FINISHED, "", sd4, ut3 });

      OnJobStatusUpdate onJobStatusUpdate = [&results](const api::JobPtr& in_job)
      {
         Expected e = results.front();
         results.pop();

         CHECK(in_job->Id == e.Id);
         CHECK(in_job->Name == e.Name);
         CHECK(in_job->Status == e.Status);
         CHECK(in_job->StatusMessage == e.StatusMessage);
         CHECK(in_job->SubmissionTime == e.SubmissionTime);
         CHECK(in_job->LastUpdateTime.getValueOr(system::DateTime()) == e.UpdateTime);
      };

      notifier->updateJob(job2, api::Job::State::CANCELED, "Canceled by user.", ut1);
      notifier->updateJob(job3, api::Job::State::FINISHED, "", ut1);
      notifier->updateJob(job4, api::Job::State::PENDING, "Waiting for resources...", ut1);
      notifier->updateJob(job1, api::Job::State::RUNNING, "Resumed", ut1);
      notifier->updateJob(job4, api::Job::State::RUNNING, "", ut2);
      notifier->updateJob(job1, api::Job::State::FINISHED, "Non-zero exit code (127)", ut2);
      notifier->updateJob(job2, api::Job::State::RUNNING, "", ut2);
      notifier->updateJob(job2, api::Job::State::FINISHED, "", ut3);
      notifier->updateJob(job4, api::Job::State::FINISHED, "", ut3);
   }

}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
