
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

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_commands:: {

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

proc menu_open {} {
    if { ! [file isdirectory $::var(directoryOpen)]} {set ::var(directoryOpen) $::env(HOME)}
    set files [tk_getOpenFile -defaultextension .pd \
                       -multiple 1 \
                       -filetypes $::var(filesTypes) \
                       -initialdir $::var(directoryOpen)]
    if {$files ne ""} {
        foreach filename $files { 
            ::pd_miscellaneous::open_file $filename
        }
        set ::var(directoryOpen) [file dirname $filename]
    }
}

# ------------------------------------------------------------------------------
# functions called from Edit menu

proc menu_editmode {state} {
    if {[winfo class $::var(windowFocused)] ne "PdPatch"} {return}
    ::pd_connect::pdsend "$::var(windowFocused) editmode $state"
}

# ------------------------------------------------------------------------------
# generic procs for sending menu events

# send a message to a pd canvas receiver
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

# send a message to a pd canvas receiver with a float arg
proc menu_send_float {window message float} {
    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PdPatch"} {
        ::pd_connect::pdsend "$mytoplevel $message $float"
    }
}

# ------------------------------------------------------------------------------
# open the dialog panels

proc menu_path_dialog {} {
    if {[winfo exists .path]} {
        raise .path
    } else {
        ::pd_connect::pdsend "pd start-path-dialog"
    }
}

proc menu_startup_dialog {} {
    if {[winfo exists .startup]} {
        raise .startup
    } else {
        ::pd_connect::pdsend "pd start-startup-dialog"
    }
}

proc menu_texteditor {} {
    # ::pd_console::error "the text editor is not implemented"
}

# ------------------------------------------------------------------------------
# window management functions

proc menu_minimize {window} {
    wm iconify [winfo toplevel $window]
}

proc menu_maximize {window} {
    wm state [winfo toplevel $window] zoomed
}

proc menu_raise_console {} {
    if {$::var(windowFocused) eq ".console" && [winfo viewable .console]} {
        lower .console
    } else {
        wm deiconify .console
        raise .console
    }
}

# used for cycling thru windows of an app
proc menu_raisepreviouswindow {} {
    lower [lindex [wm stackorder .] end] [lindex [wm stackorder .] 0]
    focus [lindex [wm stackorder .] end]
}

# used for cycling thru windows of an app the other direction
proc menu_raisenextwindow {} {
    set mytoplevel [lindex [wm stackorder .] 0]
    raise $mytoplevel
    focus $mytoplevel
}

# ------------------------------------------------------------------------------
# manage the saving of the directories for the new commands

# open HTML docs from the menu using the OS-default HTML viewer
proc menu_openfile {filename} {
    if {$::tcl_platform(os) eq "Darwin"} {
        exec sh -c [format "open '%s'" $filename]
    } elseif {$::tcl_platform(platform) eq "windows"} {
        exec rundll32 url.dll,FileProtocolHandler [format "%s" $filename] &
    } else {
        foreach candidate { gnome-open xdg-open sensible-browser iceweasel firefox \
                                mozilla galeon konqueror netscape lynx } {
            set browser [lindex [auto_execok $candidate] 0]
            if {[string length $browser] != 0} {
                exec -- sh -c [format "%s '%s'" $browser $filename] &
                break
            }
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
