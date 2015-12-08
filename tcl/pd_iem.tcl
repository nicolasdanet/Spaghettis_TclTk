
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# IEM ( http://iem.kug.ac.at/ ) objects properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_iem 1.0

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
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    ttk::frame      $top.f                              {*}[::styleFrame]
    ttk::labelframe $top.f.properties                   {*}[::styleLabelFrame]  -text [_ "Properties"]
    ttk::labelframe $top.f.label                        {*}[::styleLabelFrame]  -text [_ "Label"]
    
    pack $top.f                                         {*}[::packMain]
    pack $top.f.properties                              {*}[::packCategory]
    pack $top.f.label                                   {*}[::packCategoryNext]
    
    set row -1
    
    if {$widthLabel ne "empty"}     {
    
        ttk::label $top.f.properties.widthLabel         {*}[::styleLabel] \
                                                            -text [_ $widthLabel]
        ttk::entry $top.f.properties.width              {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemWidth($top) \
                                                            -width $::width(small)
        
        grid $top.f.properties.widthLabel               -row [incr row] -column 0 -sticky nsew
        grid $top.f.properties.width                    -row $row       -column 1 -sticky nsew

        bind $top.f.properties.width <Return>           { ::nextEntry %W }
    }
    
    if {$heightLabel ne "empty"}    {
    
        ttk::label $top.f.properties.heightLabel        {*}[::styleLabel] \
                                                            -text [_ $heightLabel]
        ttk::entry $top.f.properties.height             {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemHeight($top) \
                                                            -width $::width(small)
                                                        
        grid $top.f.properties.heightLabel              -row [incr row] -column 0 -sticky nsew
        grid $top.f.properties.height                   -row $row       -column 1 -sticky nsew
        
        bind $top.f.properties.height <Return>          { ::nextEntry %W }
    }
    
    if {$option1Label ne "empty"}   {
    
        ttk::label $top.f.properties.option1Label       {*}[::styleLabel] \
                                                            -text [_ $option1Label]
        ttk::entry $top.f.properties.option1            {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemOption1($top) \
                                                            -width $::width(small)

        grid $top.f.properties.option1Label             -row [incr row] -column 0 -sticky nsew
        grid $top.f.properties.option1                  -row $row       -column 1 -sticky nsew
        
        bind  $top.f.properties.option1 <Return>        { ::nextEntry %W }
    }
    
    if {$option2Label ne "empty"}   {
    
        ttk::label $top.f.properties.option2Label       {*}[::styleLabel] \
                                                            -text [_ $option2Label]
        ttk::entry $top.f.properties.option2            {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemOption2($top) \
                                                            -width $::width(small)
                                                        
        grid $top.f.properties.option2Label             -row [incr row] -column 0 -sticky nsew
        grid $top.f.properties.option2                  -row $row       -column 1 -sticky nsew
        
        bind $top.f.properties.option2 <Return>         { ::nextEntry %W }
    }
    
    if {$extraLabel ne "empty"}     {
    
        ttk::label $top.f.properties.extraLabel         {*}[::styleLabel] \
                                                            -text [_ $extraLabel]
        ttk::entry $top.f.properties.extra              {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemExtra($top) \
                                                            -width $::width(small)

        grid $top.f.properties.extraLabel               -row [incr row] -column 0 -sticky nsew
        grid $top.f.properties.extra                    -row $row       -column 1 -sticky nsew
        
        bind $top.f.properties.extra <Return>           { ::nextEntry %W }
    }
    
    if {$loadbang != -1}            {
    
        ttk::label $top.f.properties.loadbangLabel      {*}[::styleLabel] \
                                                            -text [_ "Load At Start"]
                                                            
        ttk::checkbutton $top.f.properties.loadbang     {*}[::styleCheckButton] \
                                                            -variable ::pd_iem::iemLoadbang($top) \
                                                            -takefocus 0
        
        grid $top.f.properties.loadbangLabel            -row [incr row] -column 0 -sticky nsew
        grid $top.f.properties.loadbang                 -row $row       -column 1 -sticky nsew
        
    }
    
    if {$check != -1}               {
    if {$check1 ne "empty"}         {
    if {$check2 ne "empty"}         {
    
        set values [list [_ $check1] [_ $check2]]
        
        ::createMenuByIndex $top.f.properties.check     $values ::pd_iem::iemCheck($top) \
                                                            -width [::measure $values]
        
        grid $top.f.properties.check                    -row [incr row] -column 1 -sticky nsew
    }
    }
    }
    
    if {$steady != -1}              {
    
        set values [list [_ "Jump"] [_ "Steady"]]
        
        ::createMenuByIndex $top.f.properties.steady    $values ::pd_iem::iemSteady($top) \
                                                            -width [::measure $values]
        
        grid $top.f.properties.steady                   -row [incr row] -column 1 -sticky nsew
    }
    
    ttk::label $top.f.label.nameLabel                   {*}[::styleLabel] \
                                                            -text [_ "Name"]
    ttk::entry $top.f.label.name                        {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemName($top) \
                                                            -width $::width(large)
    
    ttk::label $top.f.label.nameDeltaXLabel             {*}[::styleLabel] \
                                                            -text [_ "Position X"]
    ttk::entry $top.f.label.nameDeltaX                  {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemNameDeltaX($top) \
                                                            -width $::width(small)
    
    ttk::label $top.f.label.nameDeltaYLabel             {*}[::styleLabel] \
                                                            -text [_ "Position Y"]
    ttk::entry $top.f.label.nameDeltaY                  {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemNameDeltaY($top) \
                                                            -width $::width(small)

    ttk::label $top.f.label.nameFontSizeLabel           {*}[::styleLabel] \
                                                            -text [_ "Font Size"]
    ttk::entry $top.f.label.nameFontSize                {*}[::styleEntry] \
                                                            -textvariable ::pd_iem::iemNameFontSize($top) \
                                                            -width $::width(small)
    
    grid $top.f.label.nameLabel                         -row [incr row] -column 0 -sticky nsew
    grid $top.f.label.name                              -row $row       -column 1 -sticky nsew
    grid $top.f.label.nameDeltaXLabel                   -row [incr row] -column 0 -sticky nsew
    grid $top.f.label.nameDeltaX                        -row $row       -column 1 -sticky nsew
    grid $top.f.label.nameDeltaYLabel                   -row [incr row] -column 0 -sticky nsew
    grid $top.f.label.nameDeltaY                        -row $row       -column 1 -sticky nsew
    grid $top.f.label.nameFontSizeLabel                 -row [incr row] -column 0 -sticky nsew
    grid $top.f.label.nameFontSize                      -row $row       -column 1 -sticky nsew
    
    bind $top.f.label.name <Return>                     { ::nextEntry %W }
    bind $top.f.label.nameDeltaX <Return>               { ::nextEntry %W }
    bind $top.f.label.nameDeltaY <Return>               { ::nextEntry %W }
    bind $top.f.label.nameFontSize <Return>             { ::nextEntry %W }
    
    if {0} {
    
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
    
    }
    
    grid columnconfigure $top.f.properties  0 -weight 1
    grid columnconfigure $top.f.label       0 -weight 1
    
    if {[winfo exists $top.f.properties.width]} { 
        focus $top.f.properties.width
        $top.f.properties.width selection range 0 end 
    }
        
    wm protocol $top WM_DELETE_WINDOW   "::pd_iem::closed $top"
}

proc closed {top} {
    
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
        set iemHeight($top) [::ifInteger $iemHeight($top) $iemHeight(${top}.old)]
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
