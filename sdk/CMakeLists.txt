# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019 by RStudio, Inc.
#
# TODO: License
#

cmake_minimum_required(VERSION 3.14)
project(rstudio_launcher_plugin_sdk)

set(CMAKE_CXX_STANDARD 11)

# include files
file(GLOB_RECURSE RLPS_HEADER_FILES "*.h*")

# souce files
set(RLPS_SOURCE_FILES
   src/AbstractMain.cpp
   src/Error.cpp
   src/logging/Logger.cpp
   src/logging/StderrDestination.cpp
   src/logging/SyslogDestination.cpp
)

# include directory
include_directories(
   include
   src
)

# define library
add_library(rstudio-launcher-plugin-sdk-lib STATIC
        ${RLPS_SOURCE_FILES}
        ${RLPS_HEADER_FILES})
