
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

variable minimumflashHold   50
variable minimumFlashBreak  10
variable minimumFontSize    4

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  iemMainTitle
variable  iemSizeTitle
variable  iemWidth
variable  iemWidthMinimum
variable  iemWidthLabel
variable  iemHeight
variable  iemHeightMinimum
variable  iemHeightLabel
variable  iemOptionTitle
variable  iemOptionA
variable  iemOptionALabel
variable  iemOptionB
variable  iemOptionBLabel
variable  iemOptionFlags
variable  iemCheck
variable  iemCheckA
variable  iemCheckB
variable  iemIsLoadbang
variable  iemIsSteady
variable  iemExtraLabel
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

array set iemMainTitle          {}
array set iemSizeTitle          {}
array set iemWidth              {}
array set iemWidthMinimum       {}
array set iemWidthLabel         {}
array set iemHeight             {}
array set iemHeightMinimum      {}
array set iemHeightLabel        {}
array set iemOptionTitle        {}
array set iemOptionA            {}
array set iemOptionALabel       {}
array set iemOptionB            {}
array set iemOptionBLabel       {}
array set iemOptionFlags        {}
array set iemCheck              {}
array set iemCheckA             {}
array set iemCheckB             {}
array set iemIsLoadbang         {}
array set iemIsSteady           {}
array set iemExtraLabel         {}
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

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create {mytoplevel mainheader dim_header \
                                       wdt min_wdt wdt_label \
                                       hgt min_hgt hgt_label \
                                       rng_header min_rng min_rng_label max_rng \
                                       max_rng_label rng_sched \
                                       lin0_log1 lilo0_label lilo1_label \
                                       loadbang steady num_label num \
                                       snd rcv \
                                       gui_name \
                                       gn_dx gn_dy gn_f gn_fs \
                                       bcol fcol lcol} {
    
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_wdt [concat iemgui_wdt_$vid]
    global $var_iemgui_wdt
    set var_iemgui_min_wdt [concat iemgui_min_wdt_$vid]
    global $var_iemgui_min_wdt
    set var_iemgui_hgt [concat iemgui_hgt_$vid]
    global $var_iemgui_hgt
    set var_iemgui_min_hgt [concat iemgui_min_hgt_$vid]
    global $var_iemgui_min_hgt
    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]
    global $var_iemgui_min_rng
    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]
    global $var_iemgui_max_rng
    set var_iemgui_rng_sch [concat iemgui_rng_sch_$vid]
    global $var_iemgui_rng_sch
    set var_iemgui_lin0_log1 [concat iemgui_lin0_log1_$vid]
    global $var_iemgui_lin0_log1
    set var_iemgui_lilo0 [concat iemgui_lilo0_$vid]
    global $var_iemgui_lilo0
    set var_iemgui_lilo1 [concat iemgui_lilo1_$vid]
    global $var_iemgui_lilo1
    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]
    global $var_iemgui_loadbang
    set var_iemgui_num [concat iemgui_num_$vid]
    global $var_iemgui_num
    set var_iemgui_steady [concat iemgui_steady_$vid]
    global $var_iemgui_steady
    set var_iemgui_snd [concat iemgui_snd_$vid]
    global $var_iemgui_snd
    set var_iemgui_rcv [concat iemgui_rcv_$vid]
    global $var_iemgui_rcv
    set var_iemgui_gui_nam [concat iemgui_gui_nam_$vid]
    global $var_iemgui_gui_nam
    set var_iemgui_gn_dx [concat iemgui_gn_dx_$vid]
    global $var_iemgui_gn_dx
    set var_iemgui_gn_dy [concat iemgui_gn_dy_$vid]
    global $var_iemgui_gn_dy
    set var_iemgui_gn_f [concat iemgui_gn_f_$vid]
    global $var_iemgui_gn_f
    set var_iemgui_gn_fs [concat iemgui_gn_fs_$vid]
    global $var_iemgui_gn_fs
    set var_iemgui_l2_f1_b0 [concat iemgui_l2_f1_b0_$vid]
    global $var_iemgui_l2_f1_b0
    set var_iemgui_bcol [concat iemgui_bcol_$vid]
    global $var_iemgui_bcol
    set var_iemgui_fcol [concat iemgui_fcol_$vid]
    global $var_iemgui_fcol
    set var_iemgui_lcol [concat iemgui_lcol_$vid]
    global $var_iemgui_lcol
    
    set $var_iemgui_wdt $wdt
    set $var_iemgui_min_wdt $min_wdt
    set $var_iemgui_hgt $hgt
    set $var_iemgui_min_hgt $min_hgt
    set $var_iemgui_min_rng $min_rng
    set $var_iemgui_max_rng $max_rng
    set $var_iemgui_rng_sch $rng_sched
    set $var_iemgui_lin0_log1 $lin0_log1
    set $var_iemgui_lilo0 $lilo0_label
    set $var_iemgui_lilo1 $lilo1_label
    set $var_iemgui_loadbang $loadbang
    set $var_iemgui_num $num
    set $var_iemgui_steady $steady
    if {$snd == "empty"} {set $var_iemgui_snd [format ""]
    } else {set $var_iemgui_snd [format "%s" $snd]}
    if {$rcv == "empty"} {set $var_iemgui_rcv [format ""]
    } else {set $var_iemgui_rcv [format "%s" $rcv]}
    if {$gui_name == "empty"} {set $var_iemgui_gui_nam [format ""]
    } else {set $var_iemgui_gui_nam [format "%s" $gui_name]}
    
    if {[string index [eval concat $$var_iemgui_snd] 0] == "#"} {
        set $var_iemgui_snd [string replace [eval concat $$var_iemgui_snd] 0 0 $] }
    if {[string index [eval concat $$var_iemgui_rcv] 0] == "#"} {
        set $var_iemgui_rcv [string replace [eval concat $$var_iemgui_rcv] 0 0 $] }
    if {[string index [eval concat $$var_iemgui_gui_nam] 0] == "#"} {
        set $var_iemgui_gui_nam [string replace [eval concat $$var_iemgui_gui_nam] 0 0 $] }
    set $var_iemgui_gn_dx $gn_dx
    set $var_iemgui_gn_dy $gn_dy
    set $var_iemgui_gn_f $gn_f
    set $var_iemgui_gn_fs $gn_fs
    
    set $var_iemgui_bcol $bcol
    set $var_iemgui_fcol $fcol
    set $var_iemgui_lcol $lcol
    
    set $var_iemgui_l2_f1_b0 0
    
    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel [format [_ "%s Properties"] $mainheader]
    wm group $mytoplevel .
    wm resizable $mytoplevel 0 0
    $mytoplevel configure -padx 0 -pady 0

    frame $mytoplevel.dim
    pack $mytoplevel.dim -side top
    label $mytoplevel.dim.head -text [_ $dim_header]
    label $mytoplevel.dim.w_lab -text [_ $wdt_label] -width 6
    entry $mytoplevel.dim.w_ent -textvariable $var_iemgui_wdt -width 5
    label $mytoplevel.dim.dummy1 -text " " -width 10
    label $mytoplevel.dim.h_lab -text [_ $hgt_label] -width 6
    entry $mytoplevel.dim.h_ent -textvariable $var_iemgui_hgt -width 5
    pack $mytoplevel.dim.head -side top
    pack $mytoplevel.dim.w_lab $mytoplevel.dim.w_ent $mytoplevel.dim.dummy1 -side left
    if { $hgt_label ne "empty" } {
        pack $mytoplevel.dim.h_lab $mytoplevel.dim.h_ent -side left}
    
    frame $mytoplevel.rng
    pack $mytoplevel.rng -side top
    label $mytoplevel.rng.head -text [_ $rng_header]
    label $mytoplevel.rng.min_lab -text [_ $min_rng_label] -width 6
    entry $mytoplevel.rng.min_ent -textvariable $var_iemgui_min_rng -width 9
    label $mytoplevel.rng.dummy1 -text " " -width 1
    label $mytoplevel.rng.max_lab -text [_ $max_rng_label] -width 8
    entry $mytoplevel.rng.max_ent -textvariable $var_iemgui_max_rng -width 9
    if { $rng_header ne "empty" } {
        pack $mytoplevel.rng.head -side top
        if { $min_rng_label ne "empty" } {
            pack $mytoplevel.rng.min_lab $mytoplevel.rng.min_ent -side left}
        if { $max_rng_label ne "empty" } {
            pack $mytoplevel.rng.dummy1 \
                $mytoplevel.rng.max_lab $mytoplevel.rng.max_ent -side left} }
    
    if { [eval concat $$var_iemgui_lin0_log1] >= 0 || [eval concat $$var_iemgui_loadbang] >= 0 || [eval concat $$var_iemgui_num] > 0 || [eval concat $$var_iemgui_steady] >= 0 } {
        label $mytoplevel.space1 -text ""
        pack $mytoplevel.space1 -side top }
    
    frame $mytoplevel.para
    pack $mytoplevel.para -side top
    label $mytoplevel.para.dummy2 -text "" -width 1
    label $mytoplevel.para.dummy3 -text "" -width 1
    if {[eval concat $$var_iemgui_lin0_log1] == 0} {
        button $mytoplevel.para.lilo -text [_ [eval concat $$var_iemgui_lilo0]] -width 5 \
            -command "::pd_iem::lilo $mytoplevel" }
    if {[eval concat $$var_iemgui_lin0_log1] == 1} {
        button $mytoplevel.para.lilo -text [_ [eval concat $$var_iemgui_lilo1]] -width 5 \
            -command "::pd_iem::lilo $mytoplevel" }
    if {[eval concat $$var_iemgui_loadbang] == 0} {
        button $mytoplevel.para.lb -text [_ "No init"] \
            -command "::pd_iem::lb $mytoplevel" }
    if {[eval concat $$var_iemgui_loadbang] == 1} {
        button $mytoplevel.para.lb -text [_ "Save"] \
            -command "::pd_iem::lb $mytoplevel" }
    label $mytoplevel.para.num_lab -text [_ $num_label] -width 9
    entry $mytoplevel.para.num_ent -textvariable $var_iemgui_num -width 4

    if {[eval concat $$var_iemgui_steady] == 0} {
        button $mytoplevel.para.stdy_jmp -command "::pd_iem::stdy_jmp $mytoplevel" \
            -text [_ "Jump on click"] }
    if {[eval concat $$var_iemgui_steady] == 1} {
        button $mytoplevel.para.stdy_jmp -command "::pd_iem::stdy_jmp $mytoplevel" \
            -text [_ "Steady on click"] }
    if {[eval concat $$var_iemgui_lin0_log1] >= 0} {
        pack $mytoplevel.para.lilo -side left -expand 1}
    if {[eval concat $$var_iemgui_loadbang] >= 0} {
        pack $mytoplevel.para.dummy2 $mytoplevel.para.lb -side left -expand 1}
    if {[eval concat $$var_iemgui_num] > 0} {
        pack $mytoplevel.para.dummy3 $mytoplevel.para.num_lab $mytoplevel.para.num_ent -side left -expand 1}
    if {[eval concat $$var_iemgui_steady] >= 0} {
        pack $mytoplevel.para.dummy3 $mytoplevel.para.stdy_jmp -side left -expand 1}
    
    frame $mytoplevel.spacer0 -height 4
    pack $mytoplevel.spacer0 -side top
    
    labelframe $mytoplevel.s_r -borderwidth 1 -pady 4 -text [_ "Messages"]
    pack $mytoplevel.s_r -side top -fill x -ipadx 5
    frame $mytoplevel.s_r.send
    pack $mytoplevel.s_r.send -side top -padx 4 -fill x -expand 1
    label $mytoplevel.s_r.send.lab -text [_ "Send symbol:"] -justify left
    entry $mytoplevel.s_r.send.ent -textvariable $var_iemgui_snd -width 22
    if { $snd ne "nosndno" } {
        pack $mytoplevel.s_r.send.lab $mytoplevel.s_r.send.ent -side left \
            -fill x -expand 1
    }
    
    frame $mytoplevel.s_r.receive
    pack $mytoplevel.s_r.receive -side top -padx 4 -fill x -expand 1
    label $mytoplevel.s_r.receive.lab -text [_ "Receive symbol:"] -justify left
    entry $mytoplevel.s_r.receive.ent -textvariable $var_iemgui_rcv -width 22
    if { $rcv ne "norcvno" } {
        pack $mytoplevel.s_r.receive.lab $mytoplevel.s_r.receive.ent -side left \
            -fill x -expand 1
    }
    
    # get the current font name from the int given from C-space (gn_f)
    set current_font $::var(fontFamily)
    if {[eval concat $$var_iemgui_gn_f] == 1} \
        { set current_font "Helvetica" }
    if {[eval concat $$var_iemgui_gn_f] == 2} \
        { set current_font "Times" }
    
    frame $mytoplevel.spacer1 -height 7
    pack $mytoplevel.spacer1 -side top
    
    labelframe $mytoplevel.label -borderwidth 1 -text [_ "Label"] -pady 4
    pack $mytoplevel.label -side top -fill x
    entry $mytoplevel.label.name_entry -textvariable $var_iemgui_gui_nam \
        -width 30 -font [list $current_font 12 $::var(fontWeight)]
    pack $mytoplevel.label.name_entry -side top -expand yes -fill both -padx 5
    
    frame $mytoplevel.label.xy -padx 27 -pady 1
    pack $mytoplevel.label.xy -side top
    label $mytoplevel.label.xy.x_lab -text [_ "X offset"]
    entry $mytoplevel.label.xy.x_entry -textvariable $var_iemgui_gn_dx -width 5
    label $mytoplevel.label.xy.dummy1 -text " " -width 2
    label $mytoplevel.label.xy.y_lab -text [_ "Y offset"]
    entry $mytoplevel.label.xy.y_entry -textvariable $var_iemgui_gn_dy -width 5
    pack $mytoplevel.label.xy.x_lab $mytoplevel.label.xy.x_entry $mytoplevel.label.xy.dummy1 \
        $mytoplevel.label.xy.y_lab $mytoplevel.label.xy.y_entry -side left -anchor e
    
    button $mytoplevel.label.fontpopup_label -text $current_font \
        -font [list $current_font 16 $::var(fontWeight)]
    pack $mytoplevel.label.fontpopup_label -side left -anchor w \
        -expand 1 -fill x -padx 5
    label $mytoplevel.label.fontsize_label -text [_ "Size:"]
    entry $mytoplevel.label.fontsize_entry -textvariable $var_iemgui_gn_fs -width 5
    pack $mytoplevel.label.fontsize_entry $mytoplevel.label.fontsize_label \
        -side right -anchor e -padx 5 -pady 5
    menu $mytoplevel.popup
    $mytoplevel.popup add command \
        -label $::var(fontFamily) \
        -font [format {{%s} 16 %s} $::var(fontFamily) $::var(fontWeight)] \
        -command "::pd_iem::toggle_font $mytoplevel 0" 
    $mytoplevel.popup add command \
        -label "Helvetica" \
        -font [format {Helvetica 16 %s} $::var(fontWeight)] \
        -command "::pd_iem::toggle_font $mytoplevel 1" 
    $mytoplevel.popup add command \
        -label "Times" \
        -font [format {Times 16 %s} $::var(fontWeight)] \
        -command "::pd_iem::toggle_font $mytoplevel 2" 
    bind $mytoplevel.label.fontpopup_label <Button> \
        [list tk_popup $mytoplevel.popup %X %Y]
    
    frame $mytoplevel.spacer2 -height 7
    pack $mytoplevel.spacer2 -side top
    
    labelframe $mytoplevel.colors -borderwidth 1 -text [_ "Colors"]
    pack $mytoplevel.colors -fill x -ipadx 5 -ipady 4
    
    frame $mytoplevel.colors.select
    pack $mytoplevel.colors.select -side top
    radiobutton $mytoplevel.colors.select.radio0 -value 0 -variable \
        $var_iemgui_l2_f1_b0 -text [_ "Background"] -justify left
    radiobutton $mytoplevel.colors.select.radio1 -value 1 -variable \
        $var_iemgui_l2_f1_b0 -text [_ "Front"] -justify left
    radiobutton $mytoplevel.colors.select.radio2 -value 2 -variable \
        $var_iemgui_l2_f1_b0 -text [_ "Label"] -justify left
    if { [eval concat $$var_iemgui_fcol] >= 0 } {
        pack $mytoplevel.colors.select.radio0 $mytoplevel.colors.select.radio1 \
            $mytoplevel.colors.select.radio2 -side left
    } else {
        pack $mytoplevel.colors.select.radio0 $mytoplevel.colors.select.radio2 -side left
    }
    
    frame $mytoplevel.colors.sections
    pack $mytoplevel.colors.sections -side top
    button $mytoplevel.colors.sections.but -text [_ "Compose color"] \
        -command "::pd_iem::choose_col_bkfrlb $mytoplevel"
    pack $mytoplevel.colors.sections.but -side left -anchor w -padx 10 -pady 5 \
        -expand yes -fill x
    if { [eval concat $$var_iemgui_fcol] >= 0 } {
        label $mytoplevel.colors.sections.fr_bk -text "o=||=o" -width 6 \
            -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -foreground [format "#%6.6x" [eval concat $$var_iemgui_fcol]] \
            -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_fcol]] \
            -font [list $current_font 12 $::var(fontWeight)] -padx 2 -pady 2 -relief ridge
    } else {
        label $mytoplevel.colors.sections.fr_bk -text "o=||=o" -width 6 \
            -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -foreground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -font [list $current_font 12 $::var(fontWeight)] -padx 2 -pady 2 -relief ridge
    }
    label $mytoplevel.colors.sections.lb_bk -text [_ "Test label"] \
        -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
        -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
        -foreground [format "#%6.6x" [eval concat $$var_iemgui_lcol]] \
        -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_lcol]] \
        -font [list $current_font 12 $::var(fontWeight)] -padx 2 -pady 2 -relief ridge
    pack $mytoplevel.colors.sections.lb_bk $mytoplevel.colors.sections.fr_bk \
        -side right -anchor e -expand yes -fill both -pady 7
    
    # color scheme by Mary Ann Benedetto http://piR2.org
    frame $mytoplevel.colors.r1
    pack $mytoplevel.colors.r1 -side top
    foreach i { 0 1 2 3 4 5 6 7 8 9} \
        hexcol { 0xFFFFFF 0xDFDFDF 0xBBBBBB 0xFFC7C6 0xFFE3C6 \
                     0xFEFFC6 0xC6FFC7 0xc6FEFF 0xC7C6FF 0xE3C6FF } \
        {
            label $mytoplevel.colors.r1.c$i -background [format "#%6.6x" $hexcol] \
                -activebackground [format "#%6.6x" $hexcol] -relief ridge \
                -padx 7 -pady 0
            bind $mytoplevel.colors.r1.c$i <Button> [format "::pd_iem::preset_col %s %d" $mytoplevel $hexcol] 
        }
    pack $mytoplevel.colors.r1.c0 $mytoplevel.colors.r1.c1 $mytoplevel.colors.r1.c2 $mytoplevel.colors.r1.c3 \
        $mytoplevel.colors.r1.c4 $mytoplevel.colors.r1.c5 $mytoplevel.colors.r1.c6 $mytoplevel.colors.r1.c7 \
        $mytoplevel.colors.r1.c8 $mytoplevel.colors.r1.c9 -side left
    
    frame $mytoplevel.colors.r2
    pack $mytoplevel.colors.r2 -side top
    foreach i { 0 1 2 3 4 5 6 7 8 9 } \
        hexcol { 0x9F9F9F 0x7C7C7C 0x606060 0xFF0400 0xFF8300 \
                     0xFAFF00 0x00FF04 0x00FAFF 0x0400FF 0x9C00FF } \
        {
            label $mytoplevel.colors.r2.c$i -background [format "#%6.6x" $hexcol] \
                -activebackground [format "#%6.6x" $hexcol] -relief ridge \
                -padx 7 -pady 0
            bind  $mytoplevel.colors.r2.c$i <Button> \
                [format "::pd_iem::preset_col %s %d" $mytoplevel $hexcol] 
        }
    pack $mytoplevel.colors.r2.c0 $mytoplevel.colors.r2.c1 $mytoplevel.colors.r2.c2 $mytoplevel.colors.r2.c3 \
        $mytoplevel.colors.r2.c4 $mytoplevel.colors.r2.c5 $mytoplevel.colors.r2.c6 $mytoplevel.colors.r2.c7 \
        $mytoplevel.colors.r2.c8 $mytoplevel.colors.r2.c9 -side left
    
    frame $mytoplevel.colors.r3
    pack $mytoplevel.colors.r3 -side top
    foreach i { 0 1 2 3 4 5 6 7 8 9 } \
        hexcol { 0x404040 0x202020 0x000000 0x551312 0x553512 \
                     0x535512 0x0F4710 0x0E4345 0x131255 0x2F004D } \
        {
            label $mytoplevel.colors.r3.c$i -background [format "#%6.6x" $hexcol] \
                -activebackground [format "#%6.6x" $hexcol] -relief ridge \
                -padx 7 -pady 0
            bind  $mytoplevel.colors.r3.c$i <Button> \
                [format "::pd_iem::preset_col %s %d" $mytoplevel $hexcol] 
        }
    pack $mytoplevel.colors.r3.c0 $mytoplevel.colors.r3.c1 $mytoplevel.colors.r3.c2 $mytoplevel.colors.r3.c3 \
        $mytoplevel.colors.r3.c4 $mytoplevel.colors.r3.c5 $mytoplevel.colors.r3.c6 $mytoplevel.colors.r3.c7 \
        $mytoplevel.colors.r3.c8 $mytoplevel.colors.r3.c9 -side left
    
    frame $mytoplevel.cao -pady 10
    pack $mytoplevel.cao -side top -expand 1 -fill x
    button $mytoplevel.cao.cancel -text [_ "Cancel"] \
        -command "::pd_iem::cancel $mytoplevel"
    pack $mytoplevel.cao.cancel -side left -padx 10 -expand 1 -fill x
    if {[tk windowingsystem] ne "aqua"} {
        button $mytoplevel.cao.apply -text [_ "Apply"] \
            -command "::pd_iem::apply $mytoplevel"
        pack $mytoplevel.cao.apply -side left -padx 10 -expand 1 -fill x
    }
    button $mytoplevel.cao.ok -text [_ "OK"] \
        -command "::pd_iem::ok $mytoplevel"
    pack $mytoplevel.cao.ok -side left -padx 10 -expand 1 -fill x
    
    $mytoplevel.dim.w_ent select from 0
    $mytoplevel.dim.w_ent select adjust end
    focus $mytoplevel.dim.w_ent
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# TODO convert Init/No Init and Steady on click/Jump on click to checkbuttons

