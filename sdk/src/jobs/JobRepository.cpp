/*
 * JobRepository.cpp
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

#include <jobs/JobRepository.hpp>

#include <map>

#include <Error.hpp>
#include "../system/ReaderWriterMutex.hpp"

using namespace rstudio::launcher_plugins::api;

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

struct JobRepository::Impl
{
   system::ReaderWriterMutex Mutex;
   std::map<std::string, JobPtr> JobMap;
};

PRIVATE_IMPL_DELETER_IMPL(JobRepository)

JobRepository::JobRepository() :
   m_impl(new Impl())
{
}

void JobRepository::addJob(JobPtr in_job)
{
   WRITE_LOCK_BEGIN(m_impl->Mutex)

   auto itr = m_impl->JobMap.find(in_job->Id);
   if (itr == m_impl->JobMap.end())
      m_impl->JobMap[in_job->Id] = in_job;

   RW_LOCK_END(true)
}

JobPtr JobRepository::getJob(const std::string& in_jobId, const system::User& in_user) const
{
   READ_LOCK_BEGIN(m_impl->Mutex)

   auto itr = m_impl->JobMap.find(in_jobId);
   if ((itr != m_impl->JobMap.end()) &&
      (in_user.isAllUsers() || (itr->second->User == in_user)))
      return itr->second;

   RW_LOCK_END(true)

   return JobPtr();
}

JobList JobRepository::getJobs(const system::User& in_user) const
{
   JobList jobs;

   READ_LOCK_BEGIN(m_impl->Mutex)

   // Get all values if this request has admin privileges.
   const auto end = m_impl->JobMap.end();
   if (in_user.isAllUsers())
      for (auto itr = m_impl->JobMap.begin(); itr != end; ++itr)
         jobs.push_back(itr->second);
   else
   {
      for (auto itr = m_impl->JobMap.begin(); itr != end; ++itr)
      {
         if (itr->second->User == in_user)
            jobs.push_back(itr->second);
      }
   }

   RW_LOCK_END(true)

   return jobs;
}

void JobRepository::removeJob(const std::string& in_jobId)
{
   WRITE_LOCK_BEGIN(m_impl->Mutex)

   auto itr = m_impl->JobMap.find(in_jobId);
   if (itr != m_impl->JobMap.end())
   {
      // Keep the lock while invoking the inheriting class impl.
      onJobRemoved(itr->second);
      m_impl->JobMap.erase(itr);
   }

   RW_LOCK_END(true)
}

void JobRepository::onJobRemoved(JobPtr)
{
   // Do nothing.
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
