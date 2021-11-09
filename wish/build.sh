#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Build Wish.app from sources.

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

# Paths.

tcl="${rep}/tcl"
tk="${rep}/tk"
build="${rep}/build"
embedded="${rep}/embedded"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Get the sources.

urlTcl="https://github.com/tcltk/tcl.git"
urlTk="https://github.com/tcltk/tk.git"

tagTcl="core-8-6-12"
tagTk="core-8-6-12"

[ -e "${tcl}" ] || { git clone -b "${tagTcl}" --depth 1 "${urlTcl}"; }
[ -e "${tk}" ]  || { git clone -b "${tagTk}"  --depth 1 "${urlTk}";  }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Compile and make Wish application.

if [ ! -e "${build}" ]; then
    make -C tcl/macosx embedded
    make -C tk/macosx embedded
fi

if [ ! -e "${embedded}" ]; then
    make -C tcl/macosx install-embedded INSTALL_ROOT=`pwd`/embedded/
    make -C tk/macosx  install-embedded INSTALL_ROOT=`pwd`/embedded/
fi

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