proc clip_dim {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_wdt [concat iemgui_wdt_$vid]
    global $var_iemgui_wdt
    set var_iemgui_min_wdt [concat iemgui_min_wdt_$vid]
    global $var_iemgui_min_wdt
    set var_iemgui_hgt [concat iemgui_hgt_$vid]
    global $var_iemgui_hgt
    set var_iemgui_min_hgt [concat iemgui_min_hgt_$vid]
    global $var_iemgui_min_hgt
    
    if {[eval concat $$var_iemgui_wdt] < [eval concat $$var_iemgui_min_wdt]} {
        set $var_iemgui_wdt [eval concat $$var_iemgui_min_wdt]
        $mytoplevel.dim.w_ent configure -textvariable $var_iemgui_wdt
    }
    if {[eval concat $$var_iemgui_hgt] < [eval concat $$var_iemgui_min_hgt]} {
        set $var_iemgui_hgt [eval concat $$var_iemgui_min_hgt]
        $mytoplevel.dim.h_ent configure -textvariable $var_iemgui_hgt
    }
}

proc ::pd_iem::clip_num {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_num [concat iemgui_num_$vid]
    global $var_iemgui_num
    
    if {[eval concat $$var_iemgui_num] > 2000} {
        set $var_iemgui_num 2000
        $mytoplevel.para.num_ent configure -textvariable $var_iemgui_num
    }
    if {[eval concat $$var_iemgui_num] < 1} {
        set $var_iemgui_num 1
        $mytoplevel.para.num_ent configure -textvariable $var_iemgui_num
    }
}

