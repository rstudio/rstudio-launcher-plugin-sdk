#!/usr/bin/env bash

#
# base-script.sh
#
# Copyright (C) 2020 by RStudio, PBC
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

haveCommand()
{
    if [[ -z $1 ]]; then
        echo 0
    elif [[ -x "$(command -v "$1")" ]]; then
        echo 1
    else
        echo 0
    fi
}

cmakeVersion()
{
    if [[ $(haveCommand "cmake") -eq 1 ]]; then
        IFS=" " read -r -a CMAKE_VER_ARR <<< "$(cmake --version)"
        CMAKE_VER="${CMAKE_VER_ARR[2]}"
        CMAKE_VER_MAJOR="${CMAKE_VER%%.*}"
        CMAKE_VER_MINOR="${CMAKE_VER#*.}"
        CMAKE_VER_MINOR="${CMAKE_VER_MINOR%%.*}"
        CMAKE_VER_PATCH="${CMAKE_VER##*.}"

        echo "$CMAKE_VER_MAJOR $CMAKE_VER_MINOR $CMAKE_VER_PATCH"
    fi
}