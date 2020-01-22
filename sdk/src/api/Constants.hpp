/*
 * Constants.hpp
 *
 * Copyright (C) 2020 by RStudio, Inc.
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

#ifndef LAUNCHER_PLUGINS_CONSTANTS_HPP
#define LAUNCHER_PLUGINS_CONSTANTS_HPP

namespace rstudio {
namespace launcher_plugins {
namespace api {

// The RStudio Launcher Plugin API implemented by this SDK version.
constexpr int API_VERSION_MAJOR = 1;
constexpr int API_VERSION_MINOR = 0;
constexpr int API_VERSION_PATCH = 0;


// Common fields for all requests and responses.
constexpr char const* FIELD_MESSAGE_TYPE = "messageType";
constexpr char const* FIELD_REQUEST_ID = "requestId";

// Common fields for all responses.
constexpr char const* FIELD_RESPONSE_ID = "responseId";

// Bootstrap request and response fields.
constexpr char const* FIELD_VERSION = "version";
constexpr char const* FIELD_VERSION_MAJOR = "major";
constexpr char const* FIELD_VERSION_MINOR = "minor";
constexpr char const* FIELD_VERSION_PATCH = "patch";

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
