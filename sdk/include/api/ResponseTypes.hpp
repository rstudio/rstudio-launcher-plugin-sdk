/*
 * ResponseTypes.hpp
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

#ifndef LAUNCHER_PLUGINS_RESPONSE_TYPES_HPP
#define LAUNCHER_PLUGINS_RESPONSE_TYPES_HPP

#include <vector>

#include <PImpl.hpp>
#include <api/Job.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace json {

class Object;

} // namespace json
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief An identifier for a MultiStreamResponse.
 */
class StreamSequenceId final
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_requestId       The ID of the request for which this response will be streamed.
    * @param in_sequenceId      The ID of this response in the sequence of streamed responses for the specified request.
    */
   StreamSequenceId(uint64_t in_requestId, uint64_t in_sequenceId);

   /**
    * @brief Copy constructor.
    *
    * @param in_other       The StreamSequenceId to be copied into this.
    */
   StreamSequenceId(const StreamSequenceId& in_other);

   /**
    * @brief Move constructor.
    *
    * @param in_other       The StreamSequenceId to be moved into this.
    */
   StreamSequenceId(StreamSequenceId&& in_other) noexcept;

   /**
    * @brief Destructor.
    */
   ~StreamSequenceId() = default;

   /**
    * @brief Assignment operator.
    *
    * @param in_other       The StreamSequenceId to be copied into this.
    *
    * @return A reference to this StreamSequenceId.
    */
   StreamSequenceId& operator=(const StreamSequenceId& in_other);

   /**
    * @brief Move operator.
    *
    * @param in_other       The StreamSequenceId to be moved into this.
    *
    * @return A reference to this StreamSequenceId.
    */
   StreamSequenceId& operator=(StreamSequenceId&& in_other) noexcept;

   /**
    * @brief Converts this StreamSequenceId to a JSON Object.
    *
    * @return The JSON object which represents this StreamSequenceId.
    */
   json::Object toJson() const;

private:
   PRIVATE_IMPL(m_impl);
};

struct NetworkInfo
{
   /** The hostname of the machine running the job. */
   std::string Hostname;

   /** The IP Address(es) of the machine running the job. */
   std::vector<std::string> IpAddresses;
};

// Convenience typedefs.
typedef std::vector<StreamSequenceId> StreamSequences;

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
