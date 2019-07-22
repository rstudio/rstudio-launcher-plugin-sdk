/*
 * FileLogDestination.cpp
 * 
 * Copyright (C) 2019 by RStudio, Inc.
 *
 * TODO: License
 *
 */

#include "logging/FileLogDestination.hpp"

#include <boost/thread.hpp>

#include <vector>

#include "logging/Logger.hpp"

using namespace rstudio::launcher_plugins::system;

namespace rstudio {
namespace launcher_plugins {
namespace logging {

// FileLogOptions ======================================================================================================
FileLogOptions::FileLogOptions(FilePath in_directory) :
   m_directory(std::move(in_directory)),
   m_fileMode(kDefaultFileMode),
   m_maxSizeMb(kDefaultMaxSizeMb),
   m_doRotation(kDefaultDoRotation)
{
}

FileLogOptions::FileLogOptions(FilePath in_directory, std::string in_fileMode, double in_maxSizeMb, bool in_doRotation) :
   m_directory(std::move(in_directory)),
   m_fileMode(std::move(in_fileMode)),
   m_maxSizeMb(in_maxSizeMb),
   m_doRotation(in_doRotation)
{
}

const FilePath& FileLogOptions::getDirectory() const
{
   return m_directory;
}

const std::string& FileLogOptions::getFileMode() const
{
   return m_fileMode;
}

double FileLogOptions::getMaxSizeMb() const
{
   return m_maxSizeMb;
}

bool FileLogOptions::doRotation() const
{
   return m_doRotation;
}

// FileLogDestination ==================================================================================================
struct FileLogDestination::Impl
{
   Impl(unsigned int in_id, const std::string& in_name, FileLogOptions in_options) :
      LogOptions(std::move(in_options)),
      LogName(in_name + ".log"),
      RotatedLogName(in_name + ".old.log"),
      Id(in_id)
   {
   };

   void openLogFile()
   {
      LogFile = LogOptions.getDirectory().childPath(LogName);
      Error error = LogFile.ensureFileExists();

      // This will log to any other registered log destinations, or nowhere if there are none.
      if (error)
         logError(error);

      error = LogFile.openForWrite(LogOutputStream, false);
      if (error)
         logError(error);
   }

   void rotateLogFile()
   {

   }


   FileLogOptions LogOptions;
   FilePath LogFile;
   std::string LogName;
   std::string RotatedLogName;
   boost::mutex Mutex;
   unsigned int Id;
   std::shared_ptr<std::ostream> LogOutputStream;
};

FileLogDestination::FileLogDestination(unsigned int in_id, std::string in_programId, FileLogOptions in_logOptions) :
   m_impl(new Impl(in_id, in_programId, in_logOptions))
{
}

FileLogDestination::~FileLogDestination()
{
   if (*m_impl->LogOutputStream)
      m_impl->LogOutputStream->flush();
}

unsigned int FileLogDestination::getId() const
{
   return m_impl->Id;
}

void FileLogDestination::writeLog(LogLevel, const std::string& in_message)
{
   if (*m_impl->LogOutputStream)
   {
      (*m_impl->LogOutputStream) << in_message;
   }
}



} // namespace logging
} // namespace launcher_plugins
} // namespace rstudio

