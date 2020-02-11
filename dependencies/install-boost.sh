#!/usr/bin/env bash

#
# install-boost
#
# Copyright (C) 2019 by RStudio, PBC
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

INSTALL_DIR=$(pwd)

# Variables
BOOST_VERSION_NUMBER="1.70.0"
BOOST_VERSION="boost_$(echo "$BOOST_VERSION_NUMBER" | tr "." "_")"
BOOST_TAR="$BOOST_VERSION.tar.bz2"
BOOST_BUILD_DIR="boost-build"
BOOST_URL="https://dl.bintray.com/boostorg/release/$BOOST_VERSION_NUMBER/source/$BOOST_TAR"
BOOST_DEFAULT_DIR="/usr/local/lib/boost/$BOOST_VERSION"

# Install if not already installed.
PREFIX="$BOOST_DIR/boost/$BOOST_VERSION"
if [[ ( -n "$BOOST_DIR"  && ! -e "$PREFIX" ) || ( -z "$BOOST_DIR" && ! -d "$BOOST_DEFAULT_DIR" ) ]];
then
   # Download
   if [ ! -f "$BOOST_TAR" ]; then
      wget -c "$BOOST_URL" -O "$BOOST_TAR"
   fi

   # Untar
   sudo rm -rf $BOOST_BUILD_DIR
   sudo mkdir -p $BOOST_BUILD_DIR
   cd $BOOST_BUILD_DIR

   sudo tar --bzip2 -xf "../$BOOST_TAR"
   cd "$BOOST_VERSION"

   if [[ -n $BOOST_DIR ]];
   then
      mkdir -p "$PREFIX"
      BOOST_BJAM_FLAGS="$BOOST_BJAM_FLAGS --prefix=$BOOST_DIR"
   fi

   echo "$BOOST_BJAM_FLAGS"

   # Bootstrap
   sudo ./bootstrap.sh

   # Build boost with bjam
   sudo ./bjam                      \
        "$BOOST_BJAM_FLAGS"         \
        variant=release             \
        cxxflags="-fPIC -std=c++11" \
        install

   # Cleaup
   cd "$INSTALL_DIR"
   sudo rm -rf "$BOOST_BUILD_DIR"
   sudo rm "${BOOST_TAR}"

elif [[ -n $BOOST_DIR ]]; then
   echo "$BOOST_VERSION_NUMBER already installed in $BOOST_DIR"
else
   echo "$BOOST_VERSION_NUMBER already installed in $BOOST_DEFAULT_DIR"
fi
