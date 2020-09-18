#!/usr/bin/env bash

#
# compile
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

if [ -z "${CMAKE_BUILD_TYPE}" ]; then
    CMAKE_BUILD_TYPE=Release
fi

cd "$(dirname ${BASH_SOURCE[0]})/../.."

BUILD_DIR=$(readlink -f "cmake-build-${CMAKE_BUILD_TYPE}")

# clean requested
if [[ "${1}" == "clean" ]]; then
    rm -rf "${BUILD_DIR}"
fi

set -e

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"            \
      ..

cmake --build . --target all -- ${MAKEFLAGS}

# TODO: install binaries
