
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_miscellaneous 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect
package require pd_console
package require pd_patch

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_miscellaneous:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export openFile
namespace export openpanel
namespace export savepanel
namespace export ping
namespace export watchdog

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc openFile {filename} {

    set basename  [file tail $filename]
    set extension [file extension $filename]
    set directory [file normalize [file dirname $filename]]
    
    if {[file exists $filename]} {
    if {[lsearch -exact $::var(filesExtensions) $extension] > -1} {
        ::pd_patch::started_loading_file [format "%s/%s" $basename $filename]
        ::pd_connect::pdsend "pd open [::enquote $basename] [::enquote $directory]"
    }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Function called by the openpanel object.

proc openpanel {target directory} {

    if {![file isdirectory $directory]} {
        if {![file isdirectory $::var(directoryOpen)]} {
            set ::var(directoryOpen) $::env(HOME)
        }
        set directory $::var(directoryOpen)
    }
    
    set filename [tk_getOpenFile -initialdir $directory]
    
    if {$filename ne ""} {
        set ::var(directoryOpen) [file dirname $filename]
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# Function called by the savepanel object.

proc savepanel {target directory} {

    if {![file isdirectory $directory]} {
        if {![file isdirectory $::var(directoryNew)]} {
            set ::var(directoryNew) $::env(HOME)
        }
        set directory $::var(directoryNew)
    }
    
    set filename [tk_getSaveFile -initialdir $directory]
    
    if {$filename ne ""} {
        set ::var(directoryNew) [file dirname $filename]
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ping {} {
    ::pd_connect::pdsend "pd ping"
}

proc watchdog {} {
    ::pd_connect::pdsend "pd watchdog"; after 2000 { ::pd_miscellaneous::watchdog }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
