
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

variable drawAs
variable saveMe

array set drawAs {}
array set saveMe {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top name size flags} {

    ::pd_array::_create $top $name $size $flags
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top name size flags} {

    variable drawAs
    variable saveMe
    
    toplevel $top -class PdDialog
    wm title $top [_ "Array Properties"]
    wm group $top .
    
    wm resizable $top 0 0
    
    entry $top.name
    $top.name insert 0 [::dialog_gatom::unescape $name]
    
    entry $top.size
    $top.size insert 0 $size
    
    checkbutton $top.saveme     -text [_ "Save contents"] \
                                -variable ::pd_array::saveMe($top)
    
    radiobutton $top.points     -text [_ "Polygon"] \
                                -variable ::pd_array::drawAs($top) \
                                -value 0 
                                
    radiobutton $top.polygon    -text [_ "Points"] \
                                -variable ::pd_array::drawAs($top) \
                                -value 1
                                
    radiobutton $top.bezier     -text [_ "Bezier curve"] \
                                -variable ::pd_array::drawAs($top) \
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
    
    set saveMe($top) [expr {$flags & 1}]
    set drawAs($top) [expr {($flags & 6) >> 1}]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

proc _apply {top} {

    variable drawAs
    variable saveMe
    
    ::pd_connect::pdsend "$top arraydialog \
            [::dialog_gatom::escape [$top.name get]] \
            [$top.size get] \
            [expr $saveMe($top) + (2 * $drawAs($top))]"
            
    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
