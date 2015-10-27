
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_menu 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_commands

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_menu:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export initialize
namespace export configureForConsole
namespace export configureForPatch
namespace export configureForDialog

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable accelerator "Ctrl"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    variable accelerator
    
    if {[tk windowingsystem] eq "aqua"} { set accelerator "Cmd" }
    
    menu .menubar
    
    # Create system specific menus ( http://wiki.tcl.tk/1006 ).
    
    if {[tk windowingsystem] eq "aqua"}  { _apple .menubar }
    
    # Create sub-menus.
    
    foreach m {file edit object media window} {    
        menu .menubar.$m
        [format _%s $m] .menubar.$m
        .menubar add cascade -label [_ [string totitle $m]] -menu .menubar.$m
    }
    
    . configure -menu .menubar
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc configureForConsole {} {

    _editing disabled
    ::pd_patch::pdtk_canvas_editmode .console 0
}

proc configureForPatch {top} {

    _editing normal
    ::pd_patch::pdtk_canvas_editmode $top $::patch_isEditMode($top)
}

proc configureForDialog {top} {

    _editing disabled
    ::pd_patch::pdtk_canvas_editmode $top 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _editing {mode} {
    
    .menubar.file entryconfigure [_ "Save"]         -state $mode
    .menubar.file entryconfigure [_ "Save As..."]   -state $mode
    .menubar.file entryconfigure [_ "Close"]        -state $mode
    .menubar.edit entryconfigure [_ "Duplicate"]    -state $mode
    .menubar.edit entryconfigure [_ "Edit Mode"]    -state $mode
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _file {m} {

    variable accelerator
    
    switch -- [tk windowingsystem] {
        "x11"   {
            $m add command \
                -label [_ "New Patch"] \
                -accelerator "${accelerator}+N" \
                -command { ::pd_commands::menu_new }
            $m add command \
                -label [_ "Open..."] \
                -accelerator "${accelerator}+O" \
                -command { ::pd_commands::menu_open }
            $m add separator
            
            $m add command \
                -label [_ "Save"] \
                -accelerator "${accelerator}+S" \
                -command { ::pd_commands::menu_send $::var(windowFocused) menusave }
            $m add command \
                -label [_ "Save As..."] \
                -accelerator "Shift+${accelerator}+S" \
                -command { ::pd_commands::menu_send $::var(windowFocused) menusaveas }
            $m add separator
            
            _preferences $m.preferences
            
            $m add cascade \
                -label [_ "Preferences"] \
                -menu $m.preferences 
            $m add separator
            
            $m add command \
                -label [_ "Close"] \
                -accelerator "${accelerator}+W" \
                -command { ::pd_commands::menu_send_float $::var(windowFocused) menuclose 0 }
            $m add command \
                -label [_ "Quit"] \
                -accelerator "${accelerator}+Q" \
                -command { ::pd_connect::pdsend "pd verifyquit" }
        }
        "aqua"  {
            $m add command \
                -label [_ "New Patch"] \
                -accelerator "${accelerator}+N" \
                -command { ::pd_commands::menu_new }
            $m add command \
                -label [_ "Open..."] \
                -accelerator "${accelerator}+O" \
                -command { ::pd_commands::menu_open }
            $m add separator
            
            $m add command \
                -label [_ "Save"] \
                -accelerator "${accelerator}+S" \
                -command { ::pd_commands::menu_send $::var(windowFocused) menusave }
            $m add command \
                -label [_ "Save As..."] \
                -accelerator "Shift+${accelerator}+S" \
                -command { ::pd_commands::menu_send $::var(windowFocused) menusaveas }
            $m add separator
                        
            $m add command \
                -label [_ "Close"] \
                -accelerator "${accelerator}+W" \
                -command { ::pd_commands::menu_send_float $::var(windowFocused) menuclose 0 }
        }
        "win32" {
            $m add command \
                -label [_ "New Patch"] \
                -accelerator "${accelerator}+N" \
                -command { ::pd_commands::menu_new }
            $m add command \
                -label [_ "Open..."] \
                -accelerator "${accelerator}+O" \
                -command { ::pd_commands::menu_open }
            $m add separator
            
            $m add command \
                -label [_ "Save"] \
                -accelerator "${accelerator}+S" \
                -command { ::pd_commands::menu_send $::var(windowFocused) menusave }
            $m add command \
                -label [_ "Save As..."] \
                -accelerator "Shift+${accelerator}+S" \
                -command { ::pd_commands::menu_send $::var(windowFocused) menusaveas }
            $m add separator
            
            _preferences $m.preferences
            
            $m add cascade \
                -label [_ "Preferences"] \
                -menu $m.preferences             
            $m add separator
            
            $m add command \
                -label [_ "Close"] \
                -accelerator "${accelerator}+W" \
                -command { ::pd_commands::menu_send_float $::var(windowFocused) menuclose 0 }
            $m add command \
                -label [_ "Quit"] \
                -accelerator "${accelerator}+Q" \
                -command { ::pd_connect::pdsend "pd verifyquit" }
        }
    }
}

proc _edit {m} {

    variable accelerator
    
    $m add command \
        -label [_ "Cut"] \
        -accelerator "${accelerator}+X" \
        -command { ::pd_commands::menu_send $::var(windowFocused) cut }
    $m add command \
        -label [_ "Copy"] \
        -accelerator "${accelerator}+C" \
        -command { ::pd_commands::menu_send $::var(windowFocused) copy }
    $m add command \
        -label [_ "Paste"] \
        -accelerator "${accelerator}+V" \
        -command { ::pd_commands::menu_send $::var(windowFocused) paste }
    $m add separator
    
    $m add command \
        -label [_ "Duplicate"] \
        -accelerator "${accelerator}+D" \
        -command { ::pd_commands::menu_send $::var(windowFocused) duplicate }
    $m add command \
        -label [_ "Select All"] \
        -accelerator "${accelerator}+A" \
        -command { ::pd_commands::menu_send $::var(windowFocused) selectall }
    $m add separator
    
    $m add check \
        -label [_ "Edit Mode"] \
        -accelerator "${accelerator}+E" \
        -variable ::var(isEditMode) \
        -command { ::pd_commands::menu_editmode $::var(isEditMode) }
}

proc _object {m} {

    $m add command \
        -label [_ "Object"] \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) obj 0 } 
    $m add command \
        -label [_ "Message"] \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) msg 0 }
    $m add command \
        -label [_ "Float"] \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) floatatom 0 }
    $m add command \
        -label [_ "Symbol"] \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) symbolatom 0 }
    $m add command \
        -label [_ "Comment"] \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) text 0 }
    $m add  separator
    
    $m add command \
        -label [_ "Array"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) menuarray }
    $m add  separator
        
    $m add command \
        -label [_ "Bang"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) bng }
    $m add command \
        -label [_ "Toggle"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) toggle }
    $m add command \
        -label [_ "Number"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) numbox }
    $m add command \
        -label [_ "Panel"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) mycnv }
    $m add command \
        -label [_ "Meter"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) vumeter }

    $m add separator
    
    menu $m.vertical
    
    $m.vertical add command \
        -label [_ "Slider"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) vslider }
    $m.vertical add command \
        -label [_ "Radio"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) vradio }
    
    menu $m.horizontal
        
    $m.horizontal add command \
        -label [_ "Slider"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) hslider }
    $m.horizontal add command \
        -label [_ "Radio"] \
        -command { ::pd_commands::menu_send $::var(windowFocused) hradio }
        
    $m add cascade \
        -label [_ "Vertical"] \
        -menu $m.vertical             
    $m add cascade \
        -label [_ "Horizontal"] \
        -menu $m.horizontal  
}

