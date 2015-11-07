
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 2002-2012 krzYszcz and others.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_text 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_text:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export show
namespace export release
namespace export setDirty

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top geometry title fontSize} {

    if {[winfo exists $top]} {
        $top.text delete 1.0 end
    } else {
        _create $top $geometry $title $fontSize
    }
}

proc release {top} {

    destroy $top
    ::pd_connect::pdsend "$top signoff"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc setDirty {top flag} {

    if {[winfo exists $top]} { $top.text edit modified $flag }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top geometry title fontSize} {
    
    toplevel $top
    wm title $top $title
    wm group $top .
     
    wm minsize  $top 50 50
    wm geometry $top $geometry
    
    text $top.text  -font [getFont $fontSize] \
                    -yscrollcommand "$top.scroll set" \
                    -borderwidth 0 \
                    -highlightthickness 0 \
                    -background white
                    
    scrollbar $top.scroll   -command "$top.text yview"
    
    pack $top.text   -side left -fill both -expand 1
    pack $top.scroll -side left -fill y

    bind $top.text  <<Save>>            "pdtk_textwindow_send $top"
    bind $top       <<Modified>>        "::pd_text::_dirty $top"
    
    wm protocol $top WM_DELETE_WINDOW   "pdtk_textwindow_close $top 1"
        
    focus $top.text
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _dirty {top} {

    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $top -modified [$top.text edit modified]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------



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
    ::pd_text::setDirty $name 0
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
