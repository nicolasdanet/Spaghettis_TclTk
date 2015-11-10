
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Modal windows.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide dialog_confirm 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::dialog_confirm:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc checkAction {top message reply default} {

    set r [tk_messageBox -message $message -type yesno -default $default -icon question -parent $top]
    
    if {$r eq "yes"} {
        ::pd_connect::pdsend $reply
    }
}

proc checkClose {top ifYes ifNo ifCancel} {

    set message [format [_ "Save \"%s\" before closing?"] [::getTitle $top]]
    
    if {[winfo viewable $top]} {
        set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question -parent $top]
    } else {
        set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question]
    }

    switch -- $r {
        yes     { uplevel 0 $ifYes    }
        no      { uplevel 0 $ifNo     }
        cancel  { uplevel 0 $ifCancel }
    }
}

proc checkClosePatch {top reply} {

    set message [format [_ "Save \"%s\" before closing?"] [::getTitle $top]]
    
    if {[winfo viewable $top]} {
        set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question -parent $top]
    } else {
        set r [tk_messageBox -message $message -type yesnocancel -default "yes" -icon question]
    }

    switch -- $r {
        yes { ::pd_connect::pdsend "$top menusave 1" }
        no  { ::pd_connect::pdsend $reply }
        cancel {
        
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
