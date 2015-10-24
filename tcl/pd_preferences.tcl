
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 2011 Yvan Volochine.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_preferences 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_preferences:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export initialize

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {
    # assign gui preferences
    # osx special case for arrays
    set arr [expr { [tk windowingsystem] eq "aqua" }]
    set ::var(filesRecent) ""
    catch {set ::var(filesRecent) [get_config $::var(filesRecentDomain)  \
        $::var(filesRecentKey)  $arr]}
}

# ------------------------------------------------------------------------------
# write recent files
#
proc write_recentfiles {} {
    write_config $::var(filesRecent) $::var(filesRecentDomain)  $::var(filesRecentKey)  1
}

# ------------------------------------------------------------------------------
# this is called when opening a document (wheredoesthisshouldgo.tcl)
#
proc update_recentfiles {afile} {
    # remove duplicates first
    set index [lsearch -exact $::var(filesRecent) $afile]
    set ::var(filesRecent) [lreplace $::var(filesRecent) $index $index]
    # insert new one in the beginning and crop the list
    set ::var(filesRecent) [linsert $::var(filesRecent) 0 $afile]
    set ::var(filesRecent) [lrange $::var(filesRecent) 0 5]
    ::pd_menus::update_recentfiles_menu
}

#################################################################
# main read/write procedures
#################################################################

# ------------------------------------------------------------------------------
# get configs from a file or the registry
#
proc get_config {adomain {akey} {arr}} {
    switch -- [tk windowingsystem] {
        "aqua"  { set conf [get_config_aqua $adomain $akey $arr] }
        "win32" { set conf [get_config_win $adomain $akey $arr] }
        "x11"   { set conf [get_config_x11 $adomain $akey $arr] }
    }
    return $conf
}

# ------------------------------------------------------------------------------
# write configs to a file or to the registry
# $arr is true if the data needs to be written in an array
#
proc write_config {data {adomain} {akey} {arr 0}} {
    switch -- [tk windowingsystem] {
        "aqua"  { write_config_aqua $data $adomain $akey $arr }
        "win32" { write_config_win $data $adomain $akey $arr }
        "x11"   { write_config_x11 $data $adomain $akey }
    }
}

#################################################################
# os specific procedures
#################################################################

# ------------------------------------------------------------------------------
# osx: read a plist file
#
proc get_config_aqua {adomain {akey} {arr 0}} {
    if {![catch {exec defaults read $adomain $akey} conf]} {
        if {$arr} {
            set conf [plist_array_to_tcl_list $conf]
        }
    } else {
        # initialize NSRecentDocuments with an empty array
        exec defaults write $adomain $akey -array
        set conf {}
    }
    return $conf
}

# ------------------------------------------------------------------------------
# win: read in the registry
#
proc get_config_win {adomain {akey} {arr 0}} {
    package require registry
    if {![catch {registry get $adomain $akey} conf]} {
        return [expr {$conf}]
    } else {
        return {}
    }
}

# ------------------------------------------------------------------------------
# linux: read a config file and return its lines splitted.
#
proc get_config_x11 {adomain {akey} {arr 0}} {
    set filename [file join $adomain $akey]
    set conf {}
    if {
        [file exists $filename] == 1
        && [file readable $filename]
    } {
        set fl [open $filename r]
        while {[gets $fl line] >= 0} {
           lappend conf $line
        }
        close $fl
    }
    return $conf
}

# ------------------------------------------------------------------------------
# osx: write configs to plist file
# if $arr is true, we write an array
#
proc write_config_aqua {data {adomain} {akey} {arr 0}} {
    # FIXME empty and write again so we don't loose the order
    if {[catch {exec defaults write $adomain $akey -array} errorMsg]} {
        # ::pd_console::error "write_config_aqua $akey: $errorMsg"
    }
    if {$arr} {
        foreach filepath $data {
            set escaped [escape_for_plist $filepath]
            exec defaults write $adomain $akey -array-add "$escaped"
        }
    } else {
        set escaped [escape_for_plist $data]
        exec defaults write $adomain $akey '$escaped'
    }
}

# ------------------------------------------------------------------------------
# win: write configs to registry
# if $arr is true, we write an array
#
proc write_config_win {data {adomain} {akey} {arr 0}} {
    package require registry
    # FIXME: ugly
    if {$arr} {
        if {[catch {registry set $adomain $akey $data multi_sz} errorMsg]} {
            # ::pd_console::error "write_config_win $data $akey: $errorMsg"
        }
    } else {
        if {[catch {registry set $adomain $akey $data sz} errorMsg]} {
            # ::pd_console::error "write_config_win $data $akey: $errorMsg"
        }
    }
}

# ------------------------------------------------------------------------------
# linux: write configs to USER_APP_CONFIG_DIR
#
proc write_config_x11 {data {adomain} {akey}} {
    # right now I (yvan) assume that data are just \n separated, i.e. no keys
    set data [join $data "\n"]
    set filename [file join $adomain $akey]
    if {[catch {set fl [open $filename w]} errorMsg]} {
        # ::pd_console::error "write_config_x11 $data $akey: $errorMsg"
    } else {
        puts -nonewline $fl $data
        close $fl
    }
}

#################################################################
# utils
#################################################################

# ------------------------------------------------------------------------------
# osx: handles arrays in plist files (thanks hc)
#
proc plist_array_to_tcl_list {arr} {
    set result {}
    set filelist $arr
    regsub -all -- {("?),\s+("?)} $filelist {\1 \2} filelist
    regsub -all -- {\n} $filelist {} filelist
    regsub -all -- {^\(} $filelist {} filelist
    regsub -all -- {\)$} $filelist {} filelist
    regsub -line -- {^'(.*)'$} $filelist {\1} filelist

    foreach file $filelist {
        set filename [regsub -- {,$} $file {}]
        lappend result $filename
    }
    return $result
}

# the Mac OS X 'defaults' command uses single quotes to quote things,
# so they need to be escaped
proc escape_for_plist {str} {
    return [regsub -all -- {'} $str {\\'}]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
