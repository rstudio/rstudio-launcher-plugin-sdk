#!/usr/bin/env bash

#
# pull-shared-core
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

set -e # exit on failed commands.

# Get the optional branch parameter
BRANCH="main"
if [[ -n $1 ]]; then
    BRANCH=$1
fi

ROOT_DIR="$(readlink -e "$(dirname "${BASH_SOURCE[0]}")/..")"

# shellcheck source=${ROOT_DIR}/dependencies/base-script.sh
. "${ROOT_DIR}/dependencies/base-script.sh"
RSTUDIO_CLONE_DIR="$(makeTmpDir "rstudio-clone")"

SRC_INCLUDE="${RSTUDIO_CLONE_DIR}/src/cpp/shared_core/include/shared_core"
SRC_SRC="${RSTUDIO_CLONE_DIR}/src/cpp/shared_core"
DEST_INCLUDE="${ROOT_DIR}/sdk/include"
DEST_SRC="${ROOT_DIR}/sdk/src"

# Clone the RStudio repo.
pushd "${RSTUDIO_CLONE_DIR}"

if git status; then
    git fetch
    git checkout "$BRANCH"
    git pull
else
    git clone --branch "$BRANCH" --origin origin --progress -v https://github.com/rstudio/rstudio.git ./
fi

# leave /tmp/rstudio-clone
popd

# Copy the files we want.
# These arrays need to have the same length and order (e.g. index of source Json.hpp == index of destination Json.hpp). We're not using maps because Bash 3 doesn't support them.
SRC_INCLUDES=(
  "Error.hpp"                         #  1
  "PImpl.hpp"                         #  2
  "json/Json.hpp"                     #  3
  "ILogDestination.hpp"               #  4
  "FileLogDestination.hpp"            #  5
  "Logger.hpp"                        #  6
  "FilePath.hpp"                      #  7
  "system/Crypto.hpp"                 #  8
  "system/User.hpp"                   #  9
  "system/PosixSystem.hpp")           # 10
DEST_INCLUDES=(
  "Error.hpp"                         #  1
  "PImpl.hpp"                         #  2
  "json/Json.hpp"                     #  3
  "logging/ILogDestination.hpp"       #  4
  "logging/FileLogDestination.hpp"    #  5
  "logging/Logger.hpp"                #  6
  "system/FilePath.hpp"               #  7
  "system/Crypto.hpp"                 #  8
  "system/User.hpp"                   #  9
  "system/PosixSystem.hpp")           # 10

SRC_SOURCES=(
  "Error.cpp"                         #  1
  "SafeConvert.hpp"                   #  2
  "json/Json.cpp"                     #  3
  "FileLogDestination.cpp"            #  4
  "Logger.cpp"                        #  5
  "StderrLogDestination.hpp"          #  6
  "StderrLogDestination.cpp"          #  7
  "system/SyslogDestination.hpp"      #  8
  "system/SyslogDestination.cpp"      #  9
  "FilePath.cpp"                      # 10
  "ReaderWriterMutex.hpp"             # 11
  "ReaderWriterMutex.cpp"             # 12
  "system/Crypto.cpp"                 # 13
  "system/User.cpp"                   # 14
  "system/PosixSystem.cpp")           # 15
DEST_SOURCES=(
  "Error.cpp"                         #  1
  "SafeConvert.hpp"                   #  2
  "json/Json.cpp"                     #  3
  "logging/FileLogDestination.cpp"    #  4
  "logging/Logger.cpp"                #  5
  "logging/StderrLogDestination.hpp"  #  6
  "logging/StderrLogDestination.cpp"  #  7
  "logging/SyslogDestination.hpp"     #  8
  "logging/SyslogDestination.cpp"     #  9
  "system/FilePath.cpp"               # 10
  "system/ReaderWriterMutex.hpp"      # 11
  "system/ReaderWriterMutex.cpp"      # 12
  "system/Crypto.cpp"                 # 13
  "system/User.cpp"                   # 14
  "system/PosixSystem.cpp")           # 15

replace()
{
    local DEST=$1
    local BEFORE=$(echo "$2" | sed -E -e 's/\//\\\//g')
    local AFTER=$(echo "$3" | sed -E -e 's/\//\\\//g')

    if [[ $BEFORE == *"\n"* ]]; then
        sed -i -E -e ":start;N;\$!bstart;s/${BEFORE}/${AFTER}/gm" "$DEST"
    else
        sed -i -E -e "s/${BEFORE}/${AFTER}/g" "$DEST"
    fi
    ERR=$?

    if [[ ! -s "$DEST" ]]; then
        echo "No output written for 'replace $DEST $2 $3'"
        exit $ERR
    fi
}

