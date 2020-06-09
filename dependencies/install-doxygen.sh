#!/usr/bin/env bash

#
# install-doxygen
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

# Install doxygen build dependencies, if not installed
set -e

# Ensure we're in the directory of this script.
cd "$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"

# Source shared script
. ./base-script.sh

if [[ $(haveCommand "yum") -eq 1 ]]; then
  sudo yum update -y
  sudo yum install -y wget flex bison libc6 make binutils python texlive-full gcc gcc-c++ git

  if [[ $(haveCommand "cmake") -eq 0 ]]; then
    sudo yum install -y cmake
  fi
else
  sudo apt update
  sudo apt install -y wget flex bison libc6 make binutils python texlive-full gcc g++ git

  if [[ $(haveCommand "cmake") -eq 0 ]]; then
    sudo apt install -y cmake
  fi
fi

# Check python and cmake version
PYTHON_VER="$(python --version 2>&1 | cut -d ' ' -f 2)"
PY_VER_MINOR="${PYTHON_VER#*.}"
PY_VER_MINOR="${PY_VER_MINOR%%.*}"
if [[ "${PYTHON_VER%%.*}" -lt 2 ]] || [[ "${PYTHON_VER%%.*}" -eq 2 && $PY_VER_MINOR -lt 6 ]]; then
    printf "Error: Python Version 2.6+ required. Have: %s" "$PYTHON_VER" >&2
    exit 1
fi

IFS=" " read -r -a CMAKE_VER <<< "$(cmakeVersion)"
if [[ ${#CMAKE_VER[@]} -ne 3 ]] ||
   [[ ${CMAKE_VER[0]} -lt 3 ]] ||
   [[ ${CMAKE_VER[0]} -eq 3 && ${CMAKE_VER[1]} -lt 1 ]] ||
   [[ ${CMAKE_VER[1]} -eq 3 && ${CMAKE_VER[1]} -eq 1 && ${CMAKE_VER[2]} -lt 3 ]]; then
    printf "Error: CMake Version 3.1.3+ required. Have: %s.%s.%s" "${CMAKE_VER[@]}" >&2
    exit 1
fi

# Download Doxygen Source
mkdir -p temp
DOXYGEN_VER="1.8.16"
DOXYGEN_TAR="doxygen-${DOXYGEN_VER}.src.tar.gz"
wget "https://s3.amazonaws.com/rstudio-buildtools/doxygen/${DOXYGEN_TAR}" -O "temp/${DOXYGEN_TAR}"

tar -xzf "temp/${DOXYGEN_TAR}" -C temp/

mkdir -p "temp/doxygen-${DOXYGEN_VER}/build"
pushd "temp/doxygen-${DOXYGEN_VER}/build"

cmake -G "Unix Makefiles" ..
make

sudo make install

popd

rm -rf temp
