#!/usr/bin/env bash

#
# rlps-version
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
#
# This script reads, and updates, official RStudio Launcher Plugin SDK build numbers,
# based on information stored in S3 and observed in the local repository.
#
# The rules observed are as follows:
#
# 1. Every package of the RStudio Launcher Plugin SDK results in a new patch release
#    formatted as:
#
#    major.minor.patch
#
#    On S3, the file 'rlps-patch.csv' contains a list of commits and
#    their associated patch versions as a baseline for determining future
#    patch versions.
#
# The script is typically used by the build script to bump the build versions,
# but it can also be invoked manually. Pass "debug" as the last parameter to
# see what the script would do (in this mode debug output is written and no
# changes are saved to S3).

# abort on error
set -e

if [[ "$#" -lt 2 ]]; then
    # TODO: add "set" command to move forward
    echo "Usage: version.sh [get|bump] [major.minor] [debug]"
    exit 1
fi

# read arguments
ACTION=$1
VERSION=$2
if [[ "$3" == "debug" ]]; then
    DEBUG=true
else
    DEBUG=false
fi

function log() {
    if [[ $DEBUG = true ]]; then
        echo "$@"
    fi
}

if [[ $DEBUG = false ]]; then
    EXTRA_CP_ARGS=--quiet
fi

# get historical patch versions from AWS; this file is a CSV that
# contains each RLP SDK commit and the patch version associated with it
aws s3 cp s3://rstudio-launcher-plugin-sdk/version/$VERSION/rlps-patch.csv /tmp/rlps-patch.csv $EXTRA_CP_ARGS

# read the CSV listing commits and associated patch releases
MAX_PATCH=0
while read HISTORY; do
    ENTRY=(${HISTORY//,/ })
    ENTRY_COMMIT=${ENTRY[0]}
    ENTRY_PATCH=${ENTRY[1]}

    # record the highest patch we've seen so far
    if [[ $MAX_PATCH -lt $ENTRY_PATCH ]]; then
        MAX_PATCH=$ENTRY_PATCH
    fi

    # check this entry to see if it corresponds to a commit in our history
    for i in "${!COMMITS[@]}"; do
        if [[ "${COMMITS[$i]}" == "$ENTRY_COMMIT" ]]; then
            PATCH_INDEX=$i
            PATCH=${ENTRY[1]}
            log "Found patch version $PATCH at revision $PATCH_INDEX ($ENTRY_COMMIT)"
            break
        fi
    done

    # if we found a patch release, we're done
    if [[ -n "$PATCH" ]]; then
        break
    fi
done < /tmp/rlps-patch.csv

# did we find a patch version? if not, just use the highest one we found
if [[ -z "$PATCH" ]]; then
    log "Warning: no patch found for commit ${COMMITS[0]}; presuming $MAX_PATCH"
    PATCH=$MAX_PATCH
    PATCH_INDEX=1
fi
case "$ACTION" in
    get)
        echo "$VERSION.$PATCH";;

    bump)
        # record date for timestamp in CSV
        TIMESTAMP=$(date -u '+%Y-%m-%d %H:%M:%S')

        # increment to highest observed patch release
        PATCH=$(($MAX_PATCH+1))

        RLPS_VERSION="$VERSION.$PATCH"

        log "Creating new patch release $RLPS_VERSION"

        # write temporary file marking all commits until the last known one as
        # belonging to this build
        rm -f /tmp/history-prepend.csv && touch /tmp/history-prepend.csv
        for ((i = 0; i <= PATCH_INDEX - 1; i++)); do
            log "Marking commit ${COMMITS[$i]} for patch $RLPS_VERSION"
            echo "${COMMITS[$i]},$PATCH,$TIMESTAMP" >> /tmp/history-prepend.csv
        done

        # now prepend and push to s3
        cat /tmp/history-prepend.csv /tmp/rlps-patch.csv > /tmp/rlps-updated.csv
        if [[ $DEBUG = true ]]; then
            echo "Push updated patch history to S3 and git"
        else
            # upload to s3
            aws s3 cp /tmp/rlps-updated.csv s3://rstudio-launcher-plugin-sdk/version/$VERSION/rlps-patch.csv --quiet

            # tag the release on git (TODO: need Jenkins creds to do this)
            # git tag "v$RLPS_VERSION"
            # git push -q origin "v$RLPS_VERSION"
        fi

        # echo newly created version
        echo "$RLPS_VERSION"
esac
