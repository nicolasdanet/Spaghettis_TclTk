
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide dialog_startup 0.1

package require pd_scrollboxwindow

namespace eval dialog_startup {
    variable defeatrt_flag 0
}

########## pdtk_startup_dialog -- dialog window for startup options #########
# Create a simple modal window with an entry widget
# for editing/adding a startup command 
# (the next-best-thing to in-place editing)
proc ::dialog_startup::chooseCommand { prompt initialValue } {
    global cmd
    set cmd $initialValue
    
    toplevel .inputbox
    wm title .inputbox $prompt
    wm group .inputbox .
    wm minsize .inputbox 450 30
    wm resizable .inputbox 0 0
    wm geom .inputbox "450x30"
    # not all Tcl/Tk versions or platforms support -topmost, so catch the error
    catch {wm attributes $mytoplevel -topmost 1}

    button .inputbox.button -text [_ "OK"] -command { destroy .inputbox } \
        -width 50

    entry .inputbox.entry -width 50 -textvariable cmd 
    pack .inputbox.button -side right
    bind .inputbox.entry <KeyPress-Return> { destroy .inputbox }
    bind .inputbox.entry <KeyPress-Escape> { destroy .inputbox }
    pack .inputbox.entry -side right -expand 1 -fill x -padx 2m

    focus .inputbox.entry

    raise .inputbox
    wm transient .inputbox
    grab .inputbox
    tkwait window .inputbox

    return $cmd
}

proc ::dialog_startup::add {} {
    return [chooseCommand [_ "Add new library"] ""]
}

proc ::dialog_startup::edit { current_library } {
    return [chooseCommand [_ "Edit library"] $current_library]
}

proc ::dialog_startup::commit { new_startup } {
    variable defeatrt_button
    set ::var(startupLibraries) $new_startup

    ::pd_connect::pdsend "pd startup-dialog $defeatrt_button [pdtk_encodedialog $::var(startupFlags)] $::var(startupLibraries)"
}

# set up the panel with the info from pd
proc ::dialog_startup::pdtk_startup_dialog {mytoplevel defeatrt flags} {
    variable defeatrt_button $defeatrt
    if {$flags ne ""} {variable ::var(startupFlags) $flags}
    
    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel
    }
}

proc ::dialog_startup::create_dialog {mytoplevel} {
    ::pd_scrollboxwindow::make $mytoplevel $::var(startupLibraries) \
        dialog_startup::add dialog_startup::edit dialog_startup::commit \
        [_ "Pd libraries to load on startup"] \
        400 300

    label $mytoplevel.entryname -text [_ "Startup flags:"]
    entry $mytoplevel.entry -textvariable ::var(startupFlags) -width 60
    pack $mytoplevel.entryname $mytoplevel.entry -side left
    pack $mytoplevel.entry -side right -padx 2m -fill x -expand 1

    frame $mytoplevel.defeatrtframe
    pack $mytoplevel.defeatrtframe -side bottom -fill x -pady 2m
    if {[tk windowingsystem] ne "win32"} {
        checkbutton $mytoplevel.defeatrtframe.defeatrt -anchor w \
            -text [_ "Defeat real-time scheduling"] \
            -variable ::dialog_startup::defeatrt_button
        pack $mytoplevel.defeatrtframe.defeatrt -side left
    }
}

