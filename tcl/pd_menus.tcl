
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_menus 0.1

package require pd_commands

# TODO figure out Undo/Redo/Cut/Copy/Paste state changes for menus

# since there is one menubar that is used for all windows, the menu -commands
# use {} quotes so that $::pd_gui(window_focused) is interpreted when the menu item
# is called, not when the command is mapped to the menu item.  This is the
# opposite of the 'bind' commands in pd_bindings.tcl
    
namespace eval ::pd_menus:: {
    variable accelerator
    variable menubar ".menubar"

    namespace export create_menubar
    namespace export configure_for_pdwindow
    namespace export configure_for_canvas
    namespace export configure_for_dialog

    # turn off tearoff menus globally
    option add *tearOff 0
}

# ------------------------------------------------------------------------------
# 
proc ::pd_menus::create_menubar {} {
    variable accelerator
    variable menubar
    if {[tk windowingsystem] eq "aqua"} {
        set accelerator "Cmd"
    } else {
        set accelerator "Ctrl"
    }
    menu $menubar
    set menulist "file edit put find media window help"
    foreach mymenu $menulist {    
        menu $menubar.$mymenu
        $menubar add cascade -label [_ [string totitle $mymenu]] \
            -menu $menubar.$mymenu
        [format build_%s_menu $mymenu] $menubar.$mymenu
    }
    if {[tk windowingsystem] eq "aqua"} {create_apple_menu $menubar}
    if {[tk windowingsystem] eq "win32"} {create_system_menu $menubar}
    . configure -menu $menubar
}

proc ::pd_menus::configure_for_pdwindow {} {
    variable menubar
    # these are meaningless for the Pd window, so disable them
    # File menu
    $menubar.file entryconfigure [_ "Save"] -state disabled
    $menubar.file entryconfigure [_ "Save As..."] -state normal
    $menubar.file entryconfigure [_ "Print..."] -state disabled
    $menubar.file entryconfigure [_ "Close"] -state disabled
    # Edit menu
    $menubar.edit entryconfigure [_ "Duplicate"] -state disabled
    $menubar.edit entryconfigure [_ "Tidy Up"] -state disabled
    $menubar.edit entryconfigure [_ "Edit Mode"] -state disabled
    ::pdtk_canvas::pdtk_canvas_editmode .pdwindow 0
    # Undo/Redo change names, they need to have the asterisk (*) after
    $menubar.edit entryconfigure 0 -state disabled -label [_ "Undo"]
    $menubar.edit entryconfigure 1 -state disabled -label [_ "Redo"]
    # disable everything on the Put menu
    for {set i 0} {$i <= [$menubar.put index end]} {incr i} {
        # catch errors that happen when trying to disable separators
        catch {$menubar.put entryconfigure $i -state disabled }
    }
}

proc ::pd_menus::configure_for_canvas {mytoplevel} {
    variable menubar
    # File menu
    $menubar.file entryconfigure [_ "Save"] -state normal
    $menubar.file entryconfigure [_ "Save As..."] -state normal
    $menubar.file entryconfigure [_ "Print..."] -state normal
    $menubar.file entryconfigure [_ "Close"] -state normal
    # Edit menu
    $menubar.edit entryconfigure [_ "Duplicate"] -state normal
    $menubar.edit entryconfigure [_ "Tidy Up"] -state normal
    $menubar.edit entryconfigure [_ "Edit Mode"] -state normal
    ::pdtk_canvas::pdtk_canvas_editmode $mytoplevel $::patch_is_editmode($mytoplevel)
    # Put menu
    for {set i 0} {$i <= [$menubar.put index end]} {incr i} {
        # catch errors that happen when trying to disable separators
        if {[$menubar.put type $i] ne "separator"} {
            $menubar.put entryconfigure $i -state normal 
        }
    }
    update_undo_on_menu $mytoplevel
}