copyFile()
{
    local SRC=$1
    local DEST=$2

    echo "Copying $SRC to $DEST..."
    set +e # unifdef always returns 1 for some reason
    ERR_TEXT=$(unifdef -U"_WIN32" -U"__APPLE__" "$SRC" > "$DEST" 2>&1)
    set -e

    if [[ "$ERR_TEXT" != "" ]]; then
      echo "Failed to copy file: $ERR_TEXT"
      exit 1
    fi

    # Fix namespaces and header guards
    replace "$DEST" 'namespace\s*core' 'namespace launcher_plugins'
    replace "$DEST" 'SHARED_CORE_' 'LAUNCHER_PLUGINS_'
    replace "$DEST" 'namespace\s*log' 'namespace logging'
    replace "$DEST" 'core::' 'launcher_plugins::'
    replace "$DEST" 'log::' 'logging::'
    replace "$DEST" "RSTUDIO_BOOST_NAMESPACE" "boost"
    replace "$DEST" "thread::" "system::"
    replace "$DEST" "#include <boost/noncopyable.hpp>" "#include <Noncopyable.hpp>"
    replace "$DEST" "boost::noncopyable" "Noncopyable"
    replace "$DEST" "#include <boost/optional.hpp>" "#include <Optional.hpp>"
    replace "$DEST" "boost::optional" "Optional"
    replace "$DEST" "get_value_or" "getValueOr"
    replace "$DEST" "#include <shared_core/DateTime.hpp>" "#include <system/DateTime.hpp>"
    replace "$DEST" "boost::mutex" "std::mutex"
    replace "$DEST" "boost::recursive_mutex" "std::recursive_mutex"
    replace "$DEST" "boost::unique_lock" "std::unique_lock"
    replace "$DEST" "boost::lock_guard" "std::lock_guard"
    replace "$DEST" "boost::condition_variable" "std::condition_variable"
    replace "$DEST" "#include <boost/thread(/mutex)?.hpp>" "#include <mutex>"
    replace "$DEST" "#include <boost/thread/recursive_mutex.hpp>" "#include <mutex>"
    replace "$DEST" "#include <boost/thread/condition_variable.hpp>" "#include <condition_variable>"
    replace "$DEST" "BOOST_CURRENT_FUNCTION" "__FUNCTION__"
    replace "$DEST" "#include <gsl/gsl>\n" ""
    replace "$DEST" "gsl::narrow_cast" "static_cast"
    replace "$DEST" "\n\n\n" "\n\n"

    # Fix includes
    for I in "${!SRC_INCLUDES[@]}"; do
        replace "$DEST" "#include <shared_core/${SRC_INCLUDES[$I]}>" "#include <${DEST_INCLUDES[$I]}>"
        replace "$DEST" "#include \"${SRC_INCLUDES[$I]}\"" "#include <${DEST_INCLUDES[$I]}>"
    done

    # There are some private headers in SRC_SOURCES.
    for I in "${!SRC_SOURCES[@]}"; do
        replace "$DEST" "#include <shared_core/${SRC_SOURCES[$I]}>" "#include <${DEST_SOURCES[$I]}>"
        replace "$DEST" "#include \"${SRC_SOURCES[$I]}\"" "#include <${DEST_SOURCES[$I]}>"
    done
}

