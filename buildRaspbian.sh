#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only";    exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Script must be executed at the same level.

rep=$(pwd)

[ "${0%/*}" = "." ] || { echo >&2 "${0##*/}: Must be executed at the same level";       exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Paths.

destination="${rep}/build"
bin="${rep}/bin"
tcl="${rep}/tcl"
help="${rep}/resources/help"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Remove previous build.

[ -e "${destination}" ] && { rm -r "${destination}"; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the binaries (check model of the Raspberry Pi first).

echo "Build ..."

cd "${rep}/src"                                                     || exit 1
make -f makefile.linux                                              || exit 1
cd "${rep}"                                                         || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Create the directory.

echo "Create directory ..."
mkdir "${destination}"                                              || exit 1
cp -R "${bin}" "${destination}"                                     || exit 1
cp -R "${tcl}" "${destination}"                                     || exit 1
cp -R "${help}" "${destination}"                                    || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Clean the build.

echo "Clean ..."
cd "${rep}/src"                                                     || exit 1
make -f makefile.linux clean                                        || exit 1
cd "${rep}"                                                         || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# End.

echo "SUCCEEDED"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