proc ::pd_menus::configure_for_dialog {mytoplevel} {
    variable menubar
    # these are meaningless for the dialog panels, so disable them except for
    # the ones that make senes in the Find dialog panel
    # File menu
    if {$mytoplevel ne ".find"} {
        $menubar.file entryconfigure [_ "Save"] -state disabled
        $menubar.file entryconfigure [_ "Save As..."] -state disabled
        $menubar.file entryconfigure [_ "Print..."] -state disabled
    }
    $menubar.file entryconfigure [_ "Close"] -state disabled
    # Edit menu
    $menubar.edit entryconfigure [_ "Duplicate"] -state disabled
    $menubar.edit entryconfigure [_ "Tidy Up"] -state disabled
    $menubar.edit entryconfigure [_ "Edit Mode"] -state disabled
    ::pdtk_canvas::pdtk_canvas_editmode $mytoplevel 0
    # Undo/Redo change names, they need to have the asterisk (*) after
    $menubar.edit entryconfigure 0 -state disabled -label [_ "Undo"]
    $menubar.edit entryconfigure 1 -state disabled -label [_ "Redo"]
    # disable everything on the Put menu
    for {set i 0} {$i <= [$menubar.put index end]} {incr i} {
        # catch errors that happen when trying to disable separators
        catch {$menubar.put entryconfigure $i -state disabled }
    }
}


# ------------------------------------------------------------------------------
# menu building functions
proc ::pd_menus::build_file_menu {mymenu} {
    # run the platform-specific build_file_menu_* procs first, and config them
    [format build_file_menu_%s [tk windowingsystem]] $mymenu
    $mymenu entryconfigure [_ "New"]        -command {::pd_commands::menu_new}
    $mymenu entryconfigure [_ "Open"]       -command {::pd_commands::menu_open}
    $mymenu entryconfigure [_ "Save"]       -command {::pd_commands::menu_send $::pd_gui(window_focused) menusave}
    $mymenu entryconfigure [_ "Save As..."] -command {::pd_commands::menu_send $::pd_gui(window_focused) menusaveas}
    #$mymenu entryconfigure [_ "Revert*"]    -command {::pd_commands::menu_revert $::pd_gui(window_focused)}
    $mymenu entryconfigure [_ "Close"]      -command {::pd_commands::menu_send_float $::pd_gui(window_focused) menuclose 0}
    $mymenu entryconfigure [_ "Message..."] -command {::pd_commands::menu_message_dialog}
    $mymenu entryconfigure [_ "Print..."]   -command {::pd_commands::menu_print $::pd_gui(window_focused)}
    # update recent files
    if {[llength $::pd_gui(file_recent)] > 0} {
        ::pd_menus::update_recentfiles_menu false
    }
}

proc ::pd_menus::build_edit_menu {mymenu} {
    variable accelerator
    $mymenu add command -label [_ "Undo"]       -accelerator "$accelerator+Z" \
        -command {::pd_commands::menu_undo $::pd_gui(window_focused)}
    $mymenu add command -label [_ "Redo"]       -accelerator "Shift+$accelerator+Z" \
        -command {::pd_commands::menu_redo $::pd_gui(window_focused)}
    $mymenu add  separator
    $mymenu add command -label [_ "Cut"]        -accelerator "$accelerator+X" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) cut}
    $mymenu add command -label [_ "Copy"]       -accelerator "$accelerator+C" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) copy}
    $mymenu add command -label [_ "Paste"]      -accelerator "$accelerator+V" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) paste}
    $mymenu add command -label [_ "Duplicate"]  -accelerator "$accelerator+D" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) duplicate}
    $mymenu add command -label [_ "Select All"] -accelerator "$accelerator+A" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) selectall}
    $mymenu add  separator
    if {[tk windowingsystem] eq "aqua"} {
#        $mymenu add command -label [_ "Text Editor"] \
#            -command {::pd_commands::menu_texteditor}
        $mymenu add command -label [_ "Font"]  -accelerator "$accelerator+T" \
            -command {::pd_commands::menu_font_dialog}
    } else {
#        $mymenu add command -label [_ "Text Editor"] -accelerator "$accelerator+T"\
#            -command {::pd_commands::menu_texteditor}
        $mymenu add command -label [_ "Font"] \
            -command {::pd_commands::menu_font_dialog}
    }
    $mymenu add command -label [_ "Tidy Up"] \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) tidy}
    $mymenu add command -label [_ "Clear Console"] \
        -accelerator "Shift+$accelerator+L" -command {menu_clear_console}
    $mymenu add  separator
    #TODO madness! how to set the state of the check box without invoking the menu!
    $mymenu add check -label [_ "Edit Mode"] -accelerator "$accelerator+E" \
        -variable ::pd_gui(is_editmode) \
        -command {::pd_commands::menu_editmode $::pd_gui(is_editmode)}
    if {[tk windowingsystem] ne "aqua"} {
        $mymenu add  separator
        create_preferences_menu $mymenu.preferences
        $mymenu add cascade -label [_ "Preferences"] -menu $mymenu.preferences
    }
}

