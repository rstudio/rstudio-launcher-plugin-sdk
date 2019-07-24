/*
 * FilePath.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "system/FilePath.hpp"

#include <boost/filesystem.hpp>

#include "logging/Logger.hpp"

using namespace rstudio::launcher_plugins::logging;

namespace rstudio {
namespace launcher_plugins {
namespace system {

namespace {

inline Error convertFilesystemError(const boost::filesystem::filesystem_error& in_e, ErrorLocation in_location)
{

   std::string message = in_e.what();
   if (!in_e.path1().empty())
   {
      message += "\n    Path: " + in_e.path1().string();
      if (!in_e.path2().empty())
         message += "\n    Path 2: " + in_e.path2().string();
   }

   return Error(in_e.code(), message, std::move(in_location));
}

bool copySingleItem(const FilePath& in_sourceRoot, const FilePath& in_destination, const FilePath& in_sourceItem)
{
   // The source item should always be a child of the source root because this should only be called while we're
   // iterating the contents of a directory.
   std::string relativeItemPath = in_sourceItem.getRelativePath(in_sourceRoot);
   assert(!relativeItemPath.empty());
   if (relativeItemPath.empty())
   {
      logErrorMessage("Error while copying single item. Specified item (" +
         in_sourceItem.absolutePath() +
         ") is not a child of the root path (" + in_sourceRoot.absolutePath() + ")." );
      return false;
   }

   Error error = in_sourceItem.copy(in_destination.childPath(relativeItemPath));
   if (error)
   {
      logError(error);
      return false;
   }

   return true;
}

} // anonymous namespace

typedef boost::filesystem::path PathType;

struct FilePath::Impl
{
   Impl() = default;

   explicit Impl(PathType in_path) : Path(std::move(in_path)) { };

   PathType Path;
};

FilePath::FilePath(std::string in_path) :
   m_impl(new Impl(in_path))
{
}

std::string FilePath::absolutePath() const
{
   if (isEmpty())
      return "";
   else
      return m_impl->Path.generic_string();
}

FilePath FilePath::childPath(const std::string& in_path) const
{
   if (!isEmpty())
   {
      try
      {
         PathType childPath(in_path);
         if (childPath.has_root_path())
            throw boost::filesystem::filesystem_error(
               "absolute path not permitted",
               childPath,
               boost::system::error_code(
                  boost::system::errc::no_such_file_or_directory,
                  boost::system::system_category()));

         FilePath childFilePath;
         childFilePath.m_impl.reset(new FilePath::Impl(boost::filesystem::absolute(childPath, m_impl->Path)));
         return childFilePath;
      }
      catch (boost::filesystem::filesystem_error& e)
      {
         logging::logError(convertFilesystemError(e, ERROR_LOCATION));
      }
   }

   return *this;
}

Error FilePath::copy(const FilePath& in_destination) const
{
   if (isDirectory())
      return in_destination.ensureDirectoryExists();

   try
   {
      boost::filesystem::copy_file(m_impl->Path, in_destination.m_impl->Path);
   }
   catch (boost::filesystem::filesystem_error& e)
   {
      return convertFilesystemError(e, ERROR_LOCATION);
   }

   return Success();
}

Error FilePath::copyDirectoryRecursive(const FilePath& in_destination) const
{
   // Make sure the root destination folder exists.
   Error error = in_destination.ensureDirectoryExists();
   if (error)
      return error;

   // Copy all the children.
   return getChildrenRecursive(std::bind(copySingleItem, *this, in_destination, std::placeholders::_2));
}

Error FilePath::ensureDirectoryExists() const
{
   if (!exists())
   {
      try
      {
         boost::filesystem::create_directory(m_impl->Path);
      }
      catch (boost::filesystem::filesystem_error& e)
      {
         return convertFilesystemError(e, ERROR_LOCATION);
      }
   }
   else if (!isDirectory())
   {
      // It's possible this error should be EPERM instead (operation not permitted)
      return systemError(
         boost::system::errc::file_exists,
         "File already exists but is not a directory: " + absolutePath(),
         ERROR_LOCATION);
   }

   return Success();
}

Error FilePath::ensureFileExists() const
{
   if (!exists())
   {
      std::shared_ptr<std::ostream> stream;
      Error error = openForWrite(stream);
      if (error)
         return error;

      // Close the file handle.
      stream->flush();
      stream.reset();
   }
   else if (!isRegularFile())
   {
      // It's possible this error should be EPERM instead (operation not permitted)
      return systemError(
         boost::system::errc::file_exists,
         "File already exists but is not a regular file: " + absolutePath(),
         ERROR_LOCATION);
   }

   return Success();
}

bool FilePath::exists() const
{
   try
   {
      if (!m_impl->Path.empty())
         return boost::filesystem::exists(m_impl->Path);
   }
   catch (const boost::filesystem::filesystem_error& e)
   {
      logging::logError(convertFilesystemError(e, ERROR_LOCATION));
   }

   return false;
}

Error FilePath::getChildrenRecursive(const RecursiveIterationFunction& in_iterationFunction) const
{
   if (!exists())
      return systemError(
         boost::system::errc::no_such_file_or_directory,
         "No such file or directory: " + absolutePath(),
         ERROR_LOCATION);

   try
   {
      boost::filesystem::recursive_directory_iterator end;
      for (boost::filesystem::recursive_directory_iterator itr(m_impl->Path); itr != end; ++itr)
      {
         // I don't want to expose a FilePath constructor that depends on boost::filesystem::path.
         FilePath item;
         item.m_impl.reset(new Impl(itr->path()));
         if (!in_iterationFunction(itr.level(), item))
            break;
      }
   }
   catch (boost::filesystem::filesystem_error& e)
   {
      return convertFilesystemError(e, ERROR_LOCATION);
   }

   return Success();
}

std::string FilePath::getFileName() const
{
   return m_impl->Path.filename().generic_string();
}

std::string FilePath::getRelativePath(const FilePath& in_parent) const
{
  PathType relativePath = m_impl->Path.lexically_normal().lexically_relative(
     in_parent.m_impl->Path.lexically_normal());

  return relativePath.generic_string();
}

uintmax_t FilePath::getSize() const
{
   if (exists() && isRegularFile())
   {
      try
      {
         return boost::filesystem::file_size(m_impl->Path);
      }
      catch (boost::filesystem::filesystem_error& e)
      {
         logError(convertFilesystemError(e, ERROR_LOCATION));
      }
   }

   return 0;
}

bool FilePath::isEmpty() const
{
   return m_impl->Path.empty();
}

bool FilePath::isRegularFile() const
{
   try
   {
      if (exists())
         return boost::filesystem::is_regular_file(m_impl->Path);
   }
   catch (const boost::filesystem::filesystem_error& e)
   {
      logError(convertFilesystemError(e, ERROR_LOCATION));
   }

   return false;
}

bool FilePath::isDirectory() const
{
   try
   {
      if (exists())
         return boost::filesystem::is_directory(m_impl->Path);
   }
   catch (const boost::filesystem::filesystem_error& e)
   {
      logError(convertFilesystemError(e, ERROR_LOCATION));
   }

   return false;
}

Error FilePath::move(const FilePath& in_destination, bool in_moveCrossDevice) const
{
   try
   {
      boost::filesystem::rename(m_impl->Path, in_destination.m_impl->Path);
      return Success();
   }
   catch (boost::filesystem::filesystem_error& e)
   {
      // If we're not attempting to move cross device or the error isn't a cross device link error,
      // return the error.
      if (!in_moveCrossDevice || (e.code() != boost::system::errc::cross_device_link))
      {
         return convertFilesystemError(e, ERROR_LOCATION);
      }
   }

   // If we got this far we need to attempt a cross device move.

   // Moving to a directory should move the current file/directory into the destination rather than over it.
   FilePath target = in_destination.isDirectory() ? in_destination.childPath(getFileName()) : in_destination;
   Error error = copyDirectoryRecursive(target);
   if (error)
      return error;

}

Error FilePath::openForRead(std::shared_ptr<std::istream>& out_stream) const
{
   try
   {
      std::shared_ptr<std::ifstream> stream(
         new std::ifstream(absolutePath().c_str(), std::ios_base::in | std::ios_base::binary));
      if (!(*stream))
      {
         return systemError(
            boost::system::errc::no_such_file_or_directory,
            "Unable to open file for read: " + absolutePath(),
            ERROR_LOCATION);
      }

      out_stream = stream;
   }
   catch (const std::exception& e)
   {
      std::string message = e.what();
      message += "\n    Path: " + absolutePath();
      return systemError(boost::system::errc::io_error, message, ERROR_LOCATION);
   }

   return Success();
}

Error FilePath::openForWrite(std::shared_ptr<std::ostream>& out_stream, bool in_truncate) const
{
   try
   {
      std::ios_base::openmode flags =
         std::ios_base::out |
         std::ios_base::binary |
         (in_truncate ? std::ios_base::trunc : std::ios_base::app);
      std::shared_ptr<std::ostream> stream(new std::ofstream(absolutePath().c_str(), flags));

      if (!*stream)
      {
         return systemError(
            boost::system::errc::no_such_file_or_directory,
            "Unable to open file for write: " + absolutePath(),
            ERROR_LOCATION);
      }

      out_stream = stream;
   }
   catch (const std::exception& e)
   {
      std::string message = e.what();
      message += "\n    Path: " + absolutePath();
      return systemError(boost::system::errc::io_error, message, ERROR_LOCATION);
   }


   return Success();
}

Error FilePath::remove() const
{
   if (exists())
   {
      try
      {
         if (isDirectory())
            boost::filesystem::remove_all(m_impl->Path);
         else
            boost::filesystem::remove(m_impl->Path);
      }
      catch (const boost::filesystem::filesystem_error& e)
      {
         return convertFilesystemError(e, ERROR_LOCATION);
      }
   }

   return Success();
}

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

