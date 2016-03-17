
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Audio settings.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_audio 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_audio:: {

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
           blockSize \
           multiple} {
    
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
    variable audioBlockSize

    ::ui_menu::disableAudio
        
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
    set audioBlockSize      $blockSize

    set blockSizeValues  {64 128 256 512 1024 2048}
    set sampleRateValues {32000 44100 48000 88200 96000}
        
    toplevel $top -class PdDialog
    wm title $top [_ "Audio"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo .console]
    
    ttk::frame      $top.f                              {*}[::styleFrame]
    ttk::labelframe $top.f.properties                   {*}[::styleLabelFrame]  -text [_ "Settings"]
    ttk::labelframe $top.f.inputs                       {*}[::styleLabelFrame]  -text [_ "Inputs"]
    ttk::labelframe $top.f.outputs                      {*}[::styleLabelFrame]  -text [_ "Outputs"]
    
    pack $top.f                                         {*}[::packMain]
    pack $top.f.properties                              {*}[::packCategory]
    
    if {$audioIn ne ""}  { pack $top.f.inputs           {*}[::packCategoryNext] }
    if {$audioOut ne ""} { pack $top.f.outputs          {*}[::packCategoryNext] }
    
    ttk::label $top.f.properties.sampleRateLabel        {*}[::styleLabel] \
                                                            -text [_ "Sample Rate"]
    ::createMenuByValue $top.f.properties.sampleRate    $sampleRateValues ::ui_audio::audioSampleRate \
                                                            -width [::measure $sampleRateValues]   

    ttk::label $top.f.properties.blockSizeLabel         {*}[::styleLabel] \
                                                            -text [_ "Block Size"]
    ::createMenuByValue $top.f.properties.blockSize     $blockSizeValues ::ui_audio::audioBlockSize \
                                                            -width [::measure $blockSizeValues]
                                                            
    grid $top.f.properties.sampleRateLabel              -row 0 -column 0 -sticky ew
    grid $top.f.properties.sampleRate                   -row 0 -column 2 -sticky ew
    grid $top.f.properties.blockSizeLabel               -row 1 -column 0 -sticky ew
    grid $top.f.properties.blockSize                    -row 1 -column 2 -sticky ew
    
    if {$multiple > 1} {
        foreach e $audioIn  { ::ui_audio::_makeIn  $top.f.inputs  [incr i] }
        foreach e $audioOut { ::ui_audio::_makeOut $top.f.outputs [incr j] }
    }

    if {![llength [winfo children $top.f.inputs]]}  { ::ui_audio::_makeIn  $top.f.inputs  1 }
    if {![llength [winfo children $top.f.outputs]]} { ::ui_audio::_makeOut $top.f.outputs 1 }

    grid columnconfigure $top.f.properties  1 -weight 1
    grid columnconfigure $top.f.inputs      1 -weight 1
    grid columnconfigure $top.f.outputs     1 -weight 1
    
    bind $top <Destroy> { ::ui_menu::enableAudio }
        
    wm protocol $top WM_DELETE_WINDOW   "::ui_audio::closed $top"
}

proc closed {top} {

    ::ui_audio::_apply $top
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _makeIn {top k} {

    variable audioIn
    variable audioInDevice
    variable audioInChannels
    variable audioInEnabled
    
    set slot     [format "%s.inSlot%d" $top $k]
    set devices  [format "%s.inDevice%d" $top $k]
    set channels [format "%s.inChannels%d" $top $k]
    
    ttk::checkbutton $slot              {*}[::styleCheckButton] \
                                            -variable ::ui_audio::audioInEnabled($k) \
                                            -takefocus 0
    
    ::createMenuByIndex $devices        $audioIn ::ui_audio::audioInDevice($k) -width -$::width(large)
        
    ttk::entry $channels                {*}[::styleEntryNumber] \
                                        -textvariable ::ui_audio::audioInChannels($k) \
                                        -width $::width(tiny)
    
    set row [expr {$k - 1}]
    
    grid $slot                          -row $row -column 0 -sticky ew
    grid $devices                       -row $row -column 1 -sticky ew -padx {0 5}
    grid $channels                      -row $row -column 2 -sticky ew
    
    bind $channels <Return> { ::nextEntry %W }
}

proc _makeOut {top k} {

    variable audioOut
    variable audioOutDevice
    variable audioOutChannels
    variable audioOutEnabled
    
    set slot     [format "%s.outSlot%d" $top $k]
    set devices  [format "%s.outDevice%d" $top $k]
    set channels [format "%s.outChannels%d" $top $k]
    
    ttk::checkbutton $slot              {*}[::styleCheckButton] \
                                        -variable ::ui_audio::audioOutEnabled($k) \
                                        -takefocus 0

    ::createMenuByIndex $devices        $audioOut ::ui_audio::audioOutDevice($k) -width -$::width(large)
    
    ttk::entry $channels                {*}[::styleEntryNumber] \
                                        -textvariable ::ui_audio::audioOutChannels($k) \
                                        -width $::width(tiny)
    
    set row [expr {$k - 1}]
    
    grid $slot                          -row $row -column 0 -sticky ew
    grid $devices                       -row $row -column 1 -sticky ew -padx {0 5}
    grid $channels                      -row $row -column 2 -sticky ew
    
    bind $channels <Return> { ::nextEntry %W }
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
    variable audioBlockSize
    
    _forceValues
    
    ::ui_interface::pdsend "pd _audioDialog \
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
            $audioBlockSize"
    
    ::ui_interface::pdsend "pd _savePreferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceValues {} {

    variable audioInChannels
    variable audioOutChannels
    variable audioSampleRate
    variable audioSampleRateOld

    foreach i {1 2 3 4} {
    
    set audioInChannels($i)  [::ifInteger $audioInChannels($i) 2]
    set audioInChannels($i)  [::tcl::mathfunc::max $audioInChannels($i) 0]
    set audioInChannels($i)  [::tcl::mathfunc::min $audioInChannels($i) 32]
    
    set audioOutChannels($i) [::ifInteger $audioOutChannels($i) 2]
    set audioOutChannels($i) [::tcl::mathfunc::max $audioOutChannels($i) 0]
    set audioOutChannels($i) [::tcl::mathfunc::min $audioOutChannels($i) 32]
    
    }
    
    set audioSampleRate      [::ifInteger $audioSampleRate $audioSampleRateOld]
    set audioSampleRate      [::tcl::mathfunc::max $audioSampleRate 1]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
