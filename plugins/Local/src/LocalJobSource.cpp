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

#include <boost/algorithm/string.hpp>

#include <Error.hpp>
#include <system/Crypto.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

using ErrorType = api::ErrorResponse::Type;

namespace {

/** Constants used by the Local Plugin. */
const constexpr char* s_errorName = "LocalPluginError";
const constexpr char* s_pamProfile = "pamProfile";
const constexpr char* s_encryptedPassword = "encryptedPassword";
const constexpr char* s_initializationVector = "s_initializationVector";

enum class ErrorCode
{
   SUCCESS              = 0,
   INVALID_MOUNT_TYPE   = 1,
   INVALID_JOB_CONFIG   = 2,
};

Error createError(
   ErrorCode in_code,
   const std::string& in_message,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation)
{
   return Error(s_errorName, static_cast<int>(in_code), in_message, in_cause, in_errorLocation);
}

Error createError(ErrorCode in_code, const std::string& in_message, const ErrorLocation& in_errorLocation)
{
   return Error(s_errorName, static_cast<int>(in_code), in_message, in_errorLocation);
}

Error createError(ErrorCode in_code, const Error& in_cause, const ErrorLocation& in_errorLocation)
{
   return Error(s_errorName, static_cast<int>(in_code), in_cause, in_errorLocation);
}

Error createError(ErrorCode in_code, const ErrorLocation& in_errorLocation)
{
   return Error(s_errorName, static_cast<int>(in_code), in_errorLocation);
}

Error decryptPassword(const api::JobPtr& in_job, const std::string& in_key, std::string& out_password)
{
   Optional<std::string> encryptedPasswordOpt = in_job->getJobConfigValue(s_encryptedPassword);
   if (!encryptedPasswordOpt)
   {
      std::string encryptedPassword = encryptedPasswordOpt.getValueOr("");

      Optional<std::string> initVectorOpt = in_job->getJobConfigValue(s_initializationVector);
      if (!initVectorOpt)
         return createError(
            ErrorCode::INVALID_JOB_CONFIG,
            "required field 'initializationVector' missing",
            ERROR_LOCATION);

      std::string iv = initVectorOpt.getValueOr("");
      if (iv.size() < 8)
         return createError(
            ErrorCode::INVALID_JOB_CONFIG,
            "required field 'initializationVector' is too short - must be at least 8 bytes",
            ERROR_LOCATION);

      Error error = system::crypto::decryptAndBase64Decode(encryptedPassword, in_key, iv, out_password);
      if (error)
         return createError(
            ErrorCode::INVALID_JOB_CONFIG,
            "'encryptedPassword' field or 'initializationVector' field has invalid format",
            error,
            ERROR_LOCATION);
   }

   return Success();
}

Error generateJobId(std::string& out_id)
{
   // The ID just needs to be unique, so generate some random data and then base-64 encode it so it's writable to file
   // and to be used in a file name.
   std::vector<unsigned char> randomData;
   Error error = system::crypto::random(16, randomData);
   if (error)
      return error;

   std::string id;
   error = system::crypto::base64Encode(randomData, id);
   if (error)
      return error;

   // Don't allow / in the ID, as it will be used as part of a file name.
   out_id = boost::replace_all_copy(id, "/", "-");
   return Success();
}

} // anonymous namespace

LocalJobSource::LocalJobSource(
   std::string in_hostname,
   jobs::JobRepositoryPtr in_jobRepository,
   jobs::JobStatusNotifierPtr in_jobStatusNotifier) :
      api::IJobSource(std::move(in_jobRepository), std::move(in_jobStatusNotifier)),
      m_hostname(std::move(in_hostname)),
      m_jobStorage(m_hostname)
{
}

Error LocalJobSource::initialize()
{
   // TODO: Initialize communications with the other local plugins, if any, and make sure we can read and write to the
   //       file that will store job information.
   Error error = m_secureCookie.initialize();
   if (error)
      return error;

   return m_jobStorage.initialize();
}

Error LocalJobSource::getConfiguration(const system::User&, api::JobSourceConfiguration& out_configuration) const
{
   static const api::JobConfig::Type& strType = api::JobConfig::Type::STRING;

   out_configuration.CustomConfig.emplace_back(s_pamProfile, strType);
   out_configuration.CustomConfig.emplace_back(s_encryptedPassword, strType);
   out_configuration.CustomConfig.emplace_back(s_initializationVector, strType);

   return Success();
}

Error LocalJobSource::getJobs(api::JobList& out_jobs) const
{
   return m_jobStorage.loadJobs(out_jobs);
}

Error LocalJobSource::submitJob(api::JobPtr io_job, ErrorType& out_errorType) const
{
   // Give the job an ID.
   Error error = generateJobId(io_job->Id);
   if (error)
      return error;

   // Set the submission time and the hostname.
   io_job->SubmissionTime = system::DateTime();
   io_job->Host = m_hostname;

   std::string pamProfile = io_job->getJobConfigValue(s_pamProfile).getValueOr("");
   if (!pamProfile.empty())
   {
      std::string password;
      error = decryptPassword(io_job, m_secureCookie.getKey(), password);
      if (error)
      {
         out_errorType = ErrorType::INVALID_REQUEST;
         return error;
      }
   }

   return Success();
}

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio
