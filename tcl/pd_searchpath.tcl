
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

namespace export initialize

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {mytoplevel listdata add_method edit_method commit_method} {

    # Add the pd_searchpath widget
    ::pd_searchpath::make $mytoplevel $listdata $add_method

    # Use two frames for the buttons, since we want them both
    # bottom and right
    frame $mytoplevel.nb
    pack $mytoplevel.nb -side bottom -fill x -pady 2m

    frame $mytoplevel.nb.buttonframe
    pack $mytoplevel.nb.buttonframe -side right -padx 2m

    button $mytoplevel.nb.buttonframe.cancel -text [_ "Cancel"]\
        -command "::pd_searchpath::cancel $mytoplevel"
    button $mytoplevel.nb.buttonframe.apply -text [_ "Apply"]\
        -command "::pd_searchpath::apply $mytoplevel $commit_method"
    button $mytoplevel.nb.buttonframe.ok -text [_ "OK"]\
        -command "::pd_searchpath::ok $mytoplevel $commit_method"

    pack $mytoplevel.nb.buttonframe.cancel -side left -expand 1 -padx 2m
    pack $mytoplevel.nb.buttonframe.apply -side left -expand 1 -padx 2m
    pack $mytoplevel.nb.buttonframe.ok -side left -expand 1 -padx 2m
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc get_listdata {mytoplevel} {
    return [$mytoplevel.paths.box get 0 end]
}

proc do_apply {mytoplevel commit_method listdata} {
    $commit_method [pdtk_encode $listdata]
    ::pd_connect::pdsend "pd save-preferences"
}

# Cancel button action
proc cancel {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel cancel"
}

# Apply button action
proc apply {mytoplevel commit_method } {
    do_apply $mytoplevel $commit_method [get_listdata $mytoplevel]
}

# OK button action
# The "commit" action can take a second or more,
# long enough to be noticeable, so we only write
# the changes after closing the dialog
proc ok {mytoplevel commit_method } {
    set listdata [get_listdata $mytoplevel]
    cancel $mytoplevel
    do_apply $mytoplevel $commit_method $listdata
}

# "Constructor" function for building the window
# id -- the window id to use
# listdata -- the data used to populate the pd_searchpath
# add_method -- a reference to a proc to be called when the user adds a new item
# edit_method -- same as above, for editing and existing item
# commit_method -- same as above, to commit during the "apply" action
# title -- top-level title for the dialog
# width, height -- initial width and height dimensions for the window, also minimum size


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

proc pdtk_encode { listdata } {
    set outlist {}
    foreach this_path $listdata {
        if {0==[string match "" $this_path]} {
            lappend outlist [::encode $this_path]
        }
    }
    return $outlist
}
