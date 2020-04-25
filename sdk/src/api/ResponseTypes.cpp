/*
 * ResponseTypes.cpp
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

#include <api/ResponseTypes.hpp>

#include <api/Constants.hpp>
#include <json/Json.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

// StreamSequenceId ====================================================================================================
struct StreamSequenceId::Impl
{
   Impl(uint64_t in_requestId, uint64_t in_sequenceId) :
      RequestId(in_requestId),
      SequenceId(in_sequenceId)
   {
   }

   uint64_t RequestId;
   uint64_t SequenceId;
};

PRIVATE_IMPL_DELETER_IMPL(StreamSequenceId)

StreamSequenceId::StreamSequenceId(uint64_t in_requestId, uint64_t in_sequenceId) :
   m_impl(new Impl(in_requestId, in_sequenceId))
{
}

StreamSequenceId::StreamSequenceId(const StreamSequenceId& in_other) :
   m_impl(new Impl(*in_other.m_impl))
{
}

StreamSequenceId::StreamSequenceId(StreamSequenceId&& in_other) noexcept :
   m_impl(std::move(in_other.m_impl))
{
}

StreamSequenceId& StreamSequenceId::operator=(const StreamSequenceId& in_other)
{
   if (this != &in_other)
   {
      if (in_other.m_impl == nullptr)
         m_impl.reset();
      else
         m_impl.reset(new Impl(*in_other.m_impl));
   }

   return *this;
}

StreamSequenceId& StreamSequenceId::operator=(StreamSequenceId&& in_other) noexcept
{
   if (this != &in_other)
   {
      m_impl.swap(in_other.m_impl);
      in_other.m_impl.reset();
   }

   return *this;
}

json::Object StreamSequenceId::toJson() const
{
   json::Object obj;
   obj[FIELD_REQUEST_ID] = m_impl->RequestId;
   obj[FIELD_SEQUENCE_ID] = m_impl->SequenceId;

   return obj;
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
