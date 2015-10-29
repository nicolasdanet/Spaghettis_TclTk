
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Event handlers for the menu items. 

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_commands 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect
package require pd_miscellaneous

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_commands:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export newPatch
namespace export open
namespace export editMode

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable untitledName "Untitled"
variable untitledNumber "1"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc newPatch {} {

    variable untitledName
    variable untitledNumber 
    
    if {![file isdirectory $::var(directoryNew)]} { set ::var(directoryNew) $::env(HOME) }
    
    ::pd_connect::pdsend "pd menunew $untitledName-$untitledNumber [enquote_path $::var(directoryNew)]"
    
    incr untitledNumber 
}

proc open {} {

    if {![file isdirectory $::var(directoryOpen)]} { set ::var(directoryOpen) $::env(HOME) }
    
    set files [tk_getOpenFile -multiple 1 -filetypes $::var(filesTypes) -initialdir $::var(directoryOpen)]

    if {$files ne ""} {
        foreach filename $files { ::pd_miscellaneous::open_file $filename }
        set ::var(directoryOpen) [file dirname $filename]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc editMode {state} {

    if {[winfo class $::var(windowFocused)] eq "PdPatch"} {
        ::pd_connect::pdsend "$::var(windowFocused) editmode $state"
    }
}

proc menu_send {window message} {

    set mytoplevel [winfo toplevel $window]
    
    if {[winfo class $mytoplevel] eq "PdPatch"} {
        ::pd_connect::pdsend "$mytoplevel $message"
    } elseif {$mytoplevel eq ".console"} {
        if {$message eq "copy"} {
            tk_textCopy .console.text
        } elseif {$message eq "selectall"} {
            .console.text tag add sel 1.0 end
        }
    }
}

proc menu_send_float {window message float} {

    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PdPatch"} {
        ::pd_connect::pdsend "$mytoplevel $message $float"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc menu_raise_console {} {
    if {$::var(windowFocused) eq ".console" && [winfo viewable .console]} {
        lower .console
    } else {
        wm deiconify .console
        raise .console
    }
}

proc menu_raisepreviouswindow {} {
    lower [lindex [wm stackorder .] end] [lindex [wm stackorder .] 0]
    focus [lindex [wm stackorder .] end]
}

proc menu_raisenextwindow {} {
    set mytoplevel [lindex [wm stackorder .] 0]
    raise $mytoplevel
    focus $mytoplevel
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