proc sched_rng {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]
    global $var_iemgui_min_rng
    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]
    global $var_iemgui_max_rng
    set var_iemgui_rng_sch [concat iemgui_rng_sch_$vid]
    global $var_iemgui_rng_sch
    
    variable minimumflashHold
    variable minimumFlashBreak
    
    if {[eval concat $$var_iemgui_rng_sch] == 2} {
        if {[eval concat $$var_iemgui_max_rng] < [eval concat $$var_iemgui_min_rng]} {
            set hhh [eval concat $$var_iemgui_min_rng]
            set $var_iemgui_min_rng [eval concat $$var_iemgui_max_rng]
            set $var_iemgui_max_rng $hhh
            $mytoplevel.rng.max_ent configure -textvariable $var_iemgui_max_rng
            $mytoplevel.rng.min_ent configure -textvariable $var_iemgui_min_rng }
        if {[eval concat $$var_iemgui_max_rng] < $minimumflashHold} {
            set $var_iemgui_max_rng $minimumflashHold
            $mytoplevel.rng.max_ent configure -textvariable $var_iemgui_max_rng
        }
        if {[eval concat $$var_iemgui_min_rng] < $minimumFlashBreak} {
            set $var_iemgui_min_rng $minimumFlashBreak
            $mytoplevel.rng.min_ent configure -textvariable $var_iemgui_min_rng
        }
    }
    if {[eval concat $$var_iemgui_rng_sch] == 1} {
        if {[eval concat $$var_iemgui_min_rng] == 0.0} {
            set $var_iemgui_min_rng 1.0
            $mytoplevel.rng.min_ent configure -textvariable $var_iemgui_min_rng
        }
    }
}

