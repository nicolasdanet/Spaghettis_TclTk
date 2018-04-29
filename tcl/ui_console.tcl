
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# The application console.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_console 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_console:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} { ::ui_console::_create }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc post {message} { 

    .console.text insert end $message basicLog
    .console.text insert end "\n"
    
    after idle ::ui_console::_update
}

proc warning {message} { 

    .console.text insert end $message warningLog
    .console.text insert end "\n"
    
    after idle ::ui_console::_update
}

proc error {message} { 

    .console.text insert end $message errorLog
    .console.text insert end "\n"
    
    after idle ::ui_console::_update
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {} {

    toplevel .console -class Spaghettis
    wm title .console "Spaghettis"
    wm group .console .
    
    wm minsize  .console {*}[::styleMinimumSize]
    wm geometry .console "=400x300+75+75"
    
    .console configure -menu .menubar

    ttk::scrollbar  .console.scroll     -command ".console.text yview"
    text            .console.text       -font [::styleFontConsole] \
                                        -borderwidth 0 \
                                        -insertwidth 0 \
                                        -highlightthickness 0 \
                                        -undo 0 \
                                        -yscrollcommand ".console.scroll set"
        
    pack .console.text                  -side right -fill both  -expand 1
    pack .console.scroll                -side right -fill y     -before .console.text
    
    bind .console <<SelectAll>> ".console.text tag add sel 1.0 end"
    
    wm protocol .console WM_DELETE_WINDOW   { ::ui_console::closed }
    
    # Set the color layout. 
    
    .console.text tag configure errorLog    -foreground red
    .console.text tag configure warningLog  -foreground red
    .console.text tag configure basicLog    -foreground black
}

proc closed {} {

    ::ui_interface::pdsend "pd _quit"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _update {} {

    .console.text yview end
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
