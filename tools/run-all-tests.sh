#!/usr/bin/env bash

#
# run-all-tests
#
# Copyright (C) 2019-20 by RStudio, PBC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Get the build dir
BUILD_DIR=$1
TOOLS_DIR="$(readlink -e "$(dirname "${BASH_SOURCE[0]}")")"

TOTAL_FAILURES=0
runTest()
{
    CURR_DIR=$(pwd)
    cd "${BUILD_DIR}/${1}" || return $?
    ./run-tests.sh

    TOTAL_FAILURES=$(($(cat failures.log) + TOTAL_FAILURES))

    cd "${CURR_DIR}" || exit $?
}

# Create users for test cases
grep rstudio-server < /etc/passwd >/dev/null
ADD_RSTUDIO_SERVER_USER=$?
"${TOOLS_DIR}/create-test-users.sh" $ADD_RSTUDIO_SERVER_USER

RET=$?
if [[ $RET -ne 0 ]]; then
  echo "Failed to create test users."
  exit $RET
fi

# Unit tests first
runTest "sdk/src/tests"
runTest "sdk/src/api/tests"
runTest "sdk/src/comms/tests"
runTest "sdk/src/jobs/tests"
runTest "sdk/src/options/tests"
runTest "sdk/src/system/tests"

# TODO: Integration tests

# Remove test users
"${TOOLS_DIR}/delete-test-users.sh" $ADD_RSTUDIO_SERVER_USER

# Exit
echo "Test failures: $TOTAL_FAILURES"

if [[ $TOTAL_FAILURES -ne 0 ]]; then
  exit 1
fi

exit 0
