
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_audio 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_audio:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  audioIn
variable  audioOut
variable  audioInDevice
variable  audioOutDevice
variable  audioInChannels
variable  audioOutChannels
variable  audioInEnabled
variable  audioOutEnabled
variable  audioSampleRate 
variable  audioDelay 
variable  audioCallback
variable  audioBlockSize

array set audioInDevice    {}
array set audioOutDevice   {}
array set audioInChannels  {}
array set audioOutChannels {}
array set audioInEnabled   {}
array set audioOutEnabled  {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top \
           i1 i2 i3 i4 iChannels1 iChannels2 iChannels3 iChannels4 \
           o1 o2 o3 o4 oChannels1 oChannels2 oChannels3 oChannels4 \
           sampleRate \
           delay \
           multiple \
           callback \
           blockSize} {
    
    variable audioIn
    variable audioOut
    variable audioInDevice
    variable audioOutDevice
    variable audioInChannels
    variable audioOutChannels
    variable audioInEnabled
    variable audioOutEnabled
    variable audioSampleRate 
    variable audioDelay 
    variable audioCallback
    variable audioBlockSize

    array set audioInDevice  [ list 1 $i1 2 $i2 3 $i3 4 $i4 ]
    array set audioOutDevice [ list 1 $o1 2 $o2 3 $o3 4 $o4 ]

    set audioInChannels(1)  [::tcl::mathfunc::abs $iChannels1]
    set audioInChannels(2)  [::tcl::mathfunc::abs $iChannels2]
    set audioInChannels(3)  [::tcl::mathfunc::abs $iChannels3]
    set audioInChannels(4)  [::tcl::mathfunc::abs $iChannels4]
    set audioOutChannels(1) [::tcl::mathfunc::abs $oChannels1]
    set audioOutChannels(2) [::tcl::mathfunc::abs $oChannels2]
    set audioOutChannels(3) [::tcl::mathfunc::abs $oChannels3]
    set audioOutChannels(4) [::tcl::mathfunc::abs $oChannels4]
    
    set audioInEnabled(1)   [expr {$iChannels1 > 0}]
    set audioInEnabled(2)   [expr {$iChannels2 > 0}]
    set audioInEnabled(3)   [expr {$iChannels3 > 0}]
    set audioInEnabled(4)   [expr {$iChannels4 > 0}]
    set audioOutEnabled(1)  [expr {$oChannels1 > 0}]
    set audioOutEnabled(1)  [expr {$oChannels2 > 0}]
    set audioOutEnabled(1)  [expr {$oChannels3 > 0}]
    set audioOutEnabled(1)  [expr {$oChannels4 > 0}]

    set audioSampleRate     $sampleRate
    set audioDelay          $delay
    set audioCallback       $callback
    set audioBlockSize      $blockSize

    toplevel $top -class PdDialog
    wm title $top [_ "Audio"]
    wm group $top .
    
    wm resizable $top 0 0
    wm geometry  $top [::rightNextTo .console]
    
    label $top.sampleRateLabel      -text [_ "Sample Rate"]
    entry $top.sampleRate           -textvariable ::pd_audio::audioSampleRate
    
    label $top.delayLabel           -text [_ "Delay in Milliseconds"]
    entry $top.delay                -textvariable ::pd_audio::audioDelay

    label $top.blockSizeLabel       -text [_ "Block Size"]
    tk_optionMenu $top.blockSize    ::pd_audio::blockSize 64 128 256 512 1024 2048

    pack $top.sampleRateLabel       -side top -anchor w
    pack $top.sampleRate            -side top -anchor w
    pack $top.delayLabel            -side top -anchor w
    pack $top.delay                 -side top -anchor w
    pack $top.blockSizeLabel        -side top -anchor w
    pack $top.blockSize             -side top -anchor w
    
    if {$audioCallback >= 0} {
    
    checkbutton $top.callback       -text [_ "Use Callbacks"] \
                                    -variable ::pd_audio::audioCallback \
                                    -takefocus 0
    pack $top.callback              -side top -anchor w
    
    }
    
    bind  $top.sampleRate   <Return> { ::nextEntry %W }
    bind  $top.delay        <Return> { ::nextEntry %W }

    focus $top.sampleRate
    
    $top.sampleRate selection range 0 end
    
    if {0} {
    
        # input device 1
    frame $top.in1f
    pack $top.in1f -side top

    checkbutton $top.in1f.x0 -variable audio_inenable1 \
        -text [_ "Input device 1:"] -anchor e
    button $top.in1f.x1 -text [lindex $audioIn $audio_indev1] \
        -command [list ::pd_audio::audio_popup $top $top.in1f.x1 audio_indev1 $audioIn]
    label $top.in1f.l2 -text [_ "Channels:"]
    entry $top.in1f.x2 -textvariable audio_inchan1 -width 3
    pack $top.in1f.x0 $top.in1f.x1 $top.in1f.l2 \
        $top.in1f.x2 -side left -fill x

        # input device 2
    if {$longform && $multi > 1 && [llength $audioIn] > 1} {
        frame $top.in2f
        pack $top.in2f -side top

        checkbutton $top.in2f.x0 -variable audio_inenable2 \
            -text [_ "Input device 2:"] -anchor e
        button $top.in2f.x1 -text [lindex $audioIn $audio_indev2] \
            -command [list ::pd_audio::audio_popup $top $top.in2f.x1 audio_indev2 \
                $audioIn]
        label $top.in2f.l2 -text [_ "Channels:"]
        entry $top.in2f.x2 -textvariable audio_inchan2 -width 3
        pack $top.in2f.x0 $top.in2f.x1 $top.in2f.l2 \
            $top.in2f.x2 -side left -fill x
    }

        # input device 3
    if {$longform && $multi > 1 && [llength $audioIn] > 2} {
        frame $top.in3f
        pack $top.in3f -side top

        checkbutton $top.in3f.x0 -variable audio_inenable3 \
            -text [_ "Input device 3:"] -anchor e
        button $top.in3f.x1 -text [lindex $audioIn $audio_indev3] \
            -command [list ::pd_audio::audio_popup $top $top.in3f.x1 audio_indev3 \
                $audioIn]
        label $top.in3f.l2 -text [_ "Channels:"]
        entry $top.in3f.x2 -textvariable audio_inchan3 -width 3
        pack $top.in3f.x0 $top.in3f.x1 $top.in3f.l2 $top.in3f.x2 -side left
    }

        # input device 4
    if {$longform && $multi > 1 && [llength $audioIn] > 3} {
        frame $top.in4f
        pack $top.in4f -side top

        checkbutton $top.in4f.x0 -variable audio_inenable4 \
            -text [_ "Input device 4:"] -anchor e
        button $top.in4f.x1 -text [lindex $audioIn $audio_indev4] \
            -command [list ::pd_audio::audio_popup $top $top.in4f.x1 audio_indev4 \
                $audioIn]
        label $top.in4f.l2 -text [_ "Channels:"]
        entry $top.in4f.x2 -textvariable audio_inchan4 -width 3
        pack $top.in4f.x0 $top.in4f.x1 $top.in4f.l2 \
            $top.in4f.x2 -side left
    }

        # output device 1
    frame $top.out1f
    pack $top.out1f -side top

    checkbutton $top.out1f.x0 -variable audio_outenable1 \
        -text [_ "Output device 1:"] -anchor e
    if {$multi == 0} {
        label $top.out1f.l1 \
            -text [_ "(same as input device) ..............      "]
    } else {
        button $top.out1f.x1 -text [lindex $audioOut $audio_outdev1] \
            -command  [list ::pd_audio::audio_popup $top $top.out1f.x1 audio_outdev1 \
                $audioOut]
    }
    label $top.out1f.l2 -text [_ "Channels:"]
    entry $top.out1f.x2 -textvariable audio_outchan1 -width 3
    if {$multi == 0} {
        pack $top.out1f.x0 $top.out1f.l1 $top.out1f.x2 -side left -fill x
    } else {
        pack $top.out1f.x0 $top.out1f.x1 $top.out1f.l2\
            $top.out1f.x2 -side left -fill x
    }

        # output device 2
    if {$longform && $multi > 1 && [llength $audioOut] > 1} {
        frame $top.out2f
        pack $top.out2f -side top

        checkbutton $top.out2f.x0 -variable audio_outenable2 \
            -text [_ "Output device 2:"] -anchor e
        button $top.out2f.x1 -text [lindex $audioOut $audio_outdev2] \
            -command \
            [list ::pd_audio::audio_popup $top $top.out2f.x1 audio_outdev2 $audioOut]
        label $top.out2f.l2 -text [_ "Channels:"]
        entry $top.out2f.x2 -textvariable audio_outchan2 -width 3
        pack $top.out2f.x0 $top.out2f.x1 $top.out2f.l2\
            $top.out2f.x2 -side left
    }

        # output device 3
    if {$longform && $multi > 1 && [llength $audioOut] > 2} {
        frame $top.out3f
        pack $top.out3f -side top

        checkbutton $top.out3f.x0 -variable audio_outenable3 \
            -text [_ "Output device 3:"] -anchor e
        button $top.out3f.x1 -text [lindex $audioOut $audio_outdev3] \
            -command \
            [list ::pd_audio::audio_popup $top $top.out3f.x1 audio_outdev3 $audioOut]
        label $top.out3f.l2 -text [_ "Channels:"]
        entry $top.out3f.x2 -textvariable audio_outchan3 -width 3
        pack $top.out3f.x0 $top.out3f.x1 $top.out3f.l2 \
            $top.out3f.x2 -side left
    }

        # output device 4
    if {$longform && $multi > 1 && [llength $audioOut] > 3} {
        frame $top.out4f
        pack $top.out4f -side top

        checkbutton $top.out4f.x0 -variable audio_outenable4 \
            -text [_ "Output device 4:"] -anchor e
        button $top.out4f.x1 -text [lindex $audioOut $audio_outdev4] \
            -command \
            [list ::pd_audio::audio_popup $top $top.out4f.x1 audio_outdev4 $audioOut]
        label $top.out4f.l2 -text [_ "Channels:"]
        entry $top.out4f.x2 -textvariable audio_outchan4 -width 3
        pack $top.out4f.x0 $top.out4f.x1 $top.out4f.l2 \
            $top.out4f.x2 -side left
    }

    $top.srf.x1 select from 0
    $top.srf.x1 select adjust end
    focus $top.srf.x1
    
    }
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_audio::_closed $top"
}

