
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_canvas 0.1

package require pd_bindings

namespace eval ::pd_canvas:: {
}

# One thing that is tricky to understand is the difference between a Tk
# 'canvas' and a 'canvas' in terms of Pd's implementation.  They are similar,
# but not the same thing.  In Pd code, a 'canvas' is basically a patch, while
# the Tk 'canvas' is the backdrop for drawing everything that is in a patch.
# The Tk 'canvas' is contained in a 'toplevel' window. That window has a Tk
# class of 'PatchWindow'.

# TODO figure out weird frameless window when you open a graph


#TODO: http://wiki.tcl.tk/11502
# MS Windows
#wm geometry . returns contentswidthxcontentsheight+decorationTop+decorationLeftEdge.
#and
#winfo rooty . returns contentsTop
#winfo rootx . returns contentsLeftEdge


# this proc is split out on its own to make it easy to override. This makes it
# easy for people to customize these calculations based on their Window
# Manager, desires, etc.
proc pdtk_canvas_place_window {width height geometry} {
    set screenwidth [lindex [wm maxsize .] 0]
    set screenheight [lindex [wm maxsize .] 1]

    # Placement refers to the frame's corner instead of the content ( http://wiki.tcl.tk/11502 ).
    
    set windowFrameX 0
    set windowFrameY 0
    
    if {[tk windowingsystem] eq "x11"} {
        set windowFrameX 3
        set windowFrameY 53
    }
    
    # read back the current geometry +posx+posy into variables
    scan $geometry {%[+]%d%[+]%d} - x - y
    # fit the geometry onto screen
    set x [ expr $x % $screenwidth - $windowFrameX]
    set y [ expr $y % $screenheight - $windowFrameY]
    if {$x < 0} {set x 0}
    if {$y < 0} {set y 0}
    if {$width > $screenwidth} {
        set width $screenwidth
        set x 0
    }
    if {$height > $screenheight} {
        set menubarHeight 0
        if {[tk windowingsystem] eq "aqua"} { set menubarHeight 22 }
        set height [expr $screenheight - $menubarHeight - 30] ;# 30 for window framing
        set y $menubarHeight
    }
    return [list $width $height ${width}x$height+$x+$y]
}


#------------------------------------------------------------------------------#
# canvas new/saveas

proc pdtk_canvas_new {mytoplevel width height geometry editable} {
    set l [pdtk_canvas_place_window $width $height $geometry]
    set width [lindex $l 0]
    set height [lindex $l 1]
    set geometry [lindex $l 2]

    # release the window grab here so that the new window will
    # properly get the Map and FocusIn events when its created

    # set the loaded array for this new window so things can track state
    toplevel $mytoplevel -width $width -height $height -class PatchWindow
    wm group $mytoplevel .
    $mytoplevel configure -menu .menubar

    # we have to wait until $mytoplevel exists before we can generate
    # a <<Loading>> event for it, that's why this is here and not in the
    # started_loading_file proc.  Perhaps this doesn't make sense tho
    event generate $mytoplevel <<Loading>>

    wm geometry $mytoplevel $geometry
    wm minsize $mytoplevel 50 50

    set tkcanvas [tkcanvas_name $mytoplevel]
    canvas $tkcanvas -width $width -height $height \
        -highlightthickness 0 -scrollregion [list 0 0 $width $height] \
        -xscrollcommand "$mytoplevel.xscroll set" \
        -yscrollcommand "$mytoplevel.yscroll set"
    scrollbar $mytoplevel.xscroll -orient horizontal -command "$tkcanvas xview"
    scrollbar $mytoplevel.yscroll -orient vertical -command "$tkcanvas yview"
    pack $tkcanvas -side left -expand 1 -fill both

    # for some crazy reason, win32 mousewheel scrolling is in units of
    # 120, and this forces Tk to interpret 120 to mean 1 scroll unit
    if {[tk windowingsystem] eq "win32"} {
        $tkcanvas configure -xscrollincrement 1 -yscrollincrement 1
    }

    ::pd_bindings::patch_bindings $mytoplevel

    # give focus to the canvas so it gets the events rather than the window 	 
    focus $tkcanvas

    # let the scrollbar logic determine if it should make things scrollable
    set ::patch_isScrollableX($tkcanvas) 0
    set ::patch_isScrollableY($tkcanvas) 0

    # init patch properties arrays
    set ::patch_isEditing($mytoplevel) 0
    set ::patch_childs($mytoplevel) {}

    # this should be at the end so that the window and canvas are all ready
    # before this variable changes.
    set ::patch_isEditmode($mytoplevel) $editable
}