proc _media {m} {

    variable accelerator
    
    $m add radiobutton \
        -label [_ "DSP On"] \
        -accelerator "${accelerator}+/" \
        -variable ::var(isDsp) \
        -value 1 \
        -command { ::pd_connect::pdsend "pd dsp 1" }
    $m add radiobutton \
        -label [_ "DSP Off"] \
        -accelerator "${accelerator}+." \
        -variable ::var(isDsp) \
        -value 0 \
        -command { ::pd_connect::pdsend "pd dsp 0" }
    $m add separator
    
    set audioLength [llength $::var(apiAudioAvailables)]

    for {set x 0} {$x < $audioLength} {incr x} {
        $m add radiobutton \
            -label [lindex [lindex $::var(apiAudioAvailables) $x] 0] \
            -variable ::var(apiAudio) \
            -value [lindex [lindex $::var(apiAudioAvailables) $x] 1] \
            -command { ::pd_connect::pdsend "pd audio-setapi $::var(apiAudio)" }
    }
    
    if {$audioLength > 0} { $m add separator }
        
    set midiLength [llength $::var(apiMidiAvailables)]
    
    for {set x 0} {$x < $midiLength} {incr x} {
        $m add radiobutton \
            -label [lindex [lindex $::var(apiMidiAvailables) $x] 0] \
            -variable ::var(apiMidi) \
            -value [lindex [lindex $::var(apiMidiAvailables) $x] 1] \
            -command { ::pd_connect::pdsend "pd midi-setapi $::var(apiMidi)" }
    }

    if {$midiLength > 0} { $m add separator }
    
    $m add command \
        -label [_ "MIDI..."] \
        -command { ::pd_connect::pdsend "pd midi-properties" }
    $m add separator
    
    $m add command \
        -label [_ "Audio..."] \
        -command { ::pd_connect::pdsend "pd audio-properties" }
}

proc _window {m} {

    variable accelerator
    
    if {[tk windowingsystem] eq "aqua"} {
    
        $m add command \
            -label [_ "Minimize"] \
            -accelerator "${accelerator}+M"\
            -command { ::pd_commands::menu_minimize $::var(windowFocused) }
        $m add command \
            -label [_ "Zoom"] \
            -command { ::pd_commands::menu_maximize $::var(windowFocused) }
        $m add separator
        
        $m add command \
            -label [_ "Bring All to Front"] \
            -command { ::pd_commands::menu_bringalltofront }
        $m add separator
    }
    
    $m add command \
        -label [_ "Next Window"] \
        -command { ::pd_commands::menu_raisenextwindow}
    $m add command \
        -label [_ "Previous Window"] \
        -command { ::pd_commands::menu_raisepreviouswindow }
    $m add separator
    
    $m add command \
        -label [_ "Console"] \
        -accelerator "${accelerator}+R" \
        -command { ::pd_commands::menu_raise_console }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _preferences {m} {

    menu $m
    
    $m add command \
        -label [_ "Path..."] \
        -command { ::pd_connect::pdsend "pd start-path-dialog" }
    $m add command \
        -label [_ "Startup..."] \
        -command { ::pd_connect::pdsend "pd start-startup-dialog" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apple {m} {

    menu $m.apple
    
    $m.apple add command \
        -label [_ "About PureData"] \
        -command { ::pd_commands::menu_aboutpd }
    $m.apple add separator
    
    _preferences $m.apple.preferences
    
    $m.apple add cascade \
        -label [_ "Preferences"] \
        -menu $m.apple.preferences \
    
    $m add cascade -menu $m.apple
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
