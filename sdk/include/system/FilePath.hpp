/*
 * FilePath.hpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#ifndef LAUNCHER_PLUGINS_FILE_PATH_HPP
#define LAUNCHER_PLUGINS_FILE_PATH_HPP

#include "Error.hpp"
#include "PImpl.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace system {

/**
 * @brief Error codes for FilePath errors that are not from the system.
 */
enum class FilePathError
{
   WRONG_FILE_TYPE = 1
};

/**
 * @brief Class which represents a path on the system. May be any type of file (e.g. directory, symlink, regular file,
 *        etc.)
 */
class FilePath
{
public:
   /**
    * @brief Default constructor.
    */
   FilePath() = default;

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

private:
   // Shared private implementation of the file path. This is safe because all methods of this class are const.
   PRIVATE_IMPL_SHARED(m_impl);
};

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

#endif
