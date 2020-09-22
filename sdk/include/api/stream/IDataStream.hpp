/*
 * IDataStream.hpp
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

#ifndef LAUNCHER_PLUGINS_I_DATA_STREAM_HPP
#define LAUNCHER_PLUGINS_I_DATA_STREAM_HPP

namespace rstudio {
namespace launcher_plugins {

class Error;

} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace api {

/**
 * @brief Represents the common component of any class which streams data. 
 * 
 * @tparam DataType  The type of data being streamed.
 */
template <typename DataType>
class IDataStream
{
public:
   /**
    * @brief Virtual destructor for inheritance.
    */
   virtual ~IDataStream() = default;

protected:
   /**
    * @brief Reports data to the Launcher.
    *
    * @param in_data            The data to be streamed to the Launcher.
    */
   virtual void reportData(const DataType& in_data) = 0;

   /**
    * @brief Reports an error to the Launcher.
    *
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    * 
    * @param in_error           The error which occurred.
    */
   virtual void reportError(const Error& in_error) = 0;

   /**
    * @brief Notifies that the data stream has completed.
    * 
    * Additional calls to reportError, reportData, or setStreamComplete will be ignored.
    */
   virtual void setStreamComplete() = 0;
};

} // namespace api
} // namespace launcher_plugins
} // namespace rstudio

#endif
