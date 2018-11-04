
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# A patch view.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_patch 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_patch:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  patchTitle
variable  patchIsEditMode
variable  patchIsEditable

array set patchTitle        {}
array set patchIsEditMode   {}
array set patchIsEditable   {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height coordinateX coordinateY isEditMode isEditable} {

    variable patchTitle
    variable patchIsEditMode
    variable patchIsEditable

    toplevel $top -class PdPatch
    wm group $top .

    # On GNU/Linux an offset is required due to the window decorators.
    # TODO: Fetch the exact value (instead of magic number).

    if {[tk windowingsystem] eq "x11"} {
        set offset 60
    } else {
        set offset 0
    }
    
    wm minsize  $top {*}[::styleMinimumSizePatch]
    wm geometry $top [format "=%dx%d+%d+%d" $width $height $coordinateX [expr { $coordinateY - $offset }]]
    
    if {[tk windowingsystem] ne "aqua"} { $top configure -menu .menubar }
    
    if {$isEditMode} { set color $::var(backgroundColorEdit) } else { set color $::var(backgroundColorRun) }
    
    canvas $top.c   -width $width \
                    -height $height \
                    -highlightthickness 0 \
                    -background $color \
                    -insertbackground black \
                    -insertwidth 2
    
    if {[tk windowingsystem] eq "win32"} { $top.c configure -xscrollincrement 1 -yscrollincrement 1 }
    
    pack $top.c -side left -expand 1 -fill both
    
    ::ui_bind::bindPatch $top
    
    focus $top.c
    
    set patchTitle($top)        ""
    set patchIsEditMode($top)   $isEditMode
    set patchIsEditable($top)   $isEditable
}

proc willClose {top} {

    ::ui_interface::pdsend "$top close"
}

proc closed {top} {

    variable patchTitle
    variable patchIsEditMode
    variable patchIsEditable

    unset patchTitle($top)
    unset patchIsEditMode($top)
    unset patchIsEditable($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc askForUpdateMenu {top} {

    ::ui_interface::pdsend "$top _menu"
}

proc setUndoAndRedo {top undo redo} {

    variable patchIsEditMode
    
    if {$top eq $::var(windowFocused)} {
    
        ::ui_menu::setUndo $undo $patchIsEditMode($top)
        ::ui_menu::setRedo $redo $patchIsEditMode($top)
    }
}

proc setEncapsulateAndDeencapsulate {top encapsulate deencapsulate} {

    if {$top eq $::var(windowFocused)} {
    
        ::ui_menu::setEncapsulate   $encapsulate
        ::ui_menu::setDeencapsulate $deencapsulate
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc setTitle {top path name dirty} {

    variable patchTitle
    
    set patchTitle($top) "[file rootname [file tail $name]]"
    
    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $top -modified $dirty
        if {[file exists "$path/$name"]} { wm attributes $top -titlepath "$path/$name" }
    }
    
    ::ui_patch::_reflectEditmode $top
}

proc getTitle {top} {

    variable patchTitle
    
    return $patchTitle($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc isEditable {top} {

    variable patchIsEditable
    
    return $patchIsEditable($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc hasEditMode {top} {

    variable patchIsEditMode
    
    return $patchIsEditMode($top)
}

proc setEditMode {top {state {}}} {

    variable patchIsEditMode
    
    # Refresh if no state provided.
    
    if {[llength [info level 0]] == 2} { set state $patchIsEditMode($top) }
    
    set ::var(isEditMode) $state
    set patchIsEditMode($top) $state
    
    if {$state} { ::ui_menu::enableCopyingAndEditing } else { ::ui_menu::disableCopyingAndEditing }
    
    ::ui_patch::_reflectEditmode $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc scroll {c axis amount} {
    
    set top [winfo toplevel $c]
    
    if {$axis eq "x"} { $c xview scroll [expr {-($amount)}] units }
    if {$axis eq "y"} { $c yview scroll [expr {-($amount)}] units }
    
    ::ui_interface::pdsend "$top _scroll [$c canvasx 0] [$c canvasy 0]"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _reflectEditmode {top} {

    variable patchIsEditMode

    if {$patchIsEditMode($top)} {
        $top.c configure -background $::var(backgroundColorEdit)
    } else {
        $top.c configure -background $::var(backgroundColorRun)
    }
    
    set title [::ui_patch::getTitle $top]

    if {$patchIsEditMode($top)} { wm title $top "$title (Edit)" } else { wm title $top "$title" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
