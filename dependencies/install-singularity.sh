#!/usr/bin/env bash

#
# install-singularity
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

# Don't double install singularity.
SING_VER="3.4.2"
if [ -e "/opt/singularity/${SING_VER}" ]; then
    echo "Singularity is already installed at \"/opt/singularity/${SING_VER}\". Please remove the singularity installation and try again."
    exit 1
fi

# Install singularity dependencies
set -e
if [[ $HAVE_YUM -eq 1 ]]; then
    sudo yum update -y
    sudo yum groupinstall -y 'Development Tools'
    sudo yum install -y openssl-devel libuuid-devel libseccomp-devel wget squashfs-tools
else
    sudo apt update
    sudo apt install -y build-essential libssl-dev uuid-dev libgpgme11-dev squashfs-tools libseccomp-dev wget pkg-config git cryptsetup-bin
fi

# Install Go
GO_VER="1.13"
OS="linux"
shopt -s nocasematch
case $(uname -i) in
    "x86" )     ARCH="386";;
    "x86_64" )  ARCH="amd64";;
    "ARMv6" )   ARCH="armv6l";;
    "ARMv8" )   ARCH="arm64";;
    "ppc64le" ) ARCH="ppc64le";;
    "s390x" )   ARCH="s390x";;
    * )         ARCH="";;
esac

if [[ -n "$ARCH" ]]; then
    echo "Architecture not recognized: $(uname -i)"
    exit
fi

shopt -u nocasematch

GO_TAR="go${GO_VER}.${OS}-${ARCH}.tar.gz"
if [ -e ${GO_TAR} ]; then
    rm ${GO_TAR}
fi

wget "https://dl.google.com/go/${GO_TAR}"
sudo tar -C /opt -xzf "${GO_TAR}"
rm "${GO_TAR}"

export GOPATH=${HOME}/go
export PATH="${PATH}:/opt/go/bin:${GOPATH}/bin"

# Install singularity
SING_TAR="singularity-${SING_VER}.tar.gz"

wget "https://github.com/sylabs/singularity/releases/download/v${SING_VER}/${SING_TAR}"
tar -xzf "${SING_TAR}"
cd singularity

./mconfig --prefix="/opt/singularity/${SING_VER}"
make -C ./builddir
sudo make -C ./builddir install

cd ..
rm "${SING_TAR}"
sudo rm -rf singularity

# Update bashrc
echo "export GOPATH=${GOPATH}" >> ~/.bashrc
echo "export PATH=${PATH}:/opt/singularity/${SING_VER}/bin" >> ~/.bashrc
echo ". /opt/singularity/${SING_VER}/etc/bash_completion.d/singularity" >> ~/.bashrc
