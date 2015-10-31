
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_patch 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_bind

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_patch:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export create
namespace export front

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable minimumSizeX 50
variable minimumSizeY 50

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top width height geometry editable} {

    variable minimumSizeX
    variable minimumSizeY
    
    # Create a toplevel window with a menubar.
    
    toplevel $top   -width $width -height $height -class PdPatch
    $top configure  -menu .menubar

    wm minsize  $top $minimumSizeX $minimumSizeY
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
    
    ::pd_bind::patch $top
	 
    focus $top.c

    # Set various attributes.
    
    set ::patch_isEditing($top)         0
    set ::patch_isEditMode($top)        $editable
    set ::patch_isScrollableX($top.c)   0
    set ::patch_isScrollableY($top.c)   0
}

proc front {top} {
    wm deiconify $top
    raise $top
    focus $top.c
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

#------------------------------------------------------------------------------#
# canvas popup menu

proc ::pd_patch::done_popup {mytoplevel action} {
    ::pd_connect::pdsend "$mytoplevel done-popup $action $::var(windowPopupX) $::var(windowPopupY)"
}

proc ::pd_patch::pdtk_canvas_popup {mytoplevel xcanvas ycanvas hasproperties hasopen} {
    set ::var(windowPopupX) $xcanvas
    set ::var(windowPopupY) $ycanvas
    if {$hasproperties} {
        .popup entryconfigure [_ "Properties"] -state normal
    } else {
        .popup entryconfigure [_ "Properties"] -state disabled
    }
    if {$hasopen} {
        .popup entryconfigure [_ "Open"] -state normal
    } else {
        .popup entryconfigure [_ "Open"] -state disabled
    }
    set tkcanvas $mytoplevel.c
    set scrollregion [$tkcanvas cget -scrollregion]
    # get the canvas location that is currently the top left corner in the window
    set left_xview_pix [expr [lindex [$tkcanvas xview] 0] * [lindex $scrollregion 2]]
    set top_yview_pix [expr [lindex [$tkcanvas yview] 0] * [lindex $scrollregion 3]]
    # take the mouse clicks in canvas coords, add the root of the canvas
    # window, and subtract the area that is obscured by scrolling
    set xpopup [expr int($xcanvas + [winfo rootx $tkcanvas] - $left_xview_pix)]
    set ypopup [expr int($ycanvas + [winfo rooty $tkcanvas] - $top_yview_pix)]
    tk_popup .popup $xpopup $ypopup 0
}


#------------------------------------------------------------------------------#
# procs for when file loading starts/finishes

proc ::pd_patch::started_loading_file {patchname} {

}

# things to run when a patch is finished loading.  This is called when
# the OS sends the "Map" event for this window.
proc ::pd_patch::finished_loading_file {mytoplevel} {

    # set editmode to make sure the menu item is in the right state
    ::pd_patch::pdtk_canvas_editmode $mytoplevel $::patch_isEditMode($mytoplevel)
    # send the virtual events now that everything is loaded
    event generate $mytoplevel <<Loaded>>
}

#------------------------------------------------------------------------------#
# procs for canvas events

# check or uncheck the "edit" menu item
proc ::pd_patch::pdtk_canvas_editmode {mytoplevel state} {
    set ::var(isEditMode) $state
    set ::patch_isEditMode($mytoplevel) $state
    
    if {[winfo class $mytoplevel] eq "PdPatch"} { 
        if {$::var(isEditMode)} { pd_menu::enableCopying } else { pd_menu::disableCopying }
    } else { 
        pd_menu::disableEditing 
    }
}

# message from Pd to update the currently available undo/redo action
proc pdtk_undomenu {mytoplevel undoaction redoaction} {

}

# This proc configures the scrollbars whenever anything relevant has
# been updated.  It should always receive a tkcanvas, which is then
# used to generate the mytoplevel, needed to address the scrollbars.
proc ::pd_patch::pdtk_canvas_getscroll {tkcanvas} {
    set mytoplevel [winfo toplevel $tkcanvas]
    set height [winfo height $tkcanvas]
    set width [winfo width $tkcanvas]
    
    set bbox [$tkcanvas bbox all]
    if {$bbox eq "" || [llength $bbox] != 4} {return}
    set xupperleft [lindex $bbox 0]
    set yupperleft [lindex $bbox 1]
    if {$xupperleft > 0} {set xupperleft 0}
    if {$yupperleft > 0} {set yupperleft 0}
    set xlowerright [lindex $bbox 2]
    set ylowerright [lindex $bbox 3]
    if {$xlowerright < $width} {set xlowerright $width}
    if {$ylowerright < $height} {set ylowerright $height}
    set scrollregion [concat $xupperleft $yupperleft $xlowerright $ylowerright]
    $tkcanvas configure -scrollregion $scrollregion
    # X scrollbar
    if {[lindex [$tkcanvas xview] 0] == 0.0 && [lindex [$tkcanvas xview] 1] == 1.0} {
        set ::patch_isScrollableX($tkcanvas) 0
        pack forget $mytoplevel.xscroll
    } else {
        set ::patch_isScrollableX($tkcanvas) 1
        pack $mytoplevel.xscroll -side bottom -fill x -before $tkcanvas
    }
    # Y scrollbar, it gets touchy at the limit, so say > 0.995
    if {[lindex [$tkcanvas yview] 0] == 0.0 && [lindex [$tkcanvas yview] 1] > 0.995} {
        set ::patch_isScrollableY($tkcanvas) 0
        pack forget $mytoplevel.yscroll
    } else {
        set ::patch_isScrollableY($tkcanvas) 1
        pack $mytoplevel.yscroll -side right -fill y -before $tkcanvas
    }
}

proc ::pd_patch::scroll {tkcanvas axis amount} {
    if {$axis eq "x" && $::patch_isScrollableX($tkcanvas) == 1} {
        $tkcanvas xview scroll [expr {- ($amount)}] units
    }
    if {$axis eq "y" && $::patch_isScrollableY($tkcanvas) == 1} {
        $tkcanvas yview scroll [expr {- ($amount)}] units
    }
}

#------------------------------------------------------------------------------#
# get patch window child/parent relationships

# receive information for setting the info the the title bar of the window
proc ::pd_patch::pdtk_canvas_reflecttitle {mytoplevel \
                                              path name arguments dirty} {
                                              
    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $mytoplevel -modified $dirty
        if {[file exists "$path/$name"]} {
            # for some reason -titlepath can still fail so just catch it 
            if [catch {wm attributes $mytoplevel -titlepath "$path/$name"}] {
                wm title $mytoplevel "$path/$name"
            }
        }
        wm title $mytoplevel "$name$arguments"
    } else {
        if {$dirty} {set dirtychar "*"} else {set dirtychar " "}
        wm title $mytoplevel "$name$dirtychar$arguments - $path" 
    }
}
