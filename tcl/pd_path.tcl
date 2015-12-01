
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Manage search path list.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_path 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_path:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# The top paramater provided is not used. 

proc show {{top {}}} { 
    
    if {[winfo exists .path]} {
        wm deiconify .path
        raise .path
    } else {
        ::pd_path::_create
    }
}

proc hide {} {

    ::pd_path::_closed
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {} {

    toplevel .path -class PdDialog
    wm title .path [_ "Path"]
    wm group .path .
        
    wm minsize  .path 400 300
    wm geometry .path [format "=400x300%s" [::rightNextTo .console]]
    
    frame .path.paths
    frame .path.actions
    
    listbox .path.paths.box     -selectmode single \
                                -activestyle none \
                                -font [::getFont 14] \
                                -borderwidth 0
    
    button .path.actions.add    -text "Add..." \
                                -command "::pd_path::_addItem"
    button .path.actions.delete -text "Delete" \
                                -command "::pd_path::_deleteItem"
        
    pack .path.paths            -side top -padx 2m -pady 2m -fill both -expand 1
    pack .path.actions          -side top -padx 2m -fill x 
    
    pack .path.paths.box        -side left -fill both -expand 1
    
    pack .path.actions.add      -side left -pady 2m
    pack .path.actions.delete   -side left -pady 2m

    foreach item $::var(searchPath) { .path.paths.box insert end $item }
    
    wm protocol .path WM_DELETE_WINDOW { ::pd_path::_closed }
}

proc _closed {} {

    wm withdraw .path
    
    focus [lindex [wm stackorder .] end]
        
    set ::var(hasPath) 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _addItem {} {

    set item [tk_chooseDirectory -title [_ "Add a Directory"]]
    
    focus .path
    
    if {$item ne ""} { .path.paths.box insert end $item }
    
    ::pd_path::_apply
}

proc _deleteItem {} {

    foreach item [.path.paths.box curselection] { .path.paths.box delete $item }
    
    ::pd_path::_apply
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {} {

    set ::var(searchPath) {}
    
    foreach path [.path.paths.box get 0 end] { lappend ::var(searchPath) [::encoded $path] }

    ::pd_connect::pdsend "pd path-dialog $::var(searchPath)"
    
    ::pd_connect::pdsend "pd save-preferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

