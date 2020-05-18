#!/usr/bin/env bash

#
# create-test-users.sh
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

echo "Adding users..."

addGroup()
{
  if [[ -z $1 ]]; then
    echo "addGroup() requires one parameter."
    exit 1
  fi

  echo "Adding $1 group..."
  sudo groupadd $1 > /dev/null 2>&1

  # Check if it failed because of any reason except that it already exist (error code 9).
  RES=$?
  if [[ $RES -ne 0 && $RES -ne 9 ]]; then
    echo "Failed."
    exit 1
  elif [[ $RES -eq 9 ]]; then
    echo "Group $1 already exists."
  fi

  echo "Done."
}


addUser()
{
  if [[ -z $1 || -z $2 ]]; then
    echo "addUser() requires at least two parameters."
    exit 1
  fi

  USER=$1
  MAIN_GRP=$2
  SUPP_GRPS=$3

  if [[ ! -z $SUPP_GRPS ]]; then
    echo "Adding user $USER with main group $MAIN_GRP and supplemental groups $SUPP_GRPS..."
    SUPP_GRPS="-G $SUPP_GRPS"
  else
    echo "Adding user $USER with main group $MAIN_GRP..."
  fi

  # Skip the user if it already exists.
  grep "$USER:" < /etc/passwd > /dev/null 2>&1
  if [[ $? -eq 0 ]]; then
    echo "User $USER already exists."
    echo "Done."
    return 0
  fi


  sudo useradd -p "" -g "$MAIN_GRP" $SUPP_GRPS "$USER" > /dev/null
  if [[ $? -ne 0 ]]; then
    echo "Failed."
    exit 1
  fi

  echo "Done."
}

# Create the RStudio Server User, if it does not already exist
grep rstudio-server < /etc/passwd >/dev/null
export ADD_USER=$?

if [[ $ADD_USER -ne 0 ]]; then
  echo "Adding rstudio-server user..."
  sudo useradd --system "rstudio-server"
fi

addGroup "rlpstestgrpone"
addGroup "rlpstestgrptwo"
addGroup "rlpstestgrpthree"

addUser "rlpstestusrone" "rlpstestgrpone"
addUser "rlpstestusrtwo" "rlpstestgrpone" "rlpstestgrptwo,rlpstestgrpthree"
addUser "rlpstestusrthree" "rlpstestgrptwo"
addUser "rlpstestusrfour" "rlpstestgrptwo" "rlpstestgrpthree"
addUser "rlpstestusrfive" "rlpstestgrpone" "rlpstestgrpthree"

echo "Succesfully added test users."
