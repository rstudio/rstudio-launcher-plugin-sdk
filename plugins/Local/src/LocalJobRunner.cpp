/*
 * LocalJobRunner.cpp
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

#include <LocalJobRunner.hpp>

#include <boost/algorithm/string.hpp>

#include <system/Crypto.hpp>
#include <system/Process.hpp>

#include <LocalConstants.hpp>
#include <LocalError.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

using ErrorType = api::ErrorResponse::Type;

namespace {

Error decryptPassword(const api::JobPtr& in_job, const std::string& in_key, std::string& out_password)
{
   Optional<std::string> encryptedPasswordOpt = in_job->getJobConfigValue(s_encryptedPassword);
   if (!encryptedPasswordOpt)
   {
      std::string encryptedPassword = encryptedPasswordOpt.getValueOr("");

      Optional<std::string> initVectorOpt = in_job->getJobConfigValue(s_initializationVector);
      if (!initVectorOpt)
         return createError(
            LocalError::INVALID_JOB_CONFIG,
            "required field 'initializationVector' missing",
            ERROR_LOCATION);

      std::string iv = initVectorOpt.getValueOr("");
      if (iv.size() < 8)
         return createError(
            LocalError::INVALID_JOB_CONFIG,
            "required field 'initializationVector' is too short - must be at least 8 bytes",
            ERROR_LOCATION);

      Error error = system::crypto::decryptAndBase64Decode(encryptedPassword, in_key, iv, out_password);
      if (error)
         return createError(
            LocalError::INVALID_JOB_CONFIG,
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

LocalJobRunner::LocalJobRunner(const std::string& in_hostname) :
   m_hostname(in_hostname)
{
}

Error LocalJobRunner::initialize()
{
   return m_secureCookie.initialize();
}

Error LocalJobRunner::runJob(api::JobPtr& io_job, api::ErrorResponse::Type& out_errorType)
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
