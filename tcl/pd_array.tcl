
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

variable arrayName
variable arraySize
variable arrayDraw
variable arraySave

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
    wm title $top [_ "Array Properties"]
    wm group $top .
    
    wm resizable $top 0 0
    
    set arrayName($top) $name
    set arraySize($top) $size
    set arraySave($top) [expr {$flags & 1}]
    set arrayDraw($top) [expr {($flags & 6) >> 1}]
    
    entry $top.name
    $top.name insert 0 [::dialog_gatom::unescape $name]
    
    entry $top.size
    $top.size insert 0 $size
    
    checkbutton $top.saveme     -text [_ "Save contents"] \
                                -variable ::pd_array::arraySave($top)
    
    radiobutton $top.points     -text [_ "Polygon"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -value 0 
                                
    radiobutton $top.polygon    -text [_ "Points"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -value 1
                                
    radiobutton $top.bezier     -text [_ "Bezier curve"] \
                                -variable ::pd_array::arrayDraw($top) \
                                -value 2 
    
    button $top.cancel          -text [_ "Cancel"] \
                                -command "::pd_array::_cancel $top"
                                
    button $top.ok              -text [_ "Apply"] \
                                -command "::pd_array::_apply $top"
    
    pack $top.name              -side top -anchor w
    pack $top.size              -side top -anchor w
    pack $top.saveme            -side top -anchor w
    pack $top.points            -side top -anchor w
    pack $top.polygon           -side top -anchor w
    pack $top.bezier            -side top -anchor w
    pack $top.cancel            -side top -anchor w
    pack $top.ok                -side top -anchor w
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

proc _apply {top} {

    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    set name  [::dialog_gatom::escape [$top.name get]]
    set size  [$top.size get]
    set flags [expr {$::pd_array::arraySave($top) + (2 * $::pd_array::arrayDraw($top))}]
    
    ::pd_connect::pdsend "$top arraydialog $name $size $flags"
    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
