
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Atom property.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_atom 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_atom:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  atomWidth
variable  atomLow
variable  atomHigh
variable  atomSend
variable  atomReceive
variable  atomName
variable  atomPosition

array set atomWidth     {}
array set atomLow       {}
array set atomHigh      {}
array set atomSend      {}
array set atomReceive   {}
array set atomName      {}
array set atomPosition  {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top width low high send receive name position} {

    ::pd_atom::_create $top $width $low $high $send $receive $name $position
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top width low high send receive name position} {
    
    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomSend
    variable atomReceive
    variable atomName
    variable atomPosition
    
    toplevel $top -class PdDialog
    wm title $top [_ "Atom"]
    wm group $top .
    
    wm resizable $top 0 0
    wm geometry  $top [::rightNextTo $::var(windowFocused)]

    set atomWidth($top)         $width
    set atomLow($top)           $low
    set atomHigh($top)          $high
    set atomSend($top)          [::parseDash $send]
    set atomReceive($top)       [::parseDash $receive]
    set atomName($top)          [::parseDash $name]
    set atomPosition($top)      $position
    
    set atomWidth(${top}.old)   $width
    set atomLow(${top}.old)     $low
    set atomHigh(${top}.old)    $high

    label $top.widthLabel       -text [_ "Width"]
    entry $top.width            -textvariable ::pd_atom::atomWidth($top)
    
    label $top.lowLabel         -text [_ "Low Value"]
    entry $top.low              -textvariable ::pd_atom::atomLow($top)
    
    label $top.highLabel        -text [_ "High Value"]
    entry $top.high             -textvariable ::pd_atom::atomHigh($top)
    
    label $top.nameLabel        -text [_ "Name"]
    entry $top.name             -textvariable ::pd_atom::atomName($top)
    
    radiobutton $top.left       -text [_ "Left"] \
                                -variable ::pd_atom::atomPosition($top) \
                                -takefocus 0 \
                                -value 0 
    radiobutton $top.right      -text [_ "Right"] \
                                -variable ::pd_atom::atomPosition($top) \
                                -takefocus 0 \
                                -value 1
    radiobutton $top.top        -text [_ "Top"] \
                                -variable ::pd_atom::atomPosition($top) \
                                -takefocus 0 \
                                -value 2 
    radiobutton $top.bottom     -text [_ "Bottom"] \
                                -variable ::pd_atom::atomPosition($top) \
                                -takefocus 0 \
                                -value 3 
    
    label $top.sendLabel        -text [_ "Send"]
    entry $top.send             -textvariable ::pd_atom::atomSend($top)
    
    label $top.receiveLabel     -text [_ "Receive"]
    entry $top.receive          -textvariable ::pd_atom::atomReceive($top)

    pack  $top.widthLabel       -side top -anchor w
    pack  $top.width            -side top -anchor w
    pack  $top.lowLabel         -side top -anchor w
    pack  $top.low              -side top -anchor w
    pack  $top.highLabel        -side top -anchor w
    pack  $top.high             -side top -anchor w
    pack  $top.nameLabel        -side top -anchor w
    pack  $top.name             -side top -anchor w
    pack  $top.right            -side top -anchor w
    pack  $top.left             -side top -anchor w
    pack  $top.top              -side top -anchor w
    pack  $top.bottom           -side top -anchor w
    pack  $top.sendLabel        -side top -anchor w
    pack  $top.send             -side top -anchor w
    pack  $top.receiveLabel     -side top -anchor w
    pack  $top.receive          -side top -anchor w
    
    bind  $top.width   <Return> { ::nextEntry %W }
    bind  $top.low     <Return> { ::nextEntry %W }
    bind  $top.high    <Return> { ::nextEntry %W }
    bind  $top.name    <Return> { ::nextEntry %W }
    bind  $top.send    <Return> { ::nextEntry %W }
    bind  $top.receive <Return> { ::nextEntry %W }

    focus $top.width
    
    $top.width selection range 0 end
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_atom::_closed $top"
}

proc _closed {top} {

    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomSend
    variable atomReceive
    variable atomName
    variable atomPosition
    
    ::pd_atom::_apply $top
    
    unset atomWidth($top)
    unset atomLow($top)
    unset atomHigh($top)
    unset atomSend($top)
    unset atomReceive($top)
    unset atomName($top)
    unset atomPosition($top)
    
    unset atomWidth(${top}.old)
    unset atomLow(${top}.old)
    unset atomHigh(${top}.old)

    ::pd_atom::_cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {
    
    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomSend
    variable atomReceive
    variable atomName
    variable atomPosition
        
    ::pd_atom::_forceValues $top
    
    ::pd_connect::pdsend "$top param \
            $atomWidth($top) \
            $atomLow($top) \
            $atomHigh($top) \
            [::sanitized [::withDash $atomSend($top)]] \
            [::sanitized [::withDash $atomReceive($top)]] \
            [::sanitized [::withDash $atomName($top)]] \
            $atomPosition($top)"
    
}

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValues {top} {

    variable atomWidth
    variable atomLow
    variable atomHigh
    
    set atomWidth($top) [::ifInteger $atomWidth($top) $atomWidth(${top}.old)]
    set atomLow($top)   [::ifInteger $atomLow($top) $atomLow(${top}.old)]
    set atomHigh($top)  [::ifInteger $atomHigh($top) $atomHigh(${top}.old)]
    
    set min [::tcl::mathfunc::min $atomLow($top) $atomHigh($top)]
    set max [::tcl::mathfunc::max $atomLow($top) $atomHigh($top)]
    
    set atomLow($top)  $min
    set atomHigh($top) $max
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
