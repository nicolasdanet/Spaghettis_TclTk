#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# "Spaghettis.app" automagically built.

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

# Fetch or build Wish.

wish="${rep}/wish/embedded/Applications/Utilities/Wish.app"

if [ -e "${wish}" ]; then
    echo "Build with embedded Wish.app ..."
else
    echo "Build Wish.app ..."
    cd "${rep}/wish"    || exit 1
    "./build.sh"        || exit 1
    cd "${rep}"         || exit 1
fi

[ -e "${wish}" ] || { echo >&2 "${0##*/}: cannot find Wish.app"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Paths.

destination="${rep}/Application"
app="${destination}/Spaghettis.app"
plist="${rep}/resources/Info.plist"
bin="${rep}/bin"
tcl="${rep}/tcl"
help="${rep}/resources/help"
font="${rep}/resources/font"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Do NOT overwrite previous build.

[ -e "${destination}" ] && { echo >&2 "${0##*/}: ${destination} already exist"; exit 1; }
[ -e "${app}" ] && { echo >&2 "${0##*/}: ${app} already exist"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the binaries.

if [ -n "${PD_OPTIONS}" ]; then
    echo "Build ${PD_OPTIONS} ..."
else
    echo "Build ..."
fi

cd "${rep}/src"                                                         || exit 1
make -f makefile.mac                                                    || exit 1
cd "${rep}"                                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Make the application bundle.

echo "Build package ..."
mkdir "${destination}"                                                  || exit 1
cp -R "${wish}" "${app}"                                                || exit 1
rm -f "${app}/Contents/Info.plist"                                      || exit 1
rm -f "${app}/Contents/PkgInfo"                                         || exit 1
rm -f "${app}/Contents/version.plist"                                   || exit 1
rm -f "${app}/Contents/MacOS/Wish Shell"                                || exit 1
rm -rf "${app}/Contents/_CodeSignature"                                 || exit 1
rm -f "${app}/Contents/CodeResources"                                   || exit 1
rm -f "${app}/Contents/Resources/Wish.sdef"                             || exit 1
cp -p "${plist}" "${app}/Contents/Info.plist"                           || exit 1
echo "APPL????" > "${app}/Contents/PkgInfo"                             || exit 1
mv "${app}/Contents/MacOS/Wish" "${app}/Contents/MacOS/Spaghettis"      || exit 1
cp -R "${bin}" "${app}/Contents/Resources/"                             || exit 1
cp -R "${tcl}" "${app}/Contents/Resources/"                             || exit 1
cp -R "${help}" "${app}/Contents/Resources/"                            || exit 1
cp -R "${font}" "${app}/Contents/Resources/"                            || exit 1
cd "${app}/Contents/Resources/"                                         || exit 1
ln -s "tcl" "Scripts"                                                   || exit 1
cd "${rep}"                                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Clean the build.

echo "Clean ..."
cd "${rep}/src"                                                         || exit 1
make -f makefile.mac clean                                              || exit 1
cd "${rep}"                                                             || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build and launch the tests.

echo "Build tests ..."
cd "${rep}/tests"                                                       || exit 1
./build.sh                                                              || exit 1

echo "Launch tests ..."
./tests                                                                 || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# End.

echo "SUCCEEDED"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
