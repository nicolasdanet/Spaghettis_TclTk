
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Application menus.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_menu 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_menu:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable accelerator    "Ctrl"
variable popupX         0
variable popupY         0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    variable accelerator
    
    if {[tk windowingsystem] eq "aqua"} { set accelerator "Cmd" }
    
    menu .menubar
    
    # Create system specific menus ( http://wiki.tcl.tk/1006 ).
    
    if {[tk windowingsystem] eq "aqua"}  { _apple .menubar }
    
    # Create the sub-menus.
    
    foreach m {file edit object arrange tools media} {
        menu .menubar.$m
        [format _%s $m] .menubar.$m
        .menubar add cascade -label [_ [string totitle $m]] -menu .menubar.$m
    }
    
    # Create the popup menu.
    
    menu .popup; _popup .popup
    
    # Configure the application menu.
    
    . configure -menu .menubar
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc configureForPatch {} {

    .menubar.file entryconfigure [_ "Save"]                 -state normal
    .menubar.file entryconfigure [_ "Save As..."]           -state normal
    .menubar.file entryconfigure [_ "Close"]                -state normal

    .menubar.edit entryconfigure [_ "Edit Mode"]            -state normal
}

proc configureForConsole {} {
    
    .menubar.file entryconfigure [_ "Save"]                 -state disabled
    .menubar.file entryconfigure [_ "Save As..."]           -state disabled
    .menubar.file entryconfigure [_ "Close"]                -state disabled
    
    .menubar.edit entryconfigure [_ "Cut"]                  -state disabled
    .menubar.edit entryconfigure [_ "Copy"]                 -state normal
    .menubar.edit entryconfigure [_ "Paste"]                -state disabled
    .menubar.edit entryconfigure [_ "Duplicate"]            -state disabled
    .menubar.edit entryconfigure [_ "Select All"]           -state normal
    .menubar.edit entryconfigure [_ "Edit Mode"]            -state disabled

    .menubar.arrange entryconfigure [_ "Bring to Front"]    -state disabled
    .menubar.arrange entryconfigure [_ "Send to Back"]      -state disabled
    .menubar.arrange entryconfigure [_ "Snap"]              -state disabled
}

proc configureForDialog {} {

    .menubar.file entryconfigure [_ "Save"]                 -state disabled
    .menubar.file entryconfigure [_ "Save As..."]           -state disabled
    .menubar.file entryconfigure [_ "Close"]                -state normal
    
    _copying disabled

    .menubar.edit entryconfigure [_ "Edit Mode"]            -state disabled

    .menubar.arrange entryconfigure [_ "Bring to Front"]    -state disabled
    .menubar.arrange entryconfigure [_ "Send to Back"]      -state disabled
    .menubar.arrange entryconfigure [_ "Snap"]              -state disabled
}