for I in "${!SRC_INCLUDES[@]}"; do
    SRC_FILE=${SRC_INCLUDES[$I]}
    DEST_PATH="$DEST_INCLUDE/${DEST_INCLUDES[$I]}"

    copyFile "$SRC_INCLUDE/$SRC_FILE" "$DEST_PATH"

    # Special cases
    if [[ "$SRC_FILE" == "FilePath.hpp" ]]; then
        replace "$DEST_PATH" "namespace\s*launcher_plugins\s*\{\s*\n\n" "namespace launcher_plugins {\nnamespace system {\n\n"
        replace "$DEST_PATH" "\n\n\}\s*//\s*namespace\s*launcher_plugins" "\n\n} // namespace system\n} // namespace launcher_plugins"
        replace "$DEST_PATH" "#include <boost/utility.hpp>\n\n" ""
        replace "$DEST_PATH" "(#include <cstdint>)" "#include <Noncopyable.hpp>\n\n\1"
        replace "$DEST_PATH" "(#include <logging/Logger.hpp>\n)(#include <PImpl.hpp>\n)" "\2\1"
        replace "$DEST_PATH" "(bool\s*isHidden\s*\(\s*\)\s*const\s*;\s*\n)(.*\n)*\s*bool\s*isJunction\s*\(\s*\)\s*const\s*;\n\n" "\1"
    fi

    if [[ "$SRC_FILE" == "Error.hpp" ]]; then
        replace "$DEST_PATH" "\n\s*class\s*FilePath\s*;"  "\nnamespace system \{\n\nclass FilePath;\n\n \} // namespace system\n\n"
        replace "$DEST_PATH" "FilePath(\s*[^;])" "system::FilePath\1"
        replace "$DEST_PATH" "#include <boost/current_function.hpp>\n" ""
        replace "$DEST_PATH" "#include <boost/system/error_code.hpp>\n\n" ""
        replace "$DEST_PATH" "([\\])[ \t]*(\n)" "                \1\2"
        replace "$DEST_PATH" "(ErrorLocation\()[ \t]*([\\])" "\1 \2"
        replace "$DEST_PATH" "([+])[ \t]*([\\])" "\1   \2"
        replace "$DEST_PATH" "(;     )                ([\\])" "\1\2"
        replace "$DEST_PATH" "(\"Unknown exception\", \"\"\);)[ \t]*([\\])" "\1                 \2"
        replace "$DEST_PATH" "(bool\s*operator==\s*\(\s*const\s*Error&\s*in_other\s*\)\s*const\s*;\s*\n)(.*\n)*\s*bool\s*operator==\s*\(\s*const\s*boost::[^\)]*\)[^\n]*\n\n" "\1"
        replace "$DEST_PATH" "(bool\s*operator!=\s*\(\s*const\s*Error&\s*in_other\s*\)\s*const\s*;\s*\n)(.*\n)*\s*bool\s*operator!=\s*\(\s*const\s*boost::[^\)]*\)[^\n]*\n\n" "\1"
        replace "$DEST_PATH" "(Error\s*\(\s*const\s*Error&\s*in_other\s*\)\s*;\s*\n)(.*\n)*\s*Error\s*\(\s*const\s*boost::system::error_condition[^\n]*\n\s*std::string\s*in_message\s*,\s*\n[^\n]*\n[^\n]*\n\n" "\1"
        replace "$DEST_PATH" "(#include <logging/Logger.hpp>\n)(#include <PImpl.hpp>\n)" "\2\1"
        replace "$DEST_PATH" "//\s*return\s*a\s*printable.*\n//\s*might\s*require.*\nstd::string\s*errorDesc.*\nstd::string\s*errorMess.*\n*//\s*return the error.*\nstd::string\s*systemError.*\n\n" ""
    fi

    if [[ "$SRC_FILE" == "FileLogDestination.hpp" ]]; then
        replace "$DEST_PATH" "FilePath([^\.;])" "system::FilePath\1"
    fi

    if [[ "$SRC_FILE" == "json/Json.hpp" ]]; then
       replace "$DEST_PATH" "out_values = boost::none;\n\n\s*" ""
    fi

    if [[ "$SRC_FILE" == "system/Crypto.hpp" ]]; then
        replace "$DEST_PATH" "Error getLastCryptoError\(const ErrorLocation& in_location\);\n\n" ""
    fi

    if [[ "$SRC_FILE" == "system/User.hpp" ]]; then
        replace "$DEST_PATH" "(#include <string>)\n" "\1"
        replace "$DEST_PATH" "(getUserHomePath\([^\)]*\);)\n" "\1"
        replace "$DEST_PATH" "(PRIVATE_IMPL\(m_impl\);)\n" "\1"
        replace "$DEST_PATH" "class\s*FilePath;\n\n" "\nnamespace system \{\n\nclass FilePath;\n\n\} // namespace system\n"
    fi
done

