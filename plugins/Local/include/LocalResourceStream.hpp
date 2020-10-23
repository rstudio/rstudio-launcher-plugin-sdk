/*
 * LocalResourceStream.hpp
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

#ifndef LAUNCHER_PLUGINS_LOCAL_RESOURCE_STREAM_HPP
#define LAUNCHER_PLUGINS_LOCAL_RESOURCE_STREAM_HPP

#include <api/stream/AbstractTimedResourceStream.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

class LocalResourceStream : public api::AbstractTimedResourceStream
{
public:
   /**
    * @brief Constructor.
    * 
    * @param in_frequency              The frequency at which job resource utilization metrics should be polled.
    * @param in_job                    The job for which resource utilization metrics should be streamed.
    * @param in_launcherCommunicator   The communicator through which messages may be sent to the launcher.
    */
   LocalResourceStream(
      system::TimeDuration in_frequency,
      const api::ConstJobPtr& in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

private:
   /**
    * @brief Gets the percent of CPU usage of the process and all its children in the time between the last measurement
    *        and now.
    * 
    * @param out_cpuPercent      The percent of CPU usage, on Success.
    * 
    * @return Success if the CPU usage percent could be measured; the Error that occurred otherwise.
    */
   Error getCpuPercent(double& out_cpuPercent);

   /**
    * @brief Gets the total elapsed CPU time of the process and all its children in seconds.
    * 
    * @param out_cpuPercent      The total elapsed CPU Time of the process in seconds, on Success.
    * 
    * @return Success if the CPU Time could be measured; the Error that occurred otherwise.
    */
   Error getCpuSeconds(double& out_cpuTime);

   /**
    * @brief Gets the current physical and virtual memory usage of the process and all its children in MB.
    * 
    * @param out_physMem      The total physical memory in use by the process in MB, on Success.
    * @param out_virtMem      The total virtual memory in use by the process in MB, on Success.
    * 
    * @return Success if both the physical and virutal memory could be measured; the Error that occurred otherwise.
    */
   Error getMem(double& out_physMem, double& out_virtMem);

   /**
    * @brief This method will be invoked when initialized is called on the base class, allowing the inheriting class to 
    *        optionally do any initialization steps necessarfy.
    * 
    * @return Success if the inheriting class initialized correctly; the Error that occurred otherwise.
    */
   Error onInitialize() override;

   /**
    * @brief Polls resource utilization of the job.
    * 
    * This method will be invoked once every configured interval.
    * 
    * @param out_data      The current resource utilization data of the job.
    * 
    * @return Error if it was not possible to retrieve resource utilization for any reason; Success otherwise.
    */
   Error pollResourceUtilData(api::ResourceUtilData& out_data) override;

   // The PID of the process of the job.
   pid_t m_pid;

   // The last observed number of system ticks. Used to calculate CPU Percent.
   clock_t m_lastSysTicks;

   // The last observed number process ticks. Used to calculate CPU Percent.
   clock_t m_lastProcTicks;

   // The number of clock ticks per second. Used to calculate CPU Time.
   const double m_clockTicksPerSecond;

   // The number of bytes per page of memory. Used to calculate Physical and Virtual memory MB.
   const double m_bytesPerPage;
};

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