proc configureForText {} {
    
    .menubar.file entryconfigure [_ "Save"]                 -state disabled
    .menubar.file entryconfigure [_ "Save As..."]           -state disabled
    .menubar.file entryconfigure [_ "Close"]                -state normal
    
    _copying normal

    .menubar.edit entryconfigure [_ "Edit Mode"]            -state disabled

    .menubar.arrange entryconfigure [_ "Bring to Front"]    -state disabled
    .menubar.arrange entryconfigure [_ "Send to Back"]      -state disabled
    .menubar.arrange entryconfigure [_ "Snap"]              -state disabled
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc showPopup {top xcanvas ycanvas hasProperties hasOpen hasHelp hasObject hasOrder} {

    variable popupX
    variable popupY
    
    set popupX $xcanvas
    set popupY $ycanvas
    
    if {$hasProperties} {
        .popup entryconfigure [_ "Properties"]          -state normal
    } else {
        .popup entryconfigure [_ "Properties"]          -state disabled
    }
    
    if {$hasOpen} {
        .popup entryconfigure [_ "Open"]                -state normal
    } else {
        .popup entryconfigure [_ "Open"]                -state disabled
    }
    
    if {$hasHelp} {
        .popup entryconfigure [_ "Help"]                -state normal
    } else {
        .popup entryconfigure [_ "Help"]                -state disabled
    }
    
    if {$hasObject} {
        .popup entryconfigure [_ "Add Object"]          -state normal
    } else {
        .popup entryconfigure [_ "Add Object"]          -state disabled
    }
    
    if {$hasOrder} {
        .popup entryconfigure [_ "Bring to Front"]      -state normal
        .popup entryconfigure [_ "Send to Back"]        -state normal
    } else {
        .popup entryconfigure [_ "Bring to Front"]      -state disabled
        .popup entryconfigure [_ "Send to Back"]        -state disabled
    }
    
    set xpopup [expr {int([winfo rootx $top.c] + $xcanvas - [$top.c canvasx 0])}]
    set ypopup [expr {int([winfo rooty $top.c] + $ycanvas - [$top.c canvasy 0])}]
        
    tk_popup .popup $xpopup $ypopup 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc enableCopying {}               { _copying normal }
proc enableEditing {}               { _editing normal }
proc enableCopyingAndEditing {}     { _copying normal; _editing normal }
proc enableMidi  {}                 { .menubar.media entryconfigure [_ "MIDI..."]   -state normal }
proc enableAudio {}                 { .menubar.media entryconfigure [_ "Audio..."]  -state normal }

proc disableCopying {}              { _copying disabled }
proc disableEditing {}              { _editing disabled }
proc disableCopyingAndEditing {}    { _copying disabled; _editing disabled }
proc disableMidi  {}                { .menubar.media entryconfigure [_ "MIDI..."]   -state disabled }
proc disableAudio {}                { .menubar.media entryconfigure [_ "Audio..."]  -state disabled }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apple {m} {

    menu $m.apple
    
    $m.apple add command \
        -label [_ [format "About %s" $::var(appName)]] \
        -command {}

    $m add cascade -menu $m.apple
}

proc _file {m} {

    variable accelerator
    
    $m add command \
        -label [_ "New Patch"] \
        -accelerator "${accelerator}+N" \
        -command { ::ui_file::newPatch }
    $m add command \
        -label [_ "Open..."] \
        -accelerator "${accelerator}+O" \
        -command { ::ui_file::openPatch }
    $m add separator
    
    $m add command \
        -label [_ "Save"] \
        -accelerator "${accelerator}+S" \
        -command { ::ui_menu::_handle save }
    $m add command \
        -label [_ "Save As..."] \
        -accelerator "Shift+${accelerator}+S" \
        -command { ::ui_menu::_handle saveas }
    $m add separator

    $m add command \
        -label [_ "Close"] \
        -accelerator "${accelerator}+W" \
        -command { ::ui_menu::_close }
        
    if {[tk windowingsystem] ne "aqua"} {
    
    $m add command \
        -label [_ "Quit"] \
        -accelerator "${accelerator}+Q" \
        -command { ::ui_interface::pdsend "pd _quit" }
    }
}

proc _edit {m} {

    variable accelerator
    
    $m add command \
        -label [_ "Cut"] \
        -accelerator "${accelerator}+X" \
        -command { ::ui_menu::_handle _cut }
    $m add command \
        -label [_ "Copy"] \
        -accelerator "${accelerator}+C" \
        -command { ::ui_menu::_handle _copy }
    $m add command \
        -label [_ "Paste"] \
        -accelerator "${accelerator}+V" \
        -command { ::ui_menu::_handle _paste }
    $m add separator
    
    $m add command \
        -label [_ "Duplicate"] \
        -accelerator "${accelerator}+D" \
        -command { ::ui_menu::_handle _duplicate }
    $m add command \
        -label [_ "Select All"] \
        -accelerator "${accelerator}+A" \
        -command { ::ui_menu::_handle _selectall }
    $m add separator

    $m add check \
        -label [_ "Edit Mode"] \
        -accelerator "${accelerator}+E" \
        -variable ::var(isEditMode) \
        -command { ::ui_menu::_handle "editmode $::var(isEditMode)" }
}

proc _arrange {m} {

    variable accelerator
    
    $m add command \
        -label [_ "Bring to Front"] \
        -accelerator "Shift+${accelerator}+F" \
        -command { ::ui_menu::_handle _front }
    $m add command \
        -label [_ "Send to Back"] \
        -accelerator "Shift+${accelerator}+B" \
        -command { ::ui_menu::_handle _back }
    $m add separator
    
    $m add command \
        -label [_ "Snap"] \
        -accelerator "${accelerator}+Y" \
        -command { ::ui_menu::_handle _snap }
    $m add check \
        -label [_ "Snap to Grid"] \
        -accelerator "Alt+${accelerator}+G" \
        -variable ::var(isSnapToGrid) \
        -command {
            ::ui_interface::pdsend "pd _grid $::var(isSnapToGrid)"
            ::ui_interface::pdsend "pd _savepreferences"
        }
}

proc _object {m} {

    variable accelerator
    
    $m add command \
        -label [_ "Object"] \
        -accelerator "${accelerator}+1" \
        -command { ::ui_menu::_handle obj } 
    $m add command \
        -label [_ "Message"] \
        -accelerator "${accelerator}+2" \
        -command { ::ui_menu::_handle msg }
    $m add command \
        -label [_ "Atom"] \
        -accelerator "${accelerator}+3" \
        -command { ::ui_menu::_handle floatatom }
    $m add command \
        -label [_ "Symbol"] \
        -accelerator "${accelerator}+4" \
        -command { ::ui_menu::_handle symbolatom }
    $m add command \
        -label [_ "Comment"] \
        -accelerator "${accelerator}+5" \
        -command { ::ui_menu::_handle comment }
    $m add separator
    
    $m add command \
        -label [_ "Bang"] \
        -accelerator "${accelerator}+6" \
        -command { ::ui_menu::_handle bng }
    $m add command \
        -label [_ "Toggle"] \
        -accelerator "${accelerator}+7" \
        -command { ::ui_menu::_handle tgl }
    $m add command \
        -label [_ "Dial"] \
        -accelerator "${accelerator}+8" \
        -command { ::ui_menu::_handle nbx }
    $m add command \
        -label [_ "Array"] \
        -accelerator "${accelerator}+9" \
        -command { ::ui_menu::_handle _array }
    $m add separator
    
    $m add command \
        -label [_ "VU"] \
        -command { ::ui_menu::_handle vu }
    $m add command \
        -label [_ "Panel"] \
        -command { ::ui_menu::_handle cnv }
    $m add separator
    
    menu $m.vertical
    
    $m.vertical add command \
        -label [_ "Slider"] \
        -command { ::ui_menu::_handle vslider }
    $m.vertical add command \
        -label [_ "Radio Button"] \
        -command { ::ui_menu::_handle vradio }
    
    menu $m.horizontal
        
    $m.horizontal add command \
        -label [_ "Slider"] \
        -command { ::ui_menu::_handle hslider }
    $m.horizontal add command \
        -label [_ "Radio Button"] \
        -command { ::ui_menu::_handle hradio }
        
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
        -label [_ "MIDI..."] \
        -command { ::ui_interface::pdsend "pd _midiproperties" }
    $m add command \
        -label [_ "Audio..."] \
        -command { ::ui_interface::pdsend "pd _audioproperties" }
    $m add separator
    
    $m add check \
        -label [_ "Run DSP"] \
        -accelerator "${accelerator}+R" \
        -variable ::var(isDsp) \
        -command { ::ui_interface::pdsend "pd dsp $::var(isDsp)" }
}

proc _tools {m} {

    $m add check \
        -label [_ "Path"] \
        -variable ::var(isPath) \
        -command { 
            if {$::var(isPath)} { ::ui_path::show } else { ::ui_path::hide } 
        }
}

proc _popup {m} {

    $m add command \
        -label [_ "Properties"] \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 0 }
    $m add command \
        -label [_ "Open"] \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 1 }
    $m add command \
        -label [_ "Help"]       \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 2 }
    $m add separator
    
    menu $m.object
    
        $m.object add command \
            -label [_ "Object"] \
            -command { ::ui_menu::_handle obj }
        $m.object add command \
            -label [_ "Message"] \
            -command { ::ui_menu::_handle msg }
        $m.object add command \
            -label [_ "Atom"] \
            -command { ::ui_menu::_handle floatatom }
        $m.object add command \
            -label [_ "Symbol"] \
            -command { ::ui_menu::_handle symbolatom }
        $m.object add command \
            -label [_ "Comment"] \
            -command { ::ui_menu::_handle comment }
        $m.object add separator
        
        $m.object add command \
            -label [_ "Bang"] \
            -command { ::ui_menu::_handle bng }
        $m.object add command \
            -label [_ "Toggle"] \
            -command { ::ui_menu::_handle tgl }
        $m.object add command \
            -label [_ "Dial"] \
            -command { ::ui_menu::_handle nbx }
        $m.object add command \
            -label [_ "Array"] \
            -command { ::ui_menu::_handle _array }
        $m.object add separator
        
        $m.object add command \
            -label [_ "VU"] \
            -command { ::ui_menu::_handle vu }
        $m.object add command \
            -label [_ "Panel"] \
            -command { ::ui_menu::_handle cnv }
        $m.object add separator
        
        menu $m.object.vertical
        
        $m.object.vertical add command \
            -label [_ "Slider"] \
            -command { ::ui_menu::_handle vslider }
        $m.object.vertical add command \
            -label [_ "Radio Button"] \
            -command { ::ui_menu::_handle vradio }
        
        menu $m.object.horizontal
            
        $m.object.horizontal add command \
            -label [_ "Slider"] \
            -command { ::ui_menu::_handle hslider }
        $m.object.horizontal add command \
            -label [_ "Radio Button"] \
            -command { ::ui_menu::_handle hradio }
            
        $m.object add cascade \
            -label [_ "Vertical"] \
            -menu $m.object.vertical
        $m.object add cascade \
            -label [_ "Horizontal"] \
            -menu $m.object.horizontal
    
    $m add cascade \
        -label [_ "Add Object"] \
        -menu $m.object
    $m add separator
    
    $m add command \
        -label [_ "Bring to Front"] \
        -command { ::ui_menu::_handle _front }
    $m add command \
        -label [_ "Send to Back"] \
        -command { ::ui_menu::_handle _back }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _copying {mode} {

    .menubar.edit entryconfigure [_ "Cut"]              -state $mode
    .menubar.edit entryconfigure [_ "Copy"]             -state $mode
    .menubar.edit entryconfigure [_ "Paste"]            -state $mode
    .menubar.edit entryconfigure [_ "Duplicate"]        -state $mode
    .menubar.edit entryconfigure [_ "Select All"]       -state $mode

}

