
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

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
variable  patchIsScrollableX
variable  patchIsScrollableY

array set patchTitle         {}
array set patchIsEditMode    {}
array set patchIsScrollableX {}
array set patchIsScrollableY {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height coordinates editable} {

    variable patchTitle
    variable patchIsEditMode
    variable patchIsScrollableX
    variable patchIsScrollableY

    toplevel $top -class PdPatch
    wm group $top .

    wm geometry $top [format "=%dx%d%s" $width $height $coordinates]
    
    if {[tk windowingsystem] ne "aqua"} { $top configure -menu .menubar }
    
    ttk::scrollbar $top.xscroll -orient horizontal  -command "$top.c xview"
    ttk::scrollbar $top.yscroll -orient vertical    -command "$top.c yview"
    
    canvas $top.c   -width $width \
                    -height $height \
                    -highlightthickness 0 \
                    -background white \
                    -scrollregion "0 0 $width $height" \
                    -xscrollcommand "$top.xscroll set" \
                    -yscrollcommand "$top.yscroll set"
    
    if {[tk windowingsystem] eq "win32"} { $top.c configure -xscrollincrement 1 -yscrollincrement 1 }
    
    pack $top.c -side left -expand 1 -fill both
    
    ::ui_bind::bindPatch $top
	 
    focus $top.c
    
    set patchTitle($top)            ""
    set patchIsEditMode($top)       $editable
    set patchIsScrollableX($top.c)  0
    set patchIsScrollableY($top.c)  0
}

proc willClose {top} {

    ::ui_interface::pdsend "$top menuclose 0"
}

proc closed {top} {

    variable patchTitle
    variable patchIsEditMode
    variable patchIsScrollableX
    variable patchIsScrollableY

    unset patchTitle($top)
    unset patchIsEditMode($top)
    unset patchIsScrollableX($top.c)
    unset patchIsScrollableY($top.c)
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
    
    variable patchIsScrollableX
    variable patchIsScrollableY

    if {$axis eq "x" && $patchIsScrollableX($c) == 1} { $c xview scroll [expr {-($amount)}] units }
    if {$axis eq "y" && $patchIsScrollableY($c) == 1} { $c yview scroll [expr {-($amount)}] units }
}

proc updateScrollRegion {c} {

    variable patchIsScrollableX
    variable patchIsScrollableY

    set top [winfo toplevel $c]
    set box [$c bbox all]
    
    if {$box ne ""} {
    
    set w [winfo width $c]
    set h [winfo height $c]
    
    # Conditional below aims to filter annoying bad values generated at initialization time.
    
    if {$w > 1 && $h > 1} {
    
        # Bounded box is overestimate by a few pixels.
        
        set x1 [::tcl::mathfunc::min [expr {[lindex $box 0] + 2}] 0]
        set y1 [::tcl::mathfunc::min [expr {[lindex $box 1] + 2}] 0]
        set x2 [::tcl::mathfunc::max [expr {[lindex $box 2] - 2}] $w]
        set y2 [::tcl::mathfunc::max [expr {[lindex $box 3] - 2}] $h]

        $c configure -scrollregion [concat $x1 $y1 $x2 $y2]
        
        if {[lindex [$c xview] 0] == 0.0 && [lindex [$c xview] 1] == 1.0} {
            set patchIsScrollableX($c) 0
            pack forget $top.xscroll
            
        } else {
            set patchIsScrollableX($c) 1
            pack $top.xscroll -side bottom -fill x -before $c
        }
        
        if {[lindex [$c yview] 0] == 0.0 && [lindex [$c yview] 1] == 1.0} {
            set patchIsScrollableY($c) 0
            pack forget $top.yscroll
            
        } else {
            set patchIsScrollableY($c) 1
            pack $top.yscroll -side right -fill y -before $c
        }
    
    }
    
    }
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
