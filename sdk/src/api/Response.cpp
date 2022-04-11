/*
 * Response.cpp
 *
 * Copyright (C) 2019-20 by RStudio, PBC
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

#include <api/Response.hpp>

#include <atomic>

#include <json/Json.hpp>
#include <api/IJobSource.hpp>
#include "Constants.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace api {

// Response ============================================================================================================
struct Response::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_responseType    The type of the response.
    * @param in_requestId       The ID of the request for which this response is being sent.
    */
   Impl(Type in_responseType, uint64_t in_requestId) :
      ResponseType(static_cast<int>(in_responseType)),
      RequestId(in_requestId),
      ResponseId(
         ((in_responseType == Type::HEARTBEAT) || (in_responseType == Type::ERROR)) ?
            0 : NEXT_RESPONSE_ID.fetch_add(1))
   {
   }

   /** Global atomic value to keep track of all response IDs used so far. */
   static std::atomic_uint64_t NEXT_RESPONSE_ID;

   /** The type of the response. */
   const int ResponseType;

   /** The ID of the request for which this response is being sent. */
   const uint64_t RequestId;

   /** The ID of this response. */
   const uint64_t ResponseId;
};

std::atomic_uint64_t Response::Impl::NEXT_RESPONSE_ID { 0 };

PRIVATE_IMPL_DELETER_IMPL(Response)

json::Object Response::toJson() const
{
   json::Object jsonObject;
   jsonObject.insert(FIELD_MESSAGE_TYPE, json::Value(m_responseImpl->ResponseType));
   jsonObject.insert(FIELD_REQUEST_ID, json::Value(m_responseImpl->RequestId));
   jsonObject.insert(FIELD_RESPONSE_ID, json::Value(m_responseImpl->ResponseId));

   return jsonObject;
}

Response::Response(Type in_responseType, uint64_t in_requestId) :
   m_responseImpl(new Impl(in_responseType, in_requestId))
{
}

// MultiStreamResponse =================================================================================================
struct MultiStreamResponse::Impl
{
   explicit Impl(StreamSequences in_sequences) :
      Sequences(std::move(in_sequences))
   {
   }

   StreamSequences Sequences;
};

PRIVATE_IMPL_DELETER_IMPL(MultiStreamResponse)

json::Object MultiStreamResponse::toJson() const
{
   json::Object result = Response::toJson();

   json::Array arr;
   for (const StreamSequenceId& sequenceId: m_streamResponseImpl->Sequences)
      arr.push_back(sequenceId.toJson());

   result[FIELD_SEQUENCES] = arr;
   return result;
}

MultiStreamResponse::MultiStreamResponse(Type in_responseType, StreamSequences in_sequences) :
   Response(in_responseType, 0),
   m_streamResponseImpl(new Impl(std::move(in_sequences)))
{
}

// Error Response ======================================================================================================
struct ErrorResponse::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_errorType       The type of error.
    * @param in_errorMessage    The message of the error.
    */
   Impl(Type in_errorType, std::string in_errorMessage) :
      ErrorCode(static_cast<int>(in_errorType)),
      ErrorMessage(std::move(in_errorMessage))
   {
   }

   /** The error code. */
   int ErrorCode;

   /** The error message. */
   std::string ErrorMessage;
};

PRIVATE_IMPL_DELETER_IMPL(ErrorResponse)

ErrorResponse::ErrorResponse(uint64_t in_requestId, Type in_errorType, std::string in_errorMessage) :
   Response(Response::Type::ERROR, in_requestId),
   m_impl(new Impl(in_errorType, std::move(in_errorMessage)))
{
}

json::Object ErrorResponse::toJson() const
{
   json::Object responseObject = Response::toJson();
   responseObject.insert(FIELD_ERROR_CODE, json::Value(m_impl->ErrorCode));
   responseObject.insert(FIELD_ERROR_MESSAGE, json::Value(m_impl->ErrorMessage));

   return responseObject;
}