proc verify_rng {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]
    global $var_iemgui_min_rng
    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]
    global $var_iemgui_max_rng
    set var_iemgui_lin0_log1 [concat iemgui_lin0_log1_$vid]
    global $var_iemgui_lin0_log1
    
    if {[eval concat $$var_iemgui_lin0_log1] == 1} {
        if {[eval concat $$var_iemgui_max_rng] == 0.0 && [eval concat $$var_iemgui_min_rng] == 0.0} {
            set $var_iemgui_max_rng 1.0
            $mytoplevel.rng.max_ent configure -textvariable $var_iemgui_max_rng
        }
        if {[eval concat $$var_iemgui_max_rng] > 0} {
            if {[eval concat $$var_iemgui_min_rng] <= 0} {
                set $var_iemgui_min_rng [expr [eval concat $$var_iemgui_max_rng] * 0.01]
                $mytoplevel.rng.min_ent configure -textvariable $var_iemgui_min_rng
            }
        } else {
            if {[eval concat $$var_iemgui_min_rng] > 0} {
                set $var_iemgui_max_rng [expr [eval concat $$var_iemgui_min_rng] * 0.01]
                $mytoplevel.rng.max_ent configure -textvariable $var_iemgui_max_rng
            }
        }
    }
}

proc clip_fontsize {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_gn_fs [concat iemgui_gn_fs_$vid]
    global $var_iemgui_gn_fs
    
    variable minimumFontSize
    
    if {[eval concat $$var_iemgui_gn_fs] < $minimumFontSize} {
        set $var_iemgui_gn_fs $minimumFontSize
        $mytoplevel.label.fs_ent configure -textvariable $var_iemgui_gn_fs
    }
}

