
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_patch 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_bind
package require pd_connect
package require pd_menu

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_patch:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export create
namespace export willClose
namespace export setTitle
namespace export bringToFront
namespace export editMode
namespace export scrollRegion

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height geometry editable} {

    # Create a toplevel window with a menubar.
    
    toplevel $top   -width $width -height $height -class PdPatch
    $top configure  -menu .menubar

    wm minsize  $top 50 50
    wm geometry $top [format "%dx%d%s" $width $height $geometry]
    wm group    $top .

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

proc bringToFront {top} {

    wm deiconify $top
    raise $top
    focus $top.c
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc editMode {top state} {

    set ::var(isEditMode) $state
    set ::patch_isEditMode($top) $state
    
    if {$::var(isEditMode)} { pd_menu::enableCopying } else { pd_menu::disableCopying }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc scrollRegion {c} {

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