for I in "${!SRC_SOURCES[@]}"; do
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
        replace "$DEST_PATH" "(#include <logging/Logger.hpp>\n)(#include <Error.hpp>\n)" "\2\1"
        replace "$DEST_PATH" "(#include <system/User.hpp>\n)" "\1#include <utils/ErrorUtils.hpp>\n"
        replace "$DEST_PATH" "#if\s*" "#if "
        replace "$DEST_PATH" "(Error\(e.code[^\)]*\))" "utils::createErrorFromBoost\1"
        replace "$DEST_PATH" "(error)(\(e.code[^\)]*\))" "\1 = utils::createErrorFromBoostError\2"
        replace "$DEST_PATH" "[ \t]*// NOTE: On Windows, we need to explicitly[^\n]*\n[^\n]*\n[^\n]*\n[^\n]*\n" ""
        replace "$DEST_PATH" "(is_directory[^\n]*)\n[^;]*" "\1"
        replace "$DEST_PATH" "\nbool\s*FilePath::isJunction[^\n]*\n[^\n]*\n[^\n]*\n[^\n]*\n" ""
    fi

    if [[ "$SRC_FILE" == "ReaderWriterMutex.hpp" ]] || [[ "$SRC_FILE" == "ReaderWriterMutex.cpp" ]]; then
        replace "$DEST_PATH" "namespace\s*thread" "namespace system"
        replace "$DEST_PATH" "<system/ReaderWriterMutex.hpp>" "\"ReaderWriterMutex.hpp\""
        replace "$DEST_PATH" "(\(mutex\)\s*)(\\\\\n   try\s*)(\\\\\n   \{\s*)(\\\\)" "\1            \2            \3            \4"
        replace "$DEST_PATH" "(logging::logErrorMessage[^,;]*[;,])\s\s\s\s(\s*\\\\)" "\1\2"
        replace "$DEST_PATH" "(ERROR_LOCATION\s*\)\s*;)(    )" "\2\1"
        replace "$DEST_PATH" "boost/thread/exceptions.hpp" "system_error"
        replace "$DEST_PATH" "catch \(const boost::thread_resource_error[^)]*\)" "catch (const std::system_error\& e)               "
        replace "$DEST_PATH" "logErrorMessage\(\"Failed to acquire lock[^\n]*\n[^\\]*" "logError(systemError(e, ERROR_LOCATION));                                  "
    fi

    if [[ "$SRC_FILE" == "Error.cpp" ]] || [[ "$SRC_FILE" == "FileLogDestination.cpp" ]]; then
        replace "$DEST_PATH" "\n\s*namespace\s*rstudio\s*\{" "\n\nusing namespace rstudio::launcher_plugins::system;\n\nnamespace rstudio \{"
        replace "$DEST_PATH" "(#include <ostream>)" "\1\n\n#include <boost/system/error_code.hpp>"
        replace "$DEST_PATH" "Optional<Error> Cause" "Error Cause"
        replace "$DEST_PATH" "[ \t]*Error::Error\s*\(\s*const\s*boost::system::error_code\s*&\s*in_ec\s*,\s*const\s*ErrorLocation\s*&\s*in_location\s*\).*\n(.*\n)*\s*(Error::Error\(std::string\s*in_name\s*,\s*int\s*in_code\s*,\s*const\s*ErrorL)" "\2"
        replace "$DEST_PATH" "[ \t]*bool\s*Error::operator==\s*\(\s*const\s*boost::.*\n(.*\n)*\s*(bool\s*Error::operator!=\(\s*const\s*r)" "\2"
        replace "$DEST_PATH" "[ \t]*bool\s*Error::operator!=\s*\(\s*const\s*boost::.*\n(.*\n)*\s*(void\s*Error::addOrUpdateProperty\s*\(\s*const\s*std::string\s*&\s*in_name\s*,\s*const\s*std::)" "\2"
        replace "$DEST_PATH" "using\s*namespace\s*boost[^\n]*\n\s*return\s*Error\([^,]*,[^,]*(,[^\)]*)\);" "boost::system::error_code ec(in_code, boost::system::system_category());\n   Error error(\"SystemError\", in_code, ec.message()\1);\n   error.addProperty(\"subcategory\", ec.category().name());\n   return error;"
        replace "$DEST_PATH" "return\s*Error\((\s*in_error\.code\(\)\.category\(\)[^,]*)([^;]*);" "Error error(\"SystemError\"\2;\n   return error;"
        replace "$DEST_PATH" "return\s*Error\(\s*(in_error\.code\(\)\.category\(\)[^,]*),\n([^\n]*\n)([^\n]*\n)([^\n]*\n)([^;]*;\n)" "Error error(\n      \"SystemError\",\n\2\3\4\5\n   return error;\n"
        replace "$DEST_PATH" "return\s*Error\((in_code[^,]*)([^;]*);" "Error error(\"SystemError\"\2;\n   return error;"
        replace "$DEST_PATH" "(}\s*//\s*anonymous\s*namespace\s*\n\n)" "\1std::string errorDescription(const Error\& error);\nstd::string errorMessage(const launcher_plugins::Error\& error);\nstd::string systemErrorMessage(int code);\n\n"
        replace "$DEST_PATH" "(Error\(\")s(ystem)(\",)" "\1S\2Error\3"
    fi

    if [[ "$SRC_FILE" == "Logger.cpp" ]]; then
        replace "$DEST_PATH" "<system/ReaderWriterMutex.hpp>" "\"../system/ReaderWriterMutex.hpp\""
        replace "$DEST_PATH" "(#include <sstream>\n\n)" "\1#include <boost/algorithm/string.hpp>\n\n"
        replace "$DEST_PATH" "(#include <Noncopyable.hpp>\n#include <Optional.hpp>\n)\n(#include <system/DateTime.hpp>\n)(#include <Error.hpp>\n)(#include <logging/ILogDestination.hpp>\n)" "\3\1\4\2"
        replace "$DEST_PATH" "using\s*namespace\s*boost::posix_time;\n\s*ptime[^;]*;\n\n\s*oss[^,]*,\s*([^)]*)\)" "oss << system::DateTime().toString(\1)"
        replace "$DEST_PATH" "[^\n]*\n[^\n]*\n([ \t]*static\s*Logger)\*\s*(logger)\s*=\s*new\s*Logger\(\s*\);" "\1 \2;"
        replace "$DEST_PATH" "return\s*\*\s*logger;" "return logger;"
    fi

    if [[ "$SRC_FILE" == "json/Json.cpp" ]]; then
        replace "$DEST_PATH" "\"shared_core/json/rapidjson" "\"json/rapidjson"
        replace "$DEST_PATH" "(#include <Error.hpp>)" "\1\n#include <utils/ErrorUtils.hpp>"
        replace "$DEST_PATH" "json-parse" "JsonParseError"
        replace "$DEST_PATH" "json-pointer-parse" "JsonPointerParseError"
        replace "$DEST_PATH" "\s(Error\([^)\"])" " utils::createErrorFromBoost\1"
        replace "$DEST_PATH" "(\serror)(\()" "\1 = utils::createErrorFromBoostError\2"
    fi

    if [[ "$SRC_FILE" == "system/Crypto.cpp" ]]; then
        replace "$DEST_PATH" "(\n}[ \t]*//[ \t]*anonymous[ \t]*namespace\n)(([^\n]*\n)*[ \t]*boost::system::errc::bad_message[^\n]*\n[^\n]*\n[^\n]*\n[^\n]*\n)" "\2\1"
        replace "$DEST_PATH" "(return\s*)system(Error\(\s*\n\s*)boost::system::errc::not_supported" "\1\2\"OpenSSLError\",\n         -1"
        replace "$DEST_PATH" "(return\s*)system(Error\(\s*\n\s*)boost::system::errc::bad_message" "\1\2\"OpenSSLError\",\n      ec"
    fi

    if [[ "$SRC_FILE" == "system/PosixSystem.cpp" ]]; then
        replace "$DEST_PATH" "(<pwd.h>)\n" "\1"
        replace "$DEST_PATH" "\n\n}" "\n}"
    fi

    if [[ "$SRC_FILE" == "system/User.cpp" ]]; then
        replace "$DEST_PATH" "<SafeConvert.hpp>" "\"../SafeConvert.hpp\""
        replace "$DEST_PATH" "(#include \"../SafeConvert.hpp\"\n)(#include <system/PosixSystem.hpp>\n)" "\2\n\1"
    fi
done

echo "Copying rapidjson library..."
if [[ -e "${ROOT_DIR}/sdk/src/json/rapidjson" ]]; then
    sudo rm -r "${ROOT_DIR}/sdk/src/json/rapidjson"
fi
cp -r "${RSTUDIO_CLONE_DIR}/src/cpp/shared_core/include/shared_core/json/rapidjson" "${ROOT_DIR}/sdk/src/json/rapidjson"
