#! /usr/bin/env wish

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Withdraw the window first to avoid flashing.

if {[catch { wm withdraw . }]} { exit 2 }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require Tcl 8.5
package require Tk

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Handy for debugging.

if {true} {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

if {[tk windowingsystem] eq "aqua"} {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

catch { console show }

rename unknown _original_unknown

proc unknown {args} {
    set i [info level]
    while {$i > 0} {
        set stack [info level $i]
        if {[unknownIsExpected $stack]} { break } else { puts stderr "$i >>> $stack" }
        incr i -1
    }
    uplevel 1 [list _original_unknown {*}$args]
}

proc unknownIsExpected {stack} {
    set expected { "tclPkgUnknown" "tk::MenuDup" "tk_optionMenu" }
    foreach e $expected {
        if {[string first $e $stack] > -1} { return 1 }
    }
    return 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Note that ALL the Tcl files MUST be kept in the same directory that this file.

set auto_path [linsert $auto_path 0 [file dirname [info script]]]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Package specific to Mac OS X.

if {[tk windowingsystem] eq "aqua"} { package require pd_apple }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require dialog_array
package require dialog_audio
package require dialog_canvas
package require dialog_data
package require dialog_gatom
package require dialog_gui
package require dialog_message
package require dialog_midi
package require dialog_path
package require dialog_startup

package require pd_bindings
package require pd_canvas
package require pd_commands
package require pd_connect
package require pd_console
package require pd_preferences
package require pd_menus
package require pd_miscellaneous
package require pd_parser
package require pd_text
package require pd_textwindow

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _ {s} { return $s }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getFontDefaultFamily {} {
    
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

# Global variables that are used throughout the GUI.

set var(scriptName)             [file normalize [info script]]
set var(modifierKey)            ""
set var(menubar)                ""
set var(menubarHeight)          0
set var(host)                   ""
set var(port)                   0

set var(startupFlags)           {}
set var(startupLibraries)       {}

set var(apiAudioAvailables)     {}
set var(apiMidiAvailables)      {}
set var(apiAudio)               0
set var(apiMidi)                0

set var(directoryNew)           [pwd]
set var(directoryOpen)          [pwd]
set var(directorySearchPath)    {}

set var(filesOpenPended)        {}
set var(filesRecent)            {}
set var(filesTypes)             { {{PureData patch} {.pd}} {{PureData help} {.pdhelp}} }

set var(fontFamily)             [getFontDefaultFamily]
set var(fontWeight)             "normal"
set var(fontSizes)              "8 10 12 16 18 20 24 30 36"

set var(isInitialized)          0
set var(isStderr)               0
set var(isDsp)                  0
set var(isEditmode)             0

set var(windowPopupX)           0
set var(windowPopupY)           0
set var(windowFrameX)           0
set var(windowFrameY)           0
set var(windowFocused)          .

set var(cursorRunNothing)       ""
set var(cursorRunClickMe)       ""
set var(cursorRunThicken)       "sb_v_double_arrow"
set var(cursorRunAddPoint)      "plus"
set var(cursorEditNothing)      "hand2"
set var(cursorEditConnect)      "circle"
set var(cursorEditDisconnect)   "X_cursor"
set var(cursorEditResize)       "sb_h_double_arrow"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set audio_indev     {}
set audio_outdev    {}
set midi_indev      {}
set midi_outdev     {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

array set patch_isEditmode      {}
array set patch_isEditing       {}
array set patch_isScrollableX   {}
array set patch_isScrollableY   {}
array set patch_loaded          {}
array set patch_name            {}
array set patch_childs          {}
array set patch_parents         {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getFontDefaultBySize {size} { return "::var(fontDefault${size})" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initializePlatform {} {
    switch -- [tk windowingsystem] {
        "x11"   { initializePlatformX11   }
        "aqua"  { initializePlatformAqua  }
        "win32" { initializePlatformWin32 }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initializePlatformX11 {} {

    set ::var(modifierKey) "Control"
    
    option add *PatchWindow*Canvas.background "white" startupFile
    
    # Don't show hidden files ( http://wiki.tcl.tk/1060 ).
    
    catch { tk_getOpenFile -dummy } 
    set ::tk::dialog::file::showHiddenBtn 1
    set ::tk::dialog::file::showHiddenVar 0
    
    # Placement refers to the frame's corner instead of the content ( http://wiki.tcl.tk/11502 ).
    
    set ::var(windowFrameX) 3
    set ::var(windowFrameY) 53
    
    # Define mouse cursor symbols.
    
    set ::var(cursorRunNothing) "left_ptr"
    set ::var(cursorRunClickMe) "arrow"
}

proc initializePlatformAqua {} {

    set ::var(modifierKey) "Mod1"
    
    option add *DialogWindow*background "#E8E8E8" startupFile
    option add *DialogWindow*Entry.highlightBackground "#E8E8E8" startupFile
    option add *DialogWindow*Button.highlightBackground "#E8E8E8" startupFile
    option add *DialogWindow*Entry.background "white" startupFile
    
    # Mac OS X needs a menubar all the time.
    
    set ::var(menubar) ".menubar"
    set ::var(menubarHeight) 22
    
    # Define mouse cursor symbols.
    
    set ::var(cursorRunNothing) "arrow"
    set ::var(cursorRunClickMe) "center_ptr"
}

proc initializePlatformWin32 {} {

    set ::var(modifierKey) "Control"
    
    font create menufont -family Tahoma -size -11
    
    option add *PatchWindow*Canvas.background "white" startupFile
    option add *Menu.font menufont startupFile
    option add *DialogWindow*font menufont startupFile
    option add *PdWindow*font menufont startupFile
    option add *ErrorDialog*font menufont startupFile
    
    # Define mouse cursor symbols.
    
    set ::var(cursorRunNothing) "right_ptr"
    set ::var(cursorRunClickMe) "arrow"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc comInitialize {audioAPIs midiAPIs fontFamily fontWeight} {

    set ::var(apiAudioAvailables) $audioAPIs
    set ::var(apiMidiAvailables)  $midiAPIs
    
    puts $audioAPIs
    puts $midiAPIs
    
    # Overide the default font attributes.
    
    if {[lsearch -exact [font families] $fontFamily] > -1} { set ::var(fontFamily) $fontFamily }
    if {[lsearch -exact {bold normal} $fontWeight] > -1}   { set ::var(fontWeight) $fontWeight }
    
    # Create fonts (determine horizontal and vertical spacing in pixels). 
    
    set measured ""
    
    foreach size $::var(fontSizes) {
        set f [getFontDefaultBySize $size]
        font create $f -family $::var(fontFamily) -weight $::var(fontWeight) -size [expr -${size}]
        lappend measured $size 
        lappend measured [font measure $f M]
        lappend measured [font metrics $f -linespace]
    }

    ::pd_preferences::init
    ::pd_connect::pdsend "pd init [enquote_path [pwd]] $measured"
    ::pd_bindings::class_bindings
    ::pd_bindings::global_bindings
    ::pd_menus::create_menubar
    ::pd_canvas::create_popup
    ::pd_console::create_window
    ::pd_menus::configure_for_pdwindow
    open_filestoopen
    set ::var(isInitialized) 1
}

##### routine to ask user if OK and, if so, send a message on to Pd ######
proc pdtk_check {mytoplevel message reply_to_pd default} {
    wm deiconify $mytoplevel
    raise $mytoplevel
    if {[tk windowingsystem] eq "win32"} {
        set answer [tk_messageBox -message [_ $message] -type yesno -default $default \
                        -icon question -title [wm title $mytoplevel]]
    } else {
        set answer [tk_messageBox -message [_ $message] -type yesno \
                        -default $default -parent $mytoplevel -icon question]
    }
    if {$answer eq "yes"} {
        ::pd_connect::pdsend $reply_to_pd
    }
}

# store plugin callbacks for dispatched messages from running Pd patches
global plugin_dispatch_receivers

# dispatch a message from running Pd patches to the intended plugin receiver
proc pdtk_plugin_dispatch { args } {
    set receiver [ lindex $args 0 ]
    foreach callback $::pd_connect::plugin_dispatch_receivers($receiver) {
        $callback [ lrange $args 1 end ]
    }
}

proc parse_args {argc argv} {
    pd_parser::init {
        {-stderr    set {::var(isStderr)}}
        {-open      lappend {- ::var(filesOpenPended)}}
    }
    set unflagged_files [pd_parser::get_options $argv]
    # if we have a single arg that is not a file, its a port or host:port combo
    if {$argc == 1 && ! [file exists $argv]} {
        if { [string is int $argv] && $argv > 0} {
            # 'pd-gui' got the port number from 'pd'
            set ::var(host) "localhost"
            set ::var(port) $argv 
        } else {
            set hostport [split $argv ":"]
            set ::var(port) [lindex $hostport 1]
            if { [string is int $::var(port)] && $::var(port) > 0} {
                set ::var(host) [lindex $hostport 0]
            } else {
                set ::var(port) 0
            }

        }
    } elseif {$unflagged_files ne ""} {
        foreach filename $unflagged_files {
            lappend ::var(filesOpenPended) $filename
        }
    }
}

proc open_filestoopen {} {
    foreach filename $::var(filesOpenPended) {
        ::pd_miscellaneous::open_file $filename
    }
}

# ------------------------------------------------------------------------------
# X11 procs for handling singleton state and getting args from other instances

# first instance
proc singleton {key} {
    if {![catch { selection get -selection $key }]} {
        return 0
    }
    selection handle -selection $key . "singleton_request"
    selection own -command first_lost -selection $key .
    return 1
}

proc singleton_request {offset maxbytes} {
## the next 2 lines raise the focus to the given window (and change desktop)
#    wm deiconify .pdwindow
#    raise .pdwindow
    return [tk appname]
}

proc first_lost {} {
    receive_args [selection get -selection $::var(scriptName) ]
    selection own -command first_lost -selection $::var(scriptName) .
 }

proc others_lost {} {
    set ::singleton_state "exit"
    destroy .
    exit
}

# all other instances
proc send_args {offset maxChars} {
    set sendargs {}
    foreach filename $::var(filesOpenPended) {
        lappend sendargs [file normalize $filename]
    }
    return [string range $sendargs $offset [expr {$offset+$maxChars}]]
}

# this command will open files received from a 2nd instance of Pd
proc receive_args {filelist} {
    raise .
    wm deiconify .pdwindow
    raise .pdwindow
    foreach filename $filelist {
        ::pd_miscellaneous::open_file $filename
    }
}

proc dde_open_handler {cmd} {
    ::pd_miscellaneous::open_file [file normalize $cmd]
}

proc check_for_running_instances { } {
    switch -- [tk windowingsystem] {
        "aqua" {
            # handled by ::tk::mac::OpenDocument in pd_apple.tcl
        } "x11" {
            # http://wiki.tcl.tk/1558
            # TODO replace PUREDATA name with path so this code is a singleton
            # based on install location rather than this hard-coded name
            if {![singleton ${::var(scriptName)}_MANAGER ]} {
                # if pd-gui gets called from pd ('pd-gui 5400') or is told otherwise
                # to connect to a running instance of Pd (by providing [<host>:]<port>)
                # then we don't want to connect to a running instance
                if { $::var(port) > 0 && $::var(host) ne "" } { return }
                selection handle -selection $::var(scriptName) . "send_args"
                selection own -command others_lost -selection $::var(scriptName) .
                after 5000 set ::singleton_state "timeout"
                vwait ::singleton_state
                exit
            } else {
                # first instance
                selection own -command first_lost -selection $::var(scriptName) .
            }
        } "win32" {
            ## http://wiki.tcl.tk/8940
            package require dde ;# 1.4 or later needed for full unicode support
            set topic "Pure_Data_DDE_Open"
            # if no DDE service is running, start one and claim the name
            if { [dde services TclEval $topic] == {} } {
                dde servername -handler dde_open_handler $topic
            }
        }
    }
}


# ------------------------------------------------------------------------------
# main
proc main {argc argv} {
    tk appname pd-gui
    
    encoding system utf-8
    fconfigure stderr -encoding utf-8
    fconfigure stdout -encoding utf-8
    
    parse_args $argc $argv
    check_for_running_instances
    initializePlatform

    # ::var(host) and ::var(port) are parsed from argv by parse_args
    if { $::var(port) > 0 && $::var(host) ne "" } {
        # 'pd' started first and launched us, so get the port to connect to
        ::pd_connect::to_pd $::var(port) $::var(host)
    } else {
        # the GUI is starting first, so create socket and exec 'pd'
        set ::var(port) [::pd_connect::create_socket]
        set pd_exec [file join [file dirname [info script]] ../bin/pd]
        exec -- $pd_exec -guiport $::var(port) &
        if {[tk windowingsystem] eq "aqua"} {
            # on Aqua, if 'pd-gui' first, then initial dir is home
            set ::var(directoryNew) $::env(HOME)
            set ::var(directoryOpen) $::env(HOME)
        }
    }
    ::pd_console::verbose 0 "------------------ done with main ----------------------\n"
}

main $::argc $::argv
