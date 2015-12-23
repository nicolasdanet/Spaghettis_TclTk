
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Array properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_array 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_array:: {

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

    ::ui_array::_create $top $name $size $flags
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
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    set arrayName($top)         [::rauteToDollar $name]
    set arraySize($top)         $size
    set arraySize(${top}.old)   $size
    set arraySave($top)         [expr {$flags & 1}]
    set arrayDraw($top)         [expr {($flags & 6) >> 1}]
    
    set values {"Polygons" "Points" "Curves"} 
        
    ttk::frame      $top.f                          {*}[::styleFrame]
    ttk::labelframe $top.f.properties               {*}[::styleLabelFrame]  -text [_ "Properties"]

    pack $top.f                                     {*}[::packMain]
    pack $top.f.properties                          {*}[::packCategory]
    
    ttk::label $top.f.properties.nameLabel          {*}[::styleLabel] \
                                                        -text [_ "Name"]
    ttk::entry $top.f.properties.name               {*}[::styleEntry] \
                                                        -textvariable ::ui_array::arrayName($top) \
                                                        -width $::width(medium)

    ttk::label $top.f.properties.sizeLabel          {*}[::styleLabel] \
                                                        -text [_ "Size"]
    ttk::entry $top.f.properties.size               {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_array::arraySize($top) \
                                                        -width $::width(small)

    ttk::label $top.f.properties.saveLabel          {*}[::styleLabel] \
                                                        -text [_ "Save Contents"]
    ttk::checkbutton $top.f.properties.save         {*}[::styleCheckButton] \
                                                        -variable ::ui_array::arraySave($top) \
                                                        -takefocus 0

    ttk::label $top.f.properties.drawLabel          {*}[::styleLabel] \
                                                        -text [_ "Draw With"]
    
    ::createMenuByIndex $top.f.properties.draw      $values ::ui_array::arrayDraw($top) \
                                                        -width [::measure $values]
    
    grid $top.f.properties.nameLabel                -row 0 -column 0 -sticky ew
    grid $top.f.properties.name                     -row 0 -column 1 -sticky ew
    grid $top.f.properties.sizeLabel                -row 1 -column 0 -sticky ew
    grid $top.f.properties.size                     -row 1 -column 1 -sticky ew
    grid $top.f.properties.saveLabel                -row 2 -column 0 -sticky ew
    grid $top.f.properties.save                     -row 2 -column 1 -sticky ew
    grid $top.f.properties.drawLabel                -row 3 -column 0 -sticky ew
    grid $top.f.properties.draw                     -row 3 -column 1 -sticky ew
    
    grid columnconfigure $top.f.properties 0 -weight 1
    
    bind $top.f.properties.name <Return> { ::nextEntry %W }
    bind $top.f.properties.size <Return> { ::nextEntry %W }
    
    focus $top.f.properties.name
    
    after idle "$top.f.properties.name selection range 0 end"
    
    wm protocol $top WM_DELETE_WINDOW   "::ui_array::closed $top"
}

proc closed {top} {
    
    variable arrayName
    variable arraySize
    variable arrayDraw
    variable arraySave
    
    ::ui_array::_apply $top
    
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
    
    ::ui_array::_forceSize $top
    
    ::ui_connect::pdsend "$top arraydialog \
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
