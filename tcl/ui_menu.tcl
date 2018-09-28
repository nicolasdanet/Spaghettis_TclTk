
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
    
    .menubar.edit entryconfigure 0                          -state disabled
    .menubar.edit entryconfigure 1                          -state disabled
    .menubar.edit entryconfigure [_ "Cut"]                  -state normal
    .menubar.edit entryconfigure [_ "Copy"]                 -state normal
    .menubar.edit entryconfigure [_ "Paste"]                -state normal
    .menubar.edit entryconfigure [_ "Duplicate"]            -state disabled
    .menubar.edit entryconfigure [_ "Select All"]           -state normal
    
    .menubar.edit entryconfigure [_ "Edit Mode"]            -state disabled

    _editing disabled
}

proc configureForDialog {} {

    .menubar.file entryconfigure [_ "Save"]                 -state disabled
    .menubar.file entryconfigure [_ "Save As..."]           -state disabled
    .menubar.file entryconfigure [_ "Close"]                -state normal
    
    .menubar.edit entryconfigure 0                          -state disabled
    .menubar.edit entryconfigure 1                          -state disabled
    .menubar.edit entryconfigure [_ "Cut"]                  -state disabled
    .menubar.edit entryconfigure [_ "Copy"]                 -state disabled
    .menubar.edit entryconfigure [_ "Paste"]                -state disabled
    .menubar.edit entryconfigure [_ "Duplicate"]            -state disabled
    .menubar.edit entryconfigure [_ "Select All"]           -state disabled

    .menubar.edit entryconfigure [_ "Edit Mode"]            -state disabled

    _editing disabled
}

