#!/usr/bin/env bash

#
# install-rsandbox.sh
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

# Exit on failed commands
set -e

# Make sure we're in the directory of this script
cd "$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"

# Source common functions.
. ./base-script.sh

if [[ -z $1 ]]; then
  echo "Usage: ./install-rsandbox <os name>"
  echo "Valid os names:"
  echo "  centos8"
  echo "  debian9"
  exit 1
else
  OS_NAME="$1"
fi

if [[ $(haveCommand "yum") -eq 1 ]]; then
  sudo yum update -y
  sudo yum install -y wget sudo
else
  sudo apt update
  sudo apt install -y wget sudo
fi

DOWNLOAD_DIR="$(makeTmpDir rsandbox-dl)"
RSW_VERSION="1.3.1073-1"
if [[ "$OS_NAME" == "debian9" ]]; then
  TAR_DIR="rsw-monitor-connect--$RSW_VERSION"
else
  TAR_DIR="rsw-monitor-connect-$OS_NAME-$RSW_VERSION"
fi

TAR_FILE="$TAR_DIR.tar.gz"
wget "https://s3.amazonaws.com://rstudio-ide-build/monitor/$OS_NAME/$TAR_FILE" -O "${DOWNLOAD_DIR}/${TAR_FILE}"

tar -xzf "${DOWNLOAD_DIR}/${TAR_FILE}" -C "${DOWNLOAD_DIR}/"
INSTALL_DIR=/opt/tools/rstudio-launcher-plugin-sdk/bin
sudo mkdir -p "$INSTALL_DIR"
sudo cp "${DOWNLOAD_DIR}/${TAR_DIR}/rsandbox" "$INSTALL_DIR"

rm -rf temp
