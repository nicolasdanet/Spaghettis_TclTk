
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

variable midiIn
variable midiOut

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {id \
      indev1 indev2 indev3 indev4 indev5 indev6 indev7 indev8 indev9 \
      outdev1 outdev2 outdev3 outdev4 outdev5 outdev6 outdev7 outdev8 outdev9} {
    
    variable midiIn
    variable midiOut
    
    global midi_indev1 midi_indev2 midi_indev3 midi_indev4 midi_indev5 \
         midi_indev6 midi_indev7 midi_indev8 midi_indev9
    global midi_outdev1 midi_outdev2 midi_outdev3 midi_outdev4 midi_outdev5 \
         midi_outdev6 midi_outdev7 midi_outdev8 midi_outdev9
    global midi_alsain midi_alsaout

    set midi_indev1 $indev1
    set midi_indev2 $indev2
    set midi_indev3 $indev3
    set midi_indev4 $indev4
    set midi_indev5 $indev5
    set midi_indev6 $indev6
    set midi_indev7 $indev7
    set midi_indev8 $indev8
    set midi_indev9 $indev9
    set midi_outdev1 $outdev1
    set midi_outdev2 $outdev2
    set midi_outdev3 $outdev3
    set midi_outdev4 $outdev4
    set midi_outdev5 $outdev5
    set midi_outdev6 $outdev6
    set midi_outdev7 $outdev7
    set midi_outdev8 $outdev8
    set midi_outdev9 $outdev9
    set midi_alsain [llength $midiIn]
    set midi_alsaout [llength $midiOut]

    toplevel $id -class PdDialog
    wm title $id [_ "MIDI Settings"]
    wm group $id .
    wm resizable $id 0 0
    $id configure -padx 10 -pady 5
    # not all Tcl/Tk versions or platforms support -topmost, so catch the error
    catch {wm attributes $id -topmost 1}

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
    
        # input device 1
    frame $id.in1f
    pack $id.in1f -side top

    label $id.in1f.l1 -text [_ "Input device 1:"]
    button $id.in1f.x1 -text [lindex $midiIn $midi_indev1] \
        -command [list ::pd_midi::midi_popup $id $id.in1f.x1 midi_indev1 $midiIn]
    pack $id.in1f.l1 $id.in1f.x1 -side left

        # input device 2
    if {[llength $midiIn] > 2} {
        frame $id.in2f
        pack $id.in2f -side top

        label $id.in2f.l1 -text [_ "Input device 2:"]
        button $id.in2f.x1 -text [lindex $midiIn $midi_indev2] \
            -command [list ::pd_midi::midi_popup $id $id.in2f.x1 midi_indev2 \
                $midiIn]
        pack $id.in2f.l1 $id.in2f.x1 -side left
    }

        # input device 3
    if {[llength $midiIn] > 3} {
        frame $id.in3f
        pack $id.in3f -side top

        label $id.in3f.l1 -text [_ "Input device 3:"]
        button $id.in3f.x1 -text [lindex $midiIn $midi_indev3] \
            -command [list ::pd_midi::midi_popup $id $id.in3f.x1 midi_indev3 \
                $midiIn]
        pack $id.in3f.l1 $id.in3f.x1 -side left
    }

        # input device 4
    if {[llength $midiIn] > 4} {
        frame $id.in4f
        pack $id.in4f -side top

        label $id.in4f.l1 -text [_ "Input device 4:"]
        button $id.in4f.x1 -text [lindex $midiIn $midi_indev4] \
            -command [list ::pd_midi::midi_popup $id $id.in4f.x1 midi_indev4 \
                $midiIn]
        pack $id.in4f.l1 $id.in4f.x1 -side left
    }

        # input device 5
    if {[llength $midiIn] > 5} {
        frame $id.in5f
        pack $id.in5f -side top

        label $id.in5f.l1 -text [_ "Input device 5:"]
        button $id.in5f.x1 -text [lindex $midiIn $midi_indev5] \
            -command [list ::pd_midi::midi_popup $id $id.in5f.x1 midi_indev5 \
                $midiIn]
        pack $id.in5f.l1 $id.in5f.x1 -side left
    }

        # input device 6
    if {[llength $midiIn] > 6} {
        frame $id.in6f
        pack $id.in6f -side top

        label $id.in6f.l1 -text [_ "Input device 6:"]
        button $id.in6f.x1 -text [lindex $midiIn $midi_indev6] \
            -command [list ::pd_midi::midi_popup $id $id.in6f.x1 midi_indev6 \
                $midiIn]
        pack $id.in6f.l1 $id.in6f.x1 -side left
    }

        # input device 7
    if {[llength $midiIn] > 7} {
        frame $id.in7f
        pack $id.in7f -side top

        label $id.in7f.l1 -text [_ "Input device 7:"]
        button $id.in7f.x1 -text [lindex $midiIn $midi_indev7] \
            -command [list ::pd_midi::midi_popup $id $id.in7f.x1 midi_indev7 \
                $midiIn]
        pack $id.in7f.l1 $id.in7f.x1 -side left
    }

        # input device 8
    if {[llength $midiIn] > 8} {
        frame $id.in8f
        pack $id.in8f -side top

        label $id.in8f.l1 -text [_ "Input device 8:"]
        button $id.in8f.x1 -text [lindex $midiIn $midi_indev8] \
            -command [list ::pd_midi::midi_popup $id $id.in8f.x1 midi_indev8 \
                $midiIn]
        pack $id.in8f.l1 $id.in8f.x1 -side left
    }

        # input device 9
    if {[llength $midiIn] > 9} {
        frame $id.in9f
        pack $id.in9f -side top

        label $id.in9f.l1 -text [_ "Input device 9:"]
        button $id.in9f.x1 -text [lindex $midiIn $midi_indev9] \
            -command [list ::pd_midi::midi_popup $id $id.in9f.x1 midi_indev9 \
                $midiIn]
        pack $id.in9f.l1 $id.in9f.x1 -side left
    }

        # output device 1

    frame $id.out1f
    pack $id.out1f -side top
    label $id.out1f.l1 -text [_ "Output device 1:"]
    button $id.out1f.x1 -text [lindex $midiOut $midi_outdev1] \
        -command [list ::pd_midi::midi_popup $id $id.out1f.x1 midi_outdev1 \
            $midiOut]
    pack $id.out1f.l1 $id.out1f.x1 -side left

        # output device 2
    if {[llength $midiOut] > 2} {
        frame $id.out2f
        pack $id.out2f -side top
        label $id.out2f.l1 -text [_ "Output device 2:"]
        button $id.out2f.x1 -text [lindex $midiOut $midi_outdev2] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out2f.x1 midi_outdev2 $midiOut]
        pack $id.out2f.l1 $id.out2f.x1 -side left
    }

        # output device 3
    if {[llength $midiOut] > 3} {
        frame $id.out3f
        pack $id.out3f -side top
        label $id.out3f.l1 -text [_ "Output device 3:"]
        button $id.out3f.x1 -text [lindex $midiOut $midi_outdev3] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out3f.x1 midi_outdev3 $midiOut]
        pack $id.out3f.l1 $id.out3f.x1 -side left
    }

        # output device 4
    if {[llength $midiOut] > 4} {
        frame $id.out4f
        pack $id.out4f -side top
        label $id.out4f.l1 -text [_ "Output device 4:"]
        button $id.out4f.x1 -text [lindex $midiOut $midi_outdev4] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out4f.x1 midi_outdev4 $midiOut]
        pack $id.out4f.l1 $id.out4f.x1 -side left
    }

        # output device 5
    if {[llength $midiOut] > 5} {
        frame $id.out5f
        pack $id.out5f -side top
        label $id.out5f.l1 -text [_ "Output device 5:"]
        button $id.out5f.x1 -text [lindex $midiOut $midi_outdev5] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out5f.x1 midi_outdev5 $midiOut]
        pack $id.out5f.l1 $id.out5f.x1 -side left
    }

        # output device 6
    if {[llength $midiOut] > 6} {
        frame $id.out6f
        pack $id.out6f -side top
        label $id.out6f.l1 -text [_ "Output device 6:"]
        button $id.out6f.x1 -text [lindex $midiOut $midi_outdev6] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out6f.x1 midi_outdev6 $midiOut]
        pack $id.out6f.l1 $id.out6f.x1 -side left
    }

        # output device 7
    if {[llength $midiOut] > 7} {
        frame $id.out7f
        pack $id.out7f -side top
        label $id.out7f.l1 -text [_ "Output device 7:"]
        button $id.out7f.x1 -text [lindex $midiOut $midi_outdev7] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out7f.x1 midi_outdev7 $midiOut]
        pack $id.out7f.l1 $id.out7f.x1 -side left
    }

        # output device 8
    if {[llength $midiOut] > 8} {
        frame $id.out8f
        pack $id.out8f -side top
        label $id.out8f.l1 -text [_ "Output device 8:"]
        button $id.out8f.x1 -text [lindex $midiOut $midi_outdev8] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out8f.x1 midi_outdev8 $midiOut]
        pack $id.out8f.l1 $id.out8f.x1 -side left
    }

        # output device 9
    if {[llength $midiOut] > 9} {
        frame $id.out9f
        pack $id.out9f -side top
        label $id.out9f.l1 -text [_ "Output device 9:"]
        button $id.out9f.x1 -text [lindex $midiOut $midi_outdev9] \
            -command \
            [list ::pd_midi::midi_popup $id $id.out9f.x1 midi_outdev9 $midiOut]
        pack $id.out9f.l1 $id.out9f.x1 -side left
    }
}

