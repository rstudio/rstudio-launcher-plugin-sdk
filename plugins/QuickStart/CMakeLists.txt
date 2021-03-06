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
project(rstudio_quickstart_plugin)

set(CMAKE_CXX_STANDARD 11)

# include files
file (GLOB_RECURSE QUICKSTART_HEADER_FILES "*.h*")

# source files
set(QUICKSTART_SOURCE_FILES
   src/QuickStartJobRepository.cpp
   src/QuickStartJobSource.cpp
   src/QuickStartJobStatusWatcher.cpp
   src/QuickStartOptions.cpp
   src/QuickStartPluginApi.cpp
   src/QuickStartResourceStream.cpp)

# include directories
include_directories(
   include
   ${RLPS_INCLUDE_DIR}
)

# define executable
add_executable(rstudio-quickstart-launcher src/QuickStartMain.cpp
   ${QUICKSTART_SOURCE_FILES}
   ${QUICKSTART_HEADER_FILES}
)

# linke dependencies
target_link_libraries(rstudio-quickstart-launcher
   rstudio-launcher-plugin-sdk-lib
)

