/*
 * JobRepositoryTests.cpp
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

#include <Error.hpp>
#include <jobs/JobRepository.hpp>
#include <system/User.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

namespace {

constexpr const char* USER_ONE = "rlpstestusrone";
constexpr const char* USER_TWO = "rlpstestusrtwo";

inline bool isEqual(const api::JobPtr& in_lhs, const api::JobPtr& in_rhs)
{
   if (in_lhs == in_rhs)
      return true;

   if ((in_lhs == nullptr) || (in_rhs == nullptr))
      return false;

   // For this case, there shouldn't be copies of the jobs, they should be actually the same.
   return in_lhs.get() == in_rhs.get();
}

inline bool isEqual(const api::JobList& in_lhs, const api::JobList& in_rhs)
{
   if (in_lhs.size() != in_rhs.size())
      return false;

   for (auto lhItr = in_lhs.begin(), rhItr = in_rhs.begin(), end = in_lhs.end(); lhItr != end; ++lhItr, ++rhItr)
   {
      if (!isEqual(*lhItr, *rhItr))
         return false;
   }

   return true;
}

} // anonymous namespace

TEST_CASE("One job")
{
   system::User user1, user2, allUsers;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_ONE, user1));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_TWO, user2));

   api::JobPtr job(new api::Job());
   job->Name = "Job Name";
   job->Id = "341";
   job->User = user1;

   JobRepository repo;
   repo.addJob(job);

   SECTION("Get job for correct user")
   {
      CHECK(isEqual(repo.getJob(job->Id, user1), job));
   }

   SECTION("Get job with admin privileges")
   {
      CHECK(isEqual(repo.getJob(job->Id, allUsers), job));
   }

   SECTION("Get job for wrong user")
   {
      CHECK(isEqual(repo.getJob(job->Id, user2), nullptr));
   }

   SECTION("Get non-existent job")
   {
      CHECK(isEqual(repo.getJob("340", allUsers), nullptr));
      CHECK(isEqual(repo.getJob("340", user1), nullptr));
   }

   SECTION("Remove job")
   {
      repo.removeJob(job->Id);
      CHECK(isEqual(repo.getJob(job->Id, user1), nullptr));
   }
}

TEST_CASE("Multiple jobs")
{
   system::User user1, user2, allUsers;
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_ONE, user1));
   REQUIRE_FALSE(system::User::getUserFromIdentifier(USER_TWO, user2));

   api::JobPtr job1(new api::Job()),
      job2(new api::Job()),
      job3(new api::Job()),
      job4(new api::Job()),
      job5(new api::Job());

   job1->Id = "341";
   job1->User = user1;

   job2->Id = "342";
   job2->User = user2;

   job3->Id = "345";
   job3->User = user2;

   job4->Id = "344";
   job4->User = user1;

   job5->Id = "343";
   job5->User = user2;

   JobRepository repo;
   repo.addJob(job1);
   repo.addJob(job2);
   repo.addJob(job3);
   repo.addJob(job4);
   repo.addJob(job5);

   SECTION("User one jobs")
   {
      api::JobList expected;
      expected.push_back(job1);
      expected.push_back(job4);

      CHECK(isEqual(repo.getJob(job1->Id, user1), job1));
      CHECK(isEqual(repo.getJob(job2->Id, user1), nullptr));
      CHECK(isEqual(repo.getJob(job3->Id, user1), nullptr));
      CHECK(isEqual(repo.getJob(job4->Id, user1), job4));
      CHECK(isEqual(repo.getJob(job5->Id, user1), nullptr));
      CHECK(isEqual(repo.getJobs(user1), expected));
   }

   SECTION("User two jobs")
   {
      api::JobList expected;
      expected.push_back(job2);
      expected.push_back(job5);
      expected.push_back(job3);

      CHECK(isEqual(repo.getJob(job1->Id, user2), nullptr));
      CHECK(isEqual(repo.getJob(job2->Id, user2), job2));
      CHECK(isEqual(repo.getJob(job3->Id, user2), job3));
      CHECK(isEqual(repo.getJob(job4->Id, user2), nullptr));
      CHECK(isEqual(repo.getJob(job5->Id, user2), job5));
      CHECK(isEqual(repo.getJobs(user2), expected));
   }

   SECTION("All user jobs")
   {
      api::JobList expected;
      expected.push_back(job1);
      expected.push_back(job2);
      expected.push_back(job5);
      expected.push_back(job4);
      expected.push_back(job3);

      CHECK(isEqual(repo.getJob(job1->Id, allUsers), job1));
      CHECK(isEqual(repo.getJob(job2->Id, allUsers), job2));
      CHECK(isEqual(repo.getJob(job3->Id, allUsers), job3));
      CHECK(isEqual(repo.getJob(job4->Id, allUsers), job4));
      CHECK(isEqual(repo.getJob(job5->Id, allUsers), job5));
      CHECK(isEqual(repo.getJobs(allUsers), expected));
   }

   SECTION("Remove a job")
   {
      api::JobList expected;
      expected.push_back(job2);
      expected.push_back(job3);

      repo.removeJob(job5->Id);

      CHECK(isEqual(repo.getJob(job1->Id, user2), nullptr));
      CHECK(isEqual(repo.getJob(job2->Id, user2), job2));
      CHECK(isEqual(repo.getJob(job3->Id, user2), job3));
      CHECK(isEqual(repo.getJob(job4->Id, user2), nullptr));
      CHECK(isEqual(repo.getJob(job5->Id, user2), nullptr));
      CHECK(isEqual(repo.getJobs(user2), expected));
   }
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