# if the patch canvas window already exists, then make it come to the front
proc pdtk_canvas_raise {mytoplevel} {
    wm deiconify $mytoplevel
    raise $mytoplevel
    set mycanvas $mytoplevel.c
    focus $mycanvas
}

proc pdtk_canvas_saveas {name initialfile initialdir destroyflag} {
    if { ! [file isdirectory $initialdir]} {set initialdir $::env(HOME)}
    set filename [tk_getSaveFile -initialfile $initialfile -initialdir $initialdir \
                      -defaultextension .pd -filetypes $::var(filesTypes)]
    if {$filename eq ""} return; # they clicked cancel

    set extension [file extension $filename]
    set oldfilename $filename
    set filename [regsub -- "$extension$" $filename [string tolower $extension]]
    if { ! [regexp -- "\.(pd|pat|mxt)$" $filename]} {
        # we need the file extention even on Mac OS X
        set filename $filename.pd
    }
    # test again after downcasing and maybe adding a ".pd" on the end
    if {$filename ne $oldfilename && [file exists $filename]} {
        set answer [tk_messageBox -type okcancel -icon question -default cancel\
                        -message [_ "\"$filename\" already exists. Do you want to replace it?"]]
        if {$answer eq "cancel"} return; # they clicked cancel
    }
    set dirname [file dirname $filename]
    set basename [file tail $filename]
    ::pd_connect::pdsend "$name savetofile [enquote_path $basename] [enquote_path $dirname] \
 $destroyflag"
    set ::var(directoryNew) $dirname
    # add to recentfiles
    ::pd_preferences::update_recentfiles $filename
}

##### ask user Save? Discard? Cancel?, and if so, send a message on to Pd ######
proc ::pd_canvas::pdtk_canvas_menuclose {mytoplevel reply_to_pd} {
    raise $mytoplevel
    set filename [wm title $mytoplevel]
    set message [format {Do you want to save the changes you made in "%s"?} $filename]
    set answer [tk_messageBox -message $message -type yesnocancel -default "yes" \
                    -parent $mytoplevel -icon question]
    switch -- $answer {
        yes {::pd_connect::pdsend "$mytoplevel menusave 1"}
        no {::pd_connect::pdsend $reply_to_pd}
        cancel {}
    }
}

#------------------------------------------------------------------------------#
# mouse usage

# TODO put these procs into the pd_canvas namespace
proc pdtk_canvas_motion {tkcanvas x y mods} {
    set mytoplevel [winfo toplevel $tkcanvas]
    ::pd_connect::pdsend "$mytoplevel motion [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $mods"
}

proc pdtk_canvas_mouse {tkcanvas x y b f} {
    set mytoplevel [winfo toplevel $tkcanvas]
    ::pd_connect::pdsend "$mytoplevel mouse [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $b $f"
}

proc pdtk_canvas_mouseup {tkcanvas x y b} {
    set mytoplevel [winfo toplevel $tkcanvas]
    ::pd_connect::pdsend "$mytoplevel mouseup [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $b"
}

proc pdtk_canvas_rightclick {tkcanvas x y b} {
    set mytoplevel [winfo toplevel $tkcanvas]
    ::pd_connect::pdsend "$mytoplevel mouse [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $b 8"
}

# on X11, button 2 pastes from X11 clipboard, so simulate normal paste actions
proc pdtk_canvas_clickpaste {tkcanvas x y b} {
    pdtk_canvas_mouse $tkcanvas $x $y $b 0
    pdtk_canvas_mouseup $tkcanvas $x $y $b
    if { [catch {set pdtk_pastebuffer [selection get]}] } {
        # no selection... do nothing
    } else {
        for {set i 0} {$i < [string length $pdtk_pastebuffer]} {incr i 1} {
            set cha [string index $pdtk_pastebuffer $i]
            scan $cha %c keynum
            ::pd_connect::pdsend "[winfo toplevel $tkcanvas] key 1 $keynum 0"
        }
    }
}