proc set_col_example {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_bcol [concat iemgui_bcol_$vid]
    global $var_iemgui_bcol
    set var_iemgui_fcol [concat iemgui_fcol_$vid]
    global $var_iemgui_fcol
    set var_iemgui_lcol [concat iemgui_lcol_$vid]
    global $var_iemgui_lcol
    
    $mytoplevel.colors.sections.lb_bk configure \
        -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
        -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
        -foreground [format "#%6.6x" [eval concat $$var_iemgui_lcol]] \
        -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_lcol]]
    
    if { [eval concat $$var_iemgui_fcol] >= 0 } {
        $mytoplevel.colors.sections.fr_bk configure \
            -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -foreground [format "#%6.6x" [eval concat $$var_iemgui_fcol]] \
            -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_fcol]]
    } else {
        $mytoplevel.colors.sections.fr_bk configure \
            -background [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activebackground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -foreground [format "#%6.6x" [eval concat $$var_iemgui_bcol]] \
            -activeforeground [format "#%6.6x" [eval concat $$var_iemgui_bcol]]}
}

proc preset_col {mytoplevel presetcol} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_l2_f1_b0 [concat iemgui_l2_f1_b0_$vid]
    global $var_iemgui_l2_f1_b0
    set var_iemgui_bcol [concat iemgui_bcol_$vid]
    global $var_iemgui_bcol
    set var_iemgui_fcol [concat iemgui_fcol_$vid]
    global $var_iemgui_fcol
    set var_iemgui_lcol [concat iemgui_lcol_$vid]
    global $var_iemgui_lcol
    
    if { [eval concat $$var_iemgui_l2_f1_b0] == 0 } { set $var_iemgui_bcol $presetcol }
    if { [eval concat $$var_iemgui_l2_f1_b0] == 1 } { set $var_iemgui_fcol $presetcol }
    if { [eval concat $$var_iemgui_l2_f1_b0] == 2 } { set $var_iemgui_lcol $presetcol }
    ::pd_iem::set_col_example $mytoplevel
}

