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

formatHelp()
{
    printf "  %s, %-15s %s\n" "$1" "$2" "$3"
}

printHelp()
{
    echo "Usage: ./install-dependencies.sh [options]"
    echo "Options:"
    formatHelp  "-a" "--all" "Install all dependencies."
    formatHelp "-c" "--core" "Install dependencies required for compiling the RStudio Launcher Plugin SDK."
    formatHelp "-s" "--singularity" "Install dependencies required to compile and use the RStudio Singluarity Launcher Plugin. Includes core dependencies."
    formatHelp "-t" "--tools" "Install dependencies that are needed for scripts in the tools/ folder, or for other tools."
    formatHelp "-h" "--help" "Display this help menu."
}

INSTALL_ALL=0
INSTALL_CORE=0
INSTALL_SINGULARITY=0
INSTALL_TOOLS=0
DO_HELP=0
ERR=0

for ARG in "$@"; do
    case $ARG in
        "-a"|"--all" )          INSTALL_ALL=1 ;;
        "-c"|"--core" )         INSTALL_CORE=1 ;;
        "-s"|"--singularity" )  INSTALL_SINGULARITY=1 ;;
        "-t"|"--tools" )        INSTALL_TOOLS=1 ;;
        "-h"|"--help" )         DO_HELP=1 ;;
        * )                     printf "Invalid argument: \"%s\"\n" $ARG >&2 && DO_HELP=1 && ERR=1;;
    esac
done

if [[ $DO_HELP -ne 0 ]]; then
    printHelp
    exit $ERR
fi

if [[ $INSTALL_ALL -eq 0 ]] &&
   [[ $INSTALL_CORE -eq 0 ]] &&
   [[ $INSTALL_SINGULARITY -eq 0 ]] &&
   [[ $INSTALL_TOOLS -eq 0 ]]; then
    echo "No installation option specified."
    printHelp
    exit 1
fi

if [[ $INSTALL_SINGULARITY -eq 1 ]]; then
    INSTALL_CORE=1
fi

if [[ $IINSTALL_ALL -eq 1 ]]; then
    INSTALL_CORE=1
    INSTALL_SINGULARITY=1
    INSTALL_TOOLS=1
fi

if [[ $INSTALL_CORE -eq 1 ]]; then
    ./install-build-tools.sh
    ./install-boost.sh
fi

if [[ $INSTALL_SINGULARITY -eq 1 ]]; then
    ./install-singularity.sh
fi

if [[ $INSTALL_TOOLS -eq 1 ]]; then
    ./install-doxygen.sh
fi