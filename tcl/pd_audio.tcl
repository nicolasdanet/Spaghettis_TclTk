
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Audio settings.

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
variable  audioSampleRateOld 
variable  audioDelay
variable  audioDelayOld
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
    variable audioSampleRateOld
    variable audioDelay 
    variable audioDelayOld
    variable audioCallback
    variable audioBlockSize

    ::pd_menu::disableAudio
        
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
    set audioOutEnabled(2)  [expr {$oChannels2 > 0}]
    set audioOutEnabled(3)  [expr {$oChannels3 > 0}]
    set audioOutEnabled(4)  [expr {$oChannels4 > 0}]

    set audioSampleRate     $sampleRate
    set audioSampleRateOld  $sampleRate
    set audioDelay          $delay
    set audioDelayOld       $delay
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
    
    if {$multiple > 1} {
        foreach e $audioIn  { ::pd_audio::_makeIn  $top [incr i] }
        foreach e $audioOut { ::pd_audio::_makeOut $top [incr j] }
        
    } else {
        ::pd_audio::_makeIn  $top 1
        ::pd_audio::_makeOut $top 1
    }
    
    bind  $top.sampleRate   <Return> { ::nextEntry %W }
    bind  $top.delay        <Return> { ::nextEntry %W }

    focus $top.sampleRate
    
    $top.sampleRate selection range 0 end
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_audio::_closed $top"
}

proc _closed {top} {

    ::pd_audio::_apply  $top
    ::pd_audio::_cancel $top
    
    ::pd_menu::enableAudio
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _makeIn {top k} {

    variable audioIn
    variable audioInDevice
    variable audioInChannels
    variable audioInEnabled
    
    set devicesLabel  [format "%s.inDevice%dLabel" $top $k]
    set devices       [format "%s.inDevice%d" $top $k]
    set channelsLabel [format "%s.inChannels%dLabel" $top $k]
    set channels      [format "%s.inChannels%d" $top $k]
    
    checkbutton $devicesLabel           -text [format "%s %d" [_ "Input"] $k] \
                                        -variable ::pd_audio::audioInEnabled($k) \
                                        -takefocus 0
    menubutton $devices                 -text [lindex $audioIn $audioInDevice($k)]
    
    menu $devices.menu
    $devices configure                  -menu $devices.menu
    
    set i 0
    
    foreach e $audioIn {
        $devices.menu add radiobutton   -label "$e" \
                                        -variable ::pd_audio::audioInDevice($k) \
                                        -value $i \
                                        -command [list $devices configure -text [lindex $audioIn $i]]
        incr i
    }
    
    label $channelsLabel                -text [_ "Channels"]
    entry $channels                     -textvariable ::pd_audio::audioInChannels($k) \
                                        -state disabled
    
    pack $devicesLabel                  -side top -anchor w
    pack $devices                       -side top -anchor w
    pack $channelsLabel                 -side top -anchor w
    pack $channels                      -side top -anchor w
}

proc _makeOut {top k} {

    variable audioOut
    variable audioOutDevice
    variable audioOutChannels
    variable audioOutEnabled
    
    set devicesLabel  [format "%s.outDevice%dLabel" $top $k]
    set devices       [format "%s.outDevice%d" $top $k]
    set channelsLabel [format "%s.outChannels%dLabel" $top $k]
    set channels      [format "%s.outChannels%d" $top $k]
    
    checkbutton $devicesLabel           -text [format "%s %d" [_ "Output"] $k] \
                                        -variable ::pd_audio::audioOutEnabled($k) \
                                        -takefocus 0
    menubutton $devices                 -text [lindex $audioOut $audioOutDevice($k)]
    
    menu $devices.menu
    $devices configure                  -menu $devices.menu
    
    set i 0
    
    foreach e $audioOut {
        $devices.menu add radiobutton   -label "$e" \
                                        -variable ::pd_audio::audioOutDevice($k) \
                                        -value $i \
                                        -command [list $devices configure -text [lindex $audioOut $i]]
        incr i
    }
    
    label $channelsLabel                -text [_ "Channels"]
    entry $channels                     -textvariable ::pd_audio::audioOutChannels($k) \
                                        -state disabled
    
    pack $devicesLabel                  -side top -anchor w
    pack $devices                       -side top -anchor w
    pack $channelsLabel                 -side top -anchor w
    pack $channels                      -side top -anchor w
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

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
    
    _forceValues
    
    ::pd_connect::pdsend "pd audio-dialog \
        $audioInDevice(1) \
        $audioInDevice(2) \
        $audioInDevice(3) \
        $audioInDevice(4) \
        [expr {$audioInChannels(1) * ($audioInEnabled(1) ? 1 : -1)}] \
        [expr {$audioInChannels(2) * ($audioInEnabled(2) ? 1 : -1)}] \
        [expr {$audioInChannels(3) * ($audioInEnabled(3) ? 1 : -1)}] \
        [expr {$audioInChannels(4) * ($audioInEnabled(4) ? 1 : -1)}] \
        $audioOutDevice(1) \
        $audioOutDevice(2) \
        $audioOutDevice(3) \
        $audioOutDevice(4) \
        [expr {$audioOutChannels(1) * ($audioOutEnabled(1) ? 1 : -1)}] \
        [expr {$audioOutChannels(2) * ($audioOutEnabled(2) ? 1 : -1)}] \
        [expr {$audioOutChannels(3) * ($audioOutEnabled(3) ? 1 : -1)}] \
        [expr {$audioOutChannels(4) * ($audioOutEnabled(4) ? 1 : -1)}] \
        $audioSampleRate \
        $audioDelay \
        $audioCallback \
        $audioBlockSize"
    
    ::pd_connect::pdsend "pd save-preferences"
}

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValues {} {

    variable audioSampleRate
    variable audioSampleRateOld
    variable audioDelay
    variable audioDelayOld 

    set audioSampleRate [::ifInteger $audioSampleRate $audioSampleRateOld]
    set audioSampleRate [::tcl::mathfunc::max $audioSampleRate 1]
    set audioDelay      [::ifInteger $audioDelay $audioDelayOld]
    set audioDelay      [::tcl::mathfunc::max $audioDelay 0]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
