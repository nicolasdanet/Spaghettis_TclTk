
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

variable lastIndex 0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc make {top data add edit} {

    # Widgets.
    
    frame $top.paths
    frame $top.actions
        
    listbox $top.paths.box                  -selectmode browse \
                                            -activestyle dotbox \
                                            -yscrollcommand "$top.paths.scrollbar set"
    scrollbar $top.paths.scrollbar          -command "$top.paths.box yview"
    
    button $top.actions.add                 -text "New..." \
                                            -command "::pd_searchpath::add_item $top $add"
    button $top.actions.edit                -text "Edit..." \
                                            -command "::pd_searchpath::edit_item $top $edit"
    button $top.actions.delete              -text "Delete" \
                                            -command "::pd_searchpath::delete_item $top"
        
    # Bind to events.
    
    bind $top.paths.box <ButtonPress>       "::pd_searchpath::click $top %x %y"
    bind $top.paths.box <ButtonRelease>     "::pd_searchpath::release $top %x %y"
    bind $top.paths.box <Return>            "::pd_searchpath::edit_item $top $edit"
    bind $top.paths.box <Delete>            "::pd_searchpath::delete_item $top"
    
    bind $top           <Configure>         "$top.paths.box see active"

    # Populate with items.
    
    foreach item $data { $top.paths.box insert end $item }
    
    # Dispose.
    
    pack $top.paths -side top -pady 2m -padx 2m -fill both -expand 1
    pack $top.actions -side top -padx 2m -fill x 
        
    pack $top.paths.box $top.paths.scrollbar -side left -fill y -anchor w
    pack $top.paths.box -side left -fill both -expand 1
    
    pack $top.actions.delete -side right -pady 2m
    pack $top.actions.edit -side right -pady 2m
    pack $top.actions.add -side right -pady 2m


    $top.paths.box activate end
    $top.paths.box selection set end
    focus $top.paths.box
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc get_curidx { mytoplevel } {
    set idx [$mytoplevel.paths.box index active]
    if {$idx < 0 || \
            $idx == [$mytoplevel.paths.box index end]} {
        return [expr {[$mytoplevel.paths.box index end] + 1}]
    }
    return [expr $idx]
}

proc insert_item { mytoplevel idx name } {
    if {$name != ""} {
        $mytoplevel.paths.box insert $idx $name
        set activeIdx [expr {[$mytoplevel.paths.box index active] + 1}]
        $mytoplevel.paths.box see $activeIdx
        $mytoplevel.paths.box activate $activeIdx
        $mytoplevel.paths.box selection clear 0 end
        $mytoplevel.paths.box selection set active
        focus $mytoplevel.paths.box
    }
}

proc add_item { mytoplevel add_method } {
    set dir [$add_method]
    insert_item $mytoplevel [expr {[get_curidx $mytoplevel] + 1}] $dir
}

proc edit_item { mytoplevel edit_method } {
    set idx [expr {[get_curidx $mytoplevel]}]
    set initialValue [$mytoplevel.paths.box get $idx]
    if {$initialValue != ""} {
        set dir [$edit_method $initialValue]

        if {$dir != ""} {
            $mytoplevel.paths.box delete $idx
            insert_item $mytoplevel $idx $dir
        }
        $mytoplevel.paths.box activate $idx
        $mytoplevel.paths.box selection clear 0 end
        $mytoplevel.paths.box selection set active
        focus $mytoplevel.paths.box
    }
}

proc delete_item { mytoplevel } {
    set cursel [$mytoplevel.paths.box curselection]
    foreach idx $cursel {
        $mytoplevel.paths.box delete $idx
    }
}

# Double-clicking on the listbox should edit the current item,
# or add a new one if there is no current
proc dbl_click { mytoplevel edit_method add_method x y } {
    if { $x == "" || $y == "" } {
        return
    }

    set curBB [$mytoplevel.paths.box bbox @$x,$y]

    # listbox bbox returns an array of 4 items in the order:
    # left, top, width, height
    set height [lindex $curBB 3]
    set top [lindex $curBB 1]
    if { $height == "" || $top == "" } {
        # If for some reason we didn't get valid bbox info,
        # we want to default to adding a new item
        set height 0
        set top 0
        set y 1
    }

    set bottom [expr {$height + $top}]

    if {$y > $bottom} {
        add_item $mytoplevel $add_method
    } else {
        edit_item $mytoplevel $edit_method
    }
}

proc click { mytoplevel x y } {
    # record the index of the current element being
    # clicked on
    variable lastIndex 
    
    set lastIndex [$mytoplevel.paths.box index @$x,$y]

    focus $mytoplevel.paths.box
}

# For drag-and-drop reordering, recall the last-clicked index
# and move it to the position of the item currently under the mouse
proc release { mytoplevel x y } {
    variable lastIndex 
    set curIdx [$mytoplevel.paths.box index @$x,$y]

    if { $curIdx != $lastIndex  } {
        # clear any current selection
        $mytoplevel.paths.box selection clear 0 end

        set oldIdx $lastIndex 
        set newIdx [expr {$curIdx+1}]
        set selIdx $curIdx

        if { $curIdx < $lastIndex  } {
            set oldIdx [expr {$lastIndex  + 1}]
            set newIdx $curIdx
            set selIdx $newIdx
        }

        $mytoplevel.paths.box insert $newIdx [$mytoplevel.paths.box get $lastIndex ]
        $mytoplevel.paths.box delete $oldIdx
        $mytoplevel.paths.box activate $newIdx
        $mytoplevel.paths.box selection set $selIdx
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

