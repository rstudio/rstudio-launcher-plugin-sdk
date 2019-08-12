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

set(RLPS_OPTIONS_TEST_MAIN ../../tests/TestMain.cpp)

# One executable per suite of Options test because it's a singleton

# Copy the test runner that runs all options tests.
configure_file(options-tests.sh options-tests.sh COPYONLY)

# Copy configuration files used by the tests.
set(CONF_INPUT_FILES
    conf-files/Basic.conf
    conf-files/Empty.conf
    conf-files/Extra.conf
    conf-files/InvalidValue.conf
    conf-files/LogLevelConflict1.conf
    conf-files/LogLevelConflict2.conf
    conf-files/LogLevelConflict3.conf
    conf-files/MissingOptional.conf
)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/conf-files/)
foreach(INP_FILE ${CONF_INPUT_FILES})
    configure_file(${INP_FILE} conf-files/ COPYONLY)
endforeach()

# Basic options all set
add_executable(rlps-basic-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   BasicOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-basic-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Command line options
add_executable(rlps-cmd-line-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   CmdLineOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-cmd-line-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Command line options with conflicts
add_executable(rlps-cmd-line-conflicts-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   CmdLineConflictOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-cmd-line-conflicts-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Custom options 1 (custom option with default & value in config)
add_executable(rlps-custom-options-1-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   CustomOptions1Tests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-custom-options-1-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Custom options 2 (custom option with default & no value in config)
add_executable(rlps-custom-options-2-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   CustomOptions2Tests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-custom-options-2-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Empty options file
add_executable(rlps-empty-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   EmptyOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-empty-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Extra options
add_executable(rlps-extra-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   ExtraOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-extra-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Invalid option value
add_executable(rlps-invalid-value-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   InvalidValueOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-invalid-value-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Log level conflict #1 (log-level < DEBUG)
add_executable(rlps-ll-conflict-1-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   LogLevelConflict1Tests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-ll-conflict-1-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Log level conflict #2 (log-level > INFO)
add_executable(rlps-ll-conflict-2-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   LogLevelConflict2Tests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-ll-conflict-2-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Log level conflict #3 (log-level < DEBUG && enable-debug-logging=1 on the command line)
add_executable(rlps-ll-conflict-3-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   LogLevelConflict3Tests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-ll-conflict-3-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)


# Missing optional values
add_executable(rlps-missing-opt-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   MissingOptionalOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-missing-opt-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Missing required values
add_executable(rlps-missing-req-options-tests
   ${RLPS_OPTIONS_TEST_MAIN}
   MissingRequiredOptionsTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-missing-req-options-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)