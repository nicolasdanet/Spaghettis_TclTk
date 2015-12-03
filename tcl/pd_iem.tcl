
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# IEM ( http://iem.kug.ac.at/ ) objects properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_iem 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_iem:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  iemType
variable  iemWidth
variable  iemWidthMinimum
variable  iemWidthLabel
variable  iemHeight
variable  iemHeightMinimum
variable  iemHeightLabel
variable  iemOption1
variable  iemOption2
variable  iemCheck
variable  iemLoadbang
variable  iemExtra
variable  iemExtraMaximum
variable  iemExtraLabel
variable  iemSend
variable  iemReceive
variable  iemName
variable  iemNameDeltaX
variable  iemNameDeltaY
variable  iemNameFontFamily
variable  iemNameFontSize
variable  iemBackgroundColor
variable  iemFrontColor
variable  iemNameColor
variable  iemSteady
variable  iemFont

array set iemType               {}
array set iemWidth              {}
array set iemWidthMinimum       {}
array set iemWidthLabel         {}
array set iemHeight             {}
array set iemHeightMinimum      {}
array set iemHeightLabel        {}
array set iemOption1            {}
array set iemOption2            {}
array set iemCheck              {}
array set iemLoadbang           {}
array set iemExtra              {}
array set iemExtraMaximum       {}
array set iemExtraLabel         {}
array set iemSend               {}
array set iemReceive            {}
array set iemName               {}
array set iemNameDeltaX         {}
array set iemNameDeltaY         {}
array set iemNameFontFamily     {}
array set iemNameFontSize       {}
array set iemBackgroundColor    {}
array set iemFrontColor         {}
array set iemNameColor          {}
array set iemSteady             {}
array set iemFont               {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top type
             width widthMinimum widthLabel height heightMinimum heightLabel
             option1 option1Label option2 option2Label 
             check check1 check2 
             loadbang
             extra extraMaximum extraLabel
             send receive
             name nameDeltaX nameDeltaY 
             nameFontFamily nameFontSize
             backgroundColor frontColor nameColor
             steady} {
    
    variable iemType
    variable iemWidth
    variable iemWidthMinimum
    variable iemWidthLabel
    variable iemHeight
    variable iemHeightMinimum
    variable iemHeightLabel
    variable iemOption1
    variable iemOption2
    variable iemCheck
    variable iemLoadbang
    variable iemExtra
    variable iemExtraMaximum
    variable iemExtraLabel
    variable iemSend
    variable iemReceive
    variable iemName
    variable iemNameDeltaX
    variable iemNameDeltaY
    variable iemNameFontFamily
    variable iemNameFontSize
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemNameColor
    variable iemSteady
    variable iemFont

    set iemType($top)               $type
    set iemWidth($top)              $width
    set iemWidthMinimum($top)       $widthMinimum
    set iemWidthLabel($top)         $widthLabel
    set iemHeight($top)             $height
    set iemHeightMinimum($top)      $heightMinimum
    set iemHeightLabel($top)        $heightLabel
    set iemOption1($top)            $option1
    set iemOption2($top)            $option2
    set iemCheck($top)              $check
    set iemLoadbang($top)           $loadbang
    set iemExtra($top)              $extra
    set iemExtraMaximum($top)       $extraMaximum
    set iemExtraLabel($top)         $extraLabel
    set iemSend($top)               [::rauteToDollar [::parseEmpty $send]]
    set iemReceive($top)            [::rauteToDollar [::parseEmpty $receive]]
    set iemName($top)               [::rauteToDollar [::parseEmpty $name]]
    set iemNameDeltaX($top)         $nameDeltaX
    set iemNameDeltaY($top)         $nameDeltaY
    set iemNameFontFamily($top)     $nameFontFamily
    set iemNameFontSize($top)       $nameFontSize
    set iemBackgroundColor($top)    $backgroundColor
    set iemFrontColor($top)         $frontColor
    set iemNameColor($top)          $nameColor
    set iemSteady($top)             $steady
        
    set iemFont($top)               "$::var(fontFamily)"
    
    set iemWidth(${top}.old)        $width
    set iemHeight(${top}.old)       $height
    set iemOption1(${top}.old)      $option1
    set iemOption2(${top}.old)      $option2
    set iemExtra(${top}.old)        $extra
    set iemNameDeltaX(${top}.old)   $nameDeltaX
    set iemNameDeltaY(${top}.old)   $nameDeltaY
    set iemNameFontSize(${top}.old) $nameFontSize

    toplevel $top -class PdDialog
    wm title $top [_ $type]
    wm group $top .
    
    wm resizable $top 0 0
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    if {$widthLabel ne "empty"}     {
    
        label $top.widthLabel       -text [_ $widthLabel]
        entry $top.width            -textvariable ::pd_iem::iemWidth($top)
        pack  $top.widthLabel       -side top -anchor w
        pack  $top.width            -side top -anchor w
        
        bind  $top.width <Return>   { ::nextEntry %W }
    }
    
    if {$heightLabel ne "empty"}    {
    
        label $top.heightLabel      -text [_ $heightLabel]
        entry $top.height           -textvariable ::pd_iem::iemHeight($top)
        pack  $top.heightLabel      -side top -anchor w
        pack  $top.height           -side top -anchor w
        
        bind  $top.height <Return>  { ::nextEntry %W }
    }
    
    if {$option1Label ne "empty"}   {
    
        label $top.option1Label     -text [_ $option1Label]
        entry $top.option1          -textvariable ::pd_iem::iemOption1($top)
        pack  $top.option1Label     -side top -anchor w
        pack  $top.option1          -side top -anchor w
        
        bind  $top.option1 <Return> { ::nextEntry %W }
    }
    
    if {$option2Label ne "empty"}   {
    
        label $top.option2Label     -text [_ $option2Label]
        entry $top.option2          -textvariable ::pd_iem::iemOption2($top)
        pack  $top.option2Label     -side top -anchor w
        pack  $top.option2          -side top -anchor w
        
        bind  $top.option2 <Return> { ::nextEntry %W }
    }
    
    if {$extraLabel ne "empty"}     {
    
        label $top.extraLabel       -text [_ $extraLabel]
        entry $top.extra            -textvariable ::pd_iem::iemExtra($top)
        pack  $top.extraLabel       -side top -anchor w
        pack  $top.extra            -side top -anchor w
        
        bind  $top.extra <Return>   { ::nextEntry %W }
    }
    
    if {$check != -1}               {
    if {$check1 ne "empty"}         {
    if {$check2 ne "empty"}         {
    
        radiobutton $top.check1     -text [_ $check1] \
                                    -variable ::pd_iem::iemCheck($top) \
                                    -takefocus 0 \
                                    -value 0
        radiobutton $top.check2     -text [_ $check2] \
                                    -variable ::pd_iem::iemCheck($top) \
                                    -takefocus 0 \
                                    -value 1
        pack $top.check1            -side top -anchor w
        pack $top.check2            -side top -anchor w
    
    }
    }
    }
    
    if {$steady != -1}              {
    
        radiobutton $top.jump       -text [_ "Jump"] \
                                    -variable ::pd_iem::iemSteady($top) \
                                    -takefocus 0 \
                                    -value 0
        radiobutton $top.steady     -text [_ "Steady"] \
                                    -variable ::pd_iem::iemSteady($top) \
                                    -takefocus 0 \
                                    -value 1
        pack $top.jump              -side top -anchor w
        pack $top.steady            -side top -anchor w
        
    }
    
    if {$loadbang != -1}            {
    
        checkbutton $top.loadbang   -text [_ "Loadbang"] \
                                    -variable ::pd_iem::iemLoadbang($top) \
                                    -takefocus 0
        pack $top.loadbang          -side top -anchor w
        
    }
    
    label $top.nameLabel            -text [_ "Name"]
    entry $top.name                 -textvariable ::pd_iem::iemName($top)
    pack  $top.nameLabel            -side top -anchor w
    pack  $top.name                 -side top -anchor w
    
    label $top.nameDeltaXLabel      -text [_ "Offset Horizontal"]
    entry $top.nameDeltaX           -textvariable ::pd_iem::iemNameDeltaX($top)
    pack  $top.nameDeltaXLabel      -side top -anchor w
    pack  $top.nameDeltaX           -side top -anchor w
    
    label $top.nameDeltaYLabel      -text [_ "Offset Vertical"]
    entry $top.nameDeltaY           -textvariable ::pd_iem::iemNameDeltaY($top)
    pack  $top.nameDeltaYLabel      -side top -anchor w
    pack  $top.nameDeltaY           -side top -anchor w
    
    label $top.nameFontSizeLabel    -text [_ "Font Size"]
    entry $top.nameFontSize         -textvariable ::pd_iem::iemNameFontSize($top)
    pack  $top.nameFontSizeLabel    -side top -anchor w
    pack  $top.nameFontSize         -side top -anchor w
    
    bind $top.name <Return>         { ::nextEntry %W }
    bind $top.nameDeltaX <Return>   { ::nextEntry %W }
    bind $top.nameDeltaY <Return>   { ::nextEntry %W }
    bind $top.nameFontSize <Return> { ::nextEntry %W }
    
    menubutton $top.nameFontFamily  -textvariable ::pd_iem::iemFont($top) \
                                    -takefocus 0
    menu $top.nameFontFamily.menu
    $top.nameFontFamily configure   -menu $top.nameFontFamily.menu
    
    $top.nameFontFamily.menu add radiobutton    -label "$::var(fontFamily)" \
                                                -variable ::pd_iem::iemNameFontFamily($top) \
                                                -value 0 \
                                                -command "set ::pd_iem::iemFont($top) $::var(fontFamily)"
    $top.nameFontFamily.menu add radiobutton    -label "Helvetica" \
                                                -variable ::pd_iem::iemNameFontFamily($top) \
                                                -value 1 \
                                                -command "set ::pd_iem::iemFont($top) Helvetica"
    $top.nameFontFamily.menu add radiobutton    -label "Times" \
                                                -variable ::pd_iem::iemNameFontFamily($top) \
                                                -value 2 \
                                                -command "set ::pd_iem::iemFont($top) Times"
                                                
    pack $top.nameFontFamily        -side top -anchor w
    
    if {$send ne "nosndno"}         {
    
        label $top.sendLabel        -text [_ "Send"]
        entry $top.send             -textvariable ::pd_iem::iemSend($top)
        pack  $top.sendLabel        -side top -anchor w
        pack  $top.send             -side top -anchor w
        
        bind  $top.send <Return>    { ::nextEntry %W }
    }
    
    if {$send ne "norcvno"}         {
    
        label $top.receiveLabel     -text [_ "Receive"]
        entry $top.receive          -textvariable ::pd_iem::iemReceive($top)
        pack  $top.receiveLabel     -side top -anchor w
        pack  $top.receive          -side top -anchor w
        
        bind  $top.receive <Return> { ::nextEntry %W }
    }
    
    if {[winfo exists $top.width]} { focus $top.width; $top.width selection range 0 end }
        
    wm protocol $top WM_DELETE_WINDOW   "::pd_iem::_closed $top"
}

