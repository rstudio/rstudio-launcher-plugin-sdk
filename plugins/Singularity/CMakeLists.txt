# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2019 by RStudio, Inc.
#
# TODO: License
#

cmake_minimum_required(VERSION 3.14)
project(rstudio_singularity_plugin)

set(CMAKE_CXX_STANDARD 11)

# include files
file(GLOB_RECURSE SINGULARITY_HEADER_FILES "*.h*")

# source files
set(SINGULARITY_SOURCE_FILES
)

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

