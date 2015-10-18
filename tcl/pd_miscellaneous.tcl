
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_miscellaneous 0.1

# a place to temporarily store things until they find a home or go away

proc open_file {filename} {
    set directory [file normalize [file dirname $filename]]
    set basename [file tail $filename]
    if {
        [file exists $filename]
        && [regexp -nocase -- "\.(pd|pat|mxt)$" $filename]
    } then {
        ::pdtk_canvas::started_loading_file [format "%s/%s" $basename $filename]
        ::pd_connect::pdsend "pd open [enquote_path $basename] [enquote_path $directory]"
        # now this is done in pd_preferences
        ::pd_preferences::update_recentfiles $filename
    } {
        ::pd_console::post [format [_ "Ignoring '%s': doesn't look like a Pd-file"] $filename]
    }
}
    
# ------------------------------------------------------------------------------
# procs for panels (openpanel, savepanel)

proc pdtk_openpanel {target localdir} {
    if {! [file isdirectory $localdir]} {
        if { ! [file isdirectory $::pd_gui(directory_open)]} {
            set ::pd_gui(directory_open) $::env(HOME)
        }
        set localdir $::pd_gui(directory_open)
    }
    set filename [tk_getOpenFile -initialdir $localdir]
    if {$filename ne ""} {
        set ::pd_gui(directory_open) [file dirname $filename]
        ::pd_connect::pdsend "$target callback [enquote_path $filename]"
    }
}

proc pdtk_savepanel {target localdir} {
    if {! [file isdirectory $localdir]} {
        if { ! [file isdirectory $::pd_gui(directory_new)]} {
            set ::pd_gui(directory_new) $::env(HOME)
        }
        set localdir $::pd_gui(directory_new)
    }
    set filename [tk_getSaveFile -initialdir $localdir]
    if {$filename ne ""} {
        ::pd_connect::pdsend "$target callback [enquote_path $filename]"
    }
}

# ------------------------------------------------------------------------------
# window info (name, path, parents, children, etc.)

proc lookup_windowname {mytoplevel} {
    set window [array get ::patch_name $mytoplevel]
    if { $window ne ""} {
        return [lindex $window 1]
    } else {
        return ERROR
    }
}

proc tkcanvas_name {mytoplevel} {
    return "$mytoplevel.c"
}

# ------------------------------------------------------------------------------
# quoting functions

# enquote a string for find, path, and startup dialog panels, to be decoded by
# sys_decodedialog()
proc pdtk_encodedialog {x} {
    concat +[string map {" " "+_" "$" "+d" ";" "+s" "," "+c" "+" "++"} $x]
}

# encode a list with pdtk_encodedialog
proc pdtk_encode { listdata } {
    set outlist {}
    foreach this_path $listdata {
        if {0==[string match "" $this_path]} {
            lappend outlist [pdtk_encodedialog $this_path]
        }
    }
    return $outlist
}

# TODO enquote a filename to send it to pd, " isn't handled properly tho...
proc enquote_path {message} {
    string map {"," "\\," ";" "\\;" " " "\\ "} $message
}

#enquote a string to send it to Pd.  Blow off semi and comma; alias spaces
#we also blow off "{", "}", "\" because they'll just cause bad trouble later.
proc unspace_text {x} {
    set y [string map {" " "_" ";" "" "," "" "{" "" "}" "" "\\" ""} $x]
    if {$y eq ""} {set y "empty"}
    concat $y
}

# ------------------------------------------------------------------------------
# watchdog functions

proc pdtk_watchdog {} {
   ::pd_connect::pdsend "pd watchdog"
   after 2000 {pdtk_watchdog}
}

proc pdtk_ping {} {
    ::pd_connect::pdsend "pd ping"
}
