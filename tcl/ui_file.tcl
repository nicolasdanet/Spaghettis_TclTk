
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2020 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# File I/O management. 

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_file 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_file:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable untitledName       "Untitled"
variable untitledNumber     "1"
variable directorySave      [pwd]
variable directoryOpen      [pwd]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    variable directorySave
    variable directoryOpen
    
    if {[tk windowingsystem] eq "aqua"} { set directorySave $::env(HOME); set directoryOpen $::env(HOME) }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Handling menu actions.

proc newPatch {} {

    variable untitledName
    variable untitledNumber
    variable directorySave
    
    set tail [format "%d%s" $untitledNumber [lindex $::var(filesExtensions) 0]]
    
    ::ui_interface::pdsend "pd new $untitledName-$tail [::escaped $directorySave]"
    
    incr untitledNumber 
}

proc openPatch {} {

    variable directoryOpen
    
    set f [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir $directoryOpen]

    if {$f ne ""} {
        foreach filename $f { ::ui_file::openFile $filename }
    }
}

proc clearRecent {} {

    ::ui_interface::pdsend "pd clear recent"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Open a file (currently only patches).

proc openFile {filename} {

    variable directoryOpen
    
    set basename  [file tail $filename]
    set extension [file extension $filename]
    set directory [file normalize [file dirname $filename]]
    
    if {[file exists $filename]} {
    if {[lsearch -exact $::var(filesExtensions) $extension] > -1} {
        ::ui_interface::pdsend "pd open [::escaped $basename] [::escaped $directory]"
        set directoryOpen $directory
        return
    }
    }
    
    ::ui_console::post [format [_ "file: can't open %s"] $filename]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Function called by the executable.

proc saveAs {target filename directory destroy} {

    variable directorySave
    
    if {![file isdirectory $directory]} { set directory $directorySave }
    
    set filename [tk_getSaveFile    -initialfile $filename \
                                    -initialdir $directory \
                                    -filetypes $::var(filesTypes) \
                                    -defaultextension [lindex $::var(filesExtensions) 0]]
                      
    if {$filename ne ""} {
        set basename  [file tail $filename]
        set directory [file normalize [file dirname $filename]]
        ::ui_interface::pdsend "$target _savetofile [::escaped $basename] [::escaped $directory] $destroy"
        set directorySave $directory
    }
    
    ::bringToFront $target
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Functions called by the [openpanel] object.

proc openPanelFile {target directory multiple} {

    variable directoryOpen
    
    if {![file isdirectory $directory]} { set directory [file dirname $directory] }
    if {![file isdirectory $directory]} { set directory $directoryOpen }
    
    set f [tk_getOpenFile -multiple $multiple -initialdir $directory]
    
    if {$f ne ""} {
        if {$multiple == 1} {
            set filenames ""; foreach filename $f { lappend filenames [::escaped $filename] }
            ::ui_interface::pdsend "$target _callback [join $filenames]"
        } else {
            ::ui_interface::pdsend "$target _callback [::escaped $f]"
        }
        
    } else {
        ::ui_interface::pdsend "$target _cancel"
    }
}

proc openPanelDirectory {target directory} {

    variable directoryOpen
    
    set directory [file dirname $directory]
    
    if {![file isdirectory $directory]} { set directory $directoryOpen }
    
    set filename [tk_chooseDirectory -mustexist 1 -initialdir $directory]
    
    if {$filename ne ""} {
        ::ui_interface::pdsend "$target _callback [::escaped $filename]"
        
    } else {
        ::ui_interface::pdsend "$target _cancel"
    }
}

# Function called by the [savepanel] object.

proc savePanel {target directory} {

    variable directorySave
    
    if {![file isdirectory $directory]} { set directory [file dirname $directory] }
    if {![file isdirectory $directory]} { set directory $directoryOpen }
    
    set filename [tk_getSaveFile -initialdir $directory]
    
    if {$filename ne ""} {
        ::ui_interface::pdsend "$target _callback [::escaped $filename]"
    } else {
        ::ui_interface::pdsend "$target _cancel"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
