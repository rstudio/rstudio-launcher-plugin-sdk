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

} // anonymous namespace

typedef boost::filesystem::path PathType;

struct FilePath::Impl
{
   Impl() {};

   Impl(PathType in_path) : Path(std::move(in_path)) { };

   PathType Path;
};

FilePath::FilePath(std::string in_path) :
   m_impl(new Impl(std::move(in_path)))
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
         childFilePath.m_impl->Path = boost::filesystem::absolute(childPath, m_impl->Path);
         return childFilePath;
      }
      catch (boost::filesystem::filesystem_error& e)
      {
         logging::logError(convertFilesystemError(e, ERROR_LOCATION));
      }
   }

   return *this;
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

} // namespace system
} // namespace launcher_plugins
} // namespace rstudio

