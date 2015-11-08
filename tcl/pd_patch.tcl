
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_patch 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_patch:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height geometry editable} {

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
    
    set ::patch_isEditing($top)         0
    set ::patch_isEditMode($top)        $editable
    set ::patch_isScrollableX($top.c)   0
    set ::patch_isScrollableY($top.c)   0
}

proc willClose {top} {

    ::pd_connect::pdsend "$top menuclose 0"
}

proc closed {top} {

    unset ::patch_isEditing($top)
    unset ::patch_isEditMode($top)
    unset ::patch_isScrollableX($top.c)
    unset ::patch_isScrollableY($top.c)
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

proc setTitle {top path name arguments dirty} {
                                              
    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $top -modified $dirty
        if {[file exists "$path/$name"]} {
            catch {wm attributes $top -titlepath "$path/$name"}
        }
    }
    
    wm title $top "$name$arguments"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc setEditing {top state} {

    set ::patch_isEditing($top) $state
}

proc setEditMode {top state} {

    set ::var(isEditMode) $state
    set ::patch_isEditMode($top) $state
    
    if {$::var(isEditMode)} { 
        ::pd_menu::enableCopying; ::pd_menu::enableEditing
    } else { 
        ::pd_menu::disableCopying; ::pd_menu::disableEditing
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc restoreEditMode {top} {

    ::pd_patch::setEditMode $top $::patch_isEditMode($top)
}

proc configureCursor {top} {

    if {$::patch_isEditMode($top)} { $top configure -cursor $::var(cursorEditNothing) }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc scroll {c axis amount} {

    if {$axis eq "x" && $::patch_isScrollableX($c) == 1} { $c xview scroll [expr {-($amount)}] units }
    if {$axis eq "y" && $::patch_isScrollableY($c) == 1} { $c yview scroll [expr {-($amount)}] units }
}

proc updateScrollRegion {c} {

    set top [winfo toplevel $c]
    set box [$c bbox all]
    
    if {$box ne ""} {
    
    set x1 [::tcl::mathfunc::min [lindex $box 0] 0]
    set y1 [::tcl::mathfunc::min [lindex $box 1] 0]
    set x2 [::tcl::mathfunc::max [lindex $box 2] [winfo width $c]]
    set y2 [::tcl::mathfunc::max [lindex $box 3] [winfo height $c]]

    $c configure -scrollregion [concat $x1 $y1 $x2 $y2]
    
    if {[lindex [$c xview] 0] == 0.0 && [lindex [$c xview] 1] == 1.0} {
        set ::patch_isScrollableX($c) 0
        pack forget $top.xscroll
        
    } else {
        set ::patch_isScrollableX($c) 1
        pack $top.xscroll -side bottom -fill x -before $c
    }
    
    if {[lindex [$c yview] 0] == 0.0 && [lindex [$c yview] 1] == 1.0} {
        set ::patch_isScrollableY($c) 0
        pack forget $top.yscroll
        
    } else {
        set ::patch_isScrollableY($c) 1
        pack $top.yscroll -side right -fill y -before $c
    }
    
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