proc hide {top} {

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top} {

}

proc _closed {top} {

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc apply {mytoplevel} {
    global midi_indev1 midi_indev2 midi_indev3 midi_indev4 midi_indev5 \
        midi_indev6 midi_indev7 midi_indev8 midi_indev9
    global midi_outdev1 midi_outdev2 midi_outdev3 midi_outdev4 midi_outdev5 \
        midi_outdev6 midi_outdev7 midi_outdev8 midi_outdev9
    global midi_alsain midi_alsaout

    ::pd_connect::pdsend "pd midi-dialog \
        $midi_indev1 \
        $midi_indev2 \
        $midi_indev3 \
        $midi_indev4 \
        $midi_indev5 \
        $midi_indev6 \
        $midi_indev7 \
        $midi_indev8 \
        $midi_indev9 \
        $midi_outdev1 \
        $midi_outdev2 \
        $midi_outdev3 \
        $midi_outdev4 \
        $midi_outdev5 \
        $midi_outdev6 \
        $midi_outdev7 \
        $midi_outdev8 \
        $midi_outdev9 \
        $midi_alsain \
        $midi_alsaout"
}

proc cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

proc ok {mytoplevel} {
    ::pd_midi::apply $mytoplevel
    ::pd_midi::cancel $mytoplevel
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# callback from popup menu
proc midi_popup_action {buttonname varname devlist index} {
    
    variable midiIn
    variable midiOut
    
    global $varname
    $buttonname configure -text [lindex $devlist $index]
    set $varname $index
}

# create a popup menu
proc midi_popup {name buttonname varname devlist} {
    if [winfo exists $name.popup] {destroy $name.popup}
    menu $name.popup -tearoff 0
    if {[tk windowingsystem] eq "win32"} {
        $name.popup configure -font menuFont
    }
    for {set x 0} {$x<[llength $devlist]} {incr x} {
        $name.popup add command -label [lindex $devlist $x] \
            -command [list ::pd_midi::midi_popup_action \
                $buttonname $varname $devlist $x] 
    }
    tk_popup $name.popup [winfo pointerx $name] [winfo pointery $name] 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pdtk_alsa_midi_dialog {id indev1 indev2 indev3 indev4 \
        outdev1 outdev2 outdev3 outdev4 alsa} {

    variable midiIn
    variable midiOut
    
    global midi_indev1 midi_indev2 midi_indev3 midi_indev4 midi_indev5 \
         midi_indev6 midi_indev7 midi_indev8 midi_indev9
    global midi_outdev1 midi_outdev2 midi_outdev3 midi_outdev4 midi_outdev5 \
         midi_outdev6 midi_outdev7 midi_outdev8 midi_outdev9
    global midi_alsain midi_alsaout

    set midi_indev1 $indev1
    set midi_indev2 $indev2
    set midi_indev3 $indev3
    set midi_indev4 $indev4
    set midi_indev5 0
    set midi_indev6 0
    set midi_indev7 0
    set midi_indev8 0
    set midi_indev9 0
    set midi_outdev1 $outdev1
    set midi_outdev2 $outdev2
    set midi_outdev3 $outdev3
    set midi_outdev4 $outdev4
    set midi_outdev5 0
    set midi_outdev6 0
    set midi_outdev7 0
    set midi_outdev8 0
    set midi_outdev9 0
    set midi_alsain [expr [llength $midiIn] - 1]
    set midi_alsaout [expr [llength $midiOut] - 1]
    
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
        entry $id.in1f.x1 -textvariable midi_alsain -width 4
        pack $id.in1f.l1 $id.in1f.x1 -side left
        label $id.in1f.l2 -text [_ "Out Ports:"]
        entry $id.in1f.x2 -textvariable midi_alsaout -width 4
        pack $id.in1f.l2 $id.in1f.x2 -side left
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
