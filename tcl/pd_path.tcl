
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Search path list.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_path 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_path:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {{top {}}} { 
    
    ::pd_path::_create .paths
}

proc hide {} {

    destroy .paths
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top} {

    toplevel $top -class PdDialog
    wm title $top [_ "Path"]
    wm group $top .
    
    wm minsize  $top 400 300
    wm geometry $top "=400x300+30+60"
    
    frame $top.paths
    frame $top.actions
    
    listbox $top.paths.box      -selectmode single \
                                -activestyle none \
                                -font [::getFont 14] \
                                -borderwidth 0
    
    button $top.actions.add     -text "Add..." \
                                -command "::pd_path::_addItem  $top"
    button $top.actions.delete  -text "Delete" \
                                -command "::pd_path::_deleteItem $top"
        
    pack $top.paths             -side top -padx 2m -pady 2m -fill both -expand 1
    pack $top.actions           -side top -padx 2m -fill x 
    
    pack $top.paths.box         -side left -fill both -expand 1
    
    pack $top.actions.add       -side left -pady 2m
    pack $top.actions.delete    -side left -pady 2m

    foreach item $::var(searchPath) { $top.paths.box insert end $item }
    
    focus $top.paths.box
    
    bind $top <Destroy> { ::pd_path::_closed }
}

proc _closed {} {

    set ::var(hasPath) 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _addItem  {top} {

    set item [tk_chooseDirectory -title [_ "Add a Directory"]]
    
    if {$item ne ""} { $top.paths.box insert end $item }
    
    ::pd_path::_apply $top
}

proc _deleteItem {top} {

    foreach item [$top.paths.box curselection] { $top.paths.box delete $item }
    
    ::pd_path::_apply $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

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

