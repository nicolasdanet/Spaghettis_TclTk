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

package require pd_bind
package require pd_connect
package require pd_console
package require pd_handle
package require pd_menu
package require pd_miscellaneous
package require pd_patch
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

set var(filesExtensions)        ".pd .pdhelp"
set var(filesOpenPended)        {}
set var(filesTypes)             { {{PureData patch} {.pd}} {{PureData help} {.pdhelp}} }

set var(fontFamily)             [::pd_miscellaneous::getDefaultFamily]
set var(fontWeight)             "normal"
set var(fontSizes)              "8 10 12 16 18 20 24 30 36"

set var(isInitialized)          0
set var(isDsp)                  0
set var(isEditMode)             0

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

proc getFont {size}  { return "::var(font${size})" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set audio_indev     {}
set audio_outdev    {}
set midi_indev      {}
set midi_outdev     {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Per patch states.

array set patch_isEditMode      {}
array set patch_isEditing       {}
array set patch_isScrollableX   {}
array set patch_isScrollableY   {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# For future msgcat ( https://www.gnu.org/software/gettext/manual/html_node/Tcl.html ).

proc _ {s} { return $s }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc main {argc argv} {

    # Configure to UTF-8 encoding.
    
    encoding system utf-8
    fconfigure stderr -encoding utf-8
    fconfigure stdout -encoding utf-8
    
    # Don't show hidden files ( http://wiki.tcl.tk/1060 ).
    
    if {[tk windowingsystem] eq "x11"} {
        catch { tk_getOpenFile -dummy } 
        set ::tk::dialog::file::showHiddenBtn 1
        set ::tk::dialog::file::showHiddenVar 0
    }
    
    # Avoid tear-off menus.
    
    option add *tearOff 0
    
    # Set various platform specific settings.
    
    switch -- [tk windowingsystem] {
        "x11"   {
            set ::var(cursorRunNothing)     "left_ptr"
            set ::var(cursorRunClickMe)     "arrow"
        }
        "aqua"  {
            set ::var(cursorRunNothing)     "arrow"
            set ::var(cursorRunClickMe)     "center_ptr"
            set ::var(directoryNew)         $::env(HOME)
            set ::var(directoryOpen)        $::env(HOME)
        }
        "win32" { 
            set ::var(cursorRunNothing)     "right_ptr"
            set ::var(cursorRunClickMe)     "arrow"
        }
    }
    
    # Handle socket connection.
    
    if {$argc == 1 && [string is int $argv]} {
        set ::var(tcpHost) "localhost"
        set ::var(tcpPort) $argv
        ::pd_connect::clientSocket $::var(tcpPort) $::var(tcpHost)
        
    } else {
        set ::var(tcpPort) [::pd_connect::serverSocket]
        set executable [file join [file dirname [info script]] "../bin/pd"]
        exec -- $executable -guiport $::var(tcpPort) &
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Notice that main is always called first.

proc initialize {audioAPIs midiAPIs fontFamily fontWeight} {

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
    
    foreach module {pd_menu pd_console pd_bind pd_patch} { [format "::%s::initialize" $module] }
    
    # Set the menu configuration.
    
    ::pd_menu::configureForConsole
    ::pd_patch::pdtk_canvas_editmode .console 0
    
    # Respond.
    
    ::pd_connect::pdsend "pd init [enquote_path [pwd]] $measured"
    
    set ::var(isInitialized) 1
    
    # Open pended files.
    
    foreach filename $::var(filesOpenPended) { ::pd_miscellaneous::openFile $filename }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

main $::argc $::argv

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