#------------------------------------------------------------------------------#
# canvas popup menu

# since there is one popup that is used for all canvas windows, the menu
# -commands use {} quotes so that $::var(windowFocused) is interpreted when the
# menu item is called, not when the command is mapped to the menu item.  This
# is the same as the menubar in pd_menus.tcl but the opposite of the 'bind'
# commands in pd_bindings.tcl
proc ::pd_canvas::initialize {} {
    if { ! [winfo exists .popup]} {
        # the popup menu for the canvas
        menu .popup -tearoff 0
        .popup add command -label [_ "Properties"] \
            -command {::pd_canvas::done_popup $::var(windowFocused) 0}
        .popup add command -label [_ "Open"]       \
            -command {::pd_canvas::done_popup $::var(windowFocused) 1}
        .popup add command -label [_ "Help"]       \
            -command {::pd_canvas::done_popup $::var(windowFocused) 2}
    }
}

proc ::pd_canvas::done_popup {mytoplevel action} {
    ::pd_connect::pdsend "$mytoplevel done-popup $action $::var(windowPopupX) $::var(windowPopupY)"
}

proc ::pd_canvas::pdtk_canvas_popup {mytoplevel xcanvas ycanvas hasproperties hasopen} {
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
    set tkcanvas [tkcanvas_name $mytoplevel]
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

proc ::pd_canvas::started_loading_file {patchname} {

}

# things to run when a patch is finished loading.  This is called when
# the OS sends the "Map" event for this window.
proc ::pd_canvas::finished_loading_file {mytoplevel} {

    # set editmode to make sure the menu item is in the right state
    ::pd_canvas::pdtk_canvas_editmode $mytoplevel $::patch_isEditmode($mytoplevel)
    # send the virtual events now that everything is loaded
    event generate $mytoplevel <<Loaded>>
}

#------------------------------------------------------------------------------#
# procs for canvas events

# check or uncheck the "edit" menu item
proc ::pd_canvas::pdtk_canvas_editmode {mytoplevel state} {
    set ::var(isEditmode) $state
    set ::patch_isEditmode($mytoplevel) $state
    event generate $mytoplevel <<EditMode>>
}

# message from Pd to update the currently available undo/redo action
proc pdtk_undomenu {mytoplevel undoaction redoaction} {

}

# This proc configures the scrollbars whenever anything relevant has
# been updated.  It should always receive a tkcanvas, which is then
# used to generate the mytoplevel, needed to address the scrollbars.
proc ::pd_canvas::pdtk_canvas_getscroll {tkcanvas} {
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

proc ::pd_canvas::scroll {tkcanvas axis amount} {
    if {$axis eq "x" && $::patch_isScrollableX($tkcanvas) == 1} {
        $tkcanvas xview scroll [expr {- ($amount)}] units
    }
    if {$axis eq "y" && $::patch_isScrollableY($tkcanvas) == 1} {
        $tkcanvas yview scroll [expr {- ($amount)}] units
    }
}

#------------------------------------------------------------------------------#
# get patch window child/parent relationships

# add a child window ID to the list of children, if it isn't already there
proc ::pd_canvas::addchild {mytoplevel child} {
    # if either ::patch_childs($mytoplevel) does not exist, or $child does not
    # exist inside of the ::patch_childs($mytoplevel list
    if { [lsearch -exact [array names ::patch_childs $mytoplevel]] == -1 \
             || [lsearch -exact $::patch_childs($mytoplevel) $child] == -1} {
        set ::patch_childs($mytoplevel) [lappend ::patch_childs($mytoplevel) $child]
    }
}

# receive a list of all my parent windows from 'pd'
proc ::pd_canvas::pdtk_canvas_setparents {mytoplevel args} {
    set ::patch_parents($mytoplevel) $args
    foreach parent $args {
        addchild $parent $mytoplevel
    }
}

# receive information for setting the info the the title bar of the window
proc ::pd_canvas::pdtk_canvas_reflecttitle {mytoplevel \
                                              path name arguments dirty} {
    set ::patch_name($mytoplevel) $name ;# TODO add path to this
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
