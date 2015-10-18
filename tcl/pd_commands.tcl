
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_commands 0.1

namespace eval ::pd_commands:: {
    variable untitled_number "1"

    namespace export menu_*
}

# ------------------------------------------------------------------------------
# functions called from File menu

proc ::pd_commands::menu_new {} {
    variable untitled_number
    if { ! [file isdirectory $::pd_gui(directory_new)]} {set ::pd_gui(directory_new) $::env(HOME)}
    # to localize "Untitled" there will need to be changes in g_canvas.c and
    # g_readwrite.c, where it tests for the string "Untitled"
    set untitled_name "Untitled"
    pdsend "pd menunew $untitled_name-$untitled_number [enquote_path $::pd_gui(directory_new)]"
    incr untitled_number
}

proc ::pd_commands::menu_open {} {
    if { ! [file isdirectory $::pd_gui(directory_open)]} {set ::pd_gui(directory_open) $::env(HOME)}
    set files [tk_getOpenFile -defaultextension .pd \
                       -multiple true \
                       -filetypes $::filetypes \
                       -initialdir $::pd_gui(directory_open)]
    if {$files ne ""} {
        foreach filename $files { 
            open_file $filename
        }
        set ::pd_gui(directory_open) [file dirname $filename]
    }
}

proc ::pd_commands::menu_print {mytoplevel} {
    set filename [tk_getSaveFile -initialfile pd.ps \
                      -defaultextension .ps \
                      -filetypes { {{postscript} {.ps}} }]
    if {$filename ne ""} {
        set tkcanvas [tkcanvas_name $mytoplevel]
        $tkcanvas postscript -file $filename 
    }
}

# ------------------------------------------------------------------------------
# functions called from Edit menu

proc ::pd_commands::menu_undo {} {
    if {$::pd_gui(window_focused) eq $::pd_gui(undomanager_toplevel) && $::pd_gui(undomanager_undo) ne "no"} {
        pdsend "$::pd_gui(window_focused) undo"
    }
}

proc ::pd_commands::menu_redo {} {
    if {$::pd_gui(window_focused) eq $::pd_gui(undomanager_toplevel) && $::pd_gui(undomanager_redo) ne "no"} {
        pdsend "$::pd_gui(window_focused) redo"
    }
}

proc ::pd_commands::menu_editmode {state} {
    if {[winfo class $::pd_gui(window_focused)] ne "PatchWindow"} {return}
    set ::pd_gui(is_editmode) $state
# this shouldn't be necessary because 'pd' will reply with pdtk_canvas_editmode
#    set ::patch_is_editmode($::pd_gui(window_focused)) $state
    pdsend "$::pd_gui(window_focused) editmode $state"
}

proc ::pd_commands::menu_toggle_editmode {} {
    ::pd_commands::menu_editmode [expr {! $::pd_gui(is_editmode)}]
}

# ------------------------------------------------------------------------------
# generic procs for sending menu events

# send a message to a pd canvas receiver
proc ::pd_commands::menu_send {window message} {
    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PatchWindow"} {
        pdsend "$mytoplevel $message"
    } elseif {$mytoplevel eq ".pdwindow"} {
        if {$message eq "copy"} {
            tk_textCopy .pdwindow.text
        } elseif {$message eq "selectall"} {
            .pdwindow.text tag add sel 1.0 end
        } elseif {$message eq "menusaveas"} {
            ::pd_console::save_logbuffer_to_file
        }
    }
}

# send a message to a pd canvas receiver with a float arg
proc ::pd_commands::menu_send_float {window message float} {
    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PatchWindow"} {
        pdsend "$mytoplevel $message $float"
    }
}

# ------------------------------------------------------------------------------
# open the dialog panels

proc ::pd_commands::menu_message_dialog {} {
    ::dialog_message::open_message_dialog $::pd_gui(window_focused)
}

proc ::pd_commands::menu_find_dialog {} {
    ::dialog_find::open_find_dialog $::pd_gui(window_focused)
}

