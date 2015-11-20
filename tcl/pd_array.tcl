
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_array 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_array:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  arrayName
variable  arraySize
variable  arrayDraw
variable  arraySave

array set arrayName {}
array set arraySize {}
array set arrayDraw {}
array set arraySave {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top name size flags} {

    ::pd_array::_create $top $name $size $flags
}

proc _closed {top} {
    
    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    ::pd_array::_apply $top
    
    unset arrayName($top)
    unset arraySize($top)
    unset arrayDraw($top)
    unset arraySave($top)
    
    ::pd_array::_cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top name size flags} {

    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    toplevel $top -class PdDialog
    wm title $top [_ "Array"]
    wm group $top .
    
    wm resizable $top 0 0
    
    set arrayName($top) [::dialog_gatom::unescape $name]
    set arraySize($top) $size
    set arraySave($top) [expr {$flags & 1}]
    set arrayDraw($top) [expr {($flags & 6) >> 1}]
    
    entry $top.name             -textvariable ::pd_array::arrayName($top)
    entry $top.size             -textvariable ::pd_array::arraySize($top)

    checkbutton $top.save       -text [_ "Save contents"] \
                                -variable ::pd_array::arraySave($top) \
                                -takefocus 0
    
    radiobutton $top.points     -text [_ "Polygon"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -takefocus 0 \
                                -value 0 
                                
    radiobutton $top.polygon    -text [_ "Points"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -takefocus 0 \
                                -value 1
                                
    radiobutton $top.bezier     -text [_ "Bezier curve"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -takefocus 0 \
                                -value 2 
                                
    pack $top.name              -side top -anchor w
    pack $top.size              -side top -anchor w
    pack $top.save              -side top -anchor w
    pack $top.points            -side top -anchor w
    pack $top.polygon           -side top -anchor w
    pack $top.bezier            -side top -anchor w
    
    bind $top.name <Return> { focus [tk_focusNext %W] }
    bind $top.size <Return> { focus [tk_focusNext %W] }
    
    focus $top.name
    
    $top.size selection range 0 end
    $top.name selection range 0 end
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_array::_closed $top"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    set name  [::dialog_gatom::escape $::pd_array::arrayName($top)]
    set size  $::pd_array::arraySize($top)
    set flags [expr {$::pd_array::arraySave($top) + (2 * $::pd_array::arrayDraw($top))}]
    
    ::pd_connect::pdsend "$top arraydialog $name $size $flags"
}

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