proc _editing {mode} {
    
    .menubar.object entryconfigure [_ "Object"]             -state $mode
    .menubar.object entryconfigure [_ "Message"]            -state $mode
    .menubar.object entryconfigure [_ "Atom"]               -state $mode
    .menubar.object entryconfigure [_ "Symbol"]             -state $mode
    .menubar.object entryconfigure [_ "Comment"]            -state $mode
    .menubar.object entryconfigure [_ "Array"]              -state $mode
    .menubar.object entryconfigure [_ "Bang"]               -state $mode
    .menubar.object entryconfigure [_ "Toggle"]             -state $mode
    .menubar.object entryconfigure [_ "Dial"]               -state $mode
    .menubar.object entryconfigure [_ "Panel"]              -state $mode
    .menubar.object entryconfigure [_ "VU"]                 -state $mode
    .menubar.object entryconfigure [_ "Vertical"]           -state $mode
    .menubar.object entryconfigure [_ "Horizontal"]         -state $mode

    .menubar.arrange entryconfigure [_ "Bring to Front"]    -state $mode
    .menubar.arrange entryconfigure [_ "Send to Back"]      -state $mode
    .menubar.arrange entryconfigure [_ "Snap"]              -state $mode
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _handle {message} {

    set top [winfo toplevel $::var(windowFocused)]
    
    if {[winfo class $top] eq "PdPatch"} { ::ui_interface::pdsend "$top $message" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _close {} {

    set top [winfo toplevel $::var(windowFocused)]
    
    switch -regexp -- [winfo class $top] {
        "PdPatch" { 
            ::ui_interface::pdsend "$top close"
        }
        "PdDialog|PdText|PdTool" { 
            switch -- [::getTitle $top] {
                "Array"         { ::ui_array::closed  $top }
                "Atom"          { ::ui_atom::closed   $top }
                "Audio"         { ::ui_audio::closed  $top }
                "Bang"          { ::ui_iem::closed    $top }
                "Patch"         { ::ui_canvas::closed $top }
                "MIDI"          { ::ui_midi::closed   $top }
                "Dial"          { ::ui_iem::closed    $top }
                "Panel"         { ::ui_iem::closed    $top }
                "Path"          { ::ui_path::closed   $top }
                "Slider"        { ::ui_iem::closed    $top }
                "Radio Button"  { ::ui_iem::closed    $top }
                "Text"          { ::ui_text::closed   $top }
                "Toggle"        { ::ui_iem::closed    $top }
                "VU"            { ::ui_iem::closed    $top }
            }
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _doPopup {top action} {
    
    variable popupX
    variable popupY
    
    ::ui_interface::pdsend "$top _popupdialog $action $popupX $popupY"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
