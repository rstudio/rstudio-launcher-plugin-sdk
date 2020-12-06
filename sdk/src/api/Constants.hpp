/*
 * Constants.hpp
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

#ifndef LAUNCHER_PLUGINS_CONSTANTS_HPP
#define LAUNCHER_PLUGINS_CONSTANTS_HPP

namespace rstudio {
namespace launcher_plugins {
namespace api {

// The RStudio Launcher Plugin API implemented by this SDK version.
constexpr int API_VERSION_MAJOR                    = 1;
constexpr int API_VERSION_MINOR                    = 2;
constexpr int API_VERSION_PATCH                    = 0;

// Common fields for all requests and responses.
constexpr char const* FIELD_MESSAGE_TYPE           = "messageType";
constexpr char const* FIELD_REQUEST_ID             = "requestId";

// Common fields for all responses.
constexpr char const* FIELD_RESPONSE_ID            = "responseId";

// Common fields for requests which require a username.
constexpr char const* FIELD_REAL_USER              = "username";
constexpr char const* FIELD_REQUEST_USERNAME       = "requestUsername";

// Common fields for requests which require a job ID.
constexpr char const* FIELD_JOB_ID                 = "jobId";
constexpr char const* FIELD_ENCODED_JOB_ID         = "encodedJobId";

// Common fields for stream and multi-stream responses.
constexpr char const* FIELD_CANCEL_STREAM          = "cancel";
constexpr char const* FIELD_SEQUENCE_ID            = "seqId";
constexpr char const* FIELD_SEQUENCES              = "sequences";

// Error response fields.
constexpr char const* FIELD_ERROR_CODE             = "errorCode";
constexpr char const* FIELD_ERROR_MESSAGE          = "errorMessage";

// Bootstrap request and response fields.
constexpr char const* FIELD_VERSION                = "version";
constexpr char const* FIELD_VERSION_MAJOR          = "major";
constexpr char const* FIELD_VERSION_MINOR          = "minor";
constexpr char const* FIELD_VERSION_PATCH          = "patch";

// SubmitJob request fields.
constexpr char const* FIELD_JOB                    = "job";

// JobState request and response fields.
constexpr char const* FIELD_JOB_FIELDS             = "fields";
constexpr char const* FIELD_JOB_END_TIME           = "endTime";
constexpr char const* FIELD_JOB_START_TIME         = "startTime";
constexpr char const* FIELD_JOB_STATUSES           = "statuses";
constexpr char const* FIELD_JOB_TAGS               = "tags";
constexpr char const* FIELD_JOBS                   = "jobs";

// JobStatus response fields.
constexpr char const* FIELD_ID                     = "id";
constexpr char const* FIELD_NAME                   = "name";
constexpr char const* FIELD_STATUS                 = "status";
constexpr char const* FIELD_STATUS_MESSAGE         = "statusMessage";

// ControlJob request and response fields
constexpr char const* FIELD_OPERATION              = "operation";
constexpr char const* FIELD_OPERATION_COMPLETE     = "operationComplete";

// OutputStream request and response fields.
constexpr char const* FIELD_COMPLETE               = "complete";
constexpr char const* FIELD_OUTPUT                 = "output";
constexpr char const* FIELD_OUTPUT_TYPE            = "outputType";

// ResourceUtilStream response fields.
constexpr char const* FIELD_CPU_PERCENT            = "cpuPercent";
constexpr char const* FIELD_CPU_SECONDS            = "cpuTime";
constexpr char const* FIELD_VIRTUAL_MEM            = "virtualMemory";
constexpr char const* FIELD_RESIDENT_MEM           = "residentMemory";

// Network response fields.
constexpr char const* FIELD_HOST                   = "host";
constexpr char const* FIELD_IPS                    = "ipAddresses";

// ClusterInfo response fields.
constexpr char const* FIELD_ALLOW_UNKNOWN_IMAGES   = "allowUnknownImages";
constexpr char const* FIELD_CONFIG                 = "config";
constexpr char const* FIELD_CONTAINER_SUPPORT      = "supportsContainers";
constexpr char const* FIELD_DEFAULT_IMAGE          = "defaultImage";
constexpr char const* FIELD_IMAGES                 = "images";
constexpr char const* FIELD_PLACEMENT_CONSTRAINTS  = "placementConstraints";
constexpr char const* FIELD_QUEUES                 = "queues";
constexpr char const* FIELD_RESOURCE_LIMITS        = "resourceLimits";

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
