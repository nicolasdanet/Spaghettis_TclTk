#! /usr/bin/env bash

# "PureData" folder automagically filled.
# 
#   ./MakeGNU.sh

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only"; exit 1; }

# Script must be executed at the same level.

rep=$(pwd)

[ "${0%/*}" = "." ] || { echo >&2 "${0##*/}: Must be executed at the same level"; exit 1; }

# Depedencies (http://stackoverflow.com/a/22592801 ).

isAlsa=$(dpkg-query -W -f='${Status}' libasound2 2>/dev/null | grep -c "ok installed")
isTcl_5=$(dpkg-query -W -f='${Status}' tcl8.5-dev 2>/dev/null | grep -c "ok installed")
isTk_5=$(dpkg-query -W -f='${Status}' tk8.5-dev 2>/dev/null | grep -c "ok installed")
isTcl_6=$(dpkg-query -W -f='${Status}' tcl8.6-dev 2>/dev/null | grep -c "ok installed")
isTk_6=$(dpkg-query -W -f='${Status}' tk8.6-dev 2>/dev/null | grep -c "ok installed")
isJack=$(dpkg-query -W -f='${Status}' libjack-dev 2>/dev/null | grep -c "ok installed")

isTcl=$((${isTcl_5}+${isTcl_6}))
isTk=$((${isTk_5}+${isTk_6}))

[ isTk -ne 0 ] || { echo >&2 "${0##*/}: tk8.*-dev package required"; exit 1; }
[ isTcl -ne 0 ] || { echo >&2 "${0##*/}: tcl8.*-dev package required"; exit 1; }
[ isAlsa -eq 1 ] || { echo >&2 "${0##*/}: libasound2 package required"; exit 1; }

echo "SUCCEEDED"
