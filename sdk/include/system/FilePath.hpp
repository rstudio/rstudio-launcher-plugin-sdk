/*
 * FilePath.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
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

#ifndef LAUNCHER_PLUGINS_FILE_PATH_HPP
#define LAUNCHER_PLUGINS_FILE_PATH_HPP

#include <functional>

#include "Error.hpp"
#include "PImpl.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Class which represents a path on the system. May be any type of file (e.g. directory, symlink, regular file,
 *        etc.)
 */
class FilePath
{
public:
   /**
    * @brief Function which recursively iterates over FilePath objects.
    *
    * @param int            The depth of the iteration.
    * @param FilePath       The current FilePath object in the recursive iteration.
    *
    * @return True if the computation can continue; false otherwise.
    */
   typedef std::function<bool(int, const FilePath&)> RecursiveIterationFunction;

   /**
    * @brief Default constructor.
    */
   FilePath();

   /**
    * @brief Constructor.
    *
    * @param in_path    The string representation of the path.
    */
   explicit FilePath(std::string in_path);

   /**
    * @brief Returns the absolute representation of this path as a string.
    *
    * @return The absolute representation of this path as a string.
    */
   std::string absolutePath() const;
   /**
    * @brief Creates a child path of this location using the relative path in_path.
    *
    * @param in_path    The relative path to create a child path of this path from.
    *
    * @return The child path.
    */
   FilePath childPath(const std::string& in_path) const;

   /**
    * @brief Copies this FilePath non-recursively. If this FilePath is a directory, the contents of the directory will
    *        not be copied.
    *
    * @param in_destination     The destination directory into which this directory will be copied.
    *
    * @return Success on a successful copy; system error otherwise (e.g EPERM, ENOENT, etc.)
    */
   Error copy(const FilePath& in_destination) const;

   /**
    * @brief Copies the current directory recursively into the destination.
    *
    * @param in_destination     The destination directory into which this directory will be copied.
    *
    * @return Success on a successful copy; system error otherwise (e.g EPERM, ENOENT, etc.)
    */
   Error copyDirectoryRecursive(const FilePath& in_destination) const;

   /**
    * @brief Attempts to create this directory, if it does not already exist.
    *
    * @return Success if the directory was created or already existed; system error otherwise (e.g. EPERM, ENOENT, etc.)
    */
   Error ensureDirectoryExists() const;

   /**
    * @brief Attempts to create this file, if it does not already exist.
    *
    * @return Success if the file was created or already existed; system error otherwise (e.g. EPERM, ENOENT, etc.)
    */
   Error ensureFileExists() const;

   /**
    * @brief Returns whether this path exists or not.
    *
    * @return True if this path points to a directory; false otherwise.
    */
   bool exists() const;

   /**
    * @brief Recursively iterates over all children of this FilePath, calling the recursive iteration function for each
    *        child.
    *
    * @param in_iterationFunction       The iteration function which acts on each child.
    *
    * @return Success if all chidlren were processed by the iteration function; error otherwise.
    */
   Error getChildrenRecursive(const RecursiveIterationFunction& in_iterationFunction) const;

   /**
    * @brief Gets the file name without the preceding path elements of this FilePath.
    *
    * @return The file name without its preceding path elements
    */
   std::string getFileName() const;


   /**
    * @brief Gets the relative path of this FilePath with respect to the provided parent path.
    *
    * @param in_parent      The parent of this path.
    *
    * @return The relative path of this FilePath with respect to the provided path, if it is a parent of this path;
    *         empty string otherwise.
    */
   std::string getRelativePath(const FilePath& in_parent) const;

   /**
    * @brief Gets the size of the file path.
    *
    * @return 0 if this is not a regular file; the size of the file path otherwise.
    */
   uintmax_t getSize() const;

   /**
    * @brief Returns whether this path points to a directory or not.
    *
    * @return True if this path points to a directory; false otherwise.
    */
   bool isDirectory() const;

   /**
    * @brief Returns whether the FilePath is emtpy.
    *
    * @return True if the FilePath is empty, false otherwise.
    */
   bool isEmpty() const;

   /**
    * @brief Returns whether this path points to a regular file or not.
    *
    * @return True if this path points to a regular file; false otherwise.
    */
   bool isRegularFile() const;

   /**
    * @brief Moves the current file from this path to the specified path. On successful move, this FilePath object will
    *        continue to point to the original file, which will now be deleted.
    *
    * @param in_destination         The path to move this file to.
    * @param in_moveCrossDevice     Whether to attempt to move the file cross device if a direct move fails.
    *
    * @return Success if this file is moved to in_destinationa successfully; system error otherwise (e.g. EPERM, ENOACCES, etc.)
    */
   Error move(const FilePath& in_destination, bool in_moveCrossDevice = true) const;

   /**
    * @brief Opens this file for read.
    *
    * @param out_stream     The input stream for this open file.
    *
    * @return Success if the file was opened; system error otherwise (e.g. EPERM, ENOENT, etc.)
    */
   Error openForRead(std::shared_ptr<std::istream>& out_stream) const;


   /**
    * @brief Opens this file for write.
    *
    * @param out_stream     The output stream for this open file.
    * @param in_truncate    Whether to truncate the existing contents of the file. Default: true.
    *
    * @return Success if the file was opened; system error otherwise (e.g. EPERM, ENOENT, etc.)
    */
   Error openForWrite(std::shared_ptr<std::ostream>& out_stream, bool in_truncate = true) const;

   /**
    * @brief Attempts to remove the file from disk, if it exists.
    *
    * @return Success if the file was removed or didn't exist; system error otherwise (e.g. EPERM, EACCES, etc.)
    */
   Error remove() const;

private:
   // Shared private implementation of the file path. This is safe because all methods of this class are const.
   PRIVATE_IMPL_SHARED(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