// Heartbeat Response ==================================================================================================
HeartbeatResponse::HeartbeatResponse() :
   Response(Response::Type::HEARTBEAT, 0)
{
}

// Bootstrap Response ==================================================================================================
BootstrapResponse::BootstrapResponse(uint64_t in_requestId) :
   Response(Type::BOOTSTRAP, in_requestId)
{
}

json::Object BootstrapResponse::toJson() const
{
   // Get the object generated by the base class.
   json::Object jsonObject = Response::toJson();

   // Add the bootstrap specific fields.
   json::Object version;
   version.insert(FIELD_VERSION_MAJOR, json::Value(API_VERSION_MAJOR));
   version.insert(FIELD_VERSION_MINOR, json::Value(API_VERSION_MINOR));
   version.insert(FIELD_VERSION_PATCH, json::Value(API_VERSION_PATCH));

   jsonObject.insert(FIELD_VERSION, version);
   return jsonObject;
}

// Job State Response ==================================================================================================
struct JobStateResponse::Impl
{
   Impl(JobList in_jobList, Optional<std::set<std::string> > in_fields) :
      Jobs(std::move(in_jobList)),
      Fields(std::move(in_fields))
   {
      // Ensure that the ID field is included in the subset of fields as it is required.
      std::set<std::string> tmp;
      Fields.getValueOr(tmp).insert("id");
   }

   JobList Jobs;
   Optional<std::set<std::string> > Fields;
};

PRIVATE_IMPL_DELETER_IMPL(JobStateResponse)

JobStateResponse::JobStateResponse(
   uint64_t in_requestId,
   JobList in_jobs,
   Optional<std::set<std::string> > in_jobFields) :
   Response(Type::JOB_STATE, in_requestId),
   m_impl(new Impl(std::move(in_jobs), std::move(in_jobFields)))
{
}

json::Object JobStateResponse::toJson() const
{
   // Get the object generated by the base class.
   json::Object jsonObject = Response::toJson();

   json::Array jobsArray;
   for (const JobPtr& job: m_impl->Jobs)
   {
      json::Object jobObj;
      // Lock the job to ensure it doesn't change while we serialize it.
      LOCK_JOB(job)
      {
         jobObj = job->toJson();
      }
      END_LOCK_JOB

      if (m_impl->Fields)
      {
         const std::set<std::string>& fieldSet = m_impl->Fields.getValueOr({});
         const auto& fieldEnd = fieldSet.end();

         for (auto itr = jobObj.begin(); itr != jobObj.end();)
         {
            if (fieldSet.find((*itr).getName()) == fieldEnd)
               itr = jobObj.erase(itr);
            else
               ++itr;
         }
      }

      jobsArray.push_back(jobObj);
   }

   jsonObject[FIELD_JOBS] = jobsArray;
   return jsonObject;
}

// Job Status Response =================================================================================================
struct JobStatusResponse::Impl
{
   explicit Impl(const api::JobPtr& in_job) :
      JobId(in_job->Id),
      JobName(in_job->Name),
      Status(in_job->Status),
      StatusMessage(in_job->StatusMessage)
   {
   }

   std::string JobId;
   std::string JobName;
   Job::State Status;
   std::string StatusMessage;
};

PRIVATE_IMPL_DELETER_IMPL(JobStatusResponse)

JobStatusResponse::JobStatusResponse(StreamSequences in_sequences, const api::JobPtr& in_job) :
   MultiStreamResponse(Type::JOB_STATUS, std::move(in_sequences)),
   m_impl(new Impl(in_job))
{
}

json::Object JobStatusResponse::toJson() const
{
   json::Object result = MultiStreamResponse::toJson();

   result[FIELD_ID] = m_impl->JobId;
   result[FIELD_NAME] = m_impl->JobName;
   result[FIELD_STATUS] = api::Job::stateToString(m_impl->Status);

   if (!m_impl->StatusMessage.empty())
      result[FIELD_STATUS_MESSAGE] = m_impl->StatusMessage;

   return result;
}