proc ::pd_menus::build_put_menu {mymenu} {
    variable accelerator
    # The trailing 0 in ::pd_commands::menu_send_float basically means leave the object box
    # sticking to the mouse cursor. The iemguis alway do that when created
    # from the menu, as defined in canvas_iemguis()
    $mymenu add command -label [_ "Object"] -accelerator "$accelerator+1" \
        -command {::pd_commands::menu_send_float $::pd_gui(window_focused) obj 0} 
    $mymenu add command -label [_ "Message"] -accelerator "$accelerator+2" \
        -command {::pd_commands::menu_send_float $::pd_gui(window_focused) msg 0}
    $mymenu add command -label [_ "Number"] -accelerator "$accelerator+3" \
        -command {::pd_commands::menu_send_float $::pd_gui(window_focused) floatatom 0}
    $mymenu add command -label [_ "Symbol"] -accelerator "$accelerator+4" \
        -command {::pd_commands::menu_send_float $::pd_gui(window_focused) symbolatom 0}
    $mymenu add command -label [_ "Comment"] -accelerator "$accelerator+5" \
        -command {::pd_commands::menu_send_float $::pd_gui(window_focused) text 0}
    $mymenu add  separator
    $mymenu add command -label [_ "Bang"]    -accelerator "Shift+$accelerator+B" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) bng}
    $mymenu add command -label [_ "Toggle"]  -accelerator "Shift+$accelerator+T" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) toggle}
    $mymenu add command -label [_ "Number2"] -accelerator "Shift+$accelerator+N" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) numbox}
    $mymenu add command -label [_ "Vslider"] -accelerator "Shift+$accelerator+V" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) vslider}
    $mymenu add command -label [_ "Hslider"] -accelerator "Shift+$accelerator+H" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) hslider}
    $mymenu add command -label [_ "Vradio"]  -accelerator "Shift+$accelerator+D" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) vradio}
    $mymenu add command -label [_ "Hradio"]  -accelerator "Shift+$accelerator+I" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) hradio}
    $mymenu add command -label [_ "VU Meter"] -accelerator "Shift+$accelerator+U"\
        -command {::pd_commands::menu_send $::pd_gui(window_focused) vumeter}
    $mymenu add command -label [_ "Canvas"]  -accelerator "Shift+$accelerator+C" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) mycnv}
    $mymenu add  separator
    $mymenu add command -label [_ "Graph"] -command {::pd_commands::menu_send $::pd_gui(window_focused) graph}
    $mymenu add command -label [_ "Array"] -command {::pd_commands::menu_send $::pd_gui(window_focused) menuarray}
}

proc ::pd_menus::build_find_menu {mymenu} {
    variable accelerator
    $mymenu add command -label [_ "Find..."]    -accelerator "$accelerator+F" \
        -command {::pd_commands::menu_find_dialog}
    $mymenu add command -label [_ "Find Again"] -accelerator "$accelerator+G" \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) findagain}
    $mymenu add command -label [_ "Find Last Error"] \
        -command {::pd_connect::pdsend {pd finderror}} 
}

