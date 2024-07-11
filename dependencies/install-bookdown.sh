#!/usr/bin/env bash

#
# install-bookdown
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

# Exit on failed command
set -e

# Make sure we're in the directory of this script
cd "$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"

# Source common functions.
. ./base-script.sh

# Check for R.
INSTALL_R=1
if [[ $(haveCommand "R") -ne 0 ]]; then
  R_VER=$(Rscript -e "cat(R.Version()[['major']])")
  if [[ $R_VER -ge 3 ]]; then
    INSTALL_R=0
  fi
fi

if [[ $INSTALL_R -eq 1 ]]; then
  if [[ $(haveCommand "yum") -eq 1 ]]; then
    sudo yum update -y
    sudo yum install -y epel-release wget
    sudo yum install -y R
  else
    sudo apt-get -y  install apt-transport-https software-properties-common
    sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9
    sudo add-apt-repository -y 'deb https://cloud.r-project.org/bin/linux/ubuntu bionic-cran35/'
    sudo apt update
    sudo apt-get -y install r-base wget
  fi
fi

# Install Pandoc
PANDOC_VER="2.7.3"
PANDOC_INSTALL_DIR="/usr/lib/rstudio/bin/pandoc/"
PANDOC_DIR="pandoc-${PANDOC_VER}"
PANDOC_TAR="${PANDOC_DIR}-linux.tar.gz"
PANDOC_URL="https://s3.amazonaws.com/rstudio-buildtools/pandoc/${PANDOC_VER}/${PANDOC_TAR}"

if ! [ -d "${PANDOC_INSTALL_DIR}" ]; then

  mkdir -p "${PANDOC_INSTALL_DIR}"

  DOWNLOAD_DIR="$(makeTmpDir "rlps-pandoc")"
  pushd "$DOWNLOAD_DIR"

  wget "${PANDOC_URL}"
  tar -xzf "${PANDOC_TAR}"
  pushd "${PANDOC_DIR}/bin"

  chmod 755 pandoc*
  cp pandoc* "${PANDOC_INSTALL_DIR}"

  popd
  popd

  rm -rf temp
fi

# Install R libraries
R -s --vanilla <<EOF
if (!require("bookdown", quietly = TRUE)) {
  options(repos = c(CRAN = "https://packagemanager.posit.co/all/__linux__/bionic/latest"))
  install.packages("bookdown", dependencies = TRUE)

  if (!require("bookdown", quietly = TRUE)) {
    message("Unable to install bookdown.")
    warnings()
    q(status = 1)
  }
}

EOF
