#!/usr/bin/env bash

#
# install-dependencies
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

set -e

printHelp()
{
    HELP_FORMAT="  %s, %-15s %s\n"
    echo "Usage: ./install-dependencies.sh [options]"
    echo "Options:"
    printf "$HELP_FORMAT" "-a" "--all" "Install all dependencies."
    printf "$HELP_FORMAT" "-c" "--core" "Install dependencies required for compiling the RStudio Launcher Plugin SDK."
    printf "$HELP_FORMAT" "-s" "--singularity" "Install dependencies required to compile and use the RStudio Singluarity Launcher Plugin. Includes core dependencies."
    printf "$HELP_FORMAT" "-h" "--help" "Display this help menu."
}

for ARG in "$@"; do
    case $ARG in
        "(-a|--all)" )          INSTALL_ALL=1 ;;
        "(-c|--core)" )         INSTALL_CORE=1 ;;
        "(-s|--singularity)" )  INSTALL_SINGULARITY=1 ;;
        "(-h|--help)" )         DO_HELP=1 ;;
        * )                     DO_HELP=1 ;;
    esac
done

if [ ! -z DO_HELP ]; then
    printHelp
    exit 0
fi

if [ -z INSTALL_ALL ] && [ -z INSTALL_CORE ] && [ -z INSTALL_SINGULARITY ]; then
    echo "No installation option specified."
    printHelp
    exit 1
fi

if [ !-z INSTALL_SINGULARITY ]; then
    INSTALL_CORE=1
fi

if [ ! -z INSTALL_ALL ]; then
    INSTALL_CORE=1
    INSTALL_TOOLS=1
    INSTALL_SINGULARITY=1
fi

if [ ! -z INSTALL_CORE ]; then
    ./install-build-tools.sh
    ./install-boost.sh
fi

if [ ! -z INSTALL_SINGULARITY ]; then
    ./install-singularity.sh
fi
