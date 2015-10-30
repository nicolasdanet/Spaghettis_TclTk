
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_miscellaneous 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_miscellaneous:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export getDefaultFamily
namespace export openFile

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getDefaultFamily {} {
    
    set fonts { "DejaVu Sans Mono" \
                "Bitstream Vera Sans Mono" \
                "Inconsolata" \
                "Andale Mono" \
                "Droid Sans Mono" }
              
    foreach family $fonts {
        if {[lsearch -exact -nocase [font families] $family] > -1} {
            return $family
        }
    }
    
    return "courier"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc openFile {filename} {

    if {[file exists $filename]} {
        set basename  [file tail $filename]
        set extension [file extension $filename]
        set directory [file normalize [file dirname $filename]]
        
        if {[lsearch -exact $::var(filesExtensions) $extension] > -1} {
            ::pd_patch::started_loading_file [format "%s/%s" $basename $filename]
            ::pd_connect::pdsend "pd open [::enquote $basename] [::enquote $directory]"
        } else {
            pd_console::post "Sorry, couldn't open $basename file."
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Function called by the openpanel object.

proc openpanel {target localdir} {

    if {![file isdirectory $localdir]} {
        if {![file isdirectory $::var(directoryOpen)]} {
            set ::var(directoryOpen) $::env(HOME)
        }
        set localdir $::var(directoryOpen)
    }
    
    set filename [tk_getOpenFile -initialdir $localdir]
    
    if {$filename ne ""} {
        set ::var(directoryOpen) [file dirname $filename]
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# Function called by the savepanel object.

proc savepanel {target localdir} {

    if {![file isdirectory $localdir]} {
        if {![file isdirectory $::var(directoryNew)]} {
            set ::var(directoryNew) $::env(HOME)
        }
        set localdir $::var(directoryNew)
    }
    
    set filename [tk_getSaveFile -initialdir $localdir]
    
    if {$filename ne ""} {
        set ::var(directoryNew) [file dirname $filename]
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# quoting functions

# enquote a string for find, path, and startup dialog panels, to be decoded by
# sys_decodedialog()

# TODO enquote a filename to send it to pd, " isn't handled properly tho...



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