// Control Job Response ================================================================================================
struct ControlJobResponse::Impl
{
   Impl(std::string&& in_statusMessage, bool in_isComplete) :
      StatusMessage(in_statusMessage),
      IsComplete(in_isComplete)
   {
   }

   /** The detailed status of the control job operation. */
   std::string StatusMessage;

   /** Whether the operation has completed. */
   bool IsComplete;
};

PRIVATE_IMPL_DELETER_IMPL(ControlJobResponse)

ControlJobResponse::ControlJobResponse(uint64_t in_requestId, std::string in_statusMessage, bool in_isComplete) :
   Response(Type::CONTROL_JOB, in_requestId),
   m_impl(new Impl(std::move(in_statusMessage), in_isComplete))
{
}

json::Object ControlJobResponse::toJson() const
{
   json::Object result = Response::toJson();
   result[FIELD_STATUS_MESSAGE] = m_impl->StatusMessage;
   result[FIELD_OPERATION_COMPLETE] = m_impl->IsComplete;

   return result;
}

// Output Stream Response ==============================================================================================
struct OutputStreamResponse::Impl
{
   Impl(uint64_t in_sequenceId, std::string&& in_output, OutputType in_outputType) :
      IsComplete(false),
      Output(in_output),
      OutType(in_outputType),
      SequenceId(in_sequenceId)
   {
   }

   explicit Impl(uint64_t in_sequenceId) :
      IsComplete(true),
      SequenceId(in_sequenceId)
   {
   }

   bool IsComplete;
   std::string Output;
   OutputType OutType;
   uint64_t SequenceId;
};

PRIVATE_IMPL_DELETER_IMPL(OutputStreamResponse)

OutputStreamResponse::OutputStreamResponse(
   uint64_t in_requestId,
   uint64_t in_sequenceId,
   std::string in_output,
   OutputType in_outputType) :
      Response(Type::JOB_OUTPUT, in_requestId),
      m_impl(new Impl(in_sequenceId, std::move(in_output), in_outputType))
{
}

OutputStreamResponse::OutputStreamResponse(uint64_t in_requestId, uint64_t in_sequenceId)  :
   Response(Type::JOB_OUTPUT, in_requestId),
   m_impl(new Impl(in_sequenceId))
{
}

json::Object OutputStreamResponse::toJson() const
{
   json::Object result = Response::toJson();
   result[FIELD_SEQUENCE_ID] = m_impl->SequenceId;
   result[FIELD_COMPLETE] = m_impl->IsComplete;

   if (!m_impl->Output.empty())
   {
      result[FIELD_OUTPUT] = m_impl->Output;
      
      switch (m_impl->OutType)
      {
         case OutputType::STDOUT:
         {
            result[FIELD_OUTPUT_TYPE] = "stdout";
            break;
         }
         case OutputType::STDERR:
         {
            result[FIELD_OUTPUT_TYPE] = "stderr";
            break;
         }
         default:
         case OutputType::BOTH:
         {
            result[FIELD_OUTPUT_TYPE] = "mixed";
            break;
         }
      }
   }

   return result;
}

// Resource Utilization Stream Response ================================================================================
struct ResourceUtilStreamResponse::Impl
{

   Impl(const ResourceUtilData& in_data, bool in_isComplete) :
      Data(in_data),
      IsComplete(in_isComplete)
   {
   }

   ResourceUtilData Data;

   bool IsComplete;
};

PRIVATE_IMPL_DELETER_IMPL(ResourceUtilStreamResponse);

ResourceUtilStreamResponse::ResourceUtilStreamResponse(
   StreamSequences in_sequences,
   const ResourceUtilData& in_resourceData,
   bool in_isComplete) :
      MultiStreamResponse(Response::Type::JOB_RESOURCE_UTIL, in_sequences),
      m_impl(new Impl(in_resourceData, in_isComplete))
{
}

