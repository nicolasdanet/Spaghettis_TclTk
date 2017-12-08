
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2017 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Atom properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_atom 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_atom:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  atomWidth
variable  atomLow
variable  atomHigh
variable  atomType
variable  atomValue
variable  atomSend
variable  atomReceive

array set atomWidth     {}
array set atomLow       {}
array set atomHigh      {}
array set atomType      {}
array set atomValue     {}
array set atomSend      {}
array set atomReceive   {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top width low high type value send receive} {

    ::ui_atom::_create $top $width $low $high $type $value $send $receive
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top width low high type value send receive} {
    
    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomType
    variable atomValue
    variable atomSend
    variable atomReceive
    
    toplevel $top -class PdDialog
    wm title $top [_ "Atom"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]

    set atomWidth($top)         $width
    set atomLow($top)           $low
    set atomHigh($top)          $high
    set atomType($top)          $type
    set atomValue($top)         $value
    set atomSend($top)          [::hashToDollar [::parseNil $send]]
    set atomReceive($top)       [::hashToDollar [::parseNil $receive]]
    
    set atomWidth(${top}.old)   $width
    set atomLow(${top}.old)     $low
    set atomHigh(${top}.old)    $high
    set atomValue(${top}.old)   $value

    ttk::frame      $top.f                          {*}[::styleFrame]
    ttk::labelframe $top.f.properties               {*}[::styleLabelFrame]  -text [_ "Properties"]
    ttk::labelframe $top.f.binding                  {*}[::styleLabelFrame]  -text [_ "Binding"]
    
    pack $top.f                                     {*}[::packMain]
    pack $top.f.properties                          {*}[::packCategory]
    pack $top.f.binding                             {*}[::packCategoryNext]
    
    ttk::label $top.f.properties.widthLabel         {*}[::styleLabel] \
                                                        -text [_ "Width"]
    ttk::entry $top.f.properties.width              {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_atom::atomWidth($top) \
                                                        -width $::width(small)
    
    ttk::label $top.f.properties.valueLabel         {*}[::styleLabel] \
                                                        -text [_ "Value"]
    ttk::entry $top.f.properties.value              {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_atom::atomValue($top) \
                                                        -width $::width(small)
                                                        
    ttk::label $top.f.properties.lowLabel           {*}[::styleLabel] \
                                                        -text [_ "Value Low"]
    ttk::entry $top.f.properties.low                {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_atom::atomLow($top) \
                                                        -width $::width(small)
    
    ttk::label $top.f.properties.highLabel          {*}[::styleLabel] \
                                                        -text [_ "Value High"]
    ttk::entry $top.f.properties.high               {*}[::styleEntryNumber] \
                                                        -textvariable ::ui_atom::atomHigh($top) \
                                                        -width $::width(small)
                                                        
    ttk::label $top.f.binding.sendLabel             {*}[::styleLabel] \
                                                        -text [_ "Send"]
    ttk::entry $top.f.binding.send                  {*}[::styleEntry] \
                                                        -textvariable ::ui_atom::atomSend($top) \
                                                        -width $::width(large)
    
    ttk::label $top.f.binding.receiveLabel          {*}[::styleLabel] \
                                                        -text [_ "Receive"]
    ttk::entry $top.f.binding.receive               {*}[::styleEntry] \
                                                        -textvariable ::ui_atom::atomReceive($top) \
                                                        -width $::width(large)

    grid $top.f.properties.widthLabel               -row 0 -column 0 -sticky ew
    grid $top.f.properties.width                    -row 0 -column 1 -sticky ew
    grid $top.f.properties.valueLabel               -row 1 -column 0 -sticky ew
    grid $top.f.properties.value                    -row 1 -column 1 -sticky ew
    
    if {$type eq "floatatom"} {

    grid $top.f.properties.lowLabel                 -row 2 -column 0 -sticky ew
    grid $top.f.properties.low                      -row 2 -column 1 -sticky ew
    grid $top.f.properties.highLabel                -row 3 -column 0 -sticky ew
    grid $top.f.properties.high                     -row 3 -column 1 -sticky ew
    
    }
    
    grid $top.f.binding.sendLabel                   -row 0 -column 0 -sticky ew
    grid $top.f.binding.send                        -row 0 -column 1 -sticky ew
    grid $top.f.binding.receiveLabel                -row 1 -column 0 -sticky ew
    grid $top.f.binding.receive                     -row 1 -column 1 -sticky ew
    
    grid columnconfigure $top.f.properties  0 -weight 1
    grid columnconfigure $top.f.binding     0 -weight 1
    
    bind  $top.f.properties.width   <Return> { ::nextEntry %W }
    bind  $top.f.properties.value   <Return> { ::nextEntry %W }
    bind  $top.f.properties.low     <Return> { ::nextEntry %W }
    bind  $top.f.properties.high    <Return> { ::nextEntry %W }
    bind  $top.f.binding.send       <Return> { ::nextEntry %W }
    bind  $top.f.binding.receive    <Return> { ::nextEntry %W }

    focus $top.f.properties.width
    
    after idle "$top.f.properties.width selection range 0 end"
    
    wm protocol $top WM_DELETE_WINDOW   "::ui_atom::closed $top"
}

proc closed {top} {

    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomType
    variable atomValue
    variable atomSend
    variable atomReceive
    
    ::ui_atom::_apply $top
    
    unset atomWidth($top)
    unset atomLow($top)
    unset atomHigh($top)
    unset atomType($top)
    unset atomValue($top)
    unset atomSend($top)
    unset atomReceive($top)
    
    unset atomWidth(${top}.old)
    unset atomLow(${top}.old)
    unset atomHigh(${top}.old)
    unset atomValue(${top}.old)

    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {
    
    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomType
    variable atomValue
    variable atomSend
    variable atomReceive
    
    ::ui_atom::_forceValues $top
    
    ::ui_interface::pdsend "$top _gatomdialog \
            $atomWidth($top) \
            $atomLow($top) \
            $atomHigh($top) \
            $atomValue($top) \
            [::sanitized [::dollarToHash [::withNil $atomSend($top)]]] \
            [::sanitized [::dollarToHash [::withNil $atomReceive($top)]]]"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValues {top} {

    variable atomWidth
    variable atomLow
    variable atomHigh
    variable atomType
    variable atomValue
    
    if {$atomType($top) eq "floatatom"} {
     
    set atomValue($top) [::ifNumber     $atomValue($top) $atomValue(${top}.old)]
    
    }
    
    if {$atomType($top) eq "symbolatom"} {
     
    set atomValue($top) [::ifNotNumber  $atomValue($top) $atomValue(${top}.old)]
    
    }
    
    set atomWidth($top) [::ifInteger $atomWidth($top) $atomWidth(${top}.old)]
    set atomLow($top)   [::ifNumber  $atomLow($top)   $atomLow(${top}.old)]
    set atomHigh($top)  [::ifNumber  $atomHigh($top)  $atomHigh(${top}.old)]
    
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
