
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_console 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_console:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {

    toplevel .console -class PdConsole
    
    set ::patch_name(.console) [_ "PureData"]
    
    wm title    .console [_ "PureData"]
    wm minsize  .console 400 75
    wm geometry .console =500x400+20+50
    
    .console configure -menu .menubar

    scrollbar   .console.scroll         -command ".console.text.internal yview"
    text        .console.text           -font [getFontDefault 12] \
                                        -borderwidth 0 \
                                        -insertwidth 0 \
                                        -highlightthickness 0 \
                                        -undo 0 \
                                        -yscrollcommand ".console.scroll set"
        
    pack .console.scroll                -side right -fill y
    pack .console.text                  -side right -fill both  -expand 1
    
    raise .console
    
    focus .console.text

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
    after cancel .console.text.internal yview
    .console.text.internal insert end $message
    after idle  .console.text.internal yview end
}

proc clear {} {
    .console.text.internal delete 0.0 end
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
