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

cmake_minimum_required(VERSION 3.15)
project(rlp_sdk_all)

# include globals
include(CMakeGlobals.txt)
set(CMAKE_CXX_STANDARD 11)

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
if (NOT build_type STREQUAL "debug")
   add_compile_definitions(NDEBUG)
endif()

# include boost
if (NOT RLPS_BOOST_REQUESTED_VERSION)
   set(RLPS_BOOST_REQUESTED_VERSION 1.83.0)
endif()
set(Boost_NO_BOOST_CMAKE      OFF)
set(Boost_USE_STATIC_LIBS     ON)
find_package(Boost ${RLPS_BOOST_REQUESTED_VERSION} REQUIRED COMPONENTS
   filesystem
   program_options
   regex
   thread
)

set(RLPS_BOOST_LIBS
   Boost::filesystem
   Boost::program_options
   Boost::regex
   Boost::thread
)

# include openssl
find_package(OpenSSL 1.1.1 REQUIRED COMPONENTS)

set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)

# add the subdirectories
add_subdirectory("sdk")
add_subdirectory("plugins/Local")
add_subdirectory("plugins/QuickStart")
add_subdirectory("smoke-test")
