
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2016 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

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

variable suffix "(Edit)"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  patchTitle
variable  patchIsEditMode

array set patchTitle         {}
array set patchIsEditMode    {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height coordinates editable} {

    variable patchTitle
    variable patchIsEditMode

    toplevel $top -class PdPatch
    wm group $top .

    wm geometry $top [format "=%dx%d%s" $width $height $coordinates]
    
    if {[tk windowingsystem] ne "aqua"} { $top configure -menu .menubar }
        
    canvas $top.c   -width $width \
                    -height $height \
                    -highlightthickness 0 \
                    -background "#ffffff"
    
    if {[tk windowingsystem] eq "win32"} { $top.c configure -xscrollincrement 1 -yscrollincrement 1 }
    
    pack $top.c -side left -expand 1 -fill both
    
    ::ui_bind::bindPatch $top
	 
    focus $top.c
    
    set patchTitle($top)        ""
    set patchIsEditMode($top)   $editable
}

proc willClose {top} {

    ::ui_interface::pdsend "$top close"
}

proc closed {top} {

    variable patchTitle
    variable patchIsEditMode

    unset patchTitle($top)
    unset patchIsEditMode($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc setTitle {top path name dirty} {

    variable patchTitle
    
    set patchTitle($top) "[file rootname [file tail $name]]"
    
    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $top -modified $dirty
        if {[file exists "$path/$name"]} {
            catch {wm attributes $top -titlepath "$path/$name"}
        }
    }

    wm title $top $patchTitle($top)
    
    ::ui_patch::_reflectEditmode $top
}

proc getTitle {top} {

    variable patchTitle
    
    return $patchTitle($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Refresh if no state provided.

proc setEditMode {top {state {}}} {

    variable patchIsEditMode
    
    if {[llength [info level 0]] == 2} { set state $patchIsEditMode($top) }
    
    set ::var(isEditMode) $state
    set patchIsEditMode($top) $state
    
    if {$state} { ::ui_menu::enableCopyingAndEditing } else { ::ui_menu::disableCopyingAndEditing }
    
    ::ui_patch::_reflectEditmode $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc scroll {c axis amount} {
    
    if {$axis eq "x"} { $c xview scroll [expr {-($amount)}] units }
    if {$axis eq "y"} { $c yview scroll [expr {-($amount)}] units }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _reflectEditmode {top} {

    variable suffix
    variable patchIsEditMode
    
    set title [::ui_patch::getTitle $top]
    
    if {$patchIsEditMode($top)} { wm title $top "$title $suffix" } else { wm title $top "$title" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
