
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2017 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

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
variable  arrayUp
variable  arrayDown
variable  arraySave
variable  arrayDraw
variable  arrayHide

array set arrayName  {}
array set arraySize  {}
array set arrayUp    {}
array set arrayDown  {}
array set arraySave  {}
array set arrayDraw  {}
array set arrayHide  {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top name size up down save style hide} {

    ::ui_array::_create $top $name $size $up $down $save $style $hide
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top name size up down save style hide} {

    variable arrayName
    variable arraySize
    variable arrayUp
    variable arrayDown
    variable arraySave
    variable arrayDraw
    variable arrayHide
        
    toplevel $top -class PdDialog
    wm title $top [_ "Array"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    set arrayName($top)         [::hashToDollar $name]
    set arraySize($top)         $size
    set arrayUp($top)           $up
    set arrayDown($top)         $down
    set arraySave($top)         $save
    set arrayDraw($top)         $style
    set arrayHide($top)         $hide
    
    set arrayName(${top}.old)   [::dollarToHash $name]
    set arraySize(${top}.old)   $size
    set arrayUp(${top}.old)     $up
    set arrayDown(${top}.old)   $down
    
    set values {"Polygons" "Points" "Curves"} 
        
    ttk::frame      $top.f                          {*}[::styleFrame]
    ttk::labelframe $top.f.properties               {*}[::styleLabelFrame]  -text [_ "Properties"]
    ttk::labelframe $top.f.bounds                   {*}[::styleLabelFrame]  -text [_ "Range"]
    
    pack $top.f                                     {*}[::packMain]
    pack $top.f.properties                          {*}[::packCategory]
    pack $top.f.bounds                              {*}[::packCategoryNext]
    
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

    ttk::label $top.f.properties.hideLabel          {*}[::styleLabel] \
                                                        -text [_ "Hide Name"]
    ttk::checkbutton $top.f.properties.hide         {*}[::styleCheckButton] \
                                                        -variable ::ui_array::arrayHide($top) \
                                                        -takefocus 0
                                                        
    ttk::label $top.f.properties.drawLabel          {*}[::styleLabel] \
                                                        -text [_ "Draw With"]
    
    ::createMenuByIndex $top.f.properties.draw      $values ::ui_array::arrayDraw($top) \
                                                        -width [::measure $values]
    
    ttk::label $top.f.bounds.upLabel                {*}[::styleLabel] \
                                                        -text [_ "Value Top"]
    ttk::entry $top.f.bounds.up                     {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_array::arrayUp($top) \
                                                        -width $::width(small)
                                                        
    ttk::label $top.f.bounds.downLabel              {*}[::styleLabel] \
                                                        -text [_ "Value Bottom"]
    ttk::entry $top.f.bounds.down                   {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_array::arrayDown($top) \
                                                        -width $::width(small)
                                                                                                  
    grid $top.f.properties.nameLabel                -row 0 -column 0 -sticky ew
    grid $top.f.properties.name                     -row 0 -column 1 -sticky ew
    grid $top.f.properties.sizeLabel                -row 1 -column 0 -sticky ew
    grid $top.f.properties.size                     -row 1 -column 1 -sticky ew
    grid $top.f.properties.saveLabel                -row 2 -column 0 -sticky ew
    grid $top.f.properties.save                     -row 2 -column 1 -sticky ew
	grid $top.f.properties.hideLabel                -row 3 -column 0 -sticky ew
    grid $top.f.properties.hide                     -row 3 -column 1 -sticky ew
    grid $top.f.properties.drawLabel                -row 4 -column 0 -sticky ew
    grid $top.f.properties.draw                     -row 4 -column 1 -sticky ew
    
    grid $top.f.bounds.upLabel                      -row 0 -column 0 -sticky ew
    grid $top.f.bounds.up                           -row 0 -column 1 -sticky ew
    grid $top.f.bounds.downLabel                    -row 1 -column 0 -sticky ew
    grid $top.f.bounds.down                         -row 1 -column 1 -sticky ew
    
    grid columnconfigure $top.f.properties          0 -weight 1
    grid columnconfigure $top.f.bounds              0 -weight 1
    
    bind $top.f.properties.name <Return>            { ::nextEntry %W }
    bind $top.f.properties.size <Return>            { ::nextEntry %W }
    
    bind $top.f.bounds.up       <Return>            { ::nextEntry %W }
    bind $top.f.bounds.down     <Return>            { ::nextEntry %W }
    
    focus $top.f.properties.name
    
    after idle "$top.f.properties.name selection range 0 end"
    
    wm protocol $top WM_DELETE_WINDOW   "::ui_array::closed $top"
}

proc closed {top} {
    
    variable arrayName
    variable arraySize
    variable arrayUp
    variable arrayDown
    variable arraySave
    variable arrayDraw
    variable arrayHide
        
    ::ui_array::_apply $top
    
    unset arrayName($top)
    unset arraySize($top)
    unset arrayUp($top)
    unset arrayDown($top)
    unset arraySave($top)
    unset arrayDraw($top)
    unset arrayHide($top)
    
    unset arrayName(${top}.old)
    unset arraySize(${top}.old)
    unset arrayUp(${top}.old)
    unset arrayDown(${top}.old)
    
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable arrayName
    variable arraySize
    variable arrayUp
    variable arrayDown
    variable arraySave
    variable arrayDraw
    variable arrayHide
        
    ::ui_array::_forceSize $top
    ::ui_array::_forceBounds $top
    
    ::ui_interface::pdsend "$top _arraydialog \
            [::sanitized [::dollarToHash [::withNil $arrayName($top)]]] \
            $arraySize($top) \
            $arrayUp($top) \
            $arrayDown($top) \
            $arraySave($top) \
            $arrayDraw($top) \
            $arrayHide($top)"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceSize {top} {

    variable arrayName
    variable arraySize

    set arrayName($top) [::ifNotNumber $arrayName($top) $arrayName(${top}.old)]
    set arraySize($top) [::ifInteger $arraySize($top) $arraySize(${top}.old)]
    set arraySize($top) [::tcl::mathfunc::max $arraySize($top) 1]
}

proc _forceBounds {top} {

    variable arrayUp
    variable arrayDown

    set arrayUp($top)   [::ifNumber $arrayUp($top)   $arrayUp(${top}.old)]
    set arrayDown($top) [::ifNumber $arrayDown($top) $arrayDown(${top}.old)]
    
    if {$arrayUp($top) == $arrayDown($top)} {
        set arrayUp($top) $arrayUp(${top}.old); set arrayDown($top) $arrayDown(${top}.old)
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
