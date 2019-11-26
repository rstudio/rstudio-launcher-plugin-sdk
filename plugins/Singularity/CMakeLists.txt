# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019 by RStudio, Inc.
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
project(rstudio_singularity_plugin)

set(CMAKE_CXX_STANDARD 11)

# include files
file(GLOB_RECURSE SINGULARITY_HEADER_FILES "*.h*")

# source files
set(SINGULARITY_SOURCE_FILES
   src/SingularityOptions.cpp
   src/SingularityPluginApi.cpp)

# include directories
include_directories(
   include
   ${RLPS_INCLUDE_DIR}
)

# define executable
add_executable(rstudio-singularity-launcher src/SingularityMain.cpp
   ${SINGULARITY_SOURCE_FILES}
   ${SINGULARITY_HEADER_FILES}
)

#link dependencies
target_link_libraries(rstudio-singularity-launcher 
   rstudio-launcher-plugin-sdk-lib
)