proc _closed {top} {
    
    variable iemType
    variable iemWidth
    variable iemWidthMinimum
    variable iemWidthLabel
    variable iemHeight
    variable iemHeightMinimum
    variable iemHeightLabel
    variable iemOption1
    variable iemOption2
    variable iemCheck
    variable iemLoadbang
    variable iemExtra
    variable iemExtraMaximum
    variable iemExtraLabel
    variable iemSend
    variable iemReceive
    variable iemName
    variable iemNameDeltaX
    variable iemNameDeltaY
    variable iemNameFontFamily
    variable iemNameFontSize
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemNameColor
    variable iemSteady
    variable iemFont
    
    ::pd_iem::_apply $top
    
    unset iemType($top)
    unset iemWidth($top)
    unset iemWidthMinimum($top)
    unset iemWidthLabel($top)
    unset iemHeight($top)
    unset iemHeightMinimum($top)
    unset iemHeightLabel($top)
    unset iemOption1($top)
    unset iemOption2($top)
    unset iemCheck($top)
    unset iemLoadbang($top)
    unset iemExtra($top)
    unset iemExtraMaximum($top)
    unset iemExtraLabel($top)
    unset iemSend($top)
    unset iemReceive($top)
    unset iemName($top)
    unset iemNameDeltaX($top)
    unset iemNameDeltaY($top)
    unset iemNameFontFamily($top)
    unset iemNameFontSize($top)
    unset iemBackgroundColor($top)
    unset iemFrontColor($top)
    unset iemNameColor($top)
    unset iemSteady($top)
    unset iemFont($top)
    
    unset iemWidth(${top}.old)
    unset iemHeight(${top}.old)
    unset iemOption1(${top}.old)
    unset iemOption2(${top}.old)
    unset iemExtra(${top}.old)
    unset iemNameDeltaX(${top}.old)
    unset iemNameDeltaY(${top}.old)
    unset iemNameFontSize(${top}.old)

    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable iemWidth
    variable iemHeight
    variable iemOption1
    variable iemOption2
    variable iemCheck
    variable iemLoadbang
    variable iemExtra
    variable iemSend
    variable iemReceive
    variable iemName
    variable iemNameDeltaX
    variable iemNameDeltaY
    variable iemNameFontFamily
    variable iemNameFontSize
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemNameColor
    variable iemSteady
    
    _forceWidth     $top
    _forceHeight    $top
    _forceExtra     $top
    _forceOptions   $top
    _forceDelta     $top
    _forceFont      $top
    
    ::pd_connect::pdsend "$top dialog \
            $iemWidth($top) \
            $iemHeight($top) \
            $iemOption1($top) \
            $iemOption2($top) \
            $iemCheck($top) \
            $iemLoadbang($top) \
            $iemExtra($top) \
            [::sanitized [::dollarToRaute [::withEmpty $iemSend($top)]]] \
            [::sanitized [::dollarToRaute [::withEmpty $iemReceive($top)]]] \
            [::sanitized [::dollarToRaute [::withEmpty $iemName($top)]]] \
            $iemNameDeltaX($top) \
            $iemNameDeltaY($top) \
            $iemNameFontFamily($top) \
            $iemNameFontSize($top) \
            $iemBackgroundColor($top) \
            $iemFrontColor($top) \
            $iemNameColor($top) \
            $iemSteady($top)"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceWidth {top} {

    variable iemWidth
    variable iemWidthMinimum
    variable iemWidthLabel

    if {$iemWidthLabel($top) ne "empty"} {
        set iemWidth($top) [::ifInteger $iemWidth($top) $iemWidth(${top}.old)]
        set iemWidth($top) [::tcl::mathfunc::max $iemWidth($top) $iemWidthMinimum($top)]
    }
}

proc _forceHeight {top} {

    variable iemHeight
    variable iemHeightMinimum
    variable iemHeightLabel

    if {$::pd_iem::iemHeightLabel($top) ne "empty"} {
        set iemHeight($top) [::ifInteger $iemHeight($top) $iemHeigh(${top}.old)]
        set iemHeight($top) [::tcl::mathfunc::max $iemHeight($top) $iemHeightMinimum($top)]
    }
}

proc _forceExtra {top} {

    variable iemExtra
    variable iemExtraMaximum
    variable iemExtraLabel

    if {$::pd_iem::iemExtraLabel($top) ne "empty"} {
        set iemExtra($top) [::ifInteger $iemExtra($top) $iemExtra(${top}.old)]
        set iemExtra($top) [::tcl::mathfunc::max $iemExtra($top) 1]
        set iemExtra($top) [::tcl::mathfunc::min $iemExtra($top) $iemExtraMaximum($top)]
    }
}

proc _forceOptions {top} {
    
    variable iemType
    variable iemOption1
    variable iemOption2

    switch -regexp -- $iemType($top) {
        "Bang"          {
            set iemOption1($top) [::ifInteger $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption1($top) [::tcl::mathfunc::max $iemOption1($top) 10]
            set iemOption2($top) [::ifInteger $iemOption2($top) $iemOption2(${top}.old)]
            set iemOption2($top) [::tcl::mathfunc::max $iemOption2($top) 10]
            set t1 $iemOption1($top)
            set t2 $iemOption2($top)
            set iemOption1($top) [::tcl::mathfunc::min $t1 $t2]
            set iemOption2($top) [::tcl::mathfunc::max $t1 $t2]
        }
        "Toggle"        {
            set iemOption1($top) [::ifNumber  $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption1($top) [::ifNonZero $iemOption1($top) $iemOption1(${top}.old)]
        }
        "Panel"         {
            set iemOption1($top) [::ifInteger $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption1($top) [::tcl::mathfunc::max $iemOption1($top) 1]
            set iemOption2($top) [::ifInteger $iemOption2($top) $iemOption2(${top}.old)]
            set iemOption2($top) [::tcl::mathfunc::max $iemOption2($top) 1]
        }
        "Slider|Number" {
            set iemOption1($top) [::ifNumber $iemOption1($top) $iemOption1(${top}.old)]
            set iemOption2($top) [::ifNumber $iemOption2($top) $iemOption2(${top}.old)]
        } 
    }
}

proc _forceDelta {top} {

    variable iemNameDeltaX
    variable iemNameDeltaY
    
    set iemNameDeltaX($top) [::ifInteger $iemNameDeltaX($top) $iemNameDeltaX(${top}.old)]
    set iemNameDeltaY($top) [::ifInteger $iemNameDeltaY($top) $iemNameDeltaY(${top}.old)]
}

proc _forceFont {top} {

    variable iemNameFontSize
    
    set iemNameFontSize($top) [::ifInteger $iemNameFontSize($top) $iemNameFontSize(${top}.old)]
    set iemNameFontSize($top) [::tcl::mathfunc::max $iemNameFontSize($top) 4]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
