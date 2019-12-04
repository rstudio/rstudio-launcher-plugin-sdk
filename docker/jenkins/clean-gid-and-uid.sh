#!/usr/bin/env bash

#
# clean-gid-and-uid
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

set -f

# Clean up user id
USERID=$2
USERINFO=$(getent passwd $USERID)

# okay if no user
if [ $? -ne 0 ]; then
    echo "No user exists with id $USERID"
    exit 0
fi

# turn userinfo into a space-separated array and extract the first element
USERINFO=(${USERINFO//:/ })
USERNAME="${USERINFO[0]}"

echo "Removing user $USERNAME with conflicting id $USERID"

# use appropriate command for user deletion
if hash userdel 2>/dev/null; then
    userdel $USERNAME
elif hash deluser 2>/dev/null; then
    deluser $USERNAME
fi

# Clean up group id
GROUPID=$1
GROUPINFO=$(getent group $GROUPID)

# okay if no user
if [ $? -ne 0 ]; then
    echo "No group exists with id $GROUPID"
    exit 0
fi

# turn group info into a space-separated array and extract the first element
GROUPINFO=(${GROUPINFO//:/ })
GROUPNAME="${GROUPINFO[0]}"

echo "Removing group $GROUPNAME with conflicting id $GROUPID"

# use appropriate command for group deletion
if hash groupdel 2>/dev/null; then
    groupdel $GROUPNAME
elif hash delgroup 2>/dev/null; then
    delgroup $GROUPNAME
fi