proc ::pd_menus::build_media_menu {mymenu} {
    variable accelerator
    $mymenu add radiobutton -label [_ "DSP On"] -accelerator "$accelerator+/" \
        -variable ::pd_gui(is_dsp) -value 1 -command {::pd_connect::pdsend "pd dsp 1"}
    $mymenu add radiobutton -label [_ "DSP Off"] -accelerator "$accelerator+." \
        -variable ::pd_gui(is_dsp) -value 0 -command {::pd_connect::pdsend "pd dsp 0"}

    set audio_apilist_length [llength $::pd_gui(api_audio_list)]
    if {$audio_apilist_length > 0} {$mymenu add separator}
    for {set x 0} {$x<$audio_apilist_length} {incr x} {
        $mymenu add radiobutton -label [lindex [lindex $::pd_gui(api_audio_list) $x] 0] \
            -command {::pd_commands::menu_audio 0} -variable ::pd_gui(api_audio) \
            -value [lindex [lindex $::pd_gui(api_audio_list) $x] 1]\
            -command {::pd_connect::pdsend "pd audio-setapi $::pd_gui(api_audio)"}
    }
    
    set midi_api_length [llength $::pd_gui(api_midi_list)]
    if {$midi_api_length > 0} {$mymenu add separator}
    for {set x 0} {$x<$midi_api_length} {incr x} {
        $mymenu add radiobutton -label [lindex [lindex $::pd_gui(api_midi_list) $x] 0] \
            -command {::pd_commands::menu_midi 0} -variable ::pd_gui(api_midi) \
            -value [lindex [lindex $::pd_gui(api_midi_list) $x] 1]\
            -command {::pd_connect::pdsend "pd midi-setapi $::pd_gui(api_midi)"}
    }

    $mymenu add  separator
    $mymenu add command -label [_ "Audio Settings..."] \
        -command {::pd_connect::pdsend "pd audio-properties"}
    $mymenu add command -label [_ "MIDI Settings..."] \
        -command {::pd_connect::pdsend "pd midi-properties"}
}

proc ::pd_menus::build_window_menu {mymenu} {
    variable accelerator
    if {[tk windowingsystem] eq "aqua"} {
        $mymenu add command -label [_ "Minimize"] -accelerator "$accelerator+M"\
            -command {::pd_commands::menu_minimize $::pd_gui(window_focused)}
        $mymenu add command -label [_ "Zoom"] \
            -command {::pd_commands::menu_maximize $::pd_gui(window_focused)}
        $mymenu add  separator
        $mymenu add command -label [_ "Bring All to Front"] \
            -command {::pd_commands::menu_bringalltofront}
    } else {
		$mymenu add command -label [_ "Next Window"] \
            -command {::pd_commands::menu_raisenextwindow} \
            -accelerator [_ "$accelerator+Page Down"]
		$mymenu add command -label [_ "Previous Window"] \
            -command {::pd_commands::menu_raisepreviouswindow} \
            -accelerator [_ "$accelerator+Page Up"]
    }
    $mymenu add  separator
    $mymenu add command -label [_ "Pd window"] -command {::pd_commands::menu_raise_pdwindow} \
        -accelerator "$accelerator+R"
    $mymenu add command -label [_ "Parent Window"] \
        -command {::pd_commands::menu_send $::pd_gui(window_focused) findparent}
    $mymenu add  separator
}

