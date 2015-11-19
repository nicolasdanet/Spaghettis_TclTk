
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

variable drawMode
variable saveContents

array set drawMode     {}
array set saveContents {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top name size flags} {

    if {[winfo exists $top]} {
        wm deiconify $top
        raise $top
    } else {
        ::pd_array::_create $top $name $size $flags
    }
}

proc _create {top name size flags} {

    variable drawMode
    variable saveContents
    
    toplevel $top -class PdDialog
    wm title $top [_ "Array Properties"]
    wm group $top .
    
    wm resizable $top 0 0
    
    entry $top.name
    $top.name insert 0 [::dialog_gatom::unescape $name]
    
    entry $top.size
    $top.size insert 0 $size
    
    checkbutton $top.saveme     -text [_ "Save contents"] \
                                -variable ::pd_array::saveContents($top)
    
    radiobutton $top.points     -text [_ "Polygon"] \
                                -variable ::pd_array::drawMode($top) \
                                -value 0 
                                
    radiobutton $top.polygon    -text [_ "Points"] \
                                -variable ::pd_array::drawMode($top) \
                                -value 1
                                
    radiobutton $top.bezier     -text [_ "Bezier curve"] \
                                -variable ::pd_array::drawMode($top) \
                                -value 2 
    
    button $top.cancel          -text [_ "Cancel"] \
                                -command "::pd_array::cancel $top"
                                
    button $top.ok              -text [_ "OK"] \
                                -command "::pd_array::ok $top"
    
    pack $top.name      -side top -anchor w
    pack $top.size      -side top -anchor w
    pack $top.saveme    -side top -anchor w
    pack $top.points    -side top -anchor w
    pack $top.polygon   -side top -anchor w
    pack $top.bezier    -side top -anchor w
    pack $top.cancel    -side top -anchor w
    pack $top.ok        -side top -anchor w
    
    set saveContents($top)  [expr {$flags & 1}]
    set drawMode($top)      [expr {($flags & 6) >> 1}]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc apply {top} {
    
    variable drawMode
    variable saveContents
    
    ::pd_connect::pdsend "$top arraydialog \
            [::dialog_gatom::escape [$top.name get]] \
            [$top.size get] \
            [expr $saveContents($top) + (2 * $drawMode($top))]"
}

proc cancel {top} {
    ::pd_connect::pdsend "$top cancel"
}

proc ok {top} {
    ::pd_array::apply $top
    ::pd_array::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
