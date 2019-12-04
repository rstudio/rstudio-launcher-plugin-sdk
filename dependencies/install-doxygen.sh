#!/usr/bin/env bash

#
# install-doxygen
#
# Copyright (C) 2019 by RStudio, Inc.
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

HAVE_YUM=1
yum 2>/dev/null
if [[ $? -eq 127 ]]; then
  HAVE_YUM=0
fi


# Install doxygen build dependencies, if not installed
set -e
if [[ $HAVE_YUM -eq 1 ]]; then
  yum update -y
  yum install -y wget flex bison libc6 make binutils python cmake texlive-full
else
  apt update
  apt install -y wget flex bison libc6 make binutils python cmake texlive-full
fi

# Check python and cmake version
PYTHON_VER="$(python --version 2>&1 | cut -d ' ' -f 2)"
PY_VER_MINOR="${PYTHON_VER#*.}"
PY_VER_MINOR="${PY_VER_MINOR%%.*}"
if [[ "${PYTHON_VER%%.*}" -lt 2 ]] || [[ "${PYTHON_VER%%.*}" -eq 2 && $PY_VER_MINOR -lt 6 ]]; then
    printf "Error: Python Version 2.6+ required. Have: %s" "$PYTHON_VER" >&2
    exit 1
fi

CMAKE_VER=($(cmake --version))
CMAKE_VER="${CMAKE_VER[2]}"
CMAKE_VER_MAJOR="${CMAKE_VER%%.*}"
CMAKE_VER_MINOR="${CMAKE_VER#*.}"
CMAKE_VER_MINOR="${CMAKE_VER%%.*}"
CMAKE_VER_PATCH="${CMAKE_VER##*.}"
if [[ $CMAKE_VER_MAJOR -lt 3 ]] ||
   [[ $CMAKE_VER_MAJOR -eq 3 && $CMAKE_VER_MINOR -lt 1 ]] ||
   [[ $CMAKE_VER_MAJOR -eq 3 && $CMAKE_VER_MINOR -eq 1 && $CMAKE_VER_PATCH -lt 3 ]]; then
    printf "Error: CMake Version 3.1.3+ required. Have: %s" "$CMAKE_VER" >&2
    exit 1
fi

# Download Doxygen Source
mkdir -p tmp
cd tmp

DOXYGEN_VER="1.8.16"
DOXYGEN_TAR="doxygen-${DOXYGEN_VER}.src.tar.gz"
wget "http://doxygen.nl/files/${DOXYGEN_TAR}"

tar -xzf "${DOXYGEN_TAR}"
cd "doxygen-${DOXYGEN_VER}"
mkdir -p build
cd build

cmake -G "Unix Makefiles" ..
make

make install

cd ../../../
rm -rf tmp