proc ::pd_menus::build_help_menu {mymenu} {
    if {[tk windowingsystem] ne "aqua"} {
        $mymenu add command -label [_ "About Pd"] -command {::pd_commands::menu_aboutpd} 
    }
    $mymenu add command -label [_ "List of objects..."] \
        -command {::pd_connect::pdsend "pd help-intro"} 
    $mymenu add  separator
    $mymenu add command -label [_ "puredata.info"] \
        -command {::pd_commands::menu_openfile {http://puredata.info}} 
    $mymenu add command -label [_ "Report a bug"] -command {::pd_commands::menu_openfile \
        {http://sourceforge.net/tracker/?func=add&group_id=55736&atid=478070}} 
    $mymenu add  separator
    $mymenu add command -label [_ "Tcl prompt"] -command \
        {::pd_console::create_tcl_entry} 

}

#------------------------------------------------------------------------------#
# undo/redo menu items

proc ::pd_menus::update_undo_on_menu {mytoplevel} {
    variable menubar
    if {$mytoplevel eq $::pd_gui(undomanager_toplevel) && $::pd_gui(undomanager_undo) ne "no"} {
        $menubar.edit entryconfigure 0 -state normal \
            -label [_ "Undo $::pd_gui(undomanager_undo)"]
    } else {
        $menubar.edit entryconfigure 0 -state disabled -label [_ "Undo"]
    }
    if {$mytoplevel eq $::pd_gui(undomanager_toplevel) && $::pd_gui(undomanager_redo) ne "no"} {
        $menubar.edit entryconfigure 1 -state normal \
            -label [_ "Redo $::pd_gui(undomanager_redo)"]
    } else {
        $menubar.edit entryconfigure 1 -state disabled -label [_ "Redo"]
    }
}

# ------------------------------------------------------------------------------
# update the menu entries for opening recent files (write arg should always be true except the first time when pd is opened)
proc ::pd_menus::update_recentfiles_menu {{write true}} {
    variable menubar
    switch -- [tk windowingsystem] {
        "aqua"  {::pd_menus::update_openrecent_menu_aqua .openrecent $write}
        "win32" {::pd_menus::update_recentfiles_on_menu $menubar.file $write}
        "x11"   {::pd_menus::update_recentfiles_on_menu $menubar.file $write}
    }
}

proc ::pd_menus::clear_recentfiles_menu {} {
    set ::pd_gui(file_recent) {}
    ::pd_menus::update_recentfiles_menu
    # empty recentfiles in preferences (write empty array)
    ::pd_preferences::write_recentfiles
}

proc ::pd_menus::update_openrecent_menu_aqua {mymenu {write}} {
    if {! [winfo exists $mymenu]} {menu $mymenu}
    $mymenu delete 0 end

    # now the list is last first so we just add
    foreach filename $::pd_gui(file_recent) {
        $mymenu add command -label [file tail $filename] \
            -command "open_file {$filename}"
    }
    # clear button
    $mymenu add  separator
    $mymenu add command -label [_ "Clear Menu"] \
        -command "::pd_menus::clear_recentfiles_menu"
    # write to config file
    if {$write == true} { ::pd_preferences::write_recentfiles }
}

# ------------------------------------------------------------------------------
# this expects to be run on the File menu, and to insert above the last separator
proc ::pd_menus::update_recentfiles_on_menu {mymenu {write}} {
    set lastitem [$mymenu index end]
    set i 1
    while {[$mymenu type [expr $lastitem-$i]] ne "separator"} {incr i}
    set bottom_separator [expr $lastitem-$i]
    incr i

    while {[$mymenu type [expr $lastitem-$i]] ne "separator"} {incr i}
    set top_separator [expr $lastitem-$i]
    if {$top_separator < [expr $bottom_separator-1]} {
        $mymenu delete [expr $top_separator+1] [expr $bottom_separator-1]
    }
    # insert the list from the end because we insert each element on the top
    set i [llength $::pd_gui(file_recent)]
    while {[incr i -1] > 0} {

        set filename [lindex $::pd_gui(file_recent) $i]
        $mymenu insert [expr $top_separator+1] command \
            -label [file tail $filename] -command "open_file {$filename}"
    }
    set filename [lindex $::pd_gui(file_recent) 0]
    $mymenu insert [expr $top_separator+1] command \
        -label [file tail $filename] -command "open_file {$filename}"

    # write to config file
    if {$write == true} { ::pd_preferences::write_recentfiles }
}

# ------------------------------------------------------------------------------
# lots of crazy recursion to update the Window menu

# find the first parent patch that has a mapped window
proc ::pd_menus::find_mapped_parent {parentlist} {
    if {[llength $parentlist] == 0} {return "none"}
    set firstparent [lindex $parentlist 0]
    if {[winfo exists $firstparent]} {
        return $firstparent
    } elseif {[llength $parentlist] > 1} {
        return [find_mapped_parent [lrange $parentlist 1 end]]
    } else {
        # we must be the first menu item to be inserted
        return "none"
    }
}

# find the first parent patch that has a mapped window
proc ::pd_menus::insert_into_menu {mymenu entry parent} {
    set insertat [$mymenu index end]
    for {set i 0} {$i <= [$mymenu index end]} {incr i} {
        if {[$mymenu type $i] ne "command"} {continue}
        set currentcommand [$mymenu entrycget $i -command]
        if {$currentcommand eq "raise $entry"} {return} ;# it exists already
        if {$currentcommand eq "raise $parent"} {
            set insertat $i
        }
    }
    incr insertat
    set label ""
    for {set i 0} {$i < [llength $::patch_parents($entry)]} {incr i} {
        append label " "
    }
    append label $::patch_name($entry)
    $mymenu insert $insertat command -label $label -command "raise $entry"
}

# recurse through a list of parent windows and add to the menu
proc ::pd_menus::add_list_to_menu {mymenu window parentlist} {
    if {[llength $parentlist] == 0} {
        insert_into_menu $mymenu $window {}
    } else {
        set entry [lindex $parentlist end]
        if {[winfo exists $entry]} {
            insert_into_menu $mymenu $entry \
                [find_mapped_parent $::patch_parents($entry)]
        }
    }
    if {[llength $parentlist] > 1} {
        add_list_to_menu $mymenu $window [lrange $parentlist 0 end-1]
    }
}

# update the list of windows on the Window menu. This expects run on the
# Window menu, and to insert below the last separator
proc ::pd_menus::update_window_menu {} {
    set mymenu .menubar.window
    # find the last separator and delete everything after that
    for {set i 0} {$i <= [$mymenu index end]} {incr i} {
        if {[$mymenu type $i] eq "separator"} {
            set deleteat $i
        }
    }
    $mymenu delete $deleteat end
    $mymenu add  separator
    foreach window [array names ::patch_parents] {
        set parentlist $::patch_parents($window)
        add_list_to_menu $mymenu $window $parentlist
        insert_into_menu $mymenu $window [find_mapped_parent $parentlist]
    }
}

# ------------------------------------------------------------------------------
# submenu for Preferences, now used on all platforms

proc ::pd_menus::create_preferences_menu {mymenu} {
    menu $mymenu
    $mymenu add command -label [_ "Path..."] \
        -command {::pd_connect::pdsend "pd start-path-dialog"}
    $mymenu add command -label [_ "Startup..."] \
        -command {::pd_connect::pdsend "pd start-startup-dialog"}
    $mymenu add command -label [_ "Audio Settings..."] \
        -command {::pd_connect::pdsend "pd audio-properties"}
    $mymenu add command -label [_ "MIDI Settings..."] \
        -command {::pd_connect::pdsend "pd midi-properties"}
    $mymenu add  separator
    $mymenu add command -label [_ "Save All Settings"] \
        -command {::pd_connect::pdsend "pd save-preferences"}
}

# ------------------------------------------------------------------------------
# menu building functions for Mac OS X/aqua

# for Mac OS X only
proc ::pd_menus::create_apple_menu {mymenu} {
    # TODO this should open a Pd patch called about.pd
    menu $mymenu.apple
    $mymenu.apple add command -label [_ "About Pd"] -command {::pd_commands::menu_aboutpd}
    $mymenu.apple add  separator
    create_preferences_menu $mymenu.apple.preferences
    $mymenu.apple add cascade -label [_ "Preferences"] \
        -menu $mymenu.apple.preferences
    # this needs to be last for things to function properly
    $mymenu add cascade -label "Apple" -menu $mymenu.apple
    
}

proc ::pd_menus::build_file_menu_aqua {mymenu} {
    variable accelerator
    $mymenu add command -label [_ "New"]       -accelerator "$accelerator+N"
    $mymenu add command -label [_ "Open"]      -accelerator "$accelerator+O"
    # this is now done in main ::pd_menus::build_file_menu
    #::pd_menus::update_openrecent_menu_aqua .openrecent
    $mymenu add cascade -label [_ "Open Recent"] -menu .openrecent
    $mymenu add  separator
    $mymenu add command -label [_ "Close"]     -accelerator "$accelerator+W"
    $mymenu add command -label [_ "Save"]      -accelerator "$accelerator+S"
    $mymenu add command -label [_ "Save As..."] -accelerator "$accelerator+Shift+S"
    #$mymenu add command -label [_ "Save All"]
    #$mymenu add command -label [_ "Revert to Saved"]
    $mymenu add  separator
    $mymenu add command -label [_ "Message..."]
    $mymenu add  separator
    $mymenu add command -label [_ "Print..."]   -accelerator "$accelerator+P"
}

# the "Edit", "Put", and "Find" menus do not have cross-platform differences

proc ::pd_menus::build_media_menu_aqua {mymenu} {
}

proc ::pd_menus::build_window_menu_aqua {mymenu} {
}

# the "Help" does not have cross-platform differences
 
# ------------------------------------------------------------------------------
# menu building functions for UNIX/X11

proc ::pd_menus::build_file_menu_x11 {mymenu} {
    variable accelerator
    $mymenu add command -label [_ "New"]        -accelerator "$accelerator+N"
    $mymenu add command -label [_ "Open"]       -accelerator "$accelerator+O"
    $mymenu add  separator
    $mymenu add command -label [_ "Save"]       -accelerator "$accelerator+S"
    $mymenu add command -label [_ "Save As..."] -accelerator "Shift+$accelerator+S"
    #    $mymenu add command -label "Revert"
    $mymenu add  separator
    $mymenu add command -label [_ "Message..."]    -accelerator "$accelerator+M"
    create_preferences_menu $mymenu.preferences
    $mymenu add cascade -label [_ "Preferences"] -menu $mymenu.preferences
    $mymenu add command -label [_ "Print..."]   -accelerator "$accelerator+P"
    $mymenu add  separator
    # the recent files get inserted in here by update_recentfiles_on_menu
    $mymenu add  separator
    $mymenu add command -label [_ "Close"]      -accelerator "$accelerator+W"
    $mymenu add command -label [_ "Quit"]       -accelerator "$accelerator+Q" \
        -command {::pd_connect::pdsend "pd verifyquit"}
}

# the "Edit", "Put", and "Find" menus do not have cross-platform differences

proc ::pd_menus::build_media_menu_x11 {mymenu} {
}

proc ::pd_menus::build_window_menu_x11 {mymenu} {
}

# the "Help" does not have cross-platform differences

# ------------------------------------------------------------------------------
# menu building functions for Windows/Win32

# for Windows only
proc ::pd_menus::create_system_menu {mymenubar} {
    set mymenu $mymenubar.system
    $mymenubar add cascade -label System -menu $mymenu
    menu $mymenu -tearoff 0
    # placeholders
    $mymenu add command -label [_ "Edit Mode"] -command "::pd_console::verbose 0 systemmenu"
    # TODO add Close, Minimize, etc and whatever else is on the little menu
    # that is on the top left corner of the window frame
    # http://wiki.tcl.tk/1006
    # TODO add Edit Mode here
}

proc ::pd_menus::build_file_menu_win32 {mymenu} {
    variable accelerator
    $mymenu add command -label [_ "New"]      -accelerator "$accelerator+N"
    $mymenu add command -label [_ "Open"]     -accelerator "$accelerator+O"
    $mymenu add  separator
    $mymenu add command -label [_ "Save"]      -accelerator "$accelerator+S"
    $mymenu add command -label [_ "Save As..."] -accelerator "Shift+$accelerator+S"
    #    $mymenu add command -label "Revert"
    $mymenu add  separator
    $mymenu add command -label [_ "Message..."]  -accelerator "$accelerator+M"
    create_preferences_menu $mymenu.preferences
    $mymenu add cascade -label [_ "Preferences"] -menu $mymenu.preferences
    $mymenu add command -label [_ "Print..."] -accelerator "$accelerator+P"
    $mymenu add  separator
    # the recent files get inserted in here by update_recentfiles_on_menu
    $mymenu add  separator
    $mymenu add command -label [_ "Close"]    -accelerator "$accelerator+W"
    $mymenu add command -label [_ "Quit"]     -accelerator "$accelerator+Q"\
        -command {::pd_connect::pdsend "pd verifyquit"}
}

# the "Edit", "Put", and "Find" menus do not have cross-platform differences

proc ::pd_menus::build_media_menu_win32 {mymenu} {
}

proc ::pd_menus::build_window_menu_win32 {mymenu} {
}

# the "Help" does not have cross-platform differences
