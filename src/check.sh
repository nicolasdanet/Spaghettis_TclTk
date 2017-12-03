#! /usr/bin/env bash

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Handy to check C/C++ code style.
# Lines containing a "// --" comment are not checked, nor files inside "libraries" folders.
# File starting with "====================================" neither.
# Without any directory nor filename provided, files are looked up in $PWD.
#
# ./check.sh
# ./check.sh -d ../resources
# ./check.sh -d ..
# ./check.sh -f m_pd.h
# ./check.sh -f ../resources/examples/helloCPP.cpp

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

USAGE="Usage: ${0##*/} [ -hv ] [ -f file | -d directory ]"
VERSION="Version: ${0##*/} v2017.12.03"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Bash version.

[ "${BASH_VERSION%.*}" \> "3.1" ] || { echo >&2 "${0##*/}: Bash 3.1 or higher only"; exit 1; }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Expression defined.

LINK_EXPR="/\* < (ht|f)tp.*[^ ] > \*/|// --"
HIDE_EXPR="#define|#else|#endif|#if|#elif|#include|#pragma|#import|https?://|ftp://|// --|// MARK"
EXIT_EXPR="===================================="
FILE_EXPR=".*\.(h|hpp|c|cpp)"
PLUS_EXPR=".*\.(hpp|cpp)"
TEMP_EXPR="template<|template [ ]+<|< >|template <[^ >]|template.+[^ <>]>|template.+>[^ >\(\:]"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

STAT_EXPR+="$HIDE_EXPR"
STAT_EXPR+="|.*[\}\{]"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Error messages

errcarr=">>> Lines ending with carriage return"
errcomm=">>> No space after comma"
errsemi=">>> Space before semicolon"
errstat=">>> Bad spacing or missing brace after statement"
erroper=">>> No space around operator"
errincr=">>> Space around increment"
errdecr=">>> Space around decrement"
errnega=">>> Space after negation"
errpare=">>> Bad spacing around parentheses or brackets"
errtemp=">>> Bad spacing around template"
errkeyw=">>> Bad spacing around operator keyword"
errbrac=">>> No space around brace or colon"
errspac=">>> Overly spaced"
errpunc=">>> Bad comment"
errstar=">>> No space at comment start"
errfloa=">>> Malformed number"
errtabs=">>> Tabulation instead of spaces"
errhttp=">>> Bad hyperlink markers"
errleng=">>> Too many characters per line"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Check bad code style.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

