#! /usr/bin/env bash

# "PureData" folder automagically created.
# 
#   ./buildGNU.sh
#
# Tested on:
#
#   - Debian / Wheezy (i386)
#

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only"; exit 1; }

# Script must be executed at the same level.

rep=$(pwd)

[ "${0%/*}" = "." ] || { echo >&2 "${0##*/}: Must be executed at the same level"; exit 1; }

# Test depedencies ( http://stackoverflow.com/a/22592801 ).

isAlsa=$(dpkg-query -W -f='${Status}' libasound2 2>/dev/null | grep -c "ok installed")
isTcl_5=$(dpkg-query -W -f='${Status}' tcl8.5 2>/dev/null | grep -c "ok installed")
isTk_5=$(dpkg-query -W -f='${Status}' tk8.5 2>/dev/null | grep -c "ok installed")
isTcl_6=$(dpkg-query -W -f='${Status}' tcl8.6 2>/dev/null | grep -c "ok installed")
isTk_6=$(dpkg-query -W -f='${Status}' tk8.6 2>/dev/null | grep -c "ok installed")
isJack=$(dpkg-query -W -f='${Status}' libjack-dev 2>/dev/null | grep -c "ok installed")

[ ${isTk_5} -eq 1 ]  || [ ${isTk_6} -eq 1 ]  || { echo >&2 "${0##*/}: tk8.* package required"; exit 1; }
[ ${isTcl_5} -eq 1 ] || [ ${isTcl_6} -eq 1 ] || { echo >&2 "${0##*/}: tcl8.* package required"; exit 1; }
[ ${isAlsa} -eq 1 ]  || { echo >&2 "${0##*/}: libasound2 package required"; exit 1; }

# Paths.

folder="${rep}/PureData"
bin="${rep}/bin"
tcl="${rep}/tcl"
patches="${rep}/resources/patches"

# Do not overwrite previous build.

[ -e "${folder}" ] && { echo >&2 "${0##*/}: ${folder} already exist"; exit 1; }

# Build the binaries (JACK library is used by default).

cd "${rep}/src"                         || exit 1

if [ ${isJack} -eq 1 ] ; then
    echo "Build with JACK ... "
    make -f makefile.gnu JACK=TRUE      || exit 1
else
    make -f makefile.gnu                || exit 1
fi

cd "${rep}"                             || exit 1

# Create the folder.

echo "Create folder ..."
mkdir "${folder}"                       || exit 1
cp -R "${bin}" "${folder}"              || exit 1
cp -R "${tcl}" "${folder}"              || exit 1

# Install materials.

echo "Install patches ..."
cp -R "${patches}" "${folder}"          || exit 1

# Clean the build.

echo "Clean ..."
cd "${rep}/src"                         || exit 1
make -f makefile.gnu clean              || exit 1
cd "${rep}"                             || exit 1
rmdir "${bin}"                          || exit 1

# End.

echo "SUCCEEDED"
