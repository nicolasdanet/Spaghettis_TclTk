
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Array properties.

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
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    set arrayName($top)         [::rauteToDollar $name]
    set arraySize($top)         $size
    set arraySize(${top}.old)   $size
    set arraySave($top)         [expr {$flags & 1}]
    set arrayDraw($top)         [expr {($flags & 6) >> 1}]
    
    label $top.nameLabel        -text [_ "Name"]
    entry $top.name             -textvariable ::pd_array::arrayName($top)
    
    label $top.sizeLabel        -text [_ "Size"]
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
                                
    radiobutton $top.bezier     -text [_ "Bezier"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -takefocus 0 \
                                -value 2 
    
    pack $top.nameLabel         -side top -anchor w                           
    pack $top.name              -side top -anchor w
    pack $top.sizeLabel         -side top -anchor w
    pack $top.size              -side top -anchor w 
    
    pack $top.save              -side top -anchor w
    
    pack $top.points            -side top -anchor w
    pack $top.polygon           -side top -anchor w
    pack $top.bezier            -side top -anchor w
    
    bind $top.name <Return> { ::nextEntry %W }
    bind $top.size <Return> { ::nextEntry %W }
    
    focus $top.name
    
    $top.name selection range 0 end
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_array::closed $top"
}

proc closed {top} {
    
    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    ::pd_array::_apply $top
    
    unset arrayName($top)
    unset arraySize($top)
    unset arraySize(${top}.old)
    unset arrayDraw($top)
    unset arraySave($top)
    
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    ::pd_array::_forceSize $top
    
    ::pd_connect::pdsend "$top arraydialog \
            [::sanitized [::dollarToRaute [::withEmpty $arrayName($top)]]] \
            $arraySize($top) \
            [expr {$arraySave($top) + (2 * $arrayDraw($top))}]"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceSize {top} {

    variable arraySize
    
    set arraySize($top) [::ifInteger $arraySize($top) $arraySize(${top}.old)]
    set arraySize($top) [::tcl::mathfunc::max $arraySize($top) 1]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
