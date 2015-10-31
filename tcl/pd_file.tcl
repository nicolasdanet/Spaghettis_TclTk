
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

namespace export initialize
namespace export newPatch
namespace export openPatch
namespace export openFile
namespace export saveAs
namespace export openPanel
namespace export savePanel

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# For now the default file name can not be internationalized.

variable untitledName       "Untitled"
variable untitledNumber     "1"
variable directoryNew       [pwd]
variable directoryOpen      [pwd]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    variable directoryNew
    variable directoryOpen
    
    if {[tk windowingsystem] eq "aqua"} { set directoryNew $::env(HOME); set directoryOpen $::env(HOME) }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc newPatch {} {

    variable untitledName
    variable untitledNumber
    variable directoryNew
    
    ::pd_connect::pdsend "pd menunew $untitledName-$untitledNumber [::enquote $directoryNew]"
    
    incr untitledNumber 
}

proc openPatch {} {

    variable directoryOpen
    
    set f [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir $directoryOpen]

    if {$f ne ""} {
        foreach filename $f { ::pd_file::openFile $filename }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc openFile {filename} {

    variable directoryOpen
    
    set basename  [file tail $filename]
    set extension [file extension $filename]
    set directory [file normalize [file dirname $filename]]
    
    if {[file exists $filename]} {
    if {[lsearch -exact $::var(filesExtensions) $extension] > -1} {
        ::pd_patch::started_loading_file [format "%s/%s" $basename $filename]
        ::pd_connect::pdsend "pd open [::enquote $basename] [::enquote $directory]"
        set directoryOpen $directory
    }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Function called by the pd executable.

proc saveAs {target filename directory destroy} {

    variable directoryNew
    
    if {![file isdirectory $directory]} { set directory $directoryNew }
    
    set filename [tk_getSaveFile    -initialfile $filename \
                                    -initialdir $directory \
                                    -defaultextension [lindex $::var(filesExtensions) 0]]
                      
    if {$filename ne ""} {
        set basename  [file tail $filename]
        set directory [file normalize [file dirname $filename]]
        ::pd_connect::pdsend "$target savetofile [::enquote $basename] [::enquote $directory] $destroy"
        set directoryNew $directory
    }
}

# Function called by the openpanel object.

proc openPanel {target directory} {

    variable directoryOpen
    
    if {![file isdirectory $directory]} { set directory $directoryOpen }
    
    set filename [tk_getOpenFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# Function called by the savepanel object.

proc savePanel {target directory} {

    variable directoryNew
    
    if {![file isdirectory $directory]} { set directory $directoryNew }
    
    set filename [tk_getSaveFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::pd_connect::pdsend "$target callback [::enquote $filename]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
