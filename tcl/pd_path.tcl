
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
    
    if {[winfo exists .path]} { ::bringToFront .path } else { ::pd_path::_create }
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
    wm geometry .path [format "=500x300%s" [::rightNextTo .console]]
    
    ttk::frame      .path.f                 {*}[::styleMainFrame]
    ttk::labelframe .path.f.paths           {*}[::styleFrame] \
                                            -text [_ [::ifAqua "Folders" "Directories"]]
    listbox         .path.f.paths.list      -selectmode extended \
                                            -activestyle none \
                                            -borderwidth 0
    
    pack .path.f                -side top -fill both -expand 1
    pack .path.f.paths          -side top -fill both -expand 1
    pack .path.f.paths.list     -side top -fill both -expand 1

    foreach item $::var(searchPath) { .path.f.paths.list insert end $item }
    
    bind .path.f.paths.list <BackSpace>         "::pd_path::_deleteItems"
    bind .path.f.paths.list <Double-Button-1>   "::pd_path::_addItem"
    bind .path.f.paths.list <Triple-Button-1>   {}
    
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

    .path.f.paths.list selection clear 0 end
    
    set item [tk_chooseDirectory -title [_ "Add a Directory"]]
    
    if {$item ne ""} { .path.f.paths.list insert end $item; .path.f.paths.list selection set end }
    
    ::pd_path::_apply
}

proc _deleteItems {} {

    set i 0
    
    foreach item [.path.f.paths.list curselection] { .path.f.paths.list delete [expr {$item - $i}]; incr i }
    
    ::pd_path::_apply
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {} {

    set ::var(searchPath) {}
    
    foreach path [.path.f.paths.list get 0 end] { lappend ::var(searchPath) [::encoded $path] }

    ::pd_connect::pdsend "pd path-dialog $::var(searchPath)"
    ::pd_connect::pdsend "pd save-preferences"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

