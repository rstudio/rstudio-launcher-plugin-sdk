#!/usr/bin/env bash

#
# make-package
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

# ensure we're in the right directory
cd $(dirname ${BASH_SOURCE[0]})/../..

# Clean up any old version of this package
PKG_NAME="RStudioLauncherPluginSDK-${RLP_SDK_VERSION_MAJOR}.${RLP_SDK_VERSION_MINOR}.${RLP_SDK_VERSION_PATCH}.tar.gz"
ls "${PKG_NAME}" 2>/dev/null
if [[ $? -ne 2 ]]; then
    rm "${PKG_NAME}"
fi

# exit if archiving fails.
set -e

# Readmes are ignored, so explicitly include dependences/README.md
tar --exclude-from "docker/package/exclude-from-package.txt" -zcvf "docker/package/${PKG_NAME}" .