proc choose_col_bkfrlb {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_l2_f1_b0 [concat iemgui_l2_f1_b0_$vid]
    global $var_iemgui_l2_f1_b0
    set var_iemgui_bcol [concat iemgui_bcol_$vid]
    global $var_iemgui_bcol
    set var_iemgui_fcol [concat iemgui_fcol_$vid]
    global $var_iemgui_fcol
    set var_iemgui_lcol [concat iemgui_lcol_$vid]
    global $var_iemgui_lcol
    
    if {[eval concat $$var_iemgui_l2_f1_b0] == 0} {
        set $var_iemgui_bcol [expr [eval concat $$var_iemgui_bcol] & 0xFCFCFC]
        set helpstring [tk_chooseColor -title [_ "Background color"] -initialcolor [format "#%6.6x" [eval concat $$var_iemgui_bcol]]]
        if { $helpstring ne "" } {
            set $var_iemgui_bcol [string replace $helpstring 0 0 "0x"]
            set $var_iemgui_bcol [expr [eval concat $$var_iemgui_bcol] & 0xFCFCFC] }
    }
    if {[eval concat $$var_iemgui_l2_f1_b0] == 1} {
        set $var_iemgui_fcol [expr [eval concat $$var_iemgui_fcol] & 0xFCFCFC]
        set helpstring [tk_chooseColor -title [_ "Foreground color"] -initialcolor [format "#%6.6x" [eval concat $$var_iemgui_fcol]]]
        if { $helpstring ne "" } {
            set $var_iemgui_fcol [string replace $helpstring 0 0 "0x"]
            set $var_iemgui_fcol [expr [eval concat $$var_iemgui_fcol] & 0xFCFCFC] }
    }
    if {[eval concat $$var_iemgui_l2_f1_b0] == 2} {
        set $var_iemgui_lcol [expr [eval concat $$var_iemgui_lcol] & 0xFCFCFC]
        set helpstring [tk_chooseColor -title [_ "Label color"] -initialcolor [format "#%6.6x" [eval concat $$var_iemgui_lcol]]]
        if { $helpstring ne "" } {
            set $var_iemgui_lcol [string replace $helpstring 0 0 "0x"]
            set $var_iemgui_lcol [expr [eval concat $$var_iemgui_lcol] & 0xFCFCFC] }
    }
    ::pd_iem::set_col_example $mytoplevel
}

