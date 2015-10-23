
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_console 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_console:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

    variable logbuffer {}
    variable tclentry {}
    variable tclentry_history {"console show"}
    variable history_position 0
    variable linecolor 0 ;# is toggled to alternate text line colors
    variable logmenuitems
    variable maxloglevel 4

    variable lastlevel 0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# TODO make the Pd window save its size and location between running

proc set_layout {} {
    variable maxloglevel
    .console.text.internal tag configure log0 -foreground "#d00" -background "#ffe0e8"
    .console.text.internal tag configure log1 -foreground "#d00"
    # log2 messages are normal black on white
    .console.text.internal tag configure log3 -foreground "#484848"

    # 0-20(4-24) is a rough useful range of 'verbose' levels for impl debugging
    set start 4
    set end 25
    for {set i $start} {$i < $end} {incr i} {
        set B [expr int(($i - $start) * (40 / ($end - $start))) + 50]
        .console.text.internal tag configure log${i} -foreground grey${B}
    }
}


# grab focus on part of the Pd window when Pd is busy
proc busygrab {} {
    # set the mouse cursor to look busy and grab focus so it stays that way    
    .console.text configure -cursor watch
    grab set .console.text
}

# release focus on part of the Pd window when Pd is finished
proc busyrelease {} {
    .console.text configure -cursor xterm
    grab release .console.text
}

# ------------------------------------------------------------------------------
# pdtk functions for 'pd' to send data to the Pd window

proc buffer_message {object_id level message} {
    variable logbuffer
    lappend logbuffer $object_id $level $message
}

proc insert_log_line {object_id level message} {
    if {$object_id eq ""} {
        .console.text.internal insert end $message log$level
    } else {
        .console.text.internal insert end $message [list log$level obj$object_id]
        .console.text.internal tag bind obj$object_id <$::var(modifierKey)-ButtonRelease-1> \
            "::pd_console::select_by_id $object_id; break"
        .console.text.internal tag bind obj$object_id <Key-Return> \
            "::pd_console::select_by_id $object_id; break"
        .console.text.internal tag bind obj$object_id <Key-KP_Enter> \
            "::pd_console::select_by_id $object_id; break"
    }
}

# this has 'args' to satisfy trace, but its not used
proc filter_buffer_to_text {args} {
    variable logbuffer
    variable maxloglevel
    .console.text.internal delete 0.0 end
    set i 0
    foreach {object_id level message} $logbuffer {
        insert_log_line $object_id $level $message

        # this could take a while, so update the GUI every 10000 lines
        if { [expr $i % 10000] == 0} {update idletasks}
        incr i
    }
    .console.text.internal yview end
    ::pd_console::verbose 10 "The Pd window filtered $i lines\n"
}

proc select_by_id {args} {
    if [llength $args] { # Is $args empty?
        ::pd_connect::pdsend "pd findinstance $args"
    }
}

# logpost posts to Pd window with an object to trace back to and a
# 'log level'. The logpost and related procs are for generating
# messages that are useful for debugging patches.  They are messages
# that are meant for the Pd programmer to see so that they can get
# information about the patches they are building
proc logpost {object_id level message} {
    variable maxloglevel
    variable lastlevel $level

    buffer_message $object_id $level $message
    if {[llength [info commands .console.text.internal]]} {
        # cancel any pending move of the scrollbar, and schedule it
        # after writing a line. This way the scrollbar is only moved once
        # when the inserting has finished, greatly speeding things up
        after cancel .console.text.internal yview end
        insert_log_line $object_id $level $message
        after idle .console.text.internal yview end
    }
}

# shortcuts for posting to the Pd window
proc fatal {message} {logpost {} 0 $message}
proc error {message} {logpost {} 1 $message}
proc post {message} {logpost {} 2 $message}
proc debug {message} {logpost {} 3 $message}
# for backwards compatibility
proc bug {message} {logpost {} 1 \
    [concat consistency check failed: $message]}
proc pdtk_post {message} {post $message}

proc endpost {} {
    variable linecolor
    variable lastlevel
    logpost {} $lastlevel "\n"
    set linecolor [expr ! $linecolor]
}

# this verbose proc has a separate numbering scheme since its for
# debugging implementations, and therefore falls outside of the 0-3
# numbering on the Pd window.  They should only be shown in ALL mode.
proc verbose {level message} {
    incr level 4
    logpost {} $level $message
}

# clear the log and the buffer
proc clear_console {} {
    variable logbuffer {}
    .console.text.internal delete 0.0 end
}

# save the contents of the pd_console::logbuffer to a file
proc save_logbuffer_to_file {} {
    variable logbuffer
    set filename [tk_getSaveFile -initialfile "pdconsole.txt" -defaultextension .txt]
    if {$filename eq ""} return; # they clicked cancel
    set f [open $filename w]
    puts $f "Tcl/Tk [info patchlevel]"
    puts $f "------------------------------------------------------------------------------"
    puts $f $logbuffer
    close $f
}


#--compute audio/DSP checkbutton-----------------------------------------------#

# set the checkbox on the "Compute Audio" menuitem and checkbox
proc pdtk_pd_dsp {value} {
    # TODO canvas_startdsp/stopdsp should really send 1 or 0, not "ON" or "OFF"
    if {$value eq "ON"} {
        set ::var(isDsp) 1
    } else {
        set ::var(isDsp) 0
    }
}

proc pdtk_pd_dio {red} {
    if {$red == 1} {
        .console.header.ioframe.dio configure -foreground red
    } else {
        .console.header.ioframe.dio configure -foreground lightgray
    }
        
}

proc pdtk_pd_audio {state} {
    .console.header.ioframe.iostate configure -text [concat audio $state]
}

#--bindings specific to the Pd window------------------------------------------#

proc console_bindings {} {
    # these bindings are for the whole Pd window, minus the Tcl entry
    foreach window {.console.text .console.header} {
        bind $window <$::var(modifierKey)-Key-x> "tk_textCut .console.text"
        bind $window <$::var(modifierKey)-Key-c> "tk_textCopy .console.text"
        bind $window <$::var(modifierKey)-Key-v> "tk_textPaste .console.text"
    }
    # Select All doesn't seem to work unless its applied to the whole window
    bind .console <$::var(modifierKey)-Key-a> ".console.text tag add sel 1.0 end"
    # the "; break" part stops executing another binds, like from the Text class

    # these don't do anything in the Pd window, so alert the user, then break
    # so no more bindings run
    bind .console <$::var(modifierKey)-Key-s> "bell; break"
    bind .console <$::var(modifierKey)-Key-p> "bell; break"

    # ways of hiding/closing the Pd window
    if {[tk windowingsystem] eq "aqua"} {
        # on Mac OS X, you can close the Pd window, since the menubar is there
        bind .console <$::var(modifierKey)-Key-w>   "wm withdraw .console"
        wm protocol .console WM_DELETE_WINDOW "wm withdraw .console"
    } else {
        # TODO should it possible to close the Pd window and keep Pd open?
        bind .console <$::var(modifierKey)-Key-w>   "wm iconify .console"
        wm protocol .console WM_DELETE_WINDOW "::pd_connect::pdsend \"pd verifyquit\""
    }
}

#--Tcl entry procs-------------------------------------------------------------#

proc eval_tclentry {} {
    variable tclentry
    variable tclentry_history
    variable history_position 0
    if {$tclentry eq ""} {return} ;# no need to do anything if empty
    if {[catch {uplevel #0 $tclentry} errorname]} {
        global errorInfo
        switch -regexp -- $errorname { 
            "missing close-brace" {
                ::pd_console::error [concat [_ "(Tcl) MISSING CLOSE-BRACE '\}': "] $errorInfo]\n
            } "missing close-bracket" {
                ::pd_console::error [concat [_ "(Tcl) MISSING CLOSE-BRACKET '\]': "] $errorInfo]\n
            } "^invalid command name" {
                ::pd_console::error [concat [_ "(Tcl) INVALID COMMAND NAME: "] $errorInfo]\n
            } default {
                ::pd_console::error [concat [_ "(Tcl) UNHANDLED ERROR: "] $errorInfo]\n
            }
        }
    }
    lappend tclentry_history $tclentry
    set tclentry {}
}

proc get_history {direction} {
    variable tclentry_history
    variable history_position

    incr history_position $direction
    if {$history_position < 0} {set history_position 0}
    if {$history_position > [llength $tclentry_history]} {
        set history_position [llength $tclentry_history]
    }
    .console.tcl.entry delete 0 end
    .console.tcl.entry insert 0 \
        [lindex $tclentry_history end-[expr $history_position - 1]]
}

proc validate_tcl {} {
    variable tclentry
    if {[info complete $tclentry]} {
        .console.tcl.entry configure -background "white"
    } else {
        .console.tcl.entry configure -background "#FFF0F0"
    }
}

#--create tcl entry-----------------------------------------------------------#

proc set_findinstance_cursor {widget key state} {
    set triggerkeys [list Control_L Control_R Meta_L Meta_R]
    if {[lsearch -exact $triggerkeys $key] > -1} {
        if {$state == 0} {
            $widget configure -cursor xterm
        } else {
            $widget configure -cursor based_arrow_up
        }
    }
}

#--create the window-----------------------------------------------------------#

proc initialize {} {
    variable logmenuitems
    set ::patch_loaded(.console) 0

    # colorize by class before creating anything
    option add *PdConsole*Entry.highlightBackground "grey" startupFile
    option add *PdConsole*Frame.background "grey" startupFile
    option add *PdConsole*Label.background "grey" startupFile
    option add *PdConsole*Checkbutton.background "grey" startupFile
    option add *PdConsole*Menubutton.background "grey" startupFile
    option add *PdConsole*Text.background "white" startupFile
    option add *PdConsole*Entry.background "white" startupFile

    toplevel .console -class PdConsole
    wm title .console [_ "Pd"]
    set ::patch_name(.console) [_ "Pd"]
    if {[tk windowingsystem] eq "x11"} {
        wm minsize .console 400 75
    } else {
        wm minsize .console 400 51
    }
    wm geometry .console =500x400+20+50
    .console configure -menu .menubar

    frame .console.header -borderwidth 1 -relief flat -background lightgray
    pack .console.header -side top -fill x -ipady 5

    frame .console.header.pad1
    pack .console.header.pad1 -side left -padx 12

    checkbutton .console.header.dsp -text [_ "DSP"] -variable ::var(isDsp) \
        -font {$::var(fontFamily) -18 bold} -takefocus 1 -background lightgray \
        -borderwidth 0  -command {::pd_connect::pdsend "pd dsp $::var(isDsp)"}
    pack .console.header.dsp -side right -fill y -anchor e -padx 5 -pady 0

# frame for DIO error and audio in/out labels
    frame .console.header.ioframe -background lightgray
    pack .console.header.ioframe -side right -padx 30

# I/O state label (shows I/O on/off/in-only/out-only)
    label .console.header.ioframe.iostate \
        -text [_ "audio I/O off"] -borderwidth 0 \
        -background lightgray -foreground black \
        -takefocus 0 \
        -font {$::var(fontFamily) -14}

# DIO error label
    label .console.header.ioframe.dio \
        -text [_ "audio I/O error"] -borderwidth 0 \
        -background lightgray -foreground lightgray \
        -takefocus 0 \
        -font {$::var(fontFamily) -14}

    pack .console.header.ioframe.iostate .console.header.ioframe.dio \
        -side top

    frame .console.tcl -borderwidth 0
    pack .console.tcl -side bottom -fill x
# TODO this should use the pd_font_$size created in pd-gui.tcl    
    text .console.text -relief raised -bd 2 -font {$::var(fontFamily) -12} \
        -highlightthickness 0 -borderwidth 1 -relief flat \
        -yscrollcommand ".console.scroll set" -width 60 \
        -undo 0 -autoseparators 0 -maxundo 1 -takefocus 0
    scrollbar .console.scroll -command ".console.text.internal yview"
    pack .console.scroll -side right -fill y
    pack .console.text -side right -fill both -expand 1
    raise .console
    focus .console.text
    # run bindings last so that .console.tcl.entry exists
    console_bindings
    # set cursor to show when clicking in 'findinstance' mode
    bind .console <KeyPress> "+::pd_console::set_findinstance_cursor %W %K %s"
    bind .console <KeyRelease> "+::pd_console::set_findinstance_cursor %W %K %s"

    # hack to make a good read-only text widget from http://wiki.tcl.tk/1152
    rename ::.console.text ::.console.text.internal
    proc ::.console.text {args} {
        switch -exact -- [lindex $args 0] {
            "insert" {}
            "delete" {}
            "default" { return [eval ::.console.text.internal $args] }
        }
    }
    
    # print whatever is in the queue after the event loop finishes
    after idle [list after 0 ::pd_console::filter_buffer_to_text]

    set ::patch_loaded(.console) 1

    # set some layout variables
    ::pd_console::set_layout

    # wait until .console.tcl.entry is visible before opening files so that
    # the loading logic can grab it and put up the busy cursor
    tkwait visibility .console.text
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
