
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_searchpath 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval pd_searchpath {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc make {top data add} {

    frame $top.paths
    frame $top.actions
    
    listbox $top.paths.box              -selectmode browse \
                                        -activestyle dotbox \
                                        -yscrollcommand "$top.paths.scrollbar set"
    scrollbar $top.paths.scrollbar      -command "$top.paths.box yview"
    
    button $top.actions.add             -text "New..." \
                                        -command "::pd_searchpath::_new $top $add"
    button $top.actions.delete          -text "Delete" \
                                        -command "::pd_searchpath::_delete $top"
        
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

proc _new {top add} { _append $top [$add] }

proc _append {top item} {

    if {$item ne ""} { 
        $top.paths.box insert end $item
        $top.paths.box activate end
        $top.paths.box selection clear 0 end
        $top.paths.box selection set active
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