proc lilo {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_lin0_log1 [concat iemgui_lin0_log1_$vid]
    global $var_iemgui_lin0_log1
    set var_iemgui_lilo0 [concat iemgui_lilo0_$vid]
    global $var_iemgui_lilo0
    set var_iemgui_lilo1 [concat iemgui_lilo1_$vid]
    global $var_iemgui_lilo1
    
    ::pd_iem::sched_rng $mytoplevel
    
    if {[eval concat $$var_iemgui_lin0_log1] == 0} {
        set $var_iemgui_lin0_log1 1
        $mytoplevel.para.lilo configure -text [eval concat $$var_iemgui_lilo1]
        ::pd_iem::verify_rng $mytoplevel
        ::pd_iem::sched_rng $mytoplevel
    } else {
        set $var_iemgui_lin0_log1 0
        $mytoplevel.para.lilo configure -text [eval concat $$var_iemgui_lilo0]
    }
}

proc toggle_font {mytoplevel gn_f} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_gn_f [concat iemgui_gn_f_$vid]
    global $var_iemgui_gn_f
    
    set $var_iemgui_gn_f $gn_f
    
    switch -- $gn_f {
        0 { set current_font $::var(fontFamily)}
        1 { set current_font "Helvetica" }
        2 { set current_font "Times" }
    }
    set current_font_spec "{$current_font} 16 $::var(fontWeight)"
    
    $mytoplevel.label.fontpopup_label configure -text $current_font \
        -font $current_font_spec
    $mytoplevel.label.name_entry configure -font $current_font_spec
    $mytoplevel.colors.sections.fr_bk configure -font $current_font_spec
    $mytoplevel.colors.sections.lb_bk configure -font $current_font_spec
}

proc lb {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]
    global $var_iemgui_loadbang
    
    if {[eval concat $$var_iemgui_loadbang] == 0} {
        set $var_iemgui_loadbang 1
        $mytoplevel.para.lb configure -text [_ "Init"]
    } else {
        set $var_iemgui_loadbang 0
        $mytoplevel.para.lb configure -text [_ "No init"]
    }
}

proc stdy_jmp {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_steady [concat iemgui_steady_$vid]
    global $var_iemgui_steady
    
    if {[eval concat $$var_iemgui_steady]} {
        set $var_iemgui_steady 0
        $mytoplevel.para.stdy_jmp configure -text [_ "Jump on click"]
    } else {
        set $var_iemgui_steady 1
        $mytoplevel.para.stdy_jmp configure -text [_ "Steady on click"]
    }
}

