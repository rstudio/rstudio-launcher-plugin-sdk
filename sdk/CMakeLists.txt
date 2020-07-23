# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019-20 by RStudio, PBC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

cmake_minimum_required(VERSION 3.14)
project(rstudio_launcher_plugin_sdk)

set(CMAKE_CXX_STANDARD 11)

find_library(PTHREAD_LIBS pthread)

# include files
file(GLOB_RECURSE RLPS_HEADER_FILES "*.h*")

# source files
set(RLPS_SOURCE_FILES
   src/AbstractMain.cpp
   src/Error.cpp
   src/api/AbstractPluginApi.cpp
   src/api/Constants.cpp
   src/api/Job.cpp
   src/api/Request.cpp
   src/api/Response.cpp
   src/api/ResponseTypes.cpp
   src/api/stream/AbstractMultiStream.cpp
   src/api/stream/AbstractOutputStream.cpp
   src/api/stream/FileOutputStream.cpp
   src/api/stream/JobStatusStream.cpp
   src/api/stream/JobStatusStreamManager.cpp
   src/api/stream/OutputStreamManager.cpp
   src/comms/AbstractLauncherCommunicator.cpp
   src/comms/MessageHandler.cpp
   src/comms/StdIOLauncherCommunicator.cpp
   src/jobs/AbstractJobStatusWatcher.cpp
   src/jobs/AbstractTimedJobStatusWatcher.cpp
   src/jobs/JobPruner.cpp
   src/jobs/AbstractJobRepository.cpp
   src/jobs/JobStatusNotifier.cpp
   src/json/Json.cpp
   src/logging/FileLogDestination.cpp
   src/logging/Logger.cpp
   src/logging/StderrLogDestination.cpp
   src/logging/SyslogDestination.cpp
   src/options/AbstractUserProfiles.cpp
   src/options/Options.cpp
   src/system/Asio.cpp
   src/system/Crypto.cpp
   src/system/DateTime.cpp
   src/system/FilePath.cpp
   src/system/PosixSystem.cpp
   src/system/Process.cpp
   src/system/ReaderWriterMutex.cpp
   src/system/User.cpp
   src/utils/ErrorUtils.cpp
   src/utils/FileUtils.cpp
)

# include directory
include_directories(
   include
   src
   ${OPENSSL_INCLUDE_DIR}
)

# define library
add_library(rstudio-launcher-plugin-sdk-lib STATIC
   ${RLPS_SOURCE_FILES}
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
   ${PTHREAD_LIBS}
   ${OPENSSL_LIBRARIES}
)

# define executables for unit tests
if (NOT RLPS_UNIT_TESTS_DISABLED)
   add_subdirectory(src/tests)
   add_subdirectory(src/api/tests)
   add_subdirectory(src/comms/tests)
   add_subdirectory(src/jobs/tests)
   add_subdirectory(src/options/tests)
   add_subdirectory(src/system/tests)
endif()
