/*
 * AbstractOutputStream.hpp
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

#ifndef LAUNCHER_PLUGINS_ABSTRACT_OUTPUT_STREAM_HPP
#define LAUNCHER_PLUGINS_ABSTRACT_OUTPUT_STREAM_HPP

#include <api/stream/IDataStream.hpp>

#include <memory>

#include <functional>

#include <PImpl.hpp>
#include <api/Job.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief The type of job output.
 */
enum class OutputType
{
   /** The first element of Output Type is always 0. */
   FIRST    = 0,

   /** Standard output. */
   STDOUT   = 0,

   /** Standard error output. */
   STDERR   = 1,

   /** Standard output and standard error output. */
   BOTH     = 2,

   /** This element of OutputType will always be one more than the last element. */
   LAST
};

/**
 * @brief Represents a single chunk of output for the stream.
 */
struct OutputChunk
{
   /** The data of this chunk of output. */
   std::string Data;

   /** The type of data in this chunk of output. */
   OutputType Type;
};

/**
 * @brief Streams job output data to the launcher.
 */
class AbstractOutputStream : public IDataStream<OutputChunk>
{
public:
   /** Definitions for callback functions which will be invoked when certain events occur. */
   typedef std::function<void(uint64_t)> OnComplete;
   typedef std::function<void(const Error&)> OnError;
   typedef std::function<void(const OutputChunk&, uint64_t)> OnOutput;

   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~AbstractOutputStream() = default;

   /**
    * @brief Starts the output stream.
    *
    * @return Success if the stream could be started; Error otherwise.
    */
   virtual Error start() = 0;

   /**
    * @brief Stops the output stream.
    */
   virtual void stop() = 0;

protected:
   /**
    * @brief Constructor.
    *
    * @param in_requestId       The ID of the request for which job output should be streamed.
    * @param in_outputType      The type of job output to stream.
    * @param in_onOutput        Callback function which will be invoked when data is reported.
    * @param in_onComplete      Callback function which will be invoked when the stream is complete.
    * @param in_onError         Callback function which will be invoked if an error occurs.
    * @param in_job             The job for which output should be streamed.
    */
   AbstractOutputStream(
      OutputType in_outputType,
      JobPtr in_job,
      OnOutput in_onOutput,
      OnComplete in_onComplete,
      OnError in_onError);

   /**
    * @brief Reports output to the launcher.
    *
    * @param in_data            The output data.
    */
   void reportData(const OutputChunk& in_data) final;

   /**
    * @brief Reports an error to the launcher.
    * 
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    *
    * @param in_error           The error which occurred.
    */
   void reportError(const Error& in_error) final;

   /**
    * @brief Notifies the base class that the output stream has completed (i.e. all output of the specified type
    *        has been reported).
    * 
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    */
   void setStreamComplete() final;

   /** The type of output that should be streamed. */
   OutputType m_outputType;

   /** The job for which output should be streamed. */
   JobPtr m_job;

private:
   // The private implementation of AbstractOutputStream.
   PRIVATE_IMPL(m_baseImpl);
};

typedef std::shared_ptr<AbstractOutputStream> OutputStreamPtr;

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