proc ::pd_commands::menu_font_dialog {} {
    if {[winfo exists .font]} {
        raise .font
    } elseif {$::pd_gui(window_focused) eq ".pdwindow"} {
        pdtk_canvas_dofont .pdwindow [lindex [.pdwindow.text cget -font] 1]
    } else {
        pdsend "$::pd_gui(window_focused) menufont"
    }
}

proc ::pd_commands::menu_path_dialog {} {
    if {[winfo exists .path]} {
        raise .path
    } else {
        pdsend "pd start-path-dialog"
    }
}

proc ::pd_commands::menu_startup_dialog {} {
    if {[winfo exists .startup]} {
        raise .startup
    } else {
        pdsend "pd start-startup-dialog"
    }
}

proc ::pd_commands::menu_texteditor {} {
    ::pd_console::error "the text editor is not implemented"
}

# ------------------------------------------------------------------------------
# window management functions

proc ::pd_commands::menu_minimize {window} {
    wm iconify [winfo toplevel $window]
}

proc ::pd_commands::menu_maximize {window} {
    wm state [winfo toplevel $window] zoomed
}

proc ::pd_commands::menu_raise_pdwindow {} {
    if {$::pd_gui(window_focused) eq ".pdwindow" && [winfo viewable .pdwindow]} {
        lower .pdwindow
    } else {
        wm deiconify .pdwindow
        raise .pdwindow
    }
}

# used for cycling thru windows of an app
proc ::pd_commands::menu_raisepreviouswindow {} {
    lower [lindex [wm stackorder .] end] [lindex [wm stackorder .] 0]
    focus [lindex [wm stackorder .] end]
}

# used for cycling thru windows of an app the other direction
proc ::pd_commands::menu_raisenextwindow {} {
    set mytoplevel [lindex [wm stackorder .] 0]
    raise $mytoplevel
    focus $mytoplevel
}

# ------------------------------------------------------------------------------
# Pd window functions
proc menu_clear_console {} {
    ::pd_console::clear_console
}

# ------------------------------------------------------------------------------
# manage the saving of the directories for the new commands

# this gets the dir from the path of a window's title
proc ::pd_commands::set_filenewdir {mytoplevel} {
    # TODO add Aqua specifics once g_canvas.c has [wm attributes -titlepath]
    if {$mytoplevel eq ".pdwindow"} {
        set ::pd_gui(directory_new) $::pd_gui(directory_open)
    } else {
        regexp -- ".+ - (.+)" [wm title $mytoplevel] ignored ::pd_gui(directory_new)
    }
}

# parse the textfile for the About Pd page
proc ::pd_commands::menu_aboutpd {} {
    
    if {[winfo exists .aboutpd]} {
        wm deiconify .aboutpd
        raise .aboutpd
    } else {
        toplevel .aboutpd -class TextWindow
        wm title .aboutpd [_ "About Pd"]
        wm group .aboutpd .
        .aboutpd configure -menu $::pd_gui(window_menubar)
        text .aboutpd.text -relief flat -borderwidth 0 \
            -yscrollcommand ".aboutpd.scroll set" -background white
        scrollbar .aboutpd.scroll -command ".aboutpd.text yview"
        pack .aboutpd.scroll -side right -fill y
        pack .aboutpd.text -side left -fill both -expand 1
        bind .aboutpd <$::pd_gui(modifier)-Key-w>   "wm withdraw .aboutpd"
    }
}

# open HTML docs from the menu using the OS-default HTML viewer
proc ::pd_commands::menu_openfile {filename} {
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

# ------------------------------------------------------------------------------
# Mac OS X specific functions

proc ::pd_commands::menu_bringalltofront {} {
    # use [winfo children .] here to include windows that are minimized
    foreach item [winfo children .] {
        # get all toplevel windows, exclude menubar windows
        if { [string equal [winfo toplevel $item] $item] && \
                 [catch {$item cget -tearoff}]} {
            wm deiconify $item
        }
    }
    wm deiconify .
}
