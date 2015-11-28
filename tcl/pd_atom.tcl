
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Float atom property.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_atom 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_atom:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  atomPosition
array set atomPosition {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pdtk_gatom_dialog {mytoplevel initwidth initlower initupper \
                                     initgatomlabel_radio \
                                     initgatomlabel initreceive initsend} {
    
    variable atomPosition
    
    set atomPosition($mytoplevel) $initgatomlabel_radio

    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel
    }

    $mytoplevel.width.entry insert 0 $initwidth
    $mytoplevel.limits.lower.entry insert 0 $initlower
    $mytoplevel.limits.upper.entry insert 0 $initupper
    if {$initgatomlabel ne "-"} {
        $mytoplevel.gatomlabel.name.entry insert 0 \
            [::parseDash $initgatomlabel]
    }
    
    set atomPosition($mytoplevel) $initgatomlabel_radio

    if {$initsend ne "-"} {
        $mytoplevel.s_r.send.entry insert 0 \
            [::parseDash $initsend]
    }
    if {$initreceive ne "-"} {
        $mytoplevel.s_r.receive.entry insert 0 \
            [::parseDash $initreceive]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc create_dialog {mytoplevel} {
    
    variable atomPosition

    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel [_ "Atom Box Properties"]
    wm group $mytoplevel .
    wm resizable $mytoplevel 0 0
    $mytoplevel configure -padx 0 -pady 0

    frame $mytoplevel.width -height 7
    pack $mytoplevel.width -side top
    label $mytoplevel.width.label -text [_ "Width:"]
    entry $mytoplevel.width.entry -width 4
    pack $mytoplevel.width.label $mytoplevel.width.entry -side left

    labelframe $mytoplevel.limits -text [_ "Limits"] -padx 15 -pady 4 -borderwidth 1
    pack $mytoplevel.limits -side top -fill x
    frame $mytoplevel.limits.lower
    pack $mytoplevel.limits.lower -side left
    label $mytoplevel.limits.lower.label -text [_ "Lower:"]
    entry $mytoplevel.limits.lower.entry -width 7
    pack $mytoplevel.limits.lower.label $mytoplevel.limits.lower.entry -side left
    frame $mytoplevel.limits.upper
    pack $mytoplevel.limits.upper -side left
    label $mytoplevel.limits.upper.label -text [_ "Upper:"]
    entry $mytoplevel.limits.upper.entry -width 7
    pack $mytoplevel.limits.upper.label $mytoplevel.limits.upper.entry -side left

    labelframe $mytoplevel.gatomlabel -text [_ "Label"] -padx 5 -pady 5 -borderwidth 1
    pack $mytoplevel.gatomlabel -side top -fill x -pady 5
    frame $mytoplevel.gatomlabel.name
    pack $mytoplevel.gatomlabel.name -side top
    entry $mytoplevel.gatomlabel.name.entry -width 33
    pack $mytoplevel.gatomlabel.name.entry -side left
    frame $mytoplevel.gatomlabel.radio
    pack $mytoplevel.gatomlabel.radio -side top
    radiobutton $mytoplevel.gatomlabel.radio.left -value 0 -text [_ "Left   "] \
        -variable ::pd_atom::atomPosition($mytoplevel) -justify left -takefocus 0
    radiobutton $mytoplevel.gatomlabel.radio.right -value 1 -text [_ "Right"] \
        -variable ::pd_atom::atomPosition($mytoplevel) -justify left -takefocus 0
    radiobutton $mytoplevel.gatomlabel.radio.top -value 2 -text [_ "Top"] \
        -variable ::pd_atom::atomPosition($mytoplevel) -justify left -takefocus 0
    radiobutton $mytoplevel.gatomlabel.radio.bottom -value 3 -text [_ "Bottom"] \
        -variable ::pd_atom::atomPosition($mytoplevel) -justify left -takefocus 0
    pack $mytoplevel.gatomlabel.radio.left -side left -anchor w
    pack $mytoplevel.gatomlabel.radio.right -side right -anchor w
    pack $mytoplevel.gatomlabel.radio.top -side top -anchor w
    pack $mytoplevel.gatomlabel.radio.bottom -side bottom -anchor w

    labelframe $mytoplevel.s_r -text [_ "Messages"] -padx 5 -pady 5 -borderwidth 1
    pack $mytoplevel.s_r -side top -fill x
    frame $mytoplevel.s_r.send
    pack $mytoplevel.s_r.send -side top -anchor e
    label $mytoplevel.s_r.send.label -text [_ "Send symbol:"]
    entry $mytoplevel.s_r.send.entry -width 21
    pack $mytoplevel.s_r.send.entry $mytoplevel.s_r.send.label -side right

    frame $mytoplevel.s_r.receive
    pack $mytoplevel.s_r.receive -side top -anchor e
    label $mytoplevel.s_r.receive.label -text [_ "Receive symbol:"]
    entry $mytoplevel.s_r.receive.entry -width 21
    pack $mytoplevel.s_r.receive.entry $mytoplevel.s_r.receive.label -side right
    
    frame $mytoplevel.buttonframe -pady 5
    pack $mytoplevel.buttonframe -side top -fill x -expand 1 -pady 2m
    button $mytoplevel.buttonframe.cancel -text [_ "Cancel"] \
        -command "::pd_atom::cancel $mytoplevel"
    pack $mytoplevel.buttonframe.cancel -side left -expand 1 -fill x -padx 10
    if {[tk windowingsystem] ne "aqua"} {
        button $mytoplevel.buttonframe.apply -text [_ "Apply"] \
            -command "::pd_atom::apply $mytoplevel"
    pack $mytoplevel.buttonframe.apply -side left -expand 1 -fill x -padx 10
    }
    button $mytoplevel.buttonframe.ok -text [_ "OK"] \
        -command "::pd_atom::ok $mytoplevel"
    pack $mytoplevel.buttonframe.ok -side left -expand 1 -fill x -padx 10

    $mytoplevel.width.entry select from 0
    $mytoplevel.width.entry select adjust end
    focus $mytoplevel.width.entry
}

proc _closed {top} {

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc apply {mytoplevel} {
    
    variable atomPosition
    
    ::pd_connect::pdsend "$mytoplevel param \
        [$mytoplevel.width.entry get] \
        [$mytoplevel.limits.lower.entry get] \
        [$mytoplevel.limits.upper.entry get] \
        [::sanitized [::withDash [$mytoplevel.gatomlabel.name.entry get]]] \
        $atomPosition($mytoplevel) \
        [::sanitized [::withDash [$mytoplevel.s_r.receive.entry get]]] \
        [::sanitized [::withDash [$mytoplevel.s_r.send.entry get]]]"
}

proc cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

proc ok {mytoplevel} {
    ::pd_atom::apply $mytoplevel
    ::pd_atom::cancel $mytoplevel
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