proc _closed {top} {

    ::pd_audio::_apply  $top
    ::pd_audio::_cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    if {0} {

    variable audioInDevice
    variable audioOutDevice
    variable audioInChannels
    variable audioOutChannels
    variable audioInEnabled
    variable audioOutEnabled
    variable audioSampleRate 
    variable audioDelay 
    variable audioCallback
    variable audioBlockSize
    
    ::pd_connect::pdsend "pd audio-dialog \
        $audio_indev1 \
        $audio_indev2 \
        $audio_indev3 \
        $audio_indev4 \
        [expr $audio_inchan1 * ( $audio_inenable1 ? 1 : -1 ) ]\
        [expr $audio_inchan2 * ( $audio_inenable2 ? 1 : -1 ) ]\
        [expr $audio_inchan3 * ( $audio_inenable3 ? 1 : -1 ) ]\
        [expr $audio_inchan4 * ( $audio_inenable4 ? 1 : -1 ) ]\
        $audio_outdev1 \
        $audio_outdev2 \
        $audio_outdev3 \
        $audio_outdev4 \
        [expr $audio_outchan1 * ( $audio_outenable1 ? 1 : -1 ) ]\
        [expr $audio_outchan2 * ( $audio_outenable2 ? 1 : -1 ) ]\
        [expr $audio_outchan3 * ( $audio_outenable3 ? 1 : -1 ) ]\
        [expr $audio_outchan4 * ( $audio_outenable4 ? 1 : -1 ) ]\
        $audio_sr \
        $audio_advance \
        $audio_callback \
        $audio_blocksize"
    
    ::pd_connect::pdsend "pd save-preferences"
    
    }
}

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
