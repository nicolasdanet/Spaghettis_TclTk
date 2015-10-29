
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_menu 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_commands
package require pd_connect

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_menu:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export initialize
namespace export editing

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

proc editing {mode} {
    
    .menubar.file entryconfigure [_ "Save"]             -state $mode
    .menubar.file entryconfigure [_ "Save As..."]       -state $mode
    .menubar.file entryconfigure [_ "Close"]            -state $mode
    
    .menubar.edit entryconfigure [_ "Cut"]              -state $mode
    .menubar.edit entryconfigure [_ "Paste"]            -state $mode
    .menubar.edit entryconfigure [_ "Duplicate"]        -state $mode
    .menubar.edit entryconfigure [_ "Edit Mode"]        -state $mode
    
    .menubar.object entryconfigure [_ "Object"]         -state $mode
    .menubar.object entryconfigure [_ "Message"]        -state $mode
    .menubar.object entryconfigure [_ "Float"]          -state $mode
    .menubar.object entryconfigure [_ "Symbol"]         -state $mode
    .menubar.object entryconfigure [_ "Comment"]        -state $mode
    .menubar.object entryconfigure [_ "Array"]          -state $mode
    .menubar.object entryconfigure [_ "Bang"]           -state $mode
    .menubar.object entryconfigure [_ "Toggle"]         -state $mode
    .menubar.object entryconfigure [_ "Panel"]          -state $mode
    .menubar.object entryconfigure [_ "Number"]         -state $mode
    .menubar.object entryconfigure [_ "VU Meter"]       -state $mode
    .menubar.object entryconfigure [_ "Vertical"]       -state $mode
    .menubar.object entryconfigure [_ "Horizontal"]     -state $mode
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apple {m} {

    menu $m.apple
    
    $m.apple add command \
        -label [_ "About PureData"] \
        -command {}

    $m add cascade -menu $m.apple
}

proc _file {m} {

    variable accelerator
    
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
        
    if {[tk windowingsystem] ne "aqua"} {
    
    $m add command \
        -label [_ "Quit"] \
        -accelerator "${accelerator}+Q" \
        -command { ::pd_connect::pdsend "pd verifyquit" }
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

    variable accelerator
    
    $m add command \
        -label [_ "Object"] \
        -accelerator "${accelerator}+1" \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) obj 0 } 
    $m add command \
        -label [_ "Message"] \
        -accelerator "${accelerator}+2" \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) msg 0 }
    $m add command \
        -label [_ "Float"] \
        -accelerator "${accelerator}+3" \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) floatatom 0 }
    $m add command \
        -label [_ "Symbol"] \
        -accelerator "${accelerator}+4" \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) symbolatom 0 }
    $m add command \
        -label [_ "Comment"] \
        -accelerator "${accelerator}+5" \
        -command { ::pd_commands::menu_send_float $::var(windowFocused) text 0 }
    $m add separator
    
    $m add command \
        -label [_ "Array"] \
        -accelerator "Shift+${accelerator}+A" \
        -command { ::pd_commands::menu_send $::var(windowFocused) menuarray }
    $m add separator
    
    $m add command \
        -label [_ "Bang"] \
        -accelerator "Shift+${accelerator}+B" \
        -command { ::pd_commands::menu_send $::var(windowFocused) bng }
    $m add command \
        -label [_ "Toggle"] \
        -accelerator "Shift+${accelerator}+T" \
        -command { ::pd_commands::menu_send $::var(windowFocused) toggle }
    $m add command \
        -label [_ "Panel"] \
        -accelerator "Shift+${accelerator}+P" \
        -command { ::pd_commands::menu_send $::var(windowFocused) mycnv } 
    $m add command \
        -label [_ "Number"] \
        -accelerator "Shift+${accelerator}+N" \
        -command { ::pd_commands::menu_send $::var(windowFocused) numbox }
    $m add command \
        -label [_ "VU Meter"] \
        -accelerator "Shift+${accelerator}+U" \
        -command { ::pd_commands::menu_send $::var(windowFocused) vumeter }
    $m add separator
    
    menu $m.vertical
    
    $m.vertical add command \
        -label [_ "Slider"] \
        -accelerator "Shift+${accelerator}+V" \
        -command { ::pd_commands::menu_send $::var(windowFocused) vslider }
    $m.vertical add command \
        -label [_ "RadioButton"] \
        -accelerator "Shift+${accelerator}+D" \
        -command { ::pd_commands::menu_send $::var(windowFocused) vradio }
    
    menu $m.horizontal
        
    $m.horizontal add command \
        -label [_ "Slider"] \
        -accelerator "Shift+${accelerator}+H" \
        -command { ::pd_commands::menu_send $::var(windowFocused) hslider }
    $m.horizontal add command \
        -label [_ "RadioButton"] \
        -accelerator "Shift+${accelerator}+I" \
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
    
    $m add command \
        -label [_ "Path..."] \
        -accelerator "Alt+${accelerator}+P" \
        -command { ::pd_connect::pdsend "pd start-path-dialog" }
    $m add command \
        -label [_ "Libraries..."] \
        -accelerator "Alt+${accelerator}+L" \
        -command { ::pd_connect::pdsend "pd start-startup-dialog" }
    $m add command \
        -label [_ "MIDI..."] \
        -accelerator "Alt+${accelerator}+M" \
        -command { ::pd_connect::pdsend "pd midi-properties" }
    $m add command \
        -label [_ "Audio..."] \
        -accelerator "Alt+${accelerator}+A" \
        -command { ::pd_connect::pdsend "pd audio-properties" }
    $m add separator
    
    foreach e $::var(apiAudioAvailables) {
        foreach {name value} $e {
            $m add radiobutton \
                -label [string totitle $name] \
                -variable ::var(apiAudio) \
                -value $value \
                -command { ::pd_connect::pdsend "pd audio-setapi $::var(apiAudio)" }
        }
    }
    
    if {[llength $::var(apiAudioAvailables)] > 0} { $m add separator }
        
    foreach e $::var(apiMidiAvailables) {
        foreach {name value} $e {
            $m add radiobutton \
                -label [string totitle $name] \
                -variable ::var(apiMidi) \
                -value $value \
                -command { ::pd_connect::pdsend "pd midi-setapi $::var(apiMidi)" }
        }
    }
    
    if {[llength $::var(apiMidiAvailables)] > 0} { $m add separator }

    $m add check \
        -label [_ "Run DSP"] \
        -accelerator "${accelerator}+P" \
        -variable ::var(isDsp) \
        -command { ::pd_connect::pdsend "pd dsp $::var(isDsp)" }
}

proc _window {m} {

    variable accelerator

    if {[tk windowingsystem] eq "aqua"} { $m add separator }
    
    $m add command \
        -label [_ "Next"] \
        -accelerator [_ "${accelerator}+Down"] \
        -command { ::pd_commands::menu_raisenextwindow}
    $m add command \
        -label [_ "Previous"] \
        -accelerator [_ "${accelerator}+Up"] \
        -command { ::pd_commands::menu_raisepreviouswindow }
    $m add separator
    
    $m add command \
        -label [_ "PureData"] \
        -accelerator "${accelerator}+R" \
        -command { ::pd_commands::menu_raise_console }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
