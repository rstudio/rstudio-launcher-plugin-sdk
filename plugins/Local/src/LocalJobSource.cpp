/*
 * LocalJobSource.cpp
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

#include <LocalJobSource.hpp>

#include <signal.h>

#include <Error.hpp>
#include <api/stream/FileOutputStream.hpp>
#include <system/PosixSystem.hpp>
#include <system/Process.hpp>

#include <LocalConstants.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

bool signalJob(
   const std::string& in_jobId,
   const Optional<int> in_pid,
   int in_signal,
   const std::string& in_messageDetail,
   std::string& out_message)
{
   if (!in_pid)
   {
      out_message = "Cannot " + in_messageDetail + " job " + in_jobId + " because it does not have a PID.";
      return false;
   }

   // It's safe to pass 0 to `getValueOr` here since we would have exited earlier if the PID was an empty optional.
   Error error = system::process::signalProcess(in_pid.getValueOr(0), in_signal);
   if (error)
   {
      out_message = "Failed to " + in_messageDetail + " job " + in_jobId;
      logging::logErrorMessage(out_message + ": " + error.asString(), ERROR_LOCATION);
   }

   return !error;
}

LocalJobSource::LocalJobSource(
   std::string in_hostname,
   jobs::JobStatusNotifierPtr in_jobStatusNotifier,
   std::shared_ptr<LocalJobRepository> in_jobRepository) :
      api::IJobSource(in_jobRepository, std::move(in_jobStatusNotifier)),
      m_hostname(std::move(in_hostname))
{
   m_jobRunner.reset(new LocalJobRunner(m_hostname, m_jobStatusNotifier, in_jobRepository));
}

Error LocalJobSource::initialize()
{
   // TODO: Initialize communications with the other local plugins, if any.
   return m_jobRunner->initialize();
}

bool LocalJobSource::cancelJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage)
{
   out_isComplete = false;
   out_statusMessage = "The RStudio Local Launcher Plugin does not support canceling jobs.";
   return false;
}

Error LocalJobSource::getConfiguration(const system::User&, api::JobSourceConfiguration& out_configuration) const
{
   static const api::JobConfig::Type& strType = api::JobConfig::Type::STRING;

   out_configuration.CustomConfig.emplace_back(s_pamProfile, strType);
   out_configuration.CustomConfig.emplace_back(s_encryptedPassword, strType);
   out_configuration.CustomConfig.emplace_back(s_initializationVector, strType);

   return Success();
}

Error LocalJobSource::getNetworkInfo(api::JobPtr in_job, api::NetworkInfo& out_networkInfo) const
{
   using system::posix::IpAddress;
   std::vector<IpAddress> addresses;
   Error error = system::posix::getIpAddresses(addresses, true);
   if (error)
      return error;

   out_networkInfo.Hostname = in_job->Host;
   for (const IpAddress& addr: addresses)
   {
      // Return all addresses except the loop-back and link local addresses.
      if ((addr.Address.find("127") != 0) &&
         (addr.Address.find("::1") != 0) &&
         (addr.Address.find("%") == std::string::npos))
         out_networkInfo.IpAddresses.push_back(addr.Address);
   }

   return Success();
}

bool LocalJobSource::killJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage)
{
   out_isComplete = signalJob(in_job->Id, in_job->Pid, SIGKILL, "kill", out_statusMessage);
   if (out_isComplete)
      m_jobStatusNotifier->updateJob(in_job, api::Job::State::KILLED);

   return true;
}

bool LocalJobSource::resumeJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage)
{
   out_isComplete = signalJob(in_job->Id, in_job->Pid, SIGCONT, "resume", out_statusMessage);
   if (out_isComplete)
      m_jobStatusNotifier->updateJob(in_job, api::Job::State::RUNNING);

   return true;
}

bool LocalJobSource::stopJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage)
{
   out_isComplete = signalJob(in_job->Id, in_job->Pid, SIGTERM, "stop", out_statusMessage);
   return true;
}

bool LocalJobSource::suspendJob(api::JobPtr in_job, bool& out_isComplete, std::string& out_statusMessage)
{
   out_isComplete = signalJob(in_job->Id, in_job->Pid, SIGSTOP, "suspend", out_statusMessage);
   if (out_isComplete)
      m_jobStatusNotifier->updateJob(in_job, api::Job::State::SUSPENDED);

   return true;
}

Error LocalJobSource::submitJob(api::JobPtr io_job, bool& out_wasInvalidRequest) const
{
   out_wasInvalidRequest = false;
   return m_jobRunner->runJob(io_job, out_wasInvalidRequest);
}

Error LocalJobSource::createOutputStream(
   api::OutputType in_outputType,
   api::JobPtr in_job,
   api::AbstractOutputStream::OnOutput in_onOutput,
   api::AbstractOutputStream::OnComplete in_onComplete,
   api::AbstractOutputStream::OnError in_onError,
   api::OutputStreamPtr& out_outputStream)
{
   out_outputStream.reset(new api::FileOutputStream(in_outputType, in_job, in_onOutput, in_onComplete, in_onError));
   return Success();
}

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
