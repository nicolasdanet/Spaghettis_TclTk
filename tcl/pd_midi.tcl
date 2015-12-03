
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# MIDI settings.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_midi 0.1

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
    wm geometry  $top [::rightNextTo .console]

    if {[expr {[llength $midiIn] + [llength $midiOut] == 2}]} {
    
        label $top.none -text [_ "No MIDI device found."]
        pack  $top.none -side top -anchor w
        
    } else {
        foreach e $midiIn  { if {$e ne "none"} { ::pd_midi::_makeIn  $top [incr i] } }
        foreach e $midiOut { if {$e ne "none"} { ::pd_midi::_makeOut $top [incr j] } }
    }
    
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
    wm title $top [_ "ALSA MIDI"]
    wm group $top .
    
    wm resizable $top 0 0
    wm geometry  $top [::rightNextTo .console]
    
    label $top.inLabel  -text [_ "Input Ports"]
    entry $top.in       -textvariable ::pd_midi::midiAlsaIn \
                        -state disabled
    
    label $top.outLabel -text [_ "Output Ports"]
    entry $top.out      -textvariable ::pd_midi::midiAlsaOut \
                        -state disabled
                        
    pack  $top.inLabel  -side top -anchor w
    pack  $top.in       -side top -anchor w
    pack  $top.outLabel -side top -anchor w
    pack  $top.out      -side top -anchor w
    
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
    
    set devicesLabel [format "%s.inDevice%dLabel" $top $k]
    set devices      [format "%s.inDevice%d" $top $k]
    
    label $devicesLabel                 -text [format "%s %d" [_ "Input"] $k]
    menubutton $devices                 -text [lindex $midiIn $midiInDevice($k)]
    
    menu $devices.menu
    $devices configure                  -menu $devices.menu
    
    set i 0
    
    foreach e $midiIn {
        $devices.menu add radiobutton   -label "$e" \
                                        -variable ::pd_midi::midiInDevice($k) \
                                        -value $i \
                                        -command [list $devices configure -text [lindex $midiIn $i]]
        incr i
    }
    
    pack $devicesLabel                  -side top -anchor w
    pack $devices                       -side top -anchor w
}

proc _makeOut {top k} {

    variable midiOut
    variable midiOutDevice
    
    set devicesLabel [format "%s.outDevice%dLabel" $top $k]
    set devices      [format "%s.outDevice%d" $top $k]
    
    label $devicesLabel                 -text [format "%s %d" [_ "Output"] $k]
    menubutton $devices                 -text [lindex $midiOut $midiOutDevice($k)]
    
    menu $devices.menu
    $devices configure                  -menu $devices.menu
    
    set i 0
    
    foreach e $midiOut {
        $devices.menu add radiobutton   -label "$e" \
                                        -variable ::pd_midi::midiOutDevice($k) \
                                        -value $i \
                                        -command [list $devices configure -text [lindex $midiOut $i]]
        incr i
    }
    
    pack $devicesLabel                  -side top -anchor w
    pack $devices                       -side top -anchor w
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

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