proc apply {mytoplevel} {
    set vid [string trimleft $mytoplevel .]
    
    set var_iemgui_wdt [concat iemgui_wdt_$vid]
    global $var_iemgui_wdt
    set var_iemgui_min_wdt [concat iemgui_min_wdt_$vid]
    global $var_iemgui_min_wdt
    set var_iemgui_hgt [concat iemgui_hgt_$vid]
    global $var_iemgui_hgt
    set var_iemgui_min_hgt [concat iemgui_min_hgt_$vid]
    global $var_iemgui_min_hgt
    set var_iemgui_min_rng [concat iemgui_min_rng_$vid]
    global $var_iemgui_min_rng
    set var_iemgui_max_rng [concat iemgui_max_rng_$vid]
    global $var_iemgui_max_rng
    set var_iemgui_lin0_log1 [concat iemgui_lin0_log1_$vid]
    global $var_iemgui_lin0_log1
    set var_iemgui_lilo0 [concat iemgui_lilo0_$vid]
    global $var_iemgui_lilo0
    set var_iemgui_lilo1 [concat iemgui_lilo1_$vid]
    global $var_iemgui_lilo1
    set var_iemgui_loadbang [concat iemgui_loadbang_$vid]
    global $var_iemgui_loadbang
    set var_iemgui_num [concat iemgui_num_$vid]
    global $var_iemgui_num
    set var_iemgui_steady [concat iemgui_steady_$vid]
    global $var_iemgui_steady
    set var_iemgui_snd [concat iemgui_snd_$vid]
    global $var_iemgui_snd
    set var_iemgui_rcv [concat iemgui_rcv_$vid]
    global $var_iemgui_rcv
    set var_iemgui_gui_nam [concat iemgui_gui_nam_$vid]
    global $var_iemgui_gui_nam
    set var_iemgui_gn_dx [concat iemgui_gn_dx_$vid]
    global $var_iemgui_gn_dx
    set var_iemgui_gn_dy [concat iemgui_gn_dy_$vid]
    global $var_iemgui_gn_dy
    set var_iemgui_gn_f [concat iemgui_gn_f_$vid]
    global $var_iemgui_gn_f
    set var_iemgui_gn_fs [concat iemgui_gn_fs_$vid]
    global $var_iemgui_gn_fs
    set var_iemgui_bcol [concat iemgui_bcol_$vid]
    global $var_iemgui_bcol
    set var_iemgui_fcol [concat iemgui_fcol_$vid]
    global $var_iemgui_fcol
    set var_iemgui_lcol [concat iemgui_lcol_$vid]
    global $var_iemgui_lcol
    
    ::pd_iem::clip_dim $mytoplevel
    ::pd_iem::clip_num $mytoplevel
    ::pd_iem::sched_rng $mytoplevel
    ::pd_iem::verify_rng $mytoplevel
    ::pd_iem::sched_rng $mytoplevel
    ::pd_iem::clip_fontsize $mytoplevel
    
    if {[eval concat $$var_iemgui_snd] == ""} {set hhhsnd "empty"} else {set hhhsnd [eval concat $$var_iemgui_snd]}
    if {[eval concat $$var_iemgui_rcv] == ""} {set hhhrcv "empty"} else {set hhhrcv [eval concat $$var_iemgui_rcv]}
    if {[eval concat $$var_iemgui_gui_nam] == ""} {set hhhgui_nam "empty"
    } else {
        set hhhgui_nam [eval concat $$var_iemgui_gui_nam]}
    
    if {[string index $hhhsnd 0] == "$"} {
        set hhhsnd [string replace $hhhsnd 0 0 #] }
    if {[string index $hhhrcv 0] == "$"} {
        set hhhrcv [string replace $hhhrcv 0 0 #] }
    if {[string index $hhhgui_nam 0] == "$"} {
        set hhhgui_nam [string replace $hhhgui_nam 0 0 #] }
    
    set hhhsnd [::unspace $hhhsnd]
    set hhhrcv [::unspace $hhhrcv]
    set hhhgui_nam [::unspace $hhhgui_nam]

# make sure the offset boxes have a value
    if {[eval concat $$var_iemgui_gn_dx] eq ""} {set $var_iemgui_gn_dx 0}
    if {[eval concat $$var_iemgui_gn_dy] eq ""} {set $var_iemgui_gn_dy 0}

    ::pd_connect::pdsend [concat $mytoplevel dialog \
            [eval concat $$var_iemgui_wdt] \
            [eval concat $$var_iemgui_hgt] \
            [eval concat $$var_iemgui_min_rng] \
            [eval concat $$var_iemgui_max_rng] \
            [eval concat $$var_iemgui_lin0_log1] \
            [eval concat $$var_iemgui_loadbang] \
            [eval concat $$var_iemgui_num] \
            $hhhsnd \
            $hhhrcv \
            $hhhgui_nam \
            [eval concat $$var_iemgui_gn_dx] \
            [eval concat $$var_iemgui_gn_dy] \
            [eval concat $$var_iemgui_gn_f] \
            [eval concat $$var_iemgui_gn_fs] \
            [eval concat $$var_iemgui_bcol] \
            [eval concat $$var_iemgui_fcol] \
            [eval concat $$var_iemgui_lcol] \
            [eval concat $$var_iemgui_steady]]
}


proc cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

proc ok {mytoplevel} {
    ::pd_iem::apply $mytoplevel
    ::pd_iem::cancel $mytoplevel
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
