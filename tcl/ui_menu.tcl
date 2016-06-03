
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2016 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

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
    
    foreach m {file edit object tools media} {    
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

    .menubar.file entryconfigure [_ "Save"]         -state normal
    .menubar.file entryconfigure [_ "Save As..."]   -state normal
    .menubar.file entryconfigure [_ "Close"]        -state normal
    
    .menubar.edit entryconfigure [_ "Edit Mode"]    -state normal
}

proc configureForConsole {} {
    
    .menubar.file entryconfigure [_ "Save"]         -state disabled
    .menubar.file entryconfigure [_ "Save As..."]   -state disabled
    .menubar.file entryconfigure [_ "Close"]        -state disabled
    
    .menubar.edit entryconfigure [_ "Cut"]          -state disabled
    .menubar.edit entryconfigure [_ "Copy"]         -state normal
    .menubar.edit entryconfigure [_ "Paste"]        -state disabled
    .menubar.edit entryconfigure [_ "Duplicate"]    -state disabled
    .menubar.edit entryconfigure [_ "Select All"]   -state normal
    
    .menubar.edit entryconfigure [_ "Edit Mode"]    -state disabled
}

proc configureForDialog {} {

    .menubar.file entryconfigure [_ "Save"]         -state disabled
    .menubar.file entryconfigure [_ "Save As..."]   -state disabled
    .menubar.file entryconfigure [_ "Close"]        -state normal
    
    _copying disabled
    
    .menubar.edit entryconfigure [_ "Edit Mode"]    -state disabled
}

proc configureForText {} {
    
    .menubar.file entryconfigure [_ "Save"]         -state disabled
    .menubar.file entryconfigure [_ "Save As..."]   -state disabled
    .menubar.file entryconfigure [_ "Close"]        -state normal
    
    _copying normal
    
    .menubar.edit entryconfigure [_ "Edit Mode"]    -state disabled
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc showPopup {top xcanvas ycanvas hasProperties hasOpen} {

    variable popupX
    variable popupY
    
    set popupX $xcanvas
    set popupY $ycanvas
    
    if {$hasProperties} {
        .popup entryconfigure [_ "Properties"]      -state normal
    } else {
        .popup entryconfigure [_ "Properties"]      -state disabled
    }
    
    if {$hasOpen} {
        .popup entryconfigure [_ "Open"]            -state normal
    } else {
        .popup entryconfigure [_ "Open"]            -state disabled
    }
    
    set scrollregion [$top.c cget -scrollregion]
    
    set offsetX [expr {[lindex [$top.c xview] 0] * [lindex $scrollregion 2]}]
    set offsetY [expr {[lindex [$top.c yview] 0] * [lindex $scrollregion 3]}]
    
    set xpopup [expr {int($xcanvas + [winfo rootx $top.c] - $offsetX)}]
    set ypopup [expr {int($ycanvas + [winfo rooty $top.c] - $offsetY)}]
    
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
        -command { ::ui_menu::_handle menuarray }
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
    
    .menubar.object entryconfigure [_ "Object"]         -state $mode
    .menubar.object entryconfigure [_ "Message"]        -state $mode
    .menubar.object entryconfigure [_ "Atom"]           -state $mode
    .menubar.object entryconfigure [_ "Symbol"]         -state $mode
    .menubar.object entryconfigure [_ "Comment"]        -state $mode
    .menubar.object entryconfigure [_ "Array"]          -state $mode
    .menubar.object entryconfigure [_ "Bang"]           -state $mode
    .menubar.object entryconfigure [_ "Toggle"]         -state $mode
    .menubar.object entryconfigure [_ "Dial"]           -state $mode
    .menubar.object entryconfigure [_ "Panel"]          -state $mode
    .menubar.object entryconfigure [_ "VU"]             -state $mode
    .menubar.object entryconfigure [_ "Vertical"]       -state $mode
    .menubar.object entryconfigure [_ "Horizontal"]     -state $mode
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
        "PdDialog|PdText|PdData|PdTool" { 
            switch -- [::getTitle $top] {
                "Array"         { ::ui_array::closed  $top }
                "Atom"          { ::ui_atom::closed   $top }
                "Audio"         { ::ui_audio::closed  $top }
                "Bang"          { ::ui_iem::closed    $top }
                "Canvas"        { ::ui_canvas::closed $top }
                "Data"          { ::ui_data::closed   $top }
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
