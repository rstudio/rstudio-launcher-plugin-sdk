/*
 * JobPruner.hpp
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

#ifndef LAUNCHER_PLUGINS_JOB_PRUNER_HPP
#define LAUNCHER_PLUGINS_JOB_PRUNER_HPP

#include <Noncopyable.hpp>
#include <Noninheritable.hpp>

#include <PImpl.hpp>
#include <jobs/JobRepository.hpp>
#include <jobs/JobStatusNotifier.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace jobs {

/**
 * @brief Responsible for pruning expired jobs from the system as long as it is alive.
 */
class JobPruner :
   public Noncopyable,
   public Noninheritable<JobPruner>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_jobRepository       The job repository from which pruned jobs should be removed.
    * @param in_jobStatusNotifier   The job status notifier.
    */
   JobPruner(
      JobRepositoryPtr in_jobRepository,
      JobStatusNotifierPtr in_jobStatusNotifier);

private:
   // The private implementation of JobPruner
   PRIVATE_IMPL_SHARED(m_impl);
};

/** Convenience Typedef. */
typedef std::unique_ptr<JobPruner> JobPrunerPtr;

} // namespace jobs
} // namespace launcher_plugins
} // namespace rstudio

#endif