proc configureForText {} {
    
    .menubar.file entryconfigure [_ "Save"]                 -state disabled
    .menubar.file entryconfigure [_ "Save As..."]           -state disabled
    .menubar.file entryconfigure [_ "Close"]                -state normal
    
    .menubar.edit entryconfigure 0                          -state disabled
    .menubar.edit entryconfigure 1                          -state disabled
    .menubar.edit entryconfigure [_ "Cut"]                  -state normal
    .menubar.edit entryconfigure [_ "Copy"]                 -state normal
    .menubar.edit entryconfigure [_ "Paste"]                -state normal
    .menubar.edit entryconfigure [_ "Duplicate"]            -state disabled
    .menubar.edit entryconfigure [_ "Select All"]           -state normal

    .menubar.edit entryconfigure [_ "Edit Mode"]            -state disabled

    _editing disabled
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc showPopup {top xcanvas ycanvas hasValue hasProperties hasOpen hasHelp hasObject hasOrder} {

    variable popupX
    variable popupY

    if {$top eq $::var(windowFocused)} {

    set popupX $xcanvas
    set popupY $ycanvas
    
    if {$hasValue} {
        .popup entryconfigure [_ "Values"]              -state normal
    } else {
        .popup entryconfigure [_ "Values"]              -state disabled
    }
    
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
        .popup entryconfigure [_ "Add Scalar"]          -state normal
    } else {
        .popup entryconfigure [_ "Add Object"]          -state disabled
        .popup entryconfigure [_ "Add Scalar"]          -state disabled
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
    
    if {[tk windowingsystem] eq "aqua"} {
        tk_popup .popup $xpopup $ypopup 0
    } else {
        tk_popup .popup $xpopup $ypopup
    }

    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc setUndo {content check} {

    set label [_ "Undo"]
    
    if {$content ne $::var(nil)} {
        set label [_ [format "Undo %s" [string totitle $content]]]
        if {$check} { .menubar.edit entryconfigure 0 -state normal }
    } else {
        if {$check} { .menubar.edit entryconfigure 0 -state disabled }
    }
    
    .menubar.edit entryconfigure 0 -label $label
}

proc setRedo {content check} {

    set label [_ "Redo"]
    
    if {$content ne $::var(nil)} {
        set label [_ [format "Redo %s" [string totitle $content]]]
        if {$check} { .menubar.edit entryconfigure 1 -state normal }
    } else {
        if {$check} { .menubar.edit entryconfigure 1 -state disabled }
    }
    
    .menubar.edit entryconfigure 1 -label $label
}

proc resetUndoAndRedo {} {
    
    .menubar.edit entryconfigure 0 -label [_ "Undo"]
    .menubar.edit entryconfigure 1 -label [_ "Redo"]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Recent files menu must be labeled .menubar.file.recent in the initializing functions.

proc updateRecent {} {

    set m .menubar.file.recent
    
    $m delete 0 end
    
    foreach f [lreverse $::var(filesRecent)] {
        
        $m add command \
            -label [file tail $f] \
            -command [list ::ui_file::openFile $f]
    }
    
    $m add separator
    $m add command \
        -label [_ "Clear Menu"] \
        -command { ::ui_file::clearRecent }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Template cascade must be labeled .popup.scalar in the initializing functions.

proc _updateTemplates {} {

    set m .popup.scalar
    
    $m delete 0 end
    
    foreach item $::var(templates) {
        $m add command -label $item -command [list ::ui_menu::_handle "_scalar $item"]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc appendTemplate {id} {

    lappend ::var(templates) $id
    
    ::ui_menu::_updateTemplates
}

proc removeTemplate {id} {

    set index [lsearch -exact $::var(templates) $id]
    
    set ::var(templates) [lreplace $::var(templates) $index $index]
    
    ::ui_menu::_updateTemplates
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc enableCopyingAndEditing {}     { _copying normal; _editing normal }
proc enableMidi  {}                 { .menubar.media entryconfigure [_ "MIDI"]  -state normal }
proc enableAudio {}                 { .menubar.media entryconfigure [_ "Audio"] -state normal }

proc disableCopyingAndEditing {}    { _copying disabled; _editing disabled }
proc disableMidi  {}                { .menubar.media entryconfigure [_ "MIDI"]  -state disabled }
proc disableAudio {}                { .menubar.media entryconfigure [_ "Audio"] -state disabled }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _copying {mode} {

    .menubar.edit entryconfigure 0                          -state $mode
    .menubar.edit entryconfigure 1                          -state $mode
    .menubar.edit entryconfigure [_ "Cut"]                  -state $mode
    .menubar.edit entryconfigure [_ "Copy"]                 -state $mode
    .menubar.edit entryconfigure [_ "Paste"]                -state $mode
    .menubar.edit entryconfigure [_ "Duplicate"]            -state $mode
    .menubar.edit entryconfigure [_ "Select All"]           -state $mode

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
    .menubar.object entryconfigure [_ "VU"]                 -state $mode
    .menubar.object entryconfigure [_ "Panel"]              -state $mode
    .menubar.object entryconfigure [_ "Menu Button"]        -state $mode
    .menubar.object entryconfigure [_ "Vertical"]           -state $mode
    .menubar.object entryconfigure [_ "Horizontal"]         -state $mode

    .menubar.arrange entryconfigure [_ "Bring to Front"]    -state $mode
    .menubar.arrange entryconfigure [_ "Send to Back"]      -state $mode
    .menubar.arrange entryconfigure [_ "Snap"]              -state $mode
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apple {m} {

    menu $m.apple
    
    $m.apple add command \
        -label [_ "About Spaghettis"] \
        -command {}

    $m add cascade -menu $m.apple
}

proc _file {m} {

    variable accelerator
    
    menu $m.recent
    
    ::ui_menu::updateRecent
    
    $m add command \
        -label [_ "New Patch"] \
        -accelerator "${accelerator}+N" \
        -command { ::ui_file::newPatch }
    $m add command \
        -label [_ "Open..."] \
        -accelerator "${accelerator}+O" \
        -command { ::ui_file::openPatch }
    $m add cascade \
        -label [_ "Open Recent"] \
        -menu $m.recent
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
        -label [_ "Undo"] \
        -accelerator "${accelerator}+Z" \
        -command { ::ui_menu::_handle _undo }
    $m add command \
        -label [_ "Redo"] \
        -accelerator "Shift+${accelerator}+Z" \
        -command { ::ui_menu::_handle _redo }
    $m add separator
        
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
    $m add separator
    
    $m add command \
        -label [_ "Clear Console"] \
        -accelerator "${accelerator}+L" \
        -command { ::ui_console::clear }
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
    $m add separator
    
    $m add check \
        -label [_ "Snap to Grid"] \
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
        -command { ::ui_menu::_handle "obj menu" }
    $m add command \
        -label [_ "Message"] \
        -accelerator "${accelerator}+2" \
        -command { ::ui_menu::_handle "msg menu" }
    $m add command \
        -label [_ "Atom"] \
        -accelerator "${accelerator}+3" \
        -command { ::ui_menu::_handleDirty "floatatom menu" }
    $m add command \
        -label [_ "Symbol"] \
        -accelerator "${accelerator}+4" \
        -command { ::ui_menu::_handleDirty "symbolatom menu" }
    $m add command \
        -label [_ "Comment"] \
        -accelerator "${accelerator}+5" \
        -command { ::ui_menu::_handle "comment menu" }
    $m add separator
    
    $m add command \
        -label [_ "Bang"] \
        -accelerator "${accelerator}+6" \
        -command { ::ui_menu::_handleDirty "bng menu" }
    $m add command \
        -label [_ "Toggle"] \
        -accelerator "${accelerator}+7" \
        -command { ::ui_menu::_handleDirty "tgl menu" }
    $m add command \
        -label [_ "Dial"] \
        -accelerator "${accelerator}+8" \
        -command { ::ui_menu::_handleDirty "nbx menu" }
    $m add command \
        -label [_ "Array"] \
        -accelerator "${accelerator}+9" \
        -command { ::ui_menu::_handle "_array menu" }
    $m add separator
    
    $m add command \
        -label [_ "VU"] \
        -command { ::ui_menu::_handleDirty "vu menu" }
    $m add command \
        -label [_ "Panel"] \
        -command { ::ui_menu::_handleDirty "cnv menu" }
    $m add command \
        -label [_ "Menu Button"] \
        -command { ::ui_menu::_handleDirty "menubutton menu" }
    $m add separator
    
    menu $m.vertical
    
    $m.vertical add command \
        -label [_ "Slider"] \
        -command { ::ui_menu::_handleDirty "vslider menu" }
    $m.vertical add command \
        -label [_ "Radio Button"] \
        -command { ::ui_menu::_handleDirty "vradio menu" }
    
    menu $m.horizontal
        
    $m.horizontal add command \
        -label [_ "Slider"] \
        -command { ::ui_menu::_handleDirty "hslider menu" }
    $m.horizontal add command \
        -label [_ "Radio Button"] \
        -command { ::ui_menu::_handleDirty "hradio menu" }
        
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
        -label [_ "MIDI"] \
        -command { ::ui_interface::pdsend "pd _midiproperties" }
    $m add command \
        -label [_ "Audio"] \
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
    $m add separator
    $m add command \
        -label [_ "Rescan"] \
        -command { ::ui_interface::pdsend "pd scan" }
}

proc _popupObject {m} {

    $m.object add command \
        -label [_ "Object"] \
        -command { ::ui_menu::_handle obj }
    $m.object add command \
        -label [_ "Message"] \
        -command { ::ui_menu::_handle msg }
    $m.object add command \
        -label [_ "Atom"] \
        -command { ::ui_menu::_handleDirty floatatom }
    $m.object add command \
        -label [_ "Symbol"] \
        -command { ::ui_menu::_handleDirty symbolatom }
    $m.object add command \
        -label [_ "Comment"] \
        -command { ::ui_menu::_handle comment }
    $m.object add separator
    
    $m.object add command \
        -label [_ "Bang"] \
        -command { ::ui_menu::_handleDirty bng }
    $m.object add command \
        -label [_ "Toggle"] \
        -command { ::ui_menu::_handleDirty tgl }
    $m.object add command \
        -label [_ "Dial"] \
        -command { ::ui_menu::_handleDirty nbx }
    $m.object add command \
        -label [_ "Array"] \
        -command { ::ui_menu::_handle _array }
    $m.object add separator
    
    $m.object add command \
        -label [_ "VU"] \
        -command { ::ui_menu::_handleDirty vu }
    $m.object add command \
        -label [_ "Panel"] \
        -command { ::ui_menu::_handleDirty cnv }
    $m.object add command \
        -label [_ "Menu Button"] \
        -command { ::ui_menu::_handleDirty menubutton }
    $m.object add separator
    
    menu $m.object.vertical
    
    $m.object.vertical add command \
        -label [_ "Slider"] \
        -command { ::ui_menu::_handleDirty vslider }
    $m.object.vertical add command \
        -label [_ "Radio Button"] \
        -command { ::ui_menu::_handleDirty vradio }
    
    menu $m.object.horizontal
    
    $m.object.horizontal add command \
        -label [_ "Slider"] \
        -command { ::ui_menu::_handleDirty hslider }
    $m.object.horizontal add command \
        -label [_ "Radio Button"] \
        -command { ::ui_menu::_handleDirty hradio }
        
    $m.object add cascade \
        -label [_ "Vertical"] \
        -menu $m.object.vertical
    $m.object add cascade \
        -label [_ "Horizontal"] \
        -menu $m.object.horizontal
}

# POPUP_VALUE
# POPUP_PROPERTIES
# POPUP_OPEN
# POPUP_HELP

proc _popup {m} {

    $m add command \
        -label [_ "Values"] \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 0 }
    $m add command \
        -label [_ "Properties"] \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 1 }
    $m add command \
        -label [_ "Open"] \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 2 }
    $m add command \
        -label [_ "Help"]       \
        -command { ::ui_menu::_doPopup $::var(windowFocused) 3 }
    $m add separator
    
    menu $m.object; _popupObject $m
    menu $m.scalar; ::ui_menu::_updateTemplates
    
    $m add cascade \
        -label [_ "Add Object"] \
        -menu $m.object
    $m add separator
    
    $m add cascade \
        -label [_ "Add Scalar"] \
        -menu $m.scalar
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

proc _handle {message} {

    set top [winfo toplevel $::var(windowFocused)]
    
    switch -- [winfo class $top] {
        "PdPatch"       { ::ui_interface::pdsend "$top $message" }
        "PdText"        { ::ui_text::menu $top $message }
        "Spaghettis"    { ::ui_console::menu $message   }
    }
}

# Force the dirty bit also.

proc _handleDirty {message} {

    set top [winfo toplevel $::var(windowFocused)]
    
    if {[winfo class $top] eq "PdPatch"} {
    
        ::ui_interface::pdsend "$top $message"
        ::ui_interface::pdsend "$top dirty 1"
    }
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
            set command [format "%s::closed" [getNamespace $top]]
            $command $top
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
