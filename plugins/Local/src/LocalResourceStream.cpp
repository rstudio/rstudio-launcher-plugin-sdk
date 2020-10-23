/*
 * LocalResourceStream.cpp
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

#include <LocalResourceStream.hpp>

#include <set>
#include <unistd.h>
#include <sys/times.h>

#include <boost/algorithm/string.hpp>

#include <SafeConvert.hpp>
#include <api/Job.hpp>
#include <system/FilePath.hpp>
#include <system/Process.hpp>
#include <utils/FileUtils.hpp>

#include <LocalError.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

namespace {

// These values come from the proc man page documentation here: https://man7.org/linux/man-pages/man5/proc.5.html
// For specific details see the /proc/[pid]/statm section about s_virtMemField and s_physMemField and the
// /proc/[pid]/stat section about s_userProcTicksField and s_sysProcTicksField.
constexpr size_t s_virtMemField = 0;
constexpr size_t s_physMemField = 1;
constexpr size_t s_userProcTicksField = 13;
constexpr size_t s_sysProcTicksField = 14;

system::FilePath getStatRootPath(pid_t in_pid)
{
   return system::FilePath("/proc").completeChildPath(std::to_string(in_pid));
}

Error getChildPids(std::set<pid_t>& io_pids)
{
   std::vector<system::process::ProcessInfo> children;
   Error error = system::process::getChildProcesses(*io_pids.begin(), children);
   if (error)
      return error;

   for (const auto& child: children)
      io_pids.insert(child.Pid);

   return Success();
}

Error readStatFile(const system::FilePath& in_statFile, std::vector<std::string>& out_fields)
{
   if (!in_statFile.exists())
      return system::fileNotFoundError(in_statFile, ERROR_LOCATION);

   std::string contents;
   Error error = utils::readFileIntoString(in_statFile, contents);
   if (error)
      return error;

   boost::algorithm::split(out_fields, contents, boost::is_any_of(" "));

   for (auto itr = out_fields.begin(); itr != out_fields.end();)
   {
      if (boost::trim_copy(*itr).empty())
         itr = out_fields.erase(itr);
      else
         ++itr;      
   }

   return Success();
}

Error readStatFields(const std::vector<std::string>& in_statFields, size_t in_index, std::string& out_value)
{
   if (in_index >= in_statFields.size())
      return systemError(ESPIPE, "stat output did not contain the requested field", ERROR_LOCATION);

   out_value = in_statFields[in_index];
   return Success();
}

template <typename ...Args>
Error readStatFields(
   const std::vector<std::string>& in_statFields,
   size_t in_index,
   std::string& out_value,
   Args... in_args)
{
   Error error = readStatFields(in_statFields, in_index, out_value);
   if (error)
      return error;

   return readStatFields(in_statFields, in_args...);
}

template <typename ...Args>
Error readStatFields(
   const system::FilePath& in_statFile,
   Args... in_args)
{
   std::vector<std::string> fields;
   Error error = readStatFile(in_statFile, fields);
   if (error)
      return error;

   return readStatFields(fields, in_args...);
}

Error getProcessTicks(pid_t in_rootPid, clock_t& out_ticks)
{
   std::set<pid_t> pids = { in_rootPid };
   Error error = getChildPids(pids);
   if (error)
      return error;

   out_ticks = 0;
   for (pid_t pid: pids)
   {
      std::string userTicksStr;
      std::string sysTicksStr;

      system::FilePath statFile = getStatRootPath(pid).completeChildPath("stat");
      Error error = readStatFields(
         statFile,
         s_userProcTicksField, userTicksStr,
         s_sysProcTicksField, sysTicksStr);
      if (error)
      {
         // If the root process has exited, there's nothing to track so return an error.
         // Otherwise skip the exited child process - it's no longer consuming resources.
         if (pid == in_rootPid)
            return error;
      }
      else
      {
         out_ticks += safe_convert::stringTo<clock_t>(userTicksStr, 0);
         out_ticks += safe_convert::stringTo<clock_t>(sysTicksStr, 0);
      }
   }

   return Success();
}

clock_t getSystemTicks()
{
   // The tms variable won't be used but the code is more portable if we don't pass NULL to times();
   // See man page on times() for more details.
   struct tms ticks;
   return times(&ticks);
}

} // anonymous namespace

LocalResourceStream::LocalResourceStream(
   system::TimeDuration in_frequency,
   const api::ConstJobPtr& in_job,
   comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
      api::AbstractTimedResourceStream(in_frequency, in_job, in_launcherCommunicator),
      m_clockTicksPerSecond(sysconf(_SC_CLK_TCK)),
      m_bytesPerPage(sysconf(_SC_PAGESIZE))
{
}

Error LocalResourceStream::onInitialize()
{
   // We really just need the job lock here, but to be safe and avoid a possible deadlock scenario, acquire the base 
   // class' mutex first.
   LOCK_MUTEX(m_mutex)
   {
      LOCK_JOB(m_job)
      {
         if (!m_job->Pid)
         {
            return createError(
               LocalError::NO_PID,
               "Resource Utilization Metrics cannot be streamed for job " + m_job->Id + " because it does not have a PID.",
               ERROR_LOCATION);
         }

         m_pid = m_job->Pid.getValueOr(0);

         return Success();
      }
      END_LOCK_JOB
   }
   END_LOCK_MUTEX

   return unknownError(
      "An unknown error occurred while attempting to initialize the Resource Utilization Stream for job " + m_job->Id + ".",
      ERROR_LOCATION);
}

Error LocalResourceStream::getCpuPercent(double& out_cpuPercent)
{
   // Iterate a maximum of 10 times to avoid locking the CPU.
   for (int count = 0; count < 10; ++count)
   {
      clock_t procTicks = 0;
      Error error = getProcessTicks(m_pid, procTicks);
      if (error)
         return error;

      clock_t sysTicks = getSystemTicks();

      if ((procTicks < m_lastProcTicks) || (sysTicks <= m_lastSysTicks))
      {
         // If the currently measured ticks are less than the previously measured ticks the values have wrapped around
         // and we can't give a reliable measurement. Just try again.
         // Additionally, if the system ticks haven't changed we'd divide by zero, so wait a bit longer before posting
         // the next measurment.
         m_lastProcTicks = procTicks;
         m_lastSysTicks = sysTicks;
      }
      else
      {
         double procTickChange = static_cast<double>(procTicks) - static_cast<double>(m_lastProcTicks);
         double sysTickChange = static_cast<double>(sysTicks) - static_cast<double>(m_lastSysTicks);

         out_cpuPercent = (procTickChange / sysTickChange) * 100.0;

         m_lastProcTicks = procTicks;
         m_lastSysTicks = sysTicks;
         return Success();
      }      
   }

   return systemError(ETIMEDOUT, "Timed out while measuring CPU Time", ERROR_LOCATION);
}

Error LocalResourceStream::getCpuSeconds(double& out_cpuTime)
{
   clock_t procTicks = 0;
   Error error = getProcessTicks(m_pid, procTicks);
   if (error)
      return error;

   // Calculate seconds by dividing the number of ticks by the number of ticks per second.
   out_cpuTime = static_cast<double>(procTicks) / m_clockTicksPerSecond;

   return Success();
}

Error LocalResourceStream::getMem(double& out_memPhysical, double& out_memVirtual)
{
   std::set<pid_t> pids = { m_pid };
   Error error = getChildPids(pids);
   if (error)
      return error;

   out_memPhysical = 0.0;
   out_memVirtual = 0.0;

   for (pid_t pid: pids)
   {
      system::FilePath statFile = getStatRootPath(pid).completeChildPath("statm");

      // Get the number of pages of each type of memory.
      std::string physicalPageCount, virtualPageCount;
      Error error = readStatFields(
         statFile,
         s_physMemField, physicalPageCount,
         s_virtMemField, virtualPageCount);

      if (error)
      {
         // If the root process has exited, there's nothing to track so return an error.
         // Otherwise skip the exited child process - it's no longer consuming resources.
         if (pid == m_pid)
            return error;
      }
      else
      {
         // Get the memory values in MB by calculating the total number of bytes (the number of pages multiplied by the 
         // number of bytes in a page), and then dividing that by 1 million to convert from bytes to MB.
         out_memPhysical += (safe_convert::stringTo<double>(physicalPageCount, 0) * m_bytesPerPage) / 1000000.0;
         out_memVirtual += (safe_convert::stringTo<double>(virtualPageCount, 0) * m_bytesPerPage) / 1000000.0;
      }
   }

   return Success();
}

Error LocalResourceStream::pollResourceUtilData(api::ResourceUtilData& out_data)
{
   double cpuPercent, cpuTime, physMem, virtMem;
   Error error = getCpuPercent(cpuPercent);
   if (error)
      return error;
   
   error = getCpuSeconds(cpuTime);
   if (error)
      return error;
   
   error = getMem(physMem, virtMem);
   if (error)
      return error;

   out_data.CpuPercent = cpuPercent;
   out_data.CpuSeconds = cpuTime;
   out_data.ResidentMem = physMem;
   out_data.VirtualMem = virtMem;

   return Success();
}


} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
