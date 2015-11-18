
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_array 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

array set saveme_button {}
array set drawas_button {}
array set otherflag_button {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_array:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {mytoplevel name size flags newone} {

    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel $newone
    }

    $mytoplevel.name.entry insert 0 [::dialog_gatom::unescape $name]
    $mytoplevel.size.entry insert 0 $size
    set ::saveme_button($mytoplevel) [expr $flags & 1]
    set ::drawas_button($mytoplevel) [expr ( $flags & 6 ) >> 1]
    set ::otherflag_button($mytoplevel) 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc apply {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel arraydialog \
            [::dialog_gatom::escape [$mytoplevel.name.entry get]] \
            [$mytoplevel.size.entry get] \
            [expr $::saveme_button($mytoplevel) + (2 * $::drawas_button($mytoplevel))] \
            $::otherflag_button($mytoplevel)"
}

proc cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

proc ok {mytoplevel} {
    ::pd_array::apply $mytoplevel
    ::pd_array::cancel $mytoplevel
}

proc create_dialog {mytoplevel newone} {
    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel [_ "Array Properties"]
    wm group $mytoplevel .
    wm resizable $mytoplevel 0 0
    wm transient $mytoplevel $::var(windowFocused)
    
    $mytoplevel configure -padx 0 -pady 0

    frame $mytoplevel.name
    pack $mytoplevel.name -side top
    label $mytoplevel.name.label -text [_ "Name:"]
    entry $mytoplevel.name.entry
    pack $mytoplevel.name.label $mytoplevel.name.entry -anchor w

    frame $mytoplevel.size
    pack $mytoplevel.size -side top
    label $mytoplevel.size.label -text [_ "Size:"]
    entry $mytoplevel.size.entry
    pack $mytoplevel.size.label $mytoplevel.size.entry -anchor w

    checkbutton $mytoplevel.saveme -text [_ "Save contents"] \
        -variable ::saveme_button($mytoplevel) -anchor w
    pack $mytoplevel.saveme -side top

    labelframe $mytoplevel.drawas -text [_ "Draw as:"] -padx 20 -borderwidth 1
    pack $mytoplevel.drawas -side top -fill x
    radiobutton $mytoplevel.drawas.points -value 0 \
        -variable ::drawas_button($mytoplevel) -text [_ "Polygon"]
    radiobutton $mytoplevel.drawas.polygon -value 1 \
        -variable ::drawas_button($mytoplevel) -text [_ "Points"]
    radiobutton $mytoplevel.drawas.bezier -value 2 \
        -variable ::drawas_button($mytoplevel) -text [_ "Bezier curve"]
    pack $mytoplevel.drawas.points -side top -anchor w
    pack $mytoplevel.drawas.polygon -side top -anchor w
    pack $mytoplevel.drawas.bezier -side top -anchor w

    if {$newone != 0} {
        labelframe $mytoplevel.radio -text [_ "Put array into:"] -padx 20 -borderwidth 1
        pack $mytoplevel.radio -side top -fill x
        radiobutton $mytoplevel.radio.radio0 -value 0 \
            -variable ::otherflag_button($mytoplevel) -text [_ "New graph"]
        radiobutton $mytoplevel.radio.radio1 -value 1 \
            -variable ::otherflag_button($mytoplevel) -text [_ "Last graph"]
        pack $mytoplevel.radio.radio0 -side top -anchor w
        pack $mytoplevel.radio.radio1 -side top -anchor w
    } else {    
        checkbutton $mytoplevel.deletearray -text [_ "Delete array"] \
            -variable ::otherflag_button($mytoplevel) -anchor w
        pack $mytoplevel.deletearray -side top
    }

    frame $mytoplevel.buttonframe
    pack $mytoplevel.buttonframe -side bottom -expand 1 -fill x -pady 2m
    button $mytoplevel.buttonframe.cancel -text [_ "Cancel"] \
        -command "::pd_array::cancel $mytoplevel"
    pack $mytoplevel.buttonframe.cancel -side left -expand 1 -fill x -padx 10
    if {$newone == 0 && [tk windowingsystem] ne "aqua"} {
        button $mytoplevel.buttonframe.apply -text [_ "Apply"] \
            -command "::pd_array::apply $mytoplevel"
        pack $mytoplevel.buttonframe.apply -side left -expand 1 -fill x -padx 10
    }
    button $mytoplevel.buttonframe.ok -text [_ "OK"]\
        -command "::pd_array::ok $mytoplevel"
    pack $mytoplevel.buttonframe.ok -side left -expand 1 -fill x -padx 10
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