function checkCPP( ) {

# No (too much) space after template keyword.

grep -E -nH "$TEMP_EXPR"                            "$file" | grep -E -v "$HIDE_EXPR" && echo "${errtemp}"
grep -E -nH "<[^ ]+>|< [^ \"\-]+>|<[^ \"]+ >"       "$file" | grep -E -v "$HIDE_EXPR" && echo "${errtemp}"
grep -E -nH ">&|>\*|<const|< const [^ ]*[^ ]>"      "$file" | grep -E -v "$HIDE_EXPR" && echo "${errtemp}"

# No (too much) space after operator keyword.

grep -E -nH "operator[^ \[\(\+\-]|operator [ ]+"    "$file" | grep -E -v "$HIDE_EXPR" && echo "${errkeyw}"

# Overly spaced.

grep -E -nH "(public|private|protected)[ ]+\:"      "$file" | grep -E -v "$HIDE_EXPR" && echo "${errspac}"
grep -E -nH " ::|:: "                               "$file" | grep -E -v "$HIDE_EXPR" && echo "${errspac}"

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

function check( ) {

# Do NOT scan CR ending files.

grep -q "$EXIT_EXPR"    "$file" && { return 1; }
grep -q $'\r'           "$file" && { echo "${file##*/}: ${errcarr}"; return 1; }

# No space after a comma.

grep -E -nH ",[^ \'\"]"                             "$file" | grep -E -v "$HIDE_EXPR" && echo "${errcomm}"

# Space before semicolon.

grep -E -nH " ;"                                    "$file" | grep -E -v "$HIDE_EXPR" && echo "${errsemi}"

# Spacing and no brace after statements.

grep -E -nH "(if|for|switch|while) [ ]+\("          "$file" | grep -E -v "$HIDE_EXPR" && echo "${errstat}"
grep -E -nH "(if|for|switch|while) \("              "$file" | grep -E -v "$STAT_EXPR" && echo "${errstat}"
grep -E -nH "[^0-9A-Za-z]*else[ \(]"                "$file" | grep -E -v "$STAT_EXPR" && echo "${errstat}"

# No space around assignment.

grep -E -nH "[^ =!<>%/\+\&\|\^\*\-]=|=[^ =]"        "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"

# No space around arithmetic operators.

grep -E -nH "[^ \+\'\"]\+[^ =\+\'\"]"               "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"

# No space around bitwise operators.

grep -E -nH "[^ \|]\||\|[^ =\|]"                    "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\^|\^[^ =]"                        "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ <=]<<|<<[^ <=]"                     "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ >=\"]>>|>>[^ >=\"]"                 "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"

# No space around comparison and logical operators.

grep -E -nH "\&\&[^ )]"                             "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ <]\|\||\|\|[^ ]"                    "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]==|==[^ ]"                         "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]!=|!=[^ ]"                         "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ >]>=|>=[^ ]"                        "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ <]<=|<=[^ ]"                        "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"

# No space around compound operators.

grep -E -nH "[^ ]\+=|\+=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\-=|\-=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\*=|\*=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\/=|\/=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\&=|\&=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\%=|\%=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\|=|\|=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]\^=|\|=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]>>=|>>=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"
grep -E -nH "[^ ]<<=|<<=[^ ]"                       "$file" | grep -E -v "$HIDE_EXPR" && echo "${erroper}"

# Space with increment and decrement operator.

grep -E -nH "[^;=\{&|/ *\,] \+\+|\+\+ [^=\(\!]"     "$file" | grep -E -v "$HIDE_EXPR" && echo "${errincr}"
grep -E -nH "[^;=\{&|/ *\,] \-\-|\-\- [^=|(\!]"     "$file" | grep -E -v "$HIDE_EXPR" && echo "${errdecr}"

# Space after negation operators.

grep -E -nH "! [^\*]"                               "$file" | grep -E -v "$HIDE_EXPR" && echo "${errnega}"
grep -E -nH "~ "                                    "$file" | grep -E -v "$HIDE_EXPR" && echo "${errnega}"

# Space inside brackets or parentheses.

grep -E -nH "\[ | \]"                               "$file" | grep -E -v "$HIDE_EXPR" && echo "${errpare}"
grep -E -nH "\( | \)"                               "$file" | grep -E -v "$HIDE_EXPR" && echo "${errpare}"

# Space before brackets.

grep -E -nH " \["                                   "$file" | grep -E -v "$HIDE_EXPR" && echo "${errpare}"

# No space before parentheses.

grep -E -nH "[^ *!&~\"\+\\(\)\[\-]\([^\)]"          "$file" | grep -E -v "$HIDE_EXPR" && echo "${errpare}"
grep -E -nH " \(\)|\( \(|\) \)|\(& \("              "$file" | grep -E -v "$HIDE_EXPR" && echo "${errpare}"

# No space around braces and colons.

grep -E -nH "[^ \"]{|{[^ \"]|[^ \"]}|}[^ ;,\"]"     "$file" | grep -E -v "$HIDE_EXPR" && echo "${errbrac}"
grep -E -nH "[^ :\"\']:[^:\"\']|[^:\"\']:[^ :\"\']" "$file" | grep -E -v "$HIDE_EXPR" && echo "${errbrac}"

# Overly spaced.

grep -E -nH "[/=><!&~\*\{\|\^\[\+\-] [ ]+[^ /\\]"   "$file" | grep -E -v "$HIDE_EXPR" && echo "${errspac}"
grep -E -nH "(#include|#ifdef|#ifndef) [ ]+"        "$file" && echo "${errspac}"

# No space at start in comment.

grep -E -nH "//[^ ]|/\*[^ ]"                        "$file" | grep -E -v "$HIDE_EXPR" && echo "${errstar}"

# Malformed number

grep -E -nH "[0-9]\.[^ 0-9A-Za-z]"                  "$file" | grep -E -v "$HIDE_EXPR" && echo "${errfloa}"

# No uppercase letter or punctuation in C comment.

grep -E -nH "/\* [^A-Z<\"]|[^\.?!>\"] \*/"          "$file" && echo "${errpunc}"

# Tabs.

grep -nH $'\t'                                      "$file" && echo "${errtabs}"

# Hyperlink malformed.

grep -E -nH "https?://"                             "$file" | grep -E -v "$LINK_EXPR" && echo "${errhttp}"
grep -E -nH "ftp://"                                "$file" | grep -E -v "$LINK_EXPR" && echo "${errhttp}"

# Line length too long.

awk 'length > 110 {print FILENAME, ":", NR}'        "$file" | grep -E "." && echo "${errleng}"

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Variables.

files=()

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Parse the options.

rep="$PWD"

while getopts "d:f:hv" o; do
    case $o in
        d ) rep="$OPTARG";;
        f ) files+="$OPTARG";;
        h ) echo "$USAGE"; exit 0;;
        v ) echo "$VERSION"; exit 0;;
        ? ) echo >&2 "$USAGE"; exit 1;;
    esac
done

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Get files.

if [ "${#files[@]}" -eq 0 ]; then
    while read -r -d $'\0'; do
        files+=("$REPLY")
    done < <(find -E "$rep" -regex $FILE_EXPR -print0)
else
    [ -f "${files[0]}" ] || { echo >&2 "${0##*/}: ${files[0]}: Invalid file"; exit 1; }
    [[ "${files[0]}" =~ $FILE_EXPR ]] || { echo >&2 "${0##*/}: ${files[0]}: Invalid file"; exit 1; }
fi

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Iterate over files.

for file in "${files[@]}"; do
    [[ "$file" =~ ' ' ]] && { echo >&2 "${0##*/}: ${file}: Space in filepath"; }
    if ! [[ "$file" =~ 'libraries' ]]; then
        echo "$file"
        check
        if [[ "${file}" =~ $PLUS_EXPR ]]; then
            checkCPP
        fi
    fi
done

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
