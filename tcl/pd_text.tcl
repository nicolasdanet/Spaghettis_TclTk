
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 2002-2012 krzYszcz and others.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Text for qlist and textfile objects.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_text 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_text:: {

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

proc append {top contents} {

    if {[winfo exists $top]} { $top.text insert end $contents }
}

proc clear {top} {

    if {[winfo exists $top]} { $top.text delete 1.0 end }
}

proc dirty {top flag} {

    if {[winfo exists $top]} { $top.text edit modified $flag }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top geometry title fontSize} {
    
    toplevel $top -class PdText
    wm title $top $title
    wm group $top .
     
    wm minsize  $top 50 50
    wm geometry $top $geometry
    
    text $top.text  -font [::getFont $fontSize] \
                    -yscrollcommand "$top.scroll set" \
                    -borderwidth 0 \
                    -highlightthickness 0
                    
    scrollbar $top.scroll   -command "$top.text yview"
    
    pack $top.text   -side left -fill both -expand 1
    pack $top.scroll -side left -fill y

    bind $top.text  <<Save>>            "::pd_text::_save $top"
    bind $top       <<Modified>>        "::pd_text::_modified $top"
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_text::_closed $top"
        
    focus $top.text
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _modified {top} {

    if {[tk windowingsystem] eq "aqua"} {
        wm attributes $top -modified [$top.text edit modified]
    }
}

proc _save {top} {

    ::pd_connect::pdsend "$top clear"
        
    for {set i 1} {[$top.text compare $i.end < end]} {incr i 1} {
        set line [$top.text get $i.0 $i.end]
        if {$line != ""} {
            set line [string map {"," " \\, " ";" " \\; " "$" "\\$"} $line]
            ::pd_connect::pdsend "$top addline $line"
        }
    }
    
    ::pd_text::dirty $top 0
}

proc _closed {top} {

    if {[$top.text edit modified]} { 
        ::dialog_confirm::checkClose $top {::pd_text::_save $top} {} {return -level 2}
    }

    ::pd_connect::pdsend "$top close"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
