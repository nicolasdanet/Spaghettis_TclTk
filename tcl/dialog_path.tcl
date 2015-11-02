
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide dialog_path 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::dialog_path:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable use_standard_extensions_button 1
variable verbose_button 0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pdtk_path_dialog {mytoplevel extrapath verbose} {
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

proc create_dialog {mytoplevel} {

    # wm deiconify .console
    # raise .console
    toplevel $mytoplevel -class PdDialog
    wm title $mytoplevel [_ "Pd search path for objects, help, fonts, and other files"]
    # wm group $mytoplevel .
    # wm transient $mytoplevel .console
    
    wm protocol $mytoplevel WM_DELETE_WINDOW "::dialog_path::apply $mytoplevel dialog_path::commit"
    
    # Enforce a minimum size for the window
    wm minsize $mytoplevel 400 300

    # Set the current dimensions of the window
    wm geometry $mytoplevel "400x300"
    
    dialog_path::initialize $mytoplevel $::var(searchPath) dialog_path::add dialog_path::edit 
    
    frame $mytoplevel.extraframe
    pack $mytoplevel.extraframe -side bottom -pady 2m
    checkbutton $mytoplevel.extraframe.extra -text [_ "Use standard extensions"] \
        -variable use_standard_extensions_button -anchor w 
    checkbutton $mytoplevel.extraframe.verbose -text [_ "Verbose"] \
        -variable verbose_button -anchor w 
    pack $mytoplevel.extraframe.extra $mytoplevel.extraframe.verbose \
        -side left -expand 1
}

proc choosePath { currentpath title } {
    if {$currentpath == ""} {
        set currentpath "~"
    }
    return [tk_chooseDirectory -initialdir $currentpath -title $title]
}

proc add {} {
    return [::dialog_path::choosePath "" {Add a new path}]
}

proc edit { currentpath } {
    return [::dialog_path::choosePath $currentpath "Edit existing path \[$currentpath\]"]
}

proc commit { new_path } {
    global use_standard_extensions_button
    global verbose_button

    set ::var(searchPath) $new_path
    ::pd_connect::pdsend "pd path-dialog $use_standard_extensions_button $verbose_button $::var(searchPath)"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {top data add edit} {

    frame $top.paths
    frame $top.actions
    
    listbox $top.paths.box              -selectmode browse \
                                        -activestyle dotbox \
                                        -yscrollcommand "$top.paths.scrollbar set"
    scrollbar $top.paths.scrollbar      -command "$top.paths.box yview"
    
    button $top.actions.add             -text "New..." \
                                        -command "::dialog_path::_new $top $add"
    button $top.actions.delete          -text "Delete" \
                                        -command "::dialog_path::_delete $top"
        
    pack $top.paths             -side top -padx 2m -pady 2m -fill both -expand 1
    pack $top.actions           -side top -padx 2m -fill x 
        
    pack $top.paths.box         -side left -fill both -expand 1
    pack $top.paths.scrollbar   -side left -fill y -anchor w
    
    pack $top.actions.add       -side right -pady 2m
    pack $top.actions.delete    -side right -pady 2m

    foreach item $data { $top.paths.box insert end $item }
    
    focus $top.paths.box
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc apply {top commit} {

    set out {}
    foreach path [$top.paths.box get 0 end] { if {$path ne ""} { lappend out [::encode $path] } }
    $commit $out
    
    ::pd_connect::pdsend "pd save-preferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _new {top add} { _append $top [$add] }

proc _append {top item} {

    if {$item ne ""} { 
        $top.paths.box insert end $item
    }
}

proc _delete {top} {

    foreach item [$top.paths.box curselection] {
        $top.paths.box delete $item
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

