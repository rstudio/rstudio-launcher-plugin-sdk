/*
 * ErrorUtils.hpp
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

#ifndef LAUNCHER_PLUGINS_ERROR_UTILS_HPP
#define LAUNCHER_PLUGINS_ERROR_UTILS_HPP

#include <boost/system/error_code.hpp>

#include <Error.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace utils {

/**
 * @brief Utility function to convert from boost::error_code to the Error class.
 *
 * @param in_ec                 The error code to convert.
 * @param in_message            The custom error message.
 * @param in_cause              The error which caused this error.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_code& in_ec,
   const std::string& in_message,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Utility function to convert from boost::error_condition to the Error class.
 *
 * @param in_ec                 The error condition to convert.
 * @param in_message            The custom error message.
 * @param in_cause              The error which caused this error.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_condition& in_ec,
   const std::string& in_message,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Utility function to convert from boost::error_code to the Error class.
 *
 * @param in_ec                 The error code to convert.
 * @param in_cause              The error which caused this error.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_code& in_ec,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation);
/**
 * @brief Utility function to convert from boost::error_condition to the Error class.
 *
 * @param in_ec                 The error condition to convert.
 * @param in_cause              The error which caused this error.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_condition& in_ec,
   const Error& in_cause,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Utility function to convert from boost::error_code to the Error class.
 *
 * @param in_ec                 The error code to convert.
 * @param in_message            The custom error message.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_code& in_ec,
   const std::string& in_message,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Utility function to convert from boost::error_condition to the Error class.
 *
 * @param in_ec                 The error condition to convert.
 * @param in_message            The custom error message.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_condition& in_ec,
   const std::string& in_message,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Utility function to convert from boost::error_code to the Error class.
 *
 * @param in_ec                 The error code to convert.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_code& in_ec,
   const ErrorLocation& in_errorLocation);

/**
 * @brief Utility function to convert from boost::error_condition to the Error class.
 *
 * @param in_ec                 The error condition to convert.
 * @param in_errorLocation      The location at which this error occurred.
 *
 * @return The converted error.
 */
Error createErrorFromBoostError(
   const boost::system::error_condition& in_ec,
   const ErrorLocation& in_errorLocation);

} // namespace utils
} // namespace launcher_plugins
} // namespace rstudio

#endif
