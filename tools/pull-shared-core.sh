#!/usr/bin/env bash

#
# pull-shared-core
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

set -e # exit on failed commands.

# Get the optional branch parameter
BRANCH="master"
if [[ ! -z $1 ]]; then
    BRANCH=$1
fi

ROOT_DIR='..'
if [[ ${PWD##*/} == "tools" ]]; then
    ROOT_DIR="${ROOT_DIR}/.."
fi

SRC_INCLUDE="src/cpp/shared_core/include/shared_core"
SRC_SRC="src/cpp/shared_core"
DEST_INCLUDE="${ROOT_DIR}/sdk/include"
DEST_SRC="${ROOT_DIR}/sdk/src"

# Clone the RStudio repo.
if [[ -d rstudio-clone ]]; then
    cd rstudio-clone
    git status
    if [[ $? -ne 0 ]]; then
        cd ..
        git clone --branch $BRANCH --origin origin --progress -v https://github.com/rstudio/rstudio.git rstudio-clone
        cd rstudio-clone
    else
        git checkout $BRANCH
        git pull
    fi
else
    mkdir rstudio-clone
    git clone --branch $BRANCH --origin origin --progress -v https://github.com/rstudio/rstudio.git rstudio-clone
    cd rstudio-clone
fi

# Copy the files we want.
# These arrays need to have the same length and order (e.g. index of source Json.hpp == index of destination Json.hpp). We're not using maps because Bash 3 doesn't support them.
SRC_INCLUDES=( "Error.hpp" "PImpl.hpp" "json/Json.hpp" "ILogDestination.hpp" "FileLogDestination.hpp" "Logger.hpp" "DateTime.hpp" "FilePath.hpp" "system/User.hpp" )
DEST_INCLUDES=( "Error.hpp" "PImpl.hpp" "json/Json.hpp" "logging/ILogDestination.hpp" "logging/FileLogDestination.hpp" "logging/Logger.hpp" "system/DateTime.hpp" "system/FilePath.hpp" "system/User.hpp")

SRC_SOURCES=( "Error.cpp" "SafeConvert.hpp" "json/Json.cpp" "FileLogDestination.cpp" "Logger.cpp" "StderrLogDestination.hpp" "StderrLogDestination.cpp" "system/SyslogDestination.hpp" "system/SyslogDestination.cpp" "FilePath.cpp" "ReaderWriterMutex.hpp" "ReaderWriterMutex.cpp" "system/User.cpp" )
DEST_SOURCES=( "Error.cpp" "SafeConvert.hpp" "json/Json.cpp" "logging/FileLogDestination.cpp" "logging/Logger.cpp" "logging/StderrLogDestination.hpp" "logging/StderrLogDestination.cpp" "logging/SyslogDestination.hpp" "logging/SyslogDestination.cpp" "system/FilePath.cpp" "system/ReaderWriterMutex.hpp" "system/ReaderWriterMutex.cpp" "system/User.cpp" )


replace()
{
    local DEST=$1
    local BEFORE=$(echo "$2" | sed -E -e 's/\//\\\//g')
    local AFTER=$(echo "$3" | sed -E -e 's/\//\\\//g')

    if [[ $BEFORE == *"\n"* ]]; then
        sed -i -E -e ":start;N;\$!bstart;s/${BEFORE}/${AFTER}/g" "$DEST"
    else
        sed -i -E -e "s/${BEFORE}/${AFTER}/g" "$DEST"
    fi
    ERR=$?

    if [[ ! -s "$DEST" ]]; then
        echo "No output written for 'replace $DEST $2 $3'"
        exit $?
    fi
}

copyFile()
{
    local SRC=$1
    local DEST=$2

    cp -f "$SRC" "$DEST"

    echo "Copying $SRC to $DEST..."

    # Fix namespaces and header guards
    replace "$DEST" 'namespace\s*core' 'namespace launcher_plugins'
    replace "$DEST" 'SHARED_CORE_' 'LAUNCHER_PLUGINS_'
    replace "$DEST" 'namespace\s*log' 'namespace logging'
    replace "$DEST" 'core::' 'launcher_plugins::'
    replace "$DEST" 'log::' 'logging::'
    replace "$DEST" "RSTUDIO_BOOST_NAMESPACE" "boost"
    replace "$DEST" "thread::" "system::"

    # Fix includes
    for I in ${!SRC_INCLUDES[@]}; do
        replace "$DEST" "#include <shared_core/${SRC_INCLUDES[$I]}>" "#include <${DEST_INCLUDES[$I]}>"
        replace "$DEST" "#include \"${SRC_INCLUDES[$I]}\"" "#include <${DEST_INCLUDES[$I]}>"
    done

    # There are some private headers in SRC_SOURCES.
    for I in ${!SRC_SOURCES[@]}; do
        replace "$DEST" "#include <shared_core/${SRC_SOURCES[$I]}>" "#include <${DEST_SOURCES[$I]}>"
        replace "$DEST" "#include \"${SRC_SOURCES[$I]}\"" "#include <${DEST_SOURCES[$I]}>"
    done
}

for I in ${!SRC_INCLUDES[@]}; do
    SRC_FILE=${SRC_INCLUDES[$I]}
    DEST_PATH="$DEST_INCLUDE/${DEST_INCLUDES[$I]}"
    copyFile "$SRC_INCLUDE/$SRC_FILE" "$DEST_PATH"

    # Special cases
    if [[ "$SRC_FILE" == "FilePath.hpp" ]]; then
        replace "$DEST_PATH" "namespace\s*launcher_plugins\s*\{\s*\n" "namespace launcher_plugins \{\nnamespace system \{\n"
        replace "$DEST_PATH" "\n\}\s*//\s*namespace\s*launcher_plugins" "\n\} // namespace system\n\} // namespace launcher_plugins"
    fi

    if [[ "$SRC_FILE" == "Error.hpp" ]]; then
        replace "$DEST_PATH" "\n\s*class\s*FilePath\s*;"  "\nnamespace system \{\n\nclass FilePath;\n\n \} // namespace system\n\n"
        replace "$DEST_PATH" "FilePath(\s*[^;])" "system::FilePath\1"
    fi

    if [[ "$SRC_FILE" == "FileLogDestination.hpp" ]]; then
        replace "$DEST_PATH" "FilePath([^\.;])" "system::FilePath\1"
    fi

    if [[ "$SRC_FILE" == "system/User.hpp" ]]; then
        replace "$DEST_PATH" "class\s*FilePath;\n\n" "\nnamespace system \{\n\nclass FilePath;\n\n\} // namespace system\n"
    fi

done

for I in ${!SRC_SOURCES[@]}; do
    SRC_FILE="${SRC_SOURCES[$I]}"
    DEST_PATH="$DEST_SRC/${DEST_SOURCES[$I]}"

    if [[ "$SRC_FILE" == *".hpp" ]]; then
        copyFile "$SRC_INCLUDE/$SRC_FILE" "$DEST_PATH"
    else
        copyFile "$SRC_SRC/$SRC_FILE" "$DEST_PATH"
    fi

    # Special cases
    if [[ "$SRC_FILE" == "system/SyslogDestination.cpp" ]] || [[ "$SRC_FILE" == "system/SyslogDestination.hpp" ]]; then
        replace "$DEST_PATH" "namespace\s*system" "namespace logging"
        replace "$DEST_PATH" "<logging/SyslogDestination.hpp>" "\"SyslogDestination.hpp\""
    fi

    if [[ "$SRC_FILE" == "system/StderrLogDestination.cpp" ]]; then
        replace "$DEST_PATH" "<logging/StderrLogDestination.hpp>" "\"StderrLogDestination.hpp\""
    fi

    if [[ "$SRC_FILE" == "FilePath.cpp" ]]; then
        replace "$DEST_PATH" "namespace\s*launcher_plugins\s*\{\s*\n" "namespace launcher_plugins \{\nnamespace system \{\n\n"
        replace "$DEST_PATH" "\n\}\s*//\s*namespace\s*launcher_plugins" "\n\} // namespace system\n\} // namespace launcher_plugins"
    fi

    if [[ "$SRC_FILE" == "ReaderWriterMutex.hpp" ]] || [[ "$SRC_FILE" == "ReaderWriterMutex.cpp" ]]; then
        replace "$DEST_PATH" "namespace\s*thread" "namespace system"
        replace "$DEST_PATH" "<system/ReaderWriterMutex.hpp>" "\"ReaderWriterMutex.hpp\""
        replace "$DEST_PATH" "(\(mutex\)\s*)(\\\\\n   try\s*)(\\\\\n   \{\s*)(\\\\)" "\1            \2            \3            \4"
        replace "$DEST_PATH" "(logging::logErrorMessage[^,;]*[;,])\s\s\s\s(\s*\\\\)" "\1\2"
        replace "$DEST_PATH" "(ERROR_LOCATION\s*\)\s*;)(    )" "\2\1"
    fi

    if [[ "$SRC_FILE" == "Error.cpp" ]] || [[ "$SRC_FILE" == "FileLogDestination.cpp" ]]; then
        replace  "$DEST_PATH" "\n\s*namespace\s*rstudio\s*\{" "\n\nusing namespace rstudio::launcher_plugins::system;\n\nnamespace rstudio \{"
    fi

    if [[ "$SRC_FILE" == "Logger.cpp" ]]; then
        replace "$DEST_PATH" "<system/ReaderWriterMutex.hpp>" "\"../system/ReaderWriterMutex.hpp\""
    fi

    if [[ "$SRC_FILE" == "json/Json.cpp" ]]; then
        replace "$DEST_PATH" "\"shared_core/json/rapidjson" "\"json/rapidjson"
    fi

    if [[ "$SRC_FILE" == "system/User.cpp" ]]; then
        replace "$DEST_PATH" "<SafeConvert.hpp>" "\"../SafeConvert.hpp\""
    fi
done

echo "Copying rapidjson library..."
if [[ -e ../sdk/src/json/rapidjson ]]; then
    sudo rm -r ../sdk/src/json/rapidjson
fi
cp -r src/cpp/shared_core/include/shared_core/json/rapidjson ../sdk/src/json/rapidjson

cd ..
sudo rm -r rstudio-clone/
