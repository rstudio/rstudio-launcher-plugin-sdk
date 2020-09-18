/*
 * LocalError.hpp
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


#ifndef LAUNCHER_PLUGINS_LOCAL_ERROR_HPP
#define LAUNCHER_PLUGINS_LOCAL_ERROR_HPP

#include <string>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace local {

const constexpr char* s_errorName = "LocalPluginError";

/**
 * @enum LocalError
 * @brief Represents the possible Local Launcher Plugin specific errors that may occur.
 */
enum class LocalError
{
   /** No error. */
   SUCCESS              = 0,

   /** The specified mount type isn't supported. */
   INVALID_MOUNT_TYPE   = 1,

   /** The specified job config isn't supported. */
   INVALID_JOB_CONFIG   = 2,

   /** The job failed to launch. */
   JOB_LAUNCH_ERROR     = 3,

   /** An operation was attempted that requires the job's PID, but the job does not have a PID. */
   NO_PID               = 4,

   /** The operation is not supported. */
   UNSUPPORTED_OP       = 5,
};

/**
 * @brief Creates a LocalError.
 *
 * @param in_code               The error code.
 * @param in_message            The error message.
 * @param in_cause              The cause of the error.
 * @param in_errorLocation      The location at which the error occurred.
 *
 * @return The newly created LocalError object.
 */
Error createError(
   LocalError in_code,
   const std::string& in_message,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Creates a LocalError.
 *
 * @param in_code               The error code.
 * @param in_message            The error message.
 * @param in_errorLocation      The location at which the error occurred.
 *
 * @return The newly created LocalError object.
 */
Error createError(LocalError in_code, const std::string& in_message, const ErrorLocation& in_errorLocation);


/**
 * @brief Creates a LocalError.
 *
 * @param in_code               The error code.
 * @param in_cause              The cause of the error.
 * @param in_errorLocation      The location at which the error occurred.
 *
 * @return The newly created LocalError object.
 */
Error createError(LocalError in_code, const Error& in_cause, const ErrorLocation& in_errorLocation);


/**
 * @brief Creates a LocalError.
 *
 * @param in_code               The error code.
 * @param in_errorLocation      The location at which the error occurred.
 *
 * @return The newly created LocalError object.
 */
Error createError(LocalError in_code, const ErrorLocation& in_errorLocation);

} // namespace local
} // namespace launcher_plugins
} // namespace rstudio

#endif
