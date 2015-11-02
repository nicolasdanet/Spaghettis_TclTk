
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

namespace export open

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc open {top} { _create $top; ::pd_menu::disableSearchPath }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top} {

    toplevel $top -class PdDialog
    wm title $top [_ "Search Path"]
    wm group $top .
    
    wm minsize  $top 400 300
    wm geometry $top =400x300+30+60
    
    frame $top.paths
    frame $top.actions
    
    listbox $top.paths.box              -selectmode single \
                                        -activestyle none \
                                        -font [getFont 12] \
                                        -yscrollcommand "$top.paths.scrollbar set"
    scrollbar $top.paths.scrollbar      -command "$top.paths.box yview"
    
    button $top.actions.add             -text "Add..." \
                                        -command "::dialog_path::_add $top"
    button $top.actions.delete          -text "Delete" \
                                        -command "::dialog_path::_delete $top"
        
    pack $top.paths             -side top -padx 2m -pady 2m -fill both -expand 1
    pack $top.actions           -side top -padx 2m -fill x 
        
    pack $top.paths.box         -side left -fill both -expand 1
    pack $top.paths.scrollbar   -side left -fill y -anchor w
    
    pack $top.actions.add       -side left -pady 2m
    pack $top.actions.delete    -side left -pady 2m

    foreach item $::var(searchPath) { $top.paths.box insert end $item }
    
    focus $top.paths.box
    
    wm protocol $top WM_DELETE_WINDOW { ::pd_menu::enableSearchPath }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _add {top} {

    set item [tk_chooseDirectory -title [_ "Add a Directory"]]
    
    if {$item ne ""} { $top.paths.box insert end $item }
    
    ::dialog_path::_apply $top
}

proc _delete {top} {

    foreach item [$top.paths.box curselection] { $top.paths.box delete $item }
    
    ::dialog_path::_apply $top
}

proc _apply {top} {

    set ::var(searchPath) {}
    
    foreach path [$top.paths.box get 0 end] { lappend ::var(searchPath) [::encode $path] }

    ::pd_connect::pdsend "pd path-dialog $::var(searchPath)"
    ::pd_connect::pdsend "pd save-preferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

