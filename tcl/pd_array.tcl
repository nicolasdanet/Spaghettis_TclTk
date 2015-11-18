
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_array 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_array:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable drawMode
variable saveContents

array set drawMode     {}
array set saveContents {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {mytoplevel name size flags newone} {

    variable drawMode
    variable saveContents
    
    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel $newone
    }

    $mytoplevel.name.entry insert 0 [::dialog_gatom::unescape $name]
    $mytoplevel.size.entry insert 0 $size
    set saveContents($mytoplevel) [expr $flags & 1]
    set drawMode($mytoplevel) [expr ( $flags & 6 ) >> 1]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc apply {mytoplevel} {
    
    variable drawMode
    variable saveContents
    
    ::pd_connect::pdsend "$mytoplevel arraydialog \
            [::dialog_gatom::escape [$mytoplevel.name.entry get]] \
            [$mytoplevel.size.entry get] \
            [expr $saveContents($mytoplevel) + (2 * $drawMode($mytoplevel))]"
}

proc cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

proc ok {mytoplevel} {
    ::pd_array::apply $mytoplevel
    ::pd_array::cancel $mytoplevel
}

proc create_dialog {mytoplevel newone} {

    variable drawMode
    variable saveContents
    
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
        -variable ::pd_array::saveContents($mytoplevel) -anchor w
    pack $mytoplevel.saveme -side top

    labelframe $mytoplevel.drawas -text [_ "Draw as:"] -padx 20 -borderwidth 1
    pack $mytoplevel.drawas -side top -fill x
    radiobutton $mytoplevel.drawas.points -value 0 \
        -variable ::pd_array::drawMode($mytoplevel) -text [_ "Polygon"]
    radiobutton $mytoplevel.drawas.polygon -value 1 \
        -variable ::pd_array::drawMode($mytoplevel) -text [_ "Points"]
    radiobutton $mytoplevel.drawas.bezier -value 2 \
        -variable ::pd_array::drawMode($mytoplevel) -text [_ "Bezier curve"]
    pack $mytoplevel.drawas.points -side top -anchor w
    pack $mytoplevel.drawas.polygon -side top -anchor w
    pack $mytoplevel.drawas.bezier -side top -anchor w

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
