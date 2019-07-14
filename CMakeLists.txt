# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019 by RStudio, Inc.
#
# TODO: License
#

cmake_minimum_required(VERSION 3.14)
project(rlps_all)

# include globals
include(CMakeGlobals.txt)

message(${RLPS_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 11)

add_subdirectory("sdk")
add_subdirectory("plugins/Singularity")
add_subdirectory("plugins/QuickStart")

