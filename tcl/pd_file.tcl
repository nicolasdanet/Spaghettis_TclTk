
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_file 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect
package require pd_patch

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_file:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export directoryNew
namespace export directoryOpen

namespace export openPatch
namespace export openFile
namespace export openpanel
namespace export savepanel

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc directoryNew {} {

    if {![file isdirectory $::var(directoryNew)]} { set ::var(directoryNew) $::env(HOME) }
    
    return $::var(directoryNew)
}


proc directoryOpen {} {

    if {![file isdirectory $::var(directoryOpen)]} { set ::var(directoryOpen) $::env(HOME) }
    
    return $::var(directoryOpen)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc openPatch {} {

    set f [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir [::pd_file::directoryOpen]]

    if {$f ne ""} {
        foreach filename $f { ::pd_file::openFile $filename }
        set ::var(directoryOpen) [file dirname $filename]
    }
}

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

    if {![file isdirectory $directory]} { set directory [::pd_file::directoryOpen] }
    
    set filename [tk_getOpenFile -initialdir $directory]
    
    if {$filename ne ""} {
        set ::var(directoryOpen) [file dirname $filename]
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# Function called by the savepanel object.

proc savepanel {target directory} {

    if {![file isdirectory $directory]} { set directory [::pd_file::directoryNew] }
    
    set filename [tk_getSaveFile -initialdir $directory]
    
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
