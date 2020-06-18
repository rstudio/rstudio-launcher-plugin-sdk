/*
 * AbstractOutputStream.cpp
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

#include <api/AbstractOutputStream.hpp>

#include <atomic>

namespace rstudio {
namespace launcher_plugins {
namespace api {

struct AbstractOutputStream::Impl
{
   Impl(OnOutput&& in_onOutput, OnComplete&& in_onComplete, OnError&& in_onError) :
      OnOutputFunc(in_onOutput),
      OnCompleteFunc(in_onComplete),
      OnErrorFunc(in_onError),
      SequenceId(0)
   {
   }

   OnOutput OnOutputFunc;

   OnComplete OnCompleteFunc;

   OnError OnErrorFunc;

   std::atomic_uint64_t SequenceId;
};

PRIVATE_IMPL_DELETER_IMPL(AbstractOutputStream)

AbstractOutputStream::AbstractOutputStream(
   OutputType in_outputType,
   JobPtr in_job,
   OnOutput in_onOutput,
   OnComplete in_onComplete,
   OnError in_onError):
      m_outputType(in_outputType),
      m_job(std::move(in_job)),
      m_baseImpl(new Impl(std::move(in_onOutput), std::move(in_onComplete), std::move(in_onError)))
{
}

void AbstractOutputStream::reportData(const std::string& in_data, OutputType in_outputType)
{
   m_baseImpl->OnOutputFunc(in_data, in_outputType, ++m_baseImpl->SequenceId);
}

void AbstractOutputStream::reportError(const Error& in_error)
{
   m_baseImpl->OnErrorFunc(in_error);
}

void AbstractOutputStream::setStreamComplete()
{
   m_baseImpl->OnCompleteFunc(++m_baseImpl->SequenceId);
}

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
