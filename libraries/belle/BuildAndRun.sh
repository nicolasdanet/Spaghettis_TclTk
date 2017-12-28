#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Create all the PDF examples.

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

# Files.

files=("-std=c++11" "${rep}/Tools/Belle.cpp")

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Includes.

includes=("-I${rep}/Source/")

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Defines.

defines=("-DPRIM_WITH_TEST=1")

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Warnings.

warnings=("-Wall" "-Wextra" "-Wshadow")

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build the example.

echo "Build Belle ..."

g++ "${files[@]}" -o "${rep}/Belle" "${includes[@]}" "${defines[@]}" "${warnings[@]}" || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Launch it.

echo "Start Belle ..."

"./Belle" || exit 1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------