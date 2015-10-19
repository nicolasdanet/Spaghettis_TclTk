
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_console 0.1

namespace eval ::pd_console:: {
    variable logbuffer {}
    variable tclentry {}
    variable tclentry_history {"console show"}
    variable history_position 0
    variable linecolor 0 ;# is toggled to alternate text line colors
    variable logmenuitems
    variable maxloglevel 4

    variable lastlevel 0

    namespace export create_window
}

# TODO make the Pd window save its size and location between running

proc ::pd_console::set_layout {} {
    variable maxloglevel
    .pdwindow.text.internal tag configure log0 -foreground "#d00" -background "#ffe0e8"
    .pdwindow.text.internal tag configure log1 -foreground "#d00"
    # log2 messages are normal black on white
    .pdwindow.text.internal tag configure log3 -foreground "#484848"

    # 0-20(4-24) is a rough useful range of 'verbose' levels for impl debugging
    set start 4
    set end 25
    for {set i $start} {$i < $end} {incr i} {
        set B [expr int(($i - $start) * (40 / ($end - $start))) + 50]
        .pdwindow.text.internal tag configure log${i} -foreground grey${B}
    }
}


# grab focus on part of the Pd window when Pd is busy
proc ::pd_console::busygrab {} {
    # set the mouse cursor to look busy and grab focus so it stays that way    
    .pdwindow.text configure -cursor watch
    grab set .pdwindow.text
}

# release focus on part of the Pd window when Pd is finished
proc ::pd_console::busyrelease {} {
    .pdwindow.text configure -cursor xterm
    grab release .pdwindow.text
}

# ------------------------------------------------------------------------------
# pdtk functions for 'pd' to send data to the Pd window

proc ::pd_console::buffer_message {object_id level message} {
    variable logbuffer
    lappend logbuffer $object_id $level $message
}

proc ::pd_console::insert_log_line {object_id level message} {
    if {$object_id eq ""} {
        .pdwindow.text.internal insert end $message log$level
    } else {
        .pdwindow.text.internal insert end $message [list log$level obj$object_id]
        .pdwindow.text.internal tag bind obj$object_id <$::var(modifier)-ButtonRelease-1> \
            "::pd_console::select_by_id $object_id; break"
        .pdwindow.text.internal tag bind obj$object_id <Key-Return> \
            "::pd_console::select_by_id $object_id; break"
        .pdwindow.text.internal tag bind obj$object_id <Key-KP_Enter> \
            "::pd_console::select_by_id $object_id; break"
    }
}

# this has 'args' to satisfy trace, but its not used
proc ::pd_console::filter_buffer_to_text {args} {
    variable logbuffer
    variable maxloglevel
    .pdwindow.text.internal delete 0.0 end
    set i 0
    foreach {object_id level message} $logbuffer {
        insert_log_line $object_id $level $message

        # this could take a while, so update the GUI every 10000 lines
        if { [expr $i % 10000] == 0} {update idletasks}
        incr i
    }
    .pdwindow.text.internal yview end
    ::pd_console::verbose 10 "The Pd window filtered $i lines\n"
}

proc ::pd_console::select_by_id {args} {
    if [llength $args] { # Is $args empty?
        ::pd_connect::pdsend "pd findinstance $args"
    }
}

# logpost posts to Pd window with an object to trace back to and a
# 'log level'. The logpost and related procs are for generating
# messages that are useful for debugging patches.  They are messages
# that are meant for the Pd programmer to see so that they can get
# information about the patches they are building
proc ::pd_console::logpost {object_id level message} {
    variable maxloglevel
    variable lastlevel $level

    buffer_message $object_id $level $message
    if {[llength [info commands .pdwindow.text.internal]]} {
        # cancel any pending move of the scrollbar, and schedule it
        # after writing a line. This way the scrollbar is only moved once
        # when the inserting has finished, greatly speeding things up
        after cancel .pdwindow.text.internal yview end
        insert_log_line $object_id $level $message
        after idle .pdwindow.text.internal yview end
    }
    
    if {$::var(is_stderr)} {puts stderr $message}
}

# shortcuts for posting to the Pd window
proc ::pd_console::fatal {message} {logpost {} 0 $message}
proc ::pd_console::error {message} {logpost {} 1 $message}
proc ::pd_console::post {message} {logpost {} 2 $message}
proc ::pd_console::debug {message} {logpost {} 3 $message}
# for backwards compatibility
proc ::pd_console::bug {message} {logpost {} 1 \
    [concat consistency check failed: $message]}
proc ::pd_console::pdtk_post {message} {post $message}

proc ::pd_console::endpost {} {
    variable linecolor
    variable lastlevel
    logpost {} $lastlevel "\n"
    set linecolor [expr ! $linecolor]
}

# this verbose proc has a separate numbering scheme since its for
# debugging implementations, and therefore falls outside of the 0-3
# numbering on the Pd window.  They should only be shown in ALL mode.
proc ::pd_console::verbose {level message} {
    incr level 4
    logpost {} $level $message
}

# clear the log and the buffer
proc ::pd_console::clear_console {} {
    variable logbuffer {}
    .pdwindow.text.internal delete 0.0 end
}

# save the contents of the pd_console::logbuffer to a file
proc ::pd_console::save_logbuffer_to_file {} {
    variable logbuffer
    set filename [tk_getSaveFile -initialfile "pdwindow.txt" -defaultextension .txt]
    if {$filename eq ""} return; # they clicked cancel
    set f [open $filename w]
    puts $f "Tcl/Tk [info patchlevel]"
    puts $f "------------------------------------------------------------------------------"
    puts $f $logbuffer
    close $f
}


#--compute audio/DSP checkbutton-----------------------------------------------#

# set the checkbox on the "Compute Audio" menuitem and checkbox
proc ::pd_console::pdtk_pd_dsp {value} {
    # TODO canvas_startdsp/stopdsp should really send 1 or 0, not "ON" or "OFF"
    if {$value eq "ON"} {
        set ::var(is_dsp) 1
    } else {
        set ::var(is_dsp) 0
    }
}

proc ::pd_console::pdtk_pd_dio {red} {
    if {$red == 1} {
        .pdwindow.header.ioframe.dio configure -foreground red
    } else {
        .pdwindow.header.ioframe.dio configure -foreground lightgray
    }
        
}

proc ::pd_console::pdtk_pd_audio {state} {
    .pdwindow.header.ioframe.iostate configure -text [concat audio $state]
}

#--bindings specific to the Pd window------------------------------------------#

proc ::pd_console::pdwindow_bindings {} {
    # these bindings are for the whole Pd window, minus the Tcl entry
    foreach window {.pdwindow.text .pdwindow.header} {
        bind $window <$::var(modifier)-Key-x> "tk_textCut .pdwindow.text"
        bind $window <$::var(modifier)-Key-c> "tk_textCopy .pdwindow.text"
        bind $window <$::var(modifier)-Key-v> "tk_textPaste .pdwindow.text"
    }
    # Select All doesn't seem to work unless its applied to the whole window
    bind .pdwindow <$::var(modifier)-Key-a> ".pdwindow.text tag add sel 1.0 end"
    # the "; break" part stops executing another binds, like from the Text class

    # these don't do anything in the Pd window, so alert the user, then break
    # so no more bindings run
    bind .pdwindow <$::var(modifier)-Key-s> "bell; break"
    bind .pdwindow <$::var(modifier)-Key-p> "bell; break"

    # ways of hiding/closing the Pd window
    if {[tk windowingsystem] eq "aqua"} {
        # on Mac OS X, you can close the Pd window, since the menubar is there
        bind .pdwindow <$::var(modifier)-Key-w>   "wm withdraw .pdwindow"
        wm protocol .pdwindow WM_DELETE_WINDOW "wm withdraw .pdwindow"
    } else {
        # TODO should it possible to close the Pd window and keep Pd open?
        bind .pdwindow <$::var(modifier)-Key-w>   "wm iconify .pdwindow"
        wm protocol .pdwindow WM_DELETE_WINDOW "::pd_connect::pdsend \"pd verifyquit\""
    }
}

#--Tcl entry procs-------------------------------------------------------------#

proc ::pd_console::eval_tclentry {} {
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

proc ::pd_console::get_history {direction} {
    variable tclentry_history
    variable history_position

    incr history_position $direction
    if {$history_position < 0} {set history_position 0}
    if {$history_position > [llength $tclentry_history]} {
        set history_position [llength $tclentry_history]
    }
    .pdwindow.tcl.entry delete 0 end
    .pdwindow.tcl.entry insert 0 \
        [lindex $tclentry_history end-[expr $history_position - 1]]
}

proc ::pd_console::validate_tcl {} {
    variable tclentry
    if {[info complete $tclentry]} {
        .pdwindow.tcl.entry configure -background "white"
    } else {
        .pdwindow.tcl.entry configure -background "#FFF0F0"
    }
}

#--create tcl entry-----------------------------------------------------------#

proc ::pd_console::set_findinstance_cursor {widget key state} {
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

proc ::pd_console::create_window {} {
    variable logmenuitems
    set ::patch_loaded(.pdwindow) 0

    # colorize by class before creating anything
    option add *PdWindow*Entry.highlightBackground "grey" startupFile
    option add *PdWindow*Frame.background "grey" startupFile
    option add *PdWindow*Label.background "grey" startupFile
    option add *PdWindow*Checkbutton.background "grey" startupFile
    option add *PdWindow*Menubutton.background "grey" startupFile
    option add *PdWindow*Text.background "white" startupFile
    option add *PdWindow*Entry.background "white" startupFile

    toplevel .pdwindow -class PdWindow
    wm title .pdwindow [_ "Pd"]
    set ::patch_name(.pdwindow) [_ "Pd"]
    if {[tk windowingsystem] eq "x11"} {
        wm minsize .pdwindow 400 75
    } else {
        wm minsize .pdwindow 400 51
    }
    wm geometry .pdwindow =500x400+20+50
    .pdwindow configure -menu .menubar

    frame .pdwindow.header -borderwidth 1 -relief flat -background lightgray
    pack .pdwindow.header -side top -fill x -ipady 5

    frame .pdwindow.header.pad1
    pack .pdwindow.header.pad1 -side left -padx 12

    checkbutton .pdwindow.header.dsp -text [_ "DSP"] -variable ::var(is_dsp) \
        -font {$::var(font_family) -18 bold} -takefocus 1 -background lightgray \
        -borderwidth 0  -command {::pd_connect::pdsend "pd dsp $::var(is_dsp)"}
    pack .pdwindow.header.dsp -side right -fill y -anchor e -padx 5 -pady 0

# frame for DIO error and audio in/out labels
    frame .pdwindow.header.ioframe -background lightgray
    pack .pdwindow.header.ioframe -side right -padx 30

# I/O state label (shows I/O on/off/in-only/out-only)
    label .pdwindow.header.ioframe.iostate \
        -text [_ "audio I/O off"] -borderwidth 0 \
        -background lightgray -foreground black \
        -takefocus 0 \
        -font {$::var(font_family) -14}

# DIO error label
    label .pdwindow.header.ioframe.dio \
        -text [_ "audio I/O error"] -borderwidth 0 \
        -background lightgray -foreground lightgray \
        -takefocus 0 \
        -font {$::var(font_family) -14}

    pack .pdwindow.header.ioframe.iostate .pdwindow.header.ioframe.dio \
        -side top

    frame .pdwindow.tcl -borderwidth 0
    pack .pdwindow.tcl -side bottom -fill x
# TODO this should use the pd_font_$size created in pd-gui.tcl    
    text .pdwindow.text -relief raised -bd 2 -font {$::var(font_family) -12} \
        -highlightthickness 0 -borderwidth 1 -relief flat \
        -yscrollcommand ".pdwindow.scroll set" -width 60 \
        -undo false -autoseparators false -maxundo 1 -takefocus 0
    scrollbar .pdwindow.scroll -command ".pdwindow.text.internal yview"
    pack .pdwindow.scroll -side right -fill y
    pack .pdwindow.text -side right -fill both -expand 1
    raise .pdwindow
    focus .pdwindow.text
    # run bindings last so that .pdwindow.tcl.entry exists
    pdwindow_bindings
    # set cursor to show when clicking in 'findinstance' mode
    bind .pdwindow <KeyPress> "+::pd_console::set_findinstance_cursor %W %K %s"
    bind .pdwindow <KeyRelease> "+::pd_console::set_findinstance_cursor %W %K %s"

    # hack to make a good read-only text widget from http://wiki.tcl.tk/1152
    rename ::.pdwindow.text ::.pdwindow.text.internal
    proc ::.pdwindow.text {args} {
        switch -exact -- [lindex $args 0] {
            "insert" {}
            "delete" {}
            "default" { return [eval ::.pdwindow.text.internal $args] }
        }
    }
    
    # print whatever is in the queue after the event loop finishes
    after idle [list after 0 ::pd_console::filter_buffer_to_text]

    set ::patch_loaded(.pdwindow) 1

    # set some layout variables
    ::pd_console::set_layout

    # wait until .pdwindow.tcl.entry is visible before opening files so that
    # the loading logic can grab it and put up the busy cursor
    tkwait visibility .pdwindow.text
}
