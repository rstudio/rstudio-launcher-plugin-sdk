/*
 * FileUtils.hpp
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


#ifndef LAUNCHER_PLUGINS_FILEUTILS_HPP
#define LAUNCHER_PLUGINS_FILEUTILS_HPP

#include <string>

namespace rstudio {
namespace launcher_plugins {

class Error;

namespace system {

class FilePath;

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

namespace rstudio {
namespace launcher_plugins {
namespace utils {

/**
 * @brief Reads the entire contents of the specified file into a single string.
 *
 * @param in_file               The file from which to read.
 * @param out_fileContents      The contents of the file, as a string.
 *
 * @return Success if the file exists and could be read; Error otherwise.
 */
Error readFileIntoString(const system::FilePath& in_file, std::string& out_fileContents);

/**
 * @brief Writes a string to a file, optionally appending to any existing content in the file (rather than overwriting).
 *
 * @param in_contents       The data to write to the file.
 * @param in_file           The file to which to write the data.
 * @param in_truncate       Whether to truncate any existing data in the file. Default: true.
 *
 * @return Success if the data was written to the file; Error otherwise.
 */
Error writeStringToFile(const std::string& in_contents, const system::FilePath& in_file, bool in_truncate = true);

} // namespace utils
} // namespace launcher_plugins
} // namespace rstudio

#endif
