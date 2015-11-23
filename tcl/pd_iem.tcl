
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ( http://iem.kug.ac.at/ ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_iem 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_iem:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  iemWidth
variable  iemWidthMinimum
variable  iemHeight
variable  iemHeightMinimum
variable  iemOptionA
variable  iemOptionB
variable  iemOptionFlags
variable  iemCheck
variable  iemIsLoadbang
variable  iemIsSteady
variable  iemExtra
variable  iemSend
variable  iemReceive
variable  iemName
variable  iemNameDeltaX
variable  iemNameDeltaY
variable  iemNameFont
variable  iemNameFontSize
variable  iemBackgroundColor
variable  iemFrontColor
variable  iemNameColor
variable  iemColor

array set iemWidth              {}
array set iemWidthMinimum       {}
array set iemHeight             {}
array set iemHeightMinimum      {}
array set iemOptionA            {}
array set iemOptionB            {}
array set iemOptionFlags        {}
array set iemCheck              {}
array set iemIsLoadbang         {}
array set iemIsSteady           {}
array set iemExtra              {}
array set iemSend               {}
array set iemReceive            {}
array set iemName               {}
array set iemNameDeltaX         {}
array set iemNameDeltaY         {}
array set iemNameFont           {}
array set iemNameFontSize       {}
array set iemBackgroundColor    {}
array set iemFrontColor         {}
array set iemNameColor          {}
array set iemColor              {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {top 
             mainTitle
             sizeTitle width widthMinimum widthLabel height heightMinimum heightLabel
             optionTitle optionA optionALabel optionB optionBLabel 
             optionFlags
             check checkA checkB 
             isLoadbang isSteady 
             extraLabel extra
             send receive
             name nameDeltaX nameDeltaY 
             nameFont nameFontSize
             backgroundColor frontColor nameColor} {
    
    variable iemWidth
    variable iemWidthMinimum
    variable iemHeight
    variable iemHeightMinimum
    variable iemOptionA
    variable iemOptionB
    variable iemOptionFlags
    variable iemCheck
    variable iemIsLoadbang
    variable iemIsSteady
    variable iemExtra
    variable iemSend
    variable iemReceive
    variable iemName
    variable iemNameDeltaX
    variable iemNameDeltaY
    variable iemNameFont
    variable iemNameFontSize
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemNameColor
    variable iemColor

    set iemWidth($top)              $width
    set iemWidthMinimum($top)       $widthMinimum
    set iemHeight($top)             $height
    set iemHeightMinimum($top)      $heightMinimum
    set iemOptionA($top)            $optionA
    set iemOptionB($top)            $optionB
    set iemOptionFlags($top)        $optionFlags
    set iemCheck($top)              $check
    set iemIsLoadbang($top)         $isLoadbang
    set iemIsSteady($top)           $isSteady
    set iemExtra($top)              $extra
    set iemSend($top)               [::parse $send]
    set iemReceive($top)            [::parse $receive]
    set iemName($top)               [::parse $name]
    set iemNameDeltaX($top)         $nameDeltaX
    set iemNameDeltaY($top)         $nameDeltaY
    set iemNameFont($top)           $nameFont
    set iemNameFontSize($top)       $nameFontSize
    set iemBackgroundColor($top)    $backgroundColor
    set iemFrontColor($top)         $frontColor
    set iemNameColor($top)          $nameColor
    set iemColor($top)              0

    toplevel $top -class PdDialog
    wm title $top $mainTitle
    wm group $top .
    
    wm resizable $top 0 0
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    if {$widthLabel ne "empty"}     {
    
        label $top.widthLabel       -text [_ $widthLabel]
        entry $top.width            -textvariable ::pd_iem::iemWidth($top)
        pack  $top.widthLabel       -side top -anchor w
        pack  $top.width            -side top -anchor w
        
    }
    
    if {$heightLabel ne "empty"}    {
    
        label $top.heightLabel      -text [_ $heightLabel]
        entry $top.height           -textvariable ::pd_iem::iemHeight($top)
        pack  $top.heightLabel      -side top -anchor w
        pack  $top.height           -side top -anchor w
        
    }
    
    if {$optionALabel ne "empty"}   {
    
        label $top.optionALabel     -text [_ $optionALabel]
        entry $top.optionA          -textvariable ::pd_iem::iemOptionA($top)
        pack  $top.optionALabel     -side top -anchor w
        pack  $top.optionA          -side top -anchor w
        
    }
    
    if {$optionBLabel ne "empty"}   {
    
        label $top.optionBLabel     -text [_ $optionBLabel]
        entry $top.optionB          -textvariable ::pd_iem::iemOptionB($top)
        pack  $top.optionBLabel     -side top -anchor w
        pack  $top.optionB          -side top -anchor w
        
    }
    
    if {$extraLabel ne "empty"}     {
    
        label $top.extraLabel       -text [_ $extraLabel]
        entry $top.extra            -textvariable ::pd_iem::iemExtra($top)
        pack  $top.extraLabel       -side top -anchor w
        pack  $top.extra            -side top -anchor w
        
    }
    
    if {$check != -1}               {
    
        radiobutton $top.checkA     -text [_ $checkA] \
                                    -variable ::pd_iem::iemCheck($top) \
                                    -value 0
        radiobutton $top.checkB     -text [_ $checkB] \
                                    -variable ::pd_iem::iemCheck($top) \
                                    -value 1
        pack $top.checkA            -side top -anchor w
        pack $top.checkB            -side top -anchor w
        
    }
    
    if {$isSteady != -1}            {
    
        radiobutton $top.jump       -text [_ "Jump on Click"] \
                                    -variable ::pd_iem::iemIsSteady($top) \
                                    -value 0
        radiobutton $top.steady     -text [_ "Steady on Click"] \
                                    -variable ::pd_iem::iemIsSteady($top) \
                                    -value 1
        pack $top.jump              -side top -anchor w
        pack $top.steady            -side top -anchor w
        
    }
    
    if {$isLoadbang != -1}          {
    
        checkbutton $top.loadbang   -text [_ "Load at Start"] \
                                    -variable ::pd_iem::iemIsLoadbang($top)
        pack $top.loadbang          -side top -anchor w
        
    }
    
    if {0} {
    
    frame $top .spacer0 -height 4
    pack $top .spacer0 -side top
    
    labelframe $top .s_r -borderwidth 1 -pady 4 -text [_ "Messages"]
    pack $top .s_r -side top -fill x -ipadx 5
    frame $top .s_r.send
    pack $top .s_r.send -side top -padx 4 -fill x -expand 1
    label $top .s_r.send.lab -text [_ "Send symbol:"] -justify left
    entry $top .s_r.send.ent -textvariable $var_iemgui_snd -width 22
    if { $snd ne "nosndno" } {
        pack $top .s_r.send.lab $top .s_r.send.ent -side left \
            -fill x -expand 1
    }
    
    frame $top .s_r.receive
    pack $top .s_r.receive -side top -padx 4 -fill x -expand 1
    label $top .s_r.receive.lab -text [_ "Receive symbol:"] -justify left
    entry $top .s_r.receive.ent -textvariable $var_iemgui_rcv -width 22
    if { $rcv ne "norcvno" } {
        pack $top .s_r.receive.lab $top .s_r.receive.ent -side left \
            -fill x -expand 1
    }
    
    # get the current font name from the int given from C-space (gn_f)
    set current_font $::var(fontFamily)
    if {[eval concat $$var_iemgui_gn_f] == 1} \
        { set current_font "Helvetica" }
    if {[eval concat $$var_iemgui_gn_f] == 2} \
        { set current_font "Times" }
    
    frame $top .spacer1 -height 7
    pack $top .spacer1 -side top
    
    labelframe $top .label -borderwidth 1 -text [_ "Label"] -pady 4
    pack $top .label -side top -fill x
    entry $top .label.name_entry -textvariable $var_iemgui_gui_nam \
        -width 30 -font [list $current_font 12 $::var(fontWeight)]
    pack $top .label.name_entry -side top -expand yes -fill both -padx 5
    
    frame $top .label.xy -padx 27 -pady 1
    pack $top .label.xy -side top
    label $top .label.xy.x_lab -text [_ "X offset"]
    entry $top .label.xy.x_entry -textvariable $var_iemgui_gn_dx -width 5
    label $top .label.xy.dummy1 -text " " -width 2
    label $top .label.xy.y_lab -text [_ "Y offset"]
    entry $top .label.xy.y_entry -textvariable $var_iemgui_gn_dy -width 5
    pack $top .label.xy.x_lab $top .label.xy.x_entry $top .label.xy.dummy1 \
        $top .label.xy.y_lab $top .label.xy.y_entry -side left -anchor e
    
    button $top .label.fontpopup_label -text $current_font \
        -font [list $current_font 16 $::var(fontWeight)]
    pack $top .label.fontpopup_label -side left -anchor w \
        -expand 1 -fill x -padx 5
    label $top .label.fontsize_label -text [_ "Size:"]
    entry $top .label.fontsize_entry -textvariable $var_iemgui_gn_fs -width 5
    pack $top .label.fontsize_entry $top .label.fontsize_label \
        -side right -anchor e -padx 5 -pady 5
    menu $top .popup
    $top .popup add command \
        -label $::var(fontFamily) \
        -font [format {{%s} 16 %s} $::var(fontFamily) $::var(fontWeight)] \
        -command "::pd_iem::toggle_font $top  0" 
    $top .popup add command \
        -label "Helvetica" \
        -font [format {Helvetica 16 %s} $::var(fontWeight)] \
        -command "::pd_iem::toggle_font $top  1" 
    $top .popup add command \
        -label "Times" \
        -font [format {Times 16 %s} $::var(fontWeight)] \
        -command "::pd_iem::toggle_font $top  2" 
    bind $top .label.fontpopup_label <Button> \
        [list tk_popup $top .popup %X %Y]
    
    frame $top .spacer2 -height 7
    pack $top .spacer2 -side top
    
    labelframe $top .colors -borderwidth 1 -text [_ "Colors"]
    pack $top .colors -fill x -ipadx 5 -ipady 4
    
    frame $top .colors.select
    pack $top .colors.select -side top
    radiobutton $top .colors.select.radio0 -value 0 -variable \
        $var_iemgui_l2_f1_b0 -text [_ "Background"] -justify left
    radiobutton $top .colors.select.radio1 -value 1 -variable \
        $var_iemgui_l2_f1_b0 -text [_ "Front"] -justify left
    radiobutton $top .colors.select.radio2 -value 2 -variable \
        $var_iemgui_l2_f1_b0 -text [_ "Label"] -justify left
    if { [eval concat $$var_iemgui_fcol] >= 0 } {
        pack $top .colors.select.radio0 $top .colors.select.radio1 \
            $top .colors.select.radio2 -side left
    } else {
        pack $top .colors.select.radio0 $top .colors.select.radio2 -side left
    }
    
    frame $top .colors.sections
    pack $top .colors.sections -side top
    button $top .colors.sections.but -text [_ "Compose color"] \
        -command "::pd_iem::choose_col_bkfrlb $top "
    pack $top .colors.sections.but -side left -anchor w -padx 10 -pady 5 \
        -expand yes -fill x
    if { [eval concat $$var_iemgui_fcol] >= 0 } {
        label $top .colors.sections.fr_bk -text "o=||=o" -width 6 \
            -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -foreground [format "#%6.6x" [eval concat $$var_iemgui_fcol]] \
            -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_fcol]] \
            -font [list $current_font 12 $::var(fontWeight)] -padx 2 -pady 2 -relief ridge
    } else {
        label $top .colors.sections.fr_bk -text "o=||=o" -width 6 \
            -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -foreground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -font [list $current_font 12 $::var(fontWeight)] -padx 2 -pady 2 -relief ridge
    }
    label $top .colors.sections.lb_bk -text [_ "Test label"] \
        -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
        -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
        -foreground [format "#%6.6x" [eval concat $$var_iemgui_lcol]] \
        -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_lcol]] \
        -font [list $current_font 12 $::var(fontWeight)] -padx 2 -pady 2 -relief ridge
    pack $top .colors.sections.lb_bk $top .colors.sections.fr_bk \
        -side right -anchor e -expand yes -fill both -pady 7
    
    # color scheme by Mary Ann Benedetto http://piR2.org
    frame $top .colors.r1
    pack $top .colors.r1 -side top
    foreach i { 0 1 2 3 4 5 6 7 8 9} \
        hexcol { 0xFFFFFF 0xDFDFDF 0xBBBBBB 0xFFC7C6 0xFFE3C6 \
                     0xFEFFC6 0xC6FFC7 0xc6FEFF 0xC7C6FF 0xE3C6FF } \
        {
            label $top .colors.r1.c$i -background [format "#%6.6x" $hexcol] \
                -activebackground [format "#%6.6x" $hexcol] -relief ridge \
                -padx 7 -pady 0
            bind $top .colors.r1.c$i <Button> [format "::pd_iem::preset_col %s %d" $top  $hexcol] 
        }
    pack $top .colors.r1.c0 $top .colors.r1.c1 $top .colors.r1.c2 $top .colors.r1.c3 \
        $top .colors.r1.c4 $top .colors.r1.c5 $top .colors.r1.c6 $top .colors.r1.c7 \
        $top .colors.r1.c8 $top .colors.r1.c9 -side left
    
    frame $top .colors.r2
    pack $top .colors.r2 -side top
    foreach i { 0 1 2 3 4 5 6 7 8 9 } \
        hexcol { 0x9F9F9F 0x7C7C7C 0x606060 0xFF0400 0xFF8300 \
                     0xFAFF00 0x00FF04 0x00FAFF 0x0400FF 0x9C00FF } \
        {
            label $top .colors.r2.c$i -background [format "#%6.6x" $hexcol] \
                -activebackground [format "#%6.6x" $hexcol] -relief ridge \
                -padx 7 -pady 0
            bind  $top .colors.r2.c$i <Button> \
                [format "::pd_iem::preset_col %s %d" $top  $hexcol] 
        }
    pack $top .colors.r2.c0 $top .colors.r2.c1 $top .colors.r2.c2 $top .colors.r2.c3 \
        $top .colors.r2.c4 $top .colors.r2.c5 $top .colors.r2.c6 $top .colors.r2.c7 \
        $top .colors.r2.c8 $top .colors.r2.c9 -side left
    
    frame $top .colors.r3
    pack $top .colors.r3 -side top
    foreach i { 0 1 2 3 4 5 6 7 8 9 } \
        hexcol { 0x404040 0x202020 0x000000 0x551312 0x553512 \
                     0x535512 0x0F4710 0x0E4345 0x131255 0x2F004D } \
        {
            label $top .colors.r3.c$i -background [format "#%6.6x" $hexcol] \
                -activebackground [format "#%6.6x" $hexcol] -relief ridge \
                -padx 7 -pady 0
            bind  $top .colors.r3.c$i <Button> \
                [format "::pd_iem::preset_col %s %d" $top  $hexcol] 
        }
    pack $top .colors.r3.c0 $top .colors.r3.c1 $top .colors.r3.c2 $top .colors.r3.c3 \
        $top .colors.r3.c4 $top .colors.r3.c5 $top .colors.r3.c6 $top .colors.r3.c7 \
        $top .colors.r3.c8 $top .colors.r3.c9 -side left
    
    frame $top .cao -pady 10
    pack $top .cao -side top -expand 1 -fill x
    button $top .cao.cancel -text [_ "Cancel"] \
        -command "::pd_iem::cancel $top "
    pack $top .cao.cancel -side left -padx 10 -expand 1 -fill x
    if {[tk windowingsystem] ne "aqua"} {
        button $top .cao.apply -text [_ "Apply"] \
            -command "::pd_iem::apply $top "
        pack $top .cao.apply -side left -padx 10 -expand 1 -fill x
    }
    button $top .cao.ok -text [_ "OK"] \
        -command "::pd_iem::ok $top "
    pack $top .cao.ok -side left -padx 10 -expand 1 -fill x
    
    $top .dim.w_ent select from 0
    $top .dim.w_ent select adjust end
    focus $top .dim.w_ent
    
    }
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_iem::_closed $top"
}

