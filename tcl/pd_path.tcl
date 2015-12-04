
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Manage the search path.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_path 1.0

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

    ::pd_path::closed
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {} {

    toplevel .path -class PdTool
    wm title .path [_ "Path"]
    wm group .path .
        
    wm minsize  .path 400 300
    wm geometry .path [format "=400x300%s" [::rightNextTo .console]]
    
    ttk::frame      .path.f                 {*}[::styleMainFrame]
    
    ttk::labelframe .path.f.paths           {*}[::styleLabelFrame]  -text [_ "Search Paths"]
    ttk::frame      .path.f.actions         {*}[::styleFrame]
    
    listbox         .path.f.paths.box       -font [::mainFont] \
                                            -selectmode single \
                                            -activestyle none \
                                            -borderwidth 0
    
    button          .path.f.actions.add     -text "Add..." \
                                            -command "::pd_path::_addItem"
    button          .path.f.actions.delete  -text "Delete" \
                                            -command "::pd_path::_deleteItem"
    
    pack .path.f                    -side top -fill both -expand 1
    pack .path.f.paths              -side top -fill both -expand 1
    pack .path.f.actions            -side top -fill x 
    
    pack .path.f.paths.box          -side top -fill both -expand 1
    
    pack .path.f.actions.add        -side left
    pack .path.f.actions.delete     -side left

    foreach item $::var(searchPath) { .path.f.paths.box insert end $item }
    
    wm protocol .path WM_DELETE_WINDOW { ::pd_path::closed }
}

proc closed {{top {}}} {

    wm withdraw .path
    
    focus [lindex [wm stackorder .] end]
        
    set ::var(isPath) 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _addItem {} {

    set item [tk_chooseDirectory -title [_ "Add a Directory"]]
    
    focus .path
    
    if {$item ne ""} { .path.f.paths.box insert end $item }
    
    ::pd_path::_apply
}

proc _deleteItem {} {

    foreach item [.path.f.paths.box curselection] { .path.f.paths.box delete $item }
    
    ::pd_path::_apply
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {} {

    set ::var(searchPath) {}
    
    foreach path [.path.f.paths.box get 0 end] { lappend ::var(searchPath) [::encoded $path] }

    ::pd_connect::pdsend "pd path-dialog $::var(searchPath)"
    
    ::pd_connect::pdsend "pd save-preferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

