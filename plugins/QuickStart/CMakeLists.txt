# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019 by RStudio, Inc.
#
# TODO: License
#

cmake_minimum_required(VERSION 3.14)
project(rstudio_quickstart_plugin)

set(CMAKE_CXX_STANDARD 11)

# include files
file (GLOB_RECURSE QUICKSTART_HEADER_FILES "*.h*")

# source files
set(QUICKSTART_SOURCE_FILES
)

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

