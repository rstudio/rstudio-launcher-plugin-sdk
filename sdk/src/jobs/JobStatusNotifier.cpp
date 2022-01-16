/*
 * JobStatusNotifier.cpp
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


#include <jobs/JobStatusNotifier.hpp>

#include <memory>
#include <mutex>

#include <boost/signals2.hpp>

#include <utils/MutexUtils.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

namespace {

// Convenience typedefs for self.
typedef std::shared_ptr<JobStatusNotifier> SharedThis;
typedef std::shared_ptr<JobStatusNotifier> Parent;
typedef std::weak_ptr<JobStatusNotifier> WeakParent;

// Convenience typedefs for signals.
typedef boost::signals2::connection Connection;
typedef boost::signals2::signal<void(api::JobPtr)> Signal;
typedef std::map<std::string, Signal> SignalMap;

} // anonymous namespace

/**
 * @brief JobStatusNotifier implementation.
 */
struct JobStatusNotifier::Impl
{
   /** Signal to be used when something subscribes to all jobs. */
   Signal AllJobsSignal;

   /** Map of every signal per job. */
   SignalMap JobSignalMap;

   /** Mutex to protect map access. */
   std::recursive_mutex Mutex;
};

PRIVATE_IMPL_DELETER_IMPL(JobStatusNotifier)

/**
 * @brief A class which represents a subscription to one or more jobs.
 */
struct Subscription
{
   /**
    * @brief Constructor.
    *
    * @param in_parent          The JobStatusNotifier which created this subscription.
    * @param in_jobId           The ID of the job  for which this subscription was created.
    * @param in_connection      The boost signal connection.
    */
   Subscription(WeakParent in_parent, std::string in_jobId, Connection in_connection) :
      m_parent(std::move(in_parent)),
      m_jobId(std::move(in_jobId)),
      m_connection(std::move(in_connection))
   {
   }

   /**
    * @brief Destructor.
    */
   ~Subscription()
   {
      if (Parent parent = m_parent.lock())
      {
         m_connection.disconnect();

         if (!m_jobId.empty())
         {
            UNIQUE_LOCK_RECURSIVE_MUTEX(parent->m_impl->Mutex)
            {
               // Check if this is the last subscription to m_jobId. If so, remove the entry from the map.
               auto itr = parent->m_impl->JobSignalMap.find(m_jobId);
               if (itr != parent->m_impl->JobSignalMap.end() && itr->second.empty())
               {
                  parent->m_impl->JobSignalMap.erase(itr);
               }
            }
            END_LOCK_MUTEX
         }
      }
   }

private:
   /** The JobStatusNotifier which created this subscription. */
   WeakParent m_parent;

   /** The ID of the job  for which this subscription was created. */
   std::string m_jobId;

   /** The boost signal connection. */
   Connection m_connection;
};

JobStatusNotifier::JobStatusNotifier() :
   m_impl(new Impl())
{
}

SubscriptionHandle JobStatusNotifier::subscribe(const OnJobStatusUpdate& in_onJobStatusUpdate)
{
   return std::make_shared<Subscription>(
      shared_from_this(),
      "",
      m_impl->AllJobsSignal.connect(in_onJobStatusUpdate));
}

SubscriptionHandle JobStatusNotifier::subscribe(
   const std::string& in_jobId,
   const OnJobStatusUpdate& in_onJobStatusUpdate)
{
   // If for some reason the job ID is empty, treat it like subscribe all. If the job Id is *, also treat it like
   // subscribe all.
   if (in_jobId.empty() || in_jobId == "*")
      return subscribe(in_onJobStatusUpdate);

   Connection connection;

   // Get a connection. This requires accessing the map, so hold the lock.
   UNIQUE_LOCK_RECURSIVE_MUTEX(m_impl->Mutex)
   {
      auto itr = m_impl->JobSignalMap.find(in_jobId);
      if (itr == m_impl->JobSignalMap.end())
      {
         // If there's no entry for in_jobId yet, make one.
         auto signalPair = m_impl->JobSignalMap.insert(std::make_pair(in_jobId, Signal()));
         if (signalPair.second) // Check that insertion was successful. Should always be true because we have the lock.
            connection = signalPair.first->second.connect(in_onJobStatusUpdate);
      }
      else
         connection = itr->second.connect(in_onJobStatusUpdate);
   }
   END_LOCK_MUTEX

   return std::make_shared<Subscription>(shared_from_this(), in_jobId, connection);
}

void JobStatusNotifier::updateJob(
   const api::JobPtr& in_job,
   api::Job::State in_newStatus,
   const std::string& in_statusMessage,
   const system::DateTime& in_invocationTime)
{
   LOCK_JOB(in_job)
   {
      // Do nothing if the job has a newer status than this status.
      if (in_job->LastUpdateTime &&
         (in_job->LastUpdateTime.getValueOr(system::DateTime()) >= in_invocationTime))
         return;

      bool notify = (in_job->Status != in_newStatus) || (in_job->StatusMessage != in_statusMessage);

      in_job->LastUpdateTime = in_invocationTime;
      in_job->Status = in_newStatus;
      in_job->StatusMessage = in_statusMessage;

      // If there was a meaningful change to the job, notify the listeners.
      if (notify)
      {
         m_impl->AllJobsSignal(in_job);

         UNIQUE_LOCK_RECURSIVE_MUTEX(m_impl->Mutex)
         {
            auto itr = m_impl->JobSignalMap.find(in_job->Id);
            if (itr != m_impl->JobSignalMap.end())
               itr->second(in_job);
         }
         END_LOCK_MUTEX
      }
   }
   END_LOCK_JOB
}

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio
