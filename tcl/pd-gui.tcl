#! /usr/bin/env wish

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

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

# Withdraw the window first to avoid flashing.

catch { wm withdraw . }

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
package require dialog_confirm
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
package require pd_text
package require pd_textwindow

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Global variables that are used throughout the GUI.

set var(apiAudioAvailables)     {}
set var(apiMidiAvailables)      {}
set var(apiAudio)               0
set var(apiMidi)                0

set var(cursorRunNothing)       ""
set var(cursorRunClickMe)       ""
set var(cursorRunThicken)       "sb_v_double_arrow"
set var(cursorRunAddPoint)      "plus"
set var(cursorEditNothing)      "hand2"
set var(cursorEditConnect)      "circle"
set var(cursorEditDisconnect)   "X_cursor"
set var(cursorEditResize)       "sb_h_double_arrow"

set var(directoryNew)           [pwd]
set var(directoryOpen)          [pwd]
set var(directorySearchPath)    {}

set var(filesOpenPended)        {}
set var(filesRecent)            {}
set var(filesTypes)             { {{PureData patch} {.pd}} {{PureData help} {.pdhelp}} }

set var(fontFamily)             [::pd_miscellaneous::getDefaultFamily]
set var(fontWeight)             "normal"
set var(fontSizes)              "8 10 12 16 18 20 24 30 36"

set var(isInitialized)          0
set var(isDsp)                  0
set var(isEditmode)             0

set var(modifierKey)            ""
set var(scriptName)             [file normalize [info script]]

set var(startupFlags)           {}
set var(startupLibraries)       {}

set var(tcpHost)                ""
set var(tcpPort)                0

set var(windowPopupX)           0
set var(windowPopupY)           0
set var(windowFocused)          .

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getFont {size} { return "::var(font${size})" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set audio_indev     {}
set audio_outdev    {}
set midi_indev      {}
set midi_outdev     {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Per patch states.

array set patch_childs          {}
array set patch_isEditmode      {}
array set patch_isEditing       {}
array set patch_isScrollableX   {}
array set patch_isScrollableY   {}
array set patch_name            {}
array set patch_parents         {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _ {s} { return $s }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc mainSettings {} {

    # GUI attributes.
    
    option add *PdConsole*Entry.highlightBackground "grey" startupFile
    option add *PdConsole*Frame.background "grey" startupFile
    option add *PdConsole*Label.background "grey" startupFile
    option add *PdConsole*Checkbutton.background "grey" startupFile
    option add *PdConsole*Menubutton.background "grey" startupFile
    option add *PdConsole*Text.background "white" startupFile
    option add *PdConsole*Entry.background "white" startupFile
}

proc mainSettingsX11 {} {

    set ::var(modifierKey) "Control"
    
    # GUI attributes.
    
    option add *PatchWindow*Canvas.background "white" startupFile
    
    # Don't show hidden files ( http://wiki.tcl.tk/1060 ).
    
    catch { tk_getOpenFile -dummy } 
    set ::tk::dialog::file::showHiddenBtn 1
    set ::tk::dialog::file::showHiddenVar 0
    
    # Define mouse cursor symbols.
    
    set ::var(cursorRunNothing) "left_ptr"
    set ::var(cursorRunClickMe) "arrow"
}

proc mainSettingsAqua {} {

    set ::var(modifierKey) "Mod1"
    
    # GUI attributes.
    
    option add *DialogWindow*background "#E8E8E8" startupFile
    option add *DialogWindow*Entry.highlightBackground "#E8E8E8" startupFile
    option add *DialogWindow*Button.highlightBackground "#E8E8E8" startupFile
    option add *DialogWindow*Entry.background "white" startupFile
    
    # Set initial directory.
    
    set ::var(directoryNew) $::env(HOME)
    set ::var(directoryOpen) $::env(HOME)

    # Define mouse cursor symbols.
    
    set ::var(cursorRunNothing) "arrow"
    set ::var(cursorRunClickMe) "center_ptr"
}

proc mainSettingsWin32 {} {

    set ::var(modifierKey) "Control"
    
    # GUI attributes.
    
    font create menufont -family Tahoma -size -11
    
    option add *PatchWindow*Canvas.background "white" startupFile
    option add *Menu.font menufont startupFile
    option add *DialogWindow*font menufont startupFile
    option add *PdConsole*font menufont startupFile
    option add *ErrorDialog*font menufont startupFile
    
    # Define mouse cursor symbols.
    
    set ::var(cursorRunNothing) "right_ptr"
    set ::var(cursorRunClickMe) "arrow"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc main {argc argv} {

    # Configure to UTF-8 encoding.
    
    encoding system utf-8
    fconfigure stderr -encoding utf-8
    fconfigure stdout -encoding utf-8
    
    # Set various GUI settings.
    
    mainSettings
    
    # Set various platform specific GUI settings.
    
    switch -- [tk windowingsystem] {
        "x11"   { mainSettingsX11   }
        "aqua"  { mainSettingsAqua  }
        "win32" { mainSettingsWin32 }
    }
    
    # Handle socket connection.
    
    if {$argc == 1 && [string is int $argv]} {
        set ::var(tcpHost) "localhost"
        set ::var(tcpPort) $argv
        ::pd_connect::clientSocket $::var(tcpPort) $::var(tcpHost)
        
    } else {
        set ::var(tcpPort) [::pd_connect::serverSocket]
        set executable [file join [file dirname [info script]] ../bin/pd]
        exec -- $executable -guiport $::var(tcpPort) &
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Notice that main is always called first.

proc com_initialize {audioAPIs midiAPIs fontFamily fontWeight} {

    set ::var(apiAudioAvailables) $audioAPIs
    set ::var(apiMidiAvailables)  $midiAPIs
    
    # Overide the default font attributes.
    
    if {[lsearch -exact [font families] $fontFamily] > -1} { set ::var(fontFamily) $fontFamily }
    if {[lsearch -exact {bold normal} $fontWeight] > -1}   { set ::var(fontWeight) $fontWeight }
    
    # Create fonts (determine horizontal and vertical spacing in pixels). 
    
    set measured ""
    
    foreach size $::var(fontSizes) {
        set f [getFont $size]
        font create $f -family $::var(fontFamily) -weight $::var(fontWeight) -size [expr -${size}]
        lappend measured $size 
        lappend measured [font measure $f M]
        lappend measured [font metrics $f -linespace]
    }

    # Initialize some packages.
    
    foreach sub {preferences bindings menus canvas console} { pd_${sub}::initialize }
    
    # Set the menubar configuration.
    
    ::pd_menus::configureForConsole
    
    # Respond and open pended files (if any).
    
    ::pd_connect::pdsend "pd init [enquote_path [pwd]] $measured"
    
    set ::var(isInitialized) 1
    
    foreach filename $::var(filesOpenPended) { ::pd_miscellaneous::open_file $filename }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

main $::argc $::argv

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
