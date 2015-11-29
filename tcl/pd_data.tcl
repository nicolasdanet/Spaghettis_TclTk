
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Scalar properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_data 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_data:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top content} {

    ::pd_data::_create $top $content
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {mytoplevel stuff} {

    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel [_ "Data Properties"]
    wm group $mytoplevel $::var(windowFocused)
    $mytoplevel configure -padx 0 -pady 0

    frame $mytoplevel.buttonframe
    pack $mytoplevel.buttonframe -side bottom -fill x -pady 2m
    button $mytoplevel.buttonframe.send -text [_ "Send (Ctrl s)"] \
        -command "::pd_data::send $mytoplevel"
    button $mytoplevel.buttonframe.ok -text [_ "OK (Ctrl t)"] \
        -command "::pd_data::ok $mytoplevel"
    pack $mytoplevel.buttonframe.send -side left -expand 1
    pack $mytoplevel.buttonframe.ok -side left -expand 1

    text $mytoplevel.text -relief raised -bd 2 -height 40 -width 60 \
        -yscrollcommand "$mytoplevel.scroll set"
    scrollbar $mytoplevel.scroll -command "$mytoplevel.text yview"
    pack $mytoplevel.scroll -side right -fill y
    pack $mytoplevel.text -side left -fill both -expand 1
    $mytoplevel.text insert end $stuff
    focus $mytoplevel.text
    bind $mytoplevel.text <Control-t> "::pd_data::ok $mytoplevel"
    bind $mytoplevel.text <Control-s> "::pd_data::send $mytoplevel"
}

proc _closed {top} {

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

proc send {mytoplevel} {
    for {set i 1} {[$mytoplevel.text compare [concat $i.0 + 3 chars] < end]} \
        {incr i 1} {
            ::pd_connect::pdsend "$mytoplevel data [$mytoplevel.text get $i.0 [expr $i + 1].0]"
        }
    ::pd_connect::pdsend "$mytoplevel end"
}

proc ok {mytoplevel} {
    ::pd_data::send $mytoplevel
    ::pd_data::_cancel $mytoplevel
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
