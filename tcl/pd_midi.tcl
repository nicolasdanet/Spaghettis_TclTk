
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# MIDI settings.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_midi 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_midi:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  midiIn
variable  midiOut
variable  midiAlsaIn 
variable  midiAlsaOut
variable  midiInDevice
variable  midiOutDevice

array set midiInDevice  {}
array set midiOutDevice {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top i1 i2 i3 i4 i5 i6 i7 i8 i9 o1 o2 o3 o4 o5 o6 o7 o8 o9 alsa} {
    
    ::pd_menu::disableMidi
    
    if {$alsa} { 
        ::pd_midi::_createAlsa $top $i1 $i2 $i3 $i4 $i5 $i6 $i7 $i8 $i9 $o1 $o2 $o3 $o4 $o5 $o6 $o7 $o8 $o9
    } else {
        ::pd_midi::_create $top $i1 $i2 $i3 $i4 $i5 $i6 $i7 $i8 $i9 $o1 $o2 $o3 $o4 $o5 $o6 $o7 $o8 $o9
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top i1 i2 i3 i4 i5 i6 i7 i8 i9 o1 o2 o3 o4 o5 o6 o7 o8 o9} {

    variable midiIn
    variable midiOut
    variable midiAlsaIn
    variable midiAlsaOut
    variable midiInDevice
    variable midiOutDevice

    array set midiInDevice  [ list 1 $i1 2 $i2 3 $i3 4 $i4 5 $i5 6 $i6 7 $i7 8 $i8 9 $i9 ]
    array set midiOutDevice [ list 1 $o1 2 $o2 3 $o3 4 $o4 5 $o5 6 $o6 7 $o7 8 $o8 9 $o9 ]
    
    set midiAlsaIn  [llength $midiIn]
    set midiAlsaOut [llength $midiOut]

    toplevel $top -class PdDialog
    wm title $top [_ "MIDI"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo .console]

    set noInput  [expr {[llength $midiIn]  == 1}]
    set noOutput [expr {[llength $midiOut] == 1}]
    
    ttk::frame      $top.f          {*}[::styleFrame]
    ttk::labelframe $top.f.inputs   {*}[::styleLabelFrame]  -text [_ "Inputs"]
    ttk::labelframe $top.f.outputs  {*}[::styleLabelFrame]  -text [_ "Outputs"]

    pack $top.f                     {*}[::packMain]
    pack $top.f.inputs              {*}[::packCategory]
    pack $top.f.outputs             {*}[::packCategoryNext]
    
    foreach e $midiIn  { if {$e ne "none" || $noInput}  { ::pd_midi::_makeIn  $top.f.inputs  [incr i] } }
    foreach e $midiOut { if {$e ne "none" || $noOutput} { ::pd_midi::_makeOut $top.f.outputs [incr j] } }
    
    bind $top <Destroy> { ::pd_menu::enableMidi }
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_midi::closed $top"
}

proc _createAlsa {top i1 i2 i3 i4 i5 i6 i7 i8 i9 o1 o2 o3 o4 o5 o6 o7 o8 o9} {

    variable midiIn
    variable midiOut
    variable midiAlsaIn
    variable midiAlsaOut
    variable midiInDevice
    variable midiOutDevice
    
    array set midiInDevice  [ list 1 $i1 2 $i2 3 $i3 4 $i4 5 $i5 6 $i6 7 $i7 8 $i8 9 $i9 ]
    array set midiOutDevice [ list 1 $o1 2 $o2 3 $o3 4 $o4 5 $o5 6 $o6 7 $o7 8 $o8 9 $o9 ]
    
    set midiAlsaIn  [expr {[llength $midiIn] - 1}]
    set midiAlsaOut [expr {[llength $midiOut] - 1}]
    
    toplevel $top -class PdDialog
    wm title $top [_ "MIDI"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo .console]
    
    ttk::frame      $top.f              {*}[::styleFrame]
    ttk::labelframe $top.f.inputs       {*}[::styleLabelFrame]  -text [_ "Inputs"]
    ttk::labelframe $top.f.outputs      {*}[::styleLabelFrame]  -text [_ "Outputs"]
    
    pack $top.f                         {*}[::packMain]
    pack $top.f.inputs                  {*}[::packCategory]
    pack $top.f.outputs                 {*}[::packCategoryNext]
    
    ttk::label $top.f.inputs.inLabel    {*}[::styleLabel] \
                                            -text [_ "Ports"]
    ttk::entry $top.f.inputs.in         {*}[::styleEntry] \
                                            -textvariable ::pd_midi::midiAlsaIn \
                                            -width $::width(small)
    
    ttk::label $top.f.outputs.outLabel  {*}[::styleLabel] \
                                            -text [_ "Ports"]
    ttk::entry $top.f.outputs.out       {*}[::styleEntry] \
                                            -textvariable ::pd_midi::midiAlsaOut \
                                            -width $::width(small)
                        
    pack $top.f.inputs.inLabel          -side left -fill x -expand 1 -padx {0 20}
    pack $top.f.inputs.in               -side left -fill x -expand 1
    pack $top.f.outputs.outLabel        -side left -fill x -expand 1 -padx {0 20}
    pack $top.f.outputs.out             -side left -fill x -expand 1
    
    bind $top <Destroy> { ::pd_menu::enableMidi }
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_midi::closed $top"
}

proc closed {top} {

    ::pd_midi::_apply $top
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _makeIn {top k} {

    variable midiIn
    variable midiInDevice
    
    set devices [format "%s.inDevice%d" $top $k]
    
    ::createMenuByIndex $devices $midiIn ::pd_midi::midiInDevice($k) -width -$::width(large)
    
    pack $devices -side top -fill x -expand 1
}

proc _makeOut {top k} {

    variable midiOut
    variable midiOutDevice
    
    set devices [format "%s.outDevice%d" $top $k]
    
    ::createMenuByIndex $devices $midiOut ::pd_midi::midiOutDevice($k) -width -$::width(large)
    
    pack $devices -side top -fill x -expand 1
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {
    
    variable midiIn
    variable midiOut
    variable midiAlsaIn
    variable midiAlsaOut
    variable midiInDevice
    variable midiOutDevice
    
    _forceValues
    
    ::pd_connect::pdsend "pd midi-dialog \
            $midiInDevice(1) \
            $midiInDevice(2) \
            $midiInDevice(3) \
            $midiInDevice(4) \
            $midiInDevice(5) \
            $midiInDevice(6) \
            $midiInDevice(7) \
            $midiInDevice(8) \
            $midiInDevice(9) \
            $midiOutDevice(1) \
            $midiOutDevice(2) \
            $midiOutDevice(3) \
            $midiOutDevice(4) \
            $midiOutDevice(5) \
            $midiOutDevice(6) \
            $midiOutDevice(7) \
            $midiOutDevice(8) \
            $midiOutDevice(9) \
            $midiAlsaIn \
            $midiAlsaOut"
    
    ::pd_connect::pdsend "pd save-preferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValues {} {

    variable midiAlsaIn
    variable midiAlsaOut
    
    set midiAlsaIn   [::ifInteger $midiAlsaIn 0]
    set midiAlsaIn   [::tcl::mathfunc::max $midiAlsaIn 0]
    set midiAlsaOut  [::ifInteger $midiAlsaOut 0]
    set midiAlsaOut  [::tcl::mathfunc::max $midiAlsaOut 0]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
