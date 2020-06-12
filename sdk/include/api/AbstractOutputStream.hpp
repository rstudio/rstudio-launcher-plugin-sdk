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

#include <memory>

#include <PImpl.hpp>
#include <comms/AbstractLauncherCommunicator.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief The type of job output.
 */
enum class OutputType
{
   /** Standard output. */
   STDOUT,

   /** Standard error output. */
   STDERR,

   /** Standard output and standard error output. */
   BOTH
};

/**
 * @brief Streams job output data to the launcher.
 */
class AbstractOutputStream : public std::enable_shared_from_this<AbstractOutputStream>
{
public:
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
   /** Convenience typedef for inheriting classes. */
   typedef std::shared_ptr<AbstractOutputStream> SharedThis;

   /** Convenience typedef for inheriting classes. */
   typedef std::weak_ptr<AbstractOutputStream> WeakThis;

   /**
    * @brief Constructor.
    *
    * @param in_requestId                   The ID of the request for which job output should be streamed.
    * @param in_outputType                  The type of job output to stream.
    * @param in_job                         The job for which output should be streamed.
    * @param in_launcherCommunicator        The launcher communicator for sending responses to the Launcher.
    */
   AbstractOutputStream(
      uint64_t in_requestId,
      OutputType in_outputType,
      JobPtr in_job,
      comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator);

   /**
    * @brief Reports output to the launcher.
    *
    * @param in_data        The output data.
    * @param in_isStdOut    Whether the output is Standard Output (true) or Standard Error (false).
    */
   void reportData(const std::string& in_data, bool in_isStdOut);

   /**
    * @brief Notifies the base class that the output stream has completed (i.e. all output of the specified type
    *        has been reported).
    */
   void setStreamComplete();

   /** The type of output that should be streamed. */
   OutputType m_outputType;

   /** The job for which output should be streamed. */
   JobPtr m_job;

private:
   // The private implementation of AbstractOutputStream.
   PRIVATE_IMPL(m_baseImpl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