proc _closed {top} {
    
    variable iemWidth
    variable iemWidthMinimum
    variable iemHeight
    variable iemHeightMinimum
    variable iemOptionA
    variable iemOptionB
    variable iemOptionFlags
    variable iemCheck
    variable iemIsLoadbang
    variable iemIsSteady
    variable iemExtra
    variable iemSend
    variable iemReceive
    variable iemName
    variable iemNameDeltaX
    variable iemNameDeltaY
    variable iemNameFont
    variable iemNameFontSize
    variable iemBackgroundColor
    variable iemFrontColor
    variable iemNameColor
    variable iemColor
    
    ::pd_iem::_apply $top
    
    unset iemWidth($top)
    unset iemWidthMinimum($top)
    unset iemHeight($top)
    unset iemHeightMinimum($top)
    unset iemOptionA($top)
    unset iemOptionB($top)
    unset iemOptionFlags($top)
    unset iemCheck($top)
    unset iemIsLoadbang($top)
    unset iemIsSteady($top)
    unset iemExtra($top)
    unset iemSend($top)
    unset iemReceive($top)
    unset iemName($top)
    unset iemNameDeltaX($top)
    unset iemNameDeltaY($top)
    unset iemNameFont($top)
    unset iemNameFontSize($top)
    unset iemBackgroundColor($top)
    unset iemFrontColor($top)
    unset iemNameColor($top)
    unset iemColor($top)
    
    ::pd_iem::_cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

}

proc _cancel {top} {
    
    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
