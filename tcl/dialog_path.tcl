
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide dialog_path 0.1

namespace eval ::dialog_path:: {
    variable use_standard_extensions_button 1
    variable verbose_button 0
}

############ pdtk_path_dialog -- run a path dialog #########

# set up the panel with the info from pd
proc ::dialog_path::pdtk_path_dialog {mytoplevel extrapath verbose} {
    global use_standard_extensions_button
    global verbose_button
    set use_standard_extensions_button $extrapath
    set verbose_button $verbose

    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel
    }
}

proc ::dialog_path::create_dialog {mytoplevel} {

    wm deiconify .console
    raise .console
    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel [_ "Pd search path for objects, help, fonts, and other files"]
    wm group $mytoplevel .
    wm transient $mytoplevel .console
    wm protocol $mytoplevel WM_DELETE_WINDOW "::pd_searchpath::cancel $mytoplevel"
    
    # Enforce a minimum size for the window
    wm minsize $mytoplevel 400 300

    # Set the current dimensions of the window
    wm geometry $mytoplevel "400x300"
    
    pd_searchpath::initialize $mytoplevel $::var(searchPath) \
        dialog_path::add dialog_path::edit dialog_path::commit
    
    frame $mytoplevel.extraframe
    pack $mytoplevel.extraframe -side bottom -pady 2m
    checkbutton $mytoplevel.extraframe.extra -text [_ "Use standard extensions"] \
        -variable use_standard_extensions_button -anchor w 
    checkbutton $mytoplevel.extraframe.verbose -text [_ "Verbose"] \
        -variable verbose_button -anchor w 
    pack $mytoplevel.extraframe.extra $mytoplevel.extraframe.verbose \
        -side left -expand 1
}



############ pdtk_path_dialog -- dialog window for search path #########
proc ::dialog_path::choosePath { currentpath title } {
    if {$currentpath == ""} {
        set currentpath "~"
    }
    return [tk_chooseDirectory -initialdir $currentpath -title $title]
}

proc ::dialog_path::add {} {
    return [::dialog_path::choosePath "" {Add a new path}]
}

proc ::dialog_path::edit { currentpath } {
    return [::dialog_path::choosePath $currentpath "Edit existing path \[$currentpath\]"]
}

proc ::dialog_path::commit { new_path } {
    global use_standard_extensions_button
    global verbose_button

    set ::var(searchPath) $new_path
    ::pd_connect::pdsend "pd path-dialog $use_standard_extensions_button $verbose_button $::var(searchPath)"
}

