#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# "PureData.app" automagically built.
# 
# Tested on:
#
#   - Mac OS X 10.6.8 / ActiveTcl 8.5.18
#

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Script must be executed at the same level.

rep=$(pwd)

[ "${0%/*}" = "." ] || { echo >&2 "${0##*/}: Must be executed at the same level"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Frameworks ( http://jackaudio.org/downloads/ ).

jack="/System/Library/Frameworks/Jackmp.framework/Headers/jack.h"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ActiveTcl ( http://wiki.tcl.tk/1875 ).

wish="/Library/Frameworks/Tk.framework/Versions/8.5/Resources/Wish.app"

if [ -e "${wish}" ]; then
    echo "Build with ActiveTcl 8.5 ..."
else
    wish="/System/Library/Frameworks/Tk.framework/Versions/8.5/Resources/Wish.app"
fi

[ -e "${wish}" ] || { echo >&2 "${0##*/}: cannot find Tk framework"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Paths.

folder="${rep}/Application"
app="${folder}/PureData.app"
plist="${rep}/resources/Info.plist"
bin="${rep}/bin"
tcl="${rep}/tcl"
help="${rep}/resources/help"
extras="${rep}/resources/extras"
patches="${rep}/resources/patches"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Do not overwrite previous build.

[ -e "${folder}" ] && { echo >&2 "${0##*/}: ${folder} already exist"; exit 1; }
[ -e "${app}" ]    && { echo >&2 "${0##*/}: ${app} already exist";    exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Default compiler architecture ( https://stackoverflow.com/questions/246007 ).

foo=$(mktemp build.XXXXXXXXXXXXXXXX)
echo 'int main() { return 0; }' | cc -x c - -o ${foo}
test=$(file ${foo})
rm ${foo}

# Externals suffix.

extension=".pdbundle32"

[[ "${test}" =~ "x86_64" ]] && { echo "Build 64-bit ..."; extension=".pdbundle64"; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the binaries.

cd "${rep}/src"                                                 || exit 1

#echo "Build with JACK ..."
#make -f makefile.mac "WITH_JACK=TRUE"                          || exit 1

echo "Build with PORTAUDIO ..."
echo "Build with DEBUG ..."
make -f makefile.mac "WITH_PORTAUDIO=TRUE" "WITH_DEBUG=TRUE"    || exit 1

cd "${rep}"                                                     || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the hello examples. 

cd "${rep}/resources/examples"                                  || exit 1
make -f makefile.mac "EXTENSION=${extension}"                   || exit 1
cd "${rep}"                                                     || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Make the bundle.

echo "Build package ..."
mkdir "${folder}"                                               || exit 1
cp -R "${wish}" "${app}"                                        || exit 1
rm -f "${app}/Contents/Info.plist"                              || exit 1
rm -f "${app}/Contents/PkgInfo"                                 || exit 1
rm -f "${app}/Contents/MacOS/Wish Shell"                        || exit 1
rm -rf "${app}/Contents/_CodeSignature"                         || exit 1
rm -f "${app}/Contents/CodeResources"                           || exit 1
rm -f "${app}/Contents/Resources/Wish.sdef"                     || exit 1
cp -p "${plist}" "${app}/Contents/Info.plist"                   || exit 1
echo "APPL????" > "${app}/Contents/PkgInfo"                     || exit 1
mv "${app}/Contents/MacOS/Wish" "${app}/Contents/MacOS/Pd"      || exit 1
cp -R "${bin}" "${app}/Contents/Resources/"                     || exit 1
cp -R "${tcl}" "${app}/Contents/Resources/"                     || exit 1
cp -R "${help}" "${app}/Contents/Resources/"                    || exit 1
cp -R "${extras}" "${app}/Contents/Resources/"                  || exit 1
cd "${app}/Contents/Resources/"                                 || exit 1
ln -s "tcl" "Scripts"                                           || exit 1
cd "${rep}"                                                     || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Install materials.

echo "Install patches ..."
cp -R "${patches}" "${folder}"                                  || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Clean the build.

echo "Clean ..."
cd "${rep}/src"                                                 || exit 1
make -f makefile.mac clean                                      || exit 1
cd "${rep}"                                                     || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# End.

echo "SUCCEEDED"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