json::Object ResourceUtilStreamResponse::toJson() const
{
   json::Object result = MultiStreamResponse::toJson();
   
   const ResourceUtilData& data = m_impl->Data;
   if (data.CpuPercent)
      result[FIELD_CPU_PERCENT] = data.CpuPercent.getValueOr(0.0);
   if (data.CpuSeconds)
      result[FIELD_CPU_SECONDS] = data.CpuSeconds.getValueOr(0.0);
   if (data.VirtualMem)
      result[FIELD_VIRTUAL_MEM] = data.VirtualMem.getValueOr(0.0);
   if (data.ResidentMem)
      result[FIELD_RESIDENT_MEM] = data.ResidentMem.getValueOr(0.0);

   result[FIELD_COMPLETE] = m_impl->IsComplete;

   return result;
}


// Network Response ====================================================================================================
struct NetworkResponse::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_networkInfo     The network information for the requested Job.
    */
   explicit Impl(NetworkInfo&& in_networkInfo) :
      NetInfo(in_networkInfo)
   {
   }

   /** The network information for the requested Job. */
   NetworkInfo NetInfo;
};

PRIVATE_IMPL_DELETER_IMPL(NetworkResponse)

NetworkResponse::NetworkResponse(uint64_t in_requestId, NetworkInfo in_networkInfo) :
   Response(Response::Type::JOB_NETWORK, in_requestId),
   m_impl(new Impl(std::move(in_networkInfo)))
{
}

json::Object NetworkResponse::toJson() const
{
   json::Object result = Response::toJson();
   result[FIELD_HOST] = m_impl->NetInfo.Hostname;
   result[FIELD_IPS] = json::toJsonArray(m_impl->NetInfo.IpAddresses);

   return result;
}

// Cluster Info Response ===============================================================================================
struct ClusterInfoResponse::Impl
{
   explicit Impl(JobSourceConfiguration in_configuration) :
         ClusterConfig(std::move(in_configuration))
   {
   }

   JobSourceConfiguration ClusterConfig;
};

PRIVATE_IMPL_DELETER_IMPL(ClusterInfoResponse)

ClusterInfoResponse::ClusterInfoResponse(uint64_t in_requestId, const JobSourceConfiguration& in_configuration) :
   Response(Response::Type::CLUSTER_INFO, in_requestId),
   m_impl(new Impl(in_configuration))
{
}

json::Object ClusterInfoResponse::toJson() const
{
   json::Object result = Response::toJson();

   const JobSourceConfiguration& clusterConfig = m_impl->ClusterConfig;
   result[FIELD_CONTAINER_SUPPORT] = clusterConfig.ContainerConfig.SupportsContainers;

   if (clusterConfig.ContainerConfig.SupportsContainers)
   {
      if (!clusterConfig.ContainerConfig.DefaultImage.empty())
         result[FIELD_DEFAULT_IMAGE] = clusterConfig.ContainerConfig.DefaultImage;

      result[FIELD_ALLOW_UNKNOWN_IMAGES] = clusterConfig.ContainerConfig.AllowUnknownImages;
      result[FIELD_IMAGES] = json::toJsonArray(clusterConfig.ContainerConfig.ContainerImages);
   }

   if (!clusterConfig.Queues.empty())
      result[FIELD_QUEUES] = json::toJsonArray(clusterConfig.Queues);

   json::Array config, constraints, limits;

   for (const JobConfig& configVal: clusterConfig.CustomConfig)
      config.push_back(configVal.toJson());

   for (const PlacementConstraint& constraint: clusterConfig.PlacementConstraints)
      constraints.push_back(constraint.toJson());

   for (const ResourceLimit& limit: clusterConfig.ResourceLimits)
      limits.push_back(limit.toJson());

   result[FIELD_CONFIG] = config;
   result[FIELD_RESOURCE_LIMITS] = limits;
   result[FIELD_PLACEMENT_CONSTRAINTS] = constraints;

   return result;
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
