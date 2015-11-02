
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# The PureData console.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_console 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_console:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export initialize
namespace export post

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    toplevel .console -class PdConsole
    
    wm title    .console [_ "PureData"]
    wm minsize  .console 400 75
    wm geometry .console =500x400+20+50
    
    .console configure -menu .menubar

    scrollbar   .console.scroll     -command ".console.text.internal yview"
    text        .console.text       -font [getFont 12] \
                                    -borderwidth 0 \
                                    -insertwidth 0 \
                                    -highlightthickness 0 \
                                    -undo 0 \
                                    -yscrollcommand ".console.scroll set"
        
    pack .console.scroll            -side right -fill y
    pack .console.text              -side right -fill both  -expand 1
    
    # Read-only text widget ( http://wiki.tcl.tk/1152 ).
    
    rename ::.console.text ::.console.text.internal
    
    proc ::.console.text {args} {
        switch -exact -- [lindex $args 0] {
            "insert"  {}
            "delete"  {}
            "default" { return [eval ::.console.text.internal $args] }
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc post {message} {
    .console.text.internal insert end $message
    .console.text.internal insert end "\n"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
