
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Application menus.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_menu 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_menu:: {

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
    
    foreach m {file edit object media} {    
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
    .menubar.file entryconfigure [_ "Close"]        -state disabled
    
    _copying disabled
    
    .menubar.edit entryconfigure [_ "Edit Mode"]    -state disabled
}

proc configureForText {} {
    
    .menubar.file entryconfigure [_ "Save"]         -state disabled
    .menubar.file entryconfigure [_ "Save As..."]   -state disabled
    .menubar.file entryconfigure [_ "Close"]        -state disabled
    
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
        .popup entryconfigure [_ "Properties"] -state normal
    } else {
        .popup entryconfigure [_ "Properties"] -state disabled
    }
    
    if {$hasOpen} {
        .popup entryconfigure [_ "Open"] -state normal
    } else {
        .popup entryconfigure [_ "Open"] -state disabled
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
proc enablePath {}                  { .menubar.media entryconfigure [_ "Path..."] -state normal }

proc disableCopying {}              { _copying disabled }
proc disableEditing {}              { _editing disabled }
proc disableCopyingAndEditing {}    { _copying disabled; _editing disabled }
proc disablePath {}                 { .menubar.media entryconfigure [_ "Path..."] -state disabled }

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
        -command { ::pd_file::newPatch }
    $m add command \
        -label [_ "Open..."] \
        -accelerator "${accelerator}+O" \
        -command { ::pd_file::openPatch }
    $m add separator
    
    $m add command \
        -label [_ "Save"] \
        -accelerator "${accelerator}+S" \
        -command { ::pd_menu::_handle menusave }
    $m add command \
        -label [_ "Save As..."] \
        -accelerator "Shift+${accelerator}+S" \
        -command { ::pd_menu::_handle menusaveas }
    $m add separator

    $m add command \
        -label [_ "Close"] \
        -accelerator "${accelerator}+W" \
        -command { ::pd_menu::_handle "menuclose 0" }
        
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
        -command { ::pd_menu::_handle cut }
    $m add command \
        -label [_ "Copy"] \
        -accelerator "${accelerator}+C" \
        -command { ::pd_menu::_handle copy }
    $m add command \
        -label [_ "Paste"] \
        -accelerator "${accelerator}+V" \
        -command { ::pd_menu::_handle paste }
    $m add separator
    
    $m add command \
        -label [_ "Duplicate"] \
        -accelerator "${accelerator}+D" \
        -command { ::pd_menu::_handle duplicate }
    $m add command \
        -label [_ "Select All"] \
        -accelerator "${accelerator}+A" \
        -command { ::pd_menu::_handle selectall }
    $m add separator
    
    $m add check \
        -label [_ "Edit Mode"] \
        -accelerator "${accelerator}+E" \
        -variable ::var(isEditMode) \
        -command { ::pd_menu::_handle "editmode $::var(isEditMode)" }
}

proc _object {m} {

    variable accelerator
    
    $m add command \
        -label [_ "Object"] \
        -accelerator "${accelerator}+1" \
        -command { ::pd_menu::_handle "obj 0" } 
    $m add command \
        -label [_ "Message"] \
        -accelerator "${accelerator}+2" \
        -command { ::pd_menu::_handle "msg 0" }
    $m add command \
        -label [_ "Float"] \
        -accelerator "${accelerator}+3" \
        -command { ::pd_menu::_handle "floatatom 0" }
    $m add command \
        -label [_ "Symbol"] \
        -accelerator "${accelerator}+4" \
        -command { ::pd_menu::_handle "symbolatom 0" }
    $m add command \
        -label [_ "Comment"] \
        -accelerator "${accelerator}+5" \
        -command { ::pd_menu::_handle "text 0" }
    $m add separator
    
    $m add command \
        -label [_ "Bang"] \
        -accelerator "${accelerator}+6" \
        -command { ::pd_menu::_handle bng }
    $m add command \
        -label [_ "Toggle"] \
        -accelerator "${accelerator}+7" \
        -command { ::pd_menu::_handle toggle }
    $m add command \
        -label [_ "Panel"] \
        -accelerator "${accelerator}+8" \
        -command { ::pd_menu::_handle mycnv }
    $m add command \
        -label [_ "Array"] \
        -accelerator "${accelerator}+9" \
        -command { ::pd_menu::_handle menuarray }
    $m add separator
     
    $m add command \
        -label [_ "Number"] \
        -command { ::pd_menu::_handle numbox }
    $m add command \
        -label [_ "VU Meter"] \
        -command { ::pd_menu::_handle vumeter }
    $m add separator
    
    menu $m.vertical
    
    $m.vertical add command \
        -label [_ "Slider"] \
        -command { ::pd_menu::_handle vslider }
    $m.vertical add command \
        -label [_ "Radio Button"] \
        -command { ::pd_menu::_handle vradio }
    
    menu $m.horizontal
        
    $m.horizontal add command \
        -label [_ "Slider"] \
        -command { ::pd_menu::_handle hslider }
    $m.horizontal add command \
        -label [_ "Radio Button"] \
        -command { ::pd_menu::_handle hradio }
        
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
        -command { ::pd_connect::pdsend "pd midi-properties" }
    $m add command \
        -label [_ "Audio..."] \
        -command { ::pd_connect::pdsend "pd audio-properties" }
    $m add separator
    
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
    
    $m add command \
        -label [_ "Path..."] \
        -command { ::pd_connect::pdsend "pd start-path-dialog" }
    $m add separator
    
    $m add check \
        -label [_ "Run DSP"] \
        -accelerator "${accelerator}+R" \
        -variable ::var(isDsp) \
        -command { ::pd_connect::pdsend "pd dsp $::var(isDsp)" }
}

proc _popup {m} {

    $m add command \
        -label [_ "Properties"] \
        -command { ::pd_menu::_doPopup $::var(windowFocused) 0 }
    $m add command \
        -label [_ "Open"] \
        -command { ::pd_menu::_doPopup $::var(windowFocused) 1 }
    $m add command \
        -label [_ "Help"]       \
        -command { ::pd_menu::_doPopup $::var(windowFocused) 2 }
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

proc _handle {message} {

    set top [winfo toplevel $::var(windowFocused)]
    
    if {[winfo class $top] eq "PdPatch"} { ::pd_connect::pdsend "$top $message" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _doPopup {top action} {
    
    variable popupX
    variable popupY
    
    ::pd_connect::pdsend "$top done-popup $action $popupX $popupY"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
