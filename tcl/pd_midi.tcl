
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

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

proc show {top i1 i2 i3 i4 i5 i6 i7 i8 i9 o1 o2 o3 o4 o5 o6 o7 o8 o9} {
    
    ::pd_menu::disableMidi
    
    ::pd_midi::_create $top $i1 $i2 $i3 $i4 $i5 $i6 $i7 $i8 $i9 $o1 $o2 $o3 $o4 $o5 $o6 $o7 $o8 $o9
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

    set empty [expr {[llength $midiIn] + [llength $midiOut] == 2}]
    
    if {$empty} {
    
        label $top.none     -text [_ "None Detected."]
        pack  $top.none     -side top -anchor w
        
    } else {
        foreach e $midiIn  { if {$e ne "none"} { ::pd_midi::_makeIn  $top [incr i] } }
        foreach e $midiOut { if {$e ne "none"} { ::pd_midi::_makeOut $top [incr j] } }
    }
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_midi::_closed $top"
}

proc _closed {top} {

    ::pd_midi::_apply  $top
    ::pd_midi::_cancel $top
    
    ::pd_menu::enableMidi
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _makeIn {top k} {

    variable midiIn
    variable midiInDevice
    
    set title   [format "%s.inDevice%dLabel" $top $k]
    set devices [format "%s.inDevice%d" $top $k]
    
    label $title                        -text [format "%s %d" [_ "Input"] $k]
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
    
    pack $title                         -side top -anchor w
    pack $devices                       -side top -anchor w
}

proc _makeOut {top k} {

    variable midiOut
    variable midiOutDevice
    
    set title   [format "%s.outDevice%dLabel" $top $k]
    set devices [format "%s.outDevice%d" $top $k]
    
    label $title                        -text [format "%s %d" [_ "Output"] $k]
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
    
    pack $title                         -side top -anchor w
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

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pdtk_alsa_midi_dialog {id in1 in2 in3 in4 out1 out2 out3 out4 alsa} {

    if {0} {
    
    variable midiIn
    variable midiOut
    variable midiAlsaIn
    variable midiAlsaOut
    variable midiInDevice
    variable midiOutDevice
    
    set midiInDevice1 $in1
    set midiInDevice2 $in2
    set midiInDevice3 $in3
    set midiInDevice4 $in4
    set midiInDevice5 0
    set midiInDevice6 0
    set midiInDevice7 0
    set midiInDevice8 0
    set midiInDevice9 0
    set midiOutDevice1 $out1
    set midiOutDevice2 $out2
    set midiOutDevice3 $out3
    set midiOutDevice4 $out4
    set midiOutDevice5 0
    set midiOutDevice6 0
    set midiOutDevice7 0
    set midiOutDevice8 0
    set midiOutDevice9 0
    set midiAlsaIn [expr [llength $midiIn] - 1]
    set midiAlsaOut [expr [llength $midiOut] - 1]
    
    toplevel $id
    wm title $id [_ "ALSA MIDI Settings"]

    frame $id.buttonframe
    pack $id.buttonframe -side bottom -fill x -pady 2m
    button $id.buttonframe.cancel -text [_ "Cancel"]\
        -command "::pd_midi::cancel $id"
    button $id.buttonframe.apply -text [_ "Apply"]\
        -command "::pd_midi::apply $id"
    button $id.buttonframe.ok -text [_ "OK"]\
        -command "::pd_midi::ok $id"
    pack $id.buttonframe.cancel -side left -expand 1
    pack $id.buttonframe.apply -side left -expand 1
    pack $id.buttonframe.ok -side left -expand 1

    frame $id.in1f
    pack $id.in1f -side top

    if {$alsa} {
        label $id.in1f.l1 -text [_ "In Ports:"]
        entry $id.in1f.x1 -textvariable midiAlsaIn -width 4
        pack $id.in1f.l1 $id.in1f.x1 -side left
        label $id.in1f.l2 -text [_ "Out Ports:"]
        entry $id.in1f.x2 -textvariable midiAlsaOut -width 4
        pack $id.in1f.l2 $id.in1f.x2 -side left
    }
    
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
