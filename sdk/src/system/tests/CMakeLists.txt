# vi: set ft=cmake:

#
# CMakeLists.txt
#
# Copyright (C) 2020 by RStudio, PBC
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

set(RLPS_SYSTEM_TEST_MAIN ../../tests/TestMain.cpp)

# Copy the test runner that runs all options tests.
configure_file(run-tests.sh run-tests.sh COPYONLY)
configure_file(test.sh test.sh COPYONLY)

# Allow files in the tests folder to be included
include_directories(
   ../../tests
)

# Grab the empty conf file from the options tests.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/conf-files/)
configure_file("../../options/tests/conf-files/Empty.conf" conf-files/ COPYONLY)

# AsyncDeadlineEvent Tests
add_executable(rlps-async-deadline-tests
   ${RLPS_SYSTEM_TEST_MAIN}
   AsyncDeadlineTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-async-deadline-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)


# AsyncTimedEvent Tests
add_executable(rlps-async-timer-tests
   ${RLPS_SYSTEM_TEST_MAIN}
   AsyncTimerTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-async-timer-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# DateTime Tests
add_executable(rlps-date-tests
   ${RLPS_SYSTEM_TEST_MAIN}
   DateTimeTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-date-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)

# Process Tests
add_executable(rlps-process-tests
   ${RLPS_SYSTEM_TEST_MAIN}
   SyncChildProcessTests.cpp
   ${RLPS_HEADER_FILES}
)

target_link_libraries(rlps-process-tests
   rstudio-launcher-plugin-sdk-lib
   ${RLPS_BOOST_LIBS}
)
