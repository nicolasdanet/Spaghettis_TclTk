
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 2002-2012 krzYszcz and others.

# Adapted from krzYszcz's code for coll in cyclone.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_text 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_text:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export open

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc open {name geometry title font} {

    ::pd_console::post $name
    
    if {[winfo exists $name]} {
        $name.text delete 1.0 end
    } else {
        _create $name $geometry $title $font
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {name geometry title font} {
    
    toplevel $name
    wm title $name $title
    wm geometry $name $geometry
    wm protocol $name WM_DELETE_WINDOW \
        [concat pdtk_textwindow_close $name 1]
    bind $name <<Modified>> "pdtk_textwindow_dodirty $name"
    text $name.text -relief raised -bd 2 \
        -font [getFont $font] \
        -yscrollcommand "$name.scroll set" -background white
    scrollbar $name.scroll -command "$name.text yview"
    pack $name.scroll -side right -fill y
    pack $name.text -side left -fill both -expand 1
    bind $name.text <<Save>> "pdtk_textwindow_send $name"
    bind $name.text <<Close>> "pdtk_textwindow_close $name 1"
    focus $name.text
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pdtk_textwindow_dodirty {name} {
    if {[catch {$name.text edit modified} dirty]} {set dirty 1}
    set title [wm title $name]
    set dt [string equal -length 1 $title "*"]
    if {$dirty} {
        if {$dt == 0} {wm title $name *$title}
    } else {
        if {$dt} {wm title $name [string range $title 1 end]}
    }
}

proc pdtk_textwindow_setdirty {name flag} {
    if {[winfo exists $name]} {
        catch {$name.text edit modified $flag}
    }
}

proc pdtk_textwindow_doclose {name} {
    destroy $name
    ::pd_connect::pdsend [concat $name signoff]
}

proc pdtk_textwindow_append {name contents} {
    if {[winfo exists $name]} {
        $name.text insert end $contents
    }
}

proc pdtk_textwindow_clear {name} {
    if {[winfo exists $name]} {
        $name.text delete 1.0 end
    }
}

proc pdtk_textwindow_send {name} {
    if {[winfo exists $name]} {
        ::pd_connect::pdsend [concat $name clear]
        for {set i 1} \
         {[$name.text compare $i.end < end]} \
              {incr i 1} {
            set lin [$name.text get $i.0 $i.end]
            if {$lin != ""} {
                set lin [string map {"," " \\, " ";" " \\; " "$" "\\$"} $lin]
                ::pd_connect::pdsend [concat $name addline $lin]
            }
        }
    }
    pdtk_textwindow_setdirty $name 0
}

proc pdtk_textwindow_close {name ask} {
    if {[winfo exists $name]} {
        if {[catch {$name.text edit modified} dirty]} {set dirty 1}
        if {$ask && $dirty} {
            set title [wm title $name]
            if {[string equal -length 1 $title "*"]} {
                set title [string range $title 1 end]
            }
            set answer [tk_messageBox \-type yesnocancel \
             \-icon question \
             \-message [concat Save changes to \"$title\"?]]
            if {$answer == "yes"} {pdtk_textwindow_send $name}
            if {$answer != "cancel"} {::pd_connect::pdsend [concat $name close]}
        } else {::pd_connect::pdsend [concat $name close]}
    }
}
