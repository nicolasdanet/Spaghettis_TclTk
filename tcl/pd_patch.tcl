
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# A patch view.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_patch 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_patch:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable isEditMode
variable isScrollableX
variable isScrollableY

array set isEditMode    {}
array set isScrollableX {}
array set isScrollableY {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height geometry editable} {

    variable isEditMode
    variable isScrollableX
    variable isScrollableY

    # Create a toplevel window with a menubar.
    
    toplevel $top -width $width -height $height -class PdPatch
    wm group $top .

    wm minsize  $top 50 50
    wm geometry $top [format "=%dx%d%s" $width $height $geometry]

    $top configure  -menu .menubar
    
    # Create a canvas inside that fills all the window.
    
    canvas $top.c   -width $width \
                    -height $height \
                    -highlightthickness 0 \
                    -scrollregion "0 0 $width $height" \
                    -xscrollcommand "$top.xscroll set" \
                    -yscrollcommand "$top.yscroll set"
    
    if {[tk windowingsystem] eq "win32"} { $top.c configure -xscrollincrement 1 -yscrollincrement 1 }
    
    scrollbar $top.xscroll  -orient horizontal  -command "$top.c xview"
    scrollbar $top.yscroll  -orient vertical    -command "$top.c yview"
                            
    pack $top.c -side left -expand 1 -fill both

    # Bind the window to get events.
    
    ::pd_bind::bindPatch $top
	 
    focus $top.c

    # Set various attributes.
    
    set isEditMode($top)        $editable
    set isScrollableX($top.c)   0
    set isScrollableY($top.c)   0
}

proc willClose {top} {

    ::pd_connect::pdsend "$top menuclose 0"
}

proc closed {top} {

    variable isEditMode
    variable isScrollableX
    variable isScrollableY

    unset isEditMode($top)
    unset isScrollableX($top.c)
    unset isScrollableY($top.c)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc bringToFront {top} {

    wm deiconify $top
    raise $top
    focus $top.c
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc setTitle {top path name dirty} {
                                              
    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $top -modified $dirty
        if {[file exists "$path/$name"]} {
            catch {wm attributes $top -titlepath "$path/$name"}
        }
    }

    wm title $top "[file rootname [file tail $name]]"
}

proc getTitle {top} {

    return [wm title $top]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Refresh with no state provided.

proc setEditMode {top {state {}}} {

    variable isEditMode
    
    if {[llength [info level 0]] == 2} { set state $isEditMode($top) }
    
    set isEditMode($top) $state
    set ::var(isEditMode) $state
    
    if {$state} { ::pd_menu::enableCopyingAndEditing } else { ::pd_menu::disableCopyingAndEditing }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc scroll {c axis amount} {
    
    variable isScrollableX
    variable isScrollableY

    if {$axis eq "x" && $isScrollableX($c) == 1} { $c xview scroll [expr {-($amount)}] units }
    if {$axis eq "y" && $isScrollableY($c) == 1} { $c yview scroll [expr {-($amount)}] units }
}

proc updateScrollRegion {c} {

    variable isScrollableX
    variable isScrollableY

    set top [winfo toplevel $c]
    set box [$c bbox all]
    
    if {$box ne ""} {
    
    set x1 [::tcl::mathfunc::min [lindex $box 0] 0]
    set y1 [::tcl::mathfunc::min [lindex $box 1] 0]
    set x2 [::tcl::mathfunc::max [lindex $box 2] [winfo width $c]]
    set y2 [::tcl::mathfunc::max [lindex $box 3] [winfo height $c]]

    $c configure -scrollregion [concat $x1 $y1 $x2 $y2]
    
    if {[lindex [$c xview] 0] == 0.0 && [lindex [$c xview] 1] == 1.0} {
        set isScrollableX($c) 0
        pack forget $top.xscroll
        
    } else {
        set isScrollableX($c) 1
        pack $top.xscroll -side bottom -fill x -before $c
    }
    
    if {[lindex [$c yview] 0] == 0.0 && [lindex [$c yview] 1] == 1.0} {
        set isScrollableY($c) 0
        pack forget $top.yscroll
        
    } else {
        set isScrollableY($c) 1
        pack $top.yscroll -side right -fill y -before $c
    }
    
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
