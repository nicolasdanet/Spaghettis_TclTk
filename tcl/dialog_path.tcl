
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

    pd_scrollboxwindow::make $mytoplevel $::pd_gui(directory_path) \
        dialog_path::add dialog_path::edit dialog_path::commit \
        [_ "Pd search path for objects, help, fonts, and other files"] \
        400 300
    
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

    set ::pd_gui(directory_path) $new_path
    ::pd_connect::pdsend "pd path-dialog $use_standard_extensions_button $verbose_button $::pd_gui(directory_path)"
}

