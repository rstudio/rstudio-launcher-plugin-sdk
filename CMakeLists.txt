# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019 by RStudio, PBC
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

cmake_minimum_required(VERSION 3.15)
project(rlp_sdk_all)

# include globals
include(CMakeGlobals.txt)

message(${RLPS_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 11)

# include boost
if (NOT RLPS_BOOST_REQUESTED_VERSION)
   set(RLPS_BOOST_REQUESTED_VERSION 1.70.0)
endif()
set(Boost_NO_BOOST_CMAKE      OFF)
set(Boost_USE_STATIC_LIBS     ON)
set(RLPS_BOOST_LIBS
   filesystem
   program_options
   regex
   thread)
find_package(Boost ${RLPS_BOOST_REQUESTED_VERSION} REQUIRED COMPONENTS ${RLPS_BOOST_LIBS})

add_subdirectory("sdk")
add_subdirectory("plugins/Local")
add_subdirectory("plugins/QuickStart")

