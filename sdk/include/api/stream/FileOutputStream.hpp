/*
 * FileOutputStream.hpp
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


#ifndef LAUNCHER_PLUGINS_FILE_OUTPUT_STREAM_HPP
#define LAUNCHER_PLUGINS_FILE_OUTPUT_STREAM_HPP

#include <api/stream/AbstractOutputStream.hpp>

#include <memory>

#include <PImpl.hpp>
#include <api/Job.hpp>
#include <system/DateTime.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Streams job output data from a file.
 */
class FileOutputStream : public AbstractOutputStream, public std::enable_shared_from_this<FileOutputStream>
{
public:
   /**
    * @brief Constructor.
    *
    * @param in_outputType       The type of job output to stream.
    * @param in_job              The job for which output should be streamed.
    * @param in_onOutput         Callback function which will be invoked when data is reported.
    * @param in_onComplete       Callback function which will be invoked when the stream is complete.
    * @param in_onError          Callback function which will be invoked if an error occurs.
    * @param in_maxWaitTime      The maximum amount of time to wait for the output files to be created before reporting
    *                            an error.
    */
   FileOutputStream(
      OutputType in_outputType,
      api::JobPtr in_job,
      AbstractOutputStream::OnOutput in_onOutput,
      AbstractOutputStream::OnComplete in_onComplete,
      AbstractOutputStream::OnError in_onError,
      system::TimeDuration in_maxWaitTime = system::TimeDuration::Seconds(10));

   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~FileOutputStream() = default;

   /**
    * @brief Starts the output stream.
    *
    * @return Success if the stream could be started; Error otherwise.
    */
   Error start() override;

   /**
    * @brief Stops the output stream.
    */
   void stop() override;

private:
   /** Function which should be invoked on stream end. */
   typedef std::function<void()> OnStreamEnd;

   /**
    * @brief Callback to be invoked on tail child process exit.
    *
    * @param in_weakThis        A copy of a weak pointer to this object.
    * @param in_outputType      The type of output being emitted by the exited process.
    * @param in_exitCode        The exit code of the process.
    */
   static void onExitCallback(
      std::weak_ptr<FileOutputStream> in_weakThis,
      OutputType in_outputType,
      int in_exitCode);

   /**
    * @brief Callback to be invoked when the existence of the output files should be tested.
    * 
    * @param in_weakThis      A copy of a weak pointer to this object.
    */
   static void onFindFileTimerCallback(std::weak_ptr<FileOutputStream> in_weakThis);

   /**
    * @brief Invoked when output occurs.
    *
    * The default implementation reports all output. This method may be overridden to skip certain parts of the output,
    * if necessary.
    *
    * @param in_output          The output that was received from the tail process.
    * @param in_outputType      The type of output that was received.
    */
   virtual void onOutput(const std::string& in_output, OutputType in_outputType);

   /**
    * @brief Waits for the stream to end and invokes the provided callback function after.
    *
    * The default implementation waits for 2 seconds after the job completes to allow time for all of the output to be
    * written to file. This method may be overridden if a longer wait time is needed, or if there is a deterministic way
    * to be sure that the output has finished. This method should be non-blocking.
    *
    * @param in_onStreamEnd     The function to invoke when the stream has ended.
    */
   virtual void waitForStreamEnd(const OnStreamEnd& in_onStreamEnd);

   // The private implementation of FileOutputStream
   PRIVATE_IMPL(m_impl);
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
