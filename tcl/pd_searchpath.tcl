
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
    
    frame $top.f
    
    listbox $top.f.box  -selectmode browse \
                        -activestyle dotbox \
                        -yscrollcommand "$top.f.scrollbar set"

    scrollbar $top.f.scrollbar  -command "$top.f.box yview"
    
    frame $top.actions 
    
    button $top.actions.add_path -text {New...} \
        -command "::pd_searchpath::add_item $top $add"
    button $top.actions.edit_path -text {Edit...} \
        -command "::pd_searchpath::edit_item $top $edit"
    button $top.actions.delete_path -text {Delete} \
        -command "::pd_searchpath::delete_item $top"
        
    # Populate with items.
    
    foreach item $data { $top.f.box insert end $item }

    # Bind to events.
    
    bind $top.f.box <ButtonPress>       "::pd_searchpath::click $top %x %y"
    bind $top.f.box <ButtonRelease>     "::pd_searchpath::release $top %x %y"
    bind $top.f.box <Return>            "::pd_searchpath::edit_item $top $edit"
    bind $top.f.box <<DoubleClick>>     "::pd_searchpath::dbl_click $top $edit $add %x %y"
    bind $top.f.box <<Delete>>          "::pd_searchpath::delete_item $top"

    bind $top <Configure>               "$top.f.box see active"

    pack $top.f.box $top.f.scrollbar -side left -fill y -anchor w
    pack $top.f.box -side left -fill both -expand 1
    pack $top.f -side top -pady 2m -padx 2m -fill both -expand 1
    pack $top.actions.delete_path -side right -pady 2m
    pack $top.actions.edit_path -side right -pady 2m
    pack $top.actions.add_path -side right -pady 2m
    pack $top.actions -side top -padx 2m -fill x 

    $top.f.box activate end
    $top.f.box selection set end
    focus $top.f.box
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc get_curidx { mytoplevel } {
    set idx [$mytoplevel.f.box index active]
    if {$idx < 0 || \
            $idx == [$mytoplevel.f.box index end]} {
        return [expr {[$mytoplevel.f.box index end] + 1}]
    }
    return [expr $idx]
}

proc insert_item { mytoplevel idx name } {
    if {$name != ""} {
        $mytoplevel.f.box insert $idx $name
        set activeIdx [expr {[$mytoplevel.f.box index active] + 1}]
        $mytoplevel.f.box see $activeIdx
        $mytoplevel.f.box activate $activeIdx
        $mytoplevel.f.box selection clear 0 end
        $mytoplevel.f.box selection set active
        focus $mytoplevel.f.box
    }
}

proc add_item { mytoplevel add_method } {
    set dir [$add_method]
    insert_item $mytoplevel [expr {[get_curidx $mytoplevel] + 1}] $dir
}

proc edit_item { mytoplevel edit_method } {
    set idx [expr {[get_curidx $mytoplevel]}]
    set initialValue [$mytoplevel.f.box get $idx]
    if {$initialValue != ""} {
        set dir [$edit_method $initialValue]

        if {$dir != ""} {
            $mytoplevel.f.box delete $idx
            insert_item $mytoplevel $idx $dir
        }
        $mytoplevel.f.box activate $idx
        $mytoplevel.f.box selection clear 0 end
        $mytoplevel.f.box selection set active
        focus $mytoplevel.f.box
    }
}

proc delete_item { mytoplevel } {
    set cursel [$mytoplevel.f.box curselection]
    foreach idx $cursel {
        $mytoplevel.f.box delete $idx
    }
}

# Double-clicking on the listbox should edit the current item,
# or add a new one if there is no current
proc dbl_click { mytoplevel edit_method add_method x y } {
    if { $x == "" || $y == "" } {
        return
    }

    set curBB [$mytoplevel.f.box bbox @$x,$y]

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
    
    set lastIndex [$mytoplevel.f.box index @$x,$y]

    focus $mytoplevel.f.box
}

# For drag-and-drop reordering, recall the last-clicked index
# and move it to the position of the item currently under the mouse
proc release { mytoplevel x y } {
    variable lastIndex 
    set curIdx [$mytoplevel.f.box index @$x,$y]

    if { $curIdx != $lastIndex  } {
        # clear any current selection
        $mytoplevel.f.box selection clear 0 end

        set oldIdx $lastIndex 
        set newIdx [expr {$curIdx+1}]
        set selIdx $curIdx

        if { $curIdx < $lastIndex  } {
            set oldIdx [expr {$lastIndex  + 1}]
            set newIdx $curIdx
            set selIdx $newIdx
        }

        $mytoplevel.f.box insert $newIdx [$mytoplevel.f.box get $lastIndex ]
        $mytoplevel.f.box delete $oldIdx
        $mytoplevel.f.box activate $newIdx
        $mytoplevel.f.box selection set $selIdx
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

