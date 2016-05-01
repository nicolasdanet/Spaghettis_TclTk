
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Scalar properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_data 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_data:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top content} {

    ::ui_data::_create $top $content
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top content} {

    toplevel $top -class PdData
    wm title $top [_ "Data"]
    wm group $top .
    
    wm minsize  $top {*}[::styleMinimumSize]
    wm geometry $top [format "=600x400%s" [::rightNextTo $::var(windowFocused)]]

    text $top.text  -font [::styleFontText] \
                    -borderwidth 0 \
                    -highlightthickness 0
    
    pack $top.text  -side left -fill both -expand 1

    $top.text insert end $content
    
    wm protocol $top WM_DELETE_WINDOW   "::ui_data::closed $top"
        
    focus $top.text
}

proc closed {top} {

    # ::ui_data::_apply $top
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    for {set i 1} {[$top.text compare $i.end < end]} {incr i 1} {
        set line [$top.text get $i.0 $i.end]
        if {$line != ""} {
            ::ui_interface::pdsend "$top _data $line"
        }
    }
    
    ::ui_interface::pdsend "$top _end"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
