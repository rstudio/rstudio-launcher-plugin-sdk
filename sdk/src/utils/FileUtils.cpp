/*
 * FileUtils.cpp
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


#include <utils/FileUtils.hpp>

#include <iostream>
#include <sstream>

#include <boost/iostreams/copy.hpp>

#include <Error.hpp>
#include <system/FilePath.hpp>

namespace rstudio {
namespace launcher_plugins {
namespace utils {

Error readFileIntoString(const system::FilePath& in_file, std::string& out_fileContents)
{
   std::shared_ptr<std::istream> inputStream;
   Error error = in_file.openForRead(inputStream);
   if (error)
      return error;

   // Read the whole file into a string stream.
   std::ostringstream oStrStream;
   try
   {
      // Ensure an exception will be thrown if the failbit or badbit is set.
      inputStream->exceptions(std::istream::failbit | std::istream::badbit);

      boost::iostreams::copy(*inputStream, oStrStream);
   }
   catch (std::exception& e)
   {
      // TODO: good error code for this?
      error = systemError(1, "Failed to read file: " + std::string(e.what()), ERROR_LOCATION);
      error.addProperty("file", in_file.getAbsolutePath());
      return error;
   }

   out_fileContents = oStrStream.str();

   return Success();
}

} // namespace utils
} // namespace launcher_plugins
} // namespace rstudio
