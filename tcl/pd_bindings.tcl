
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_bindings 0.1

package require pd_commands

namespace eval ::pd_bindings:: {

}

# TODO rename pd_bindings to window_bindings after merge is done

# Some commands are bound using "" quotations so that the $mytoplevel is
# interpreted immediately.  Since the command is being bound to $mytoplevel,
# it makes sense to have value of $mytoplevel already in the command.  This is
# the opposite of most menu/bind commands here and in pd_menus.tcl, which use
# {} to force execution of any variables (i.e. $::var(windowFocused)) until later

proc ::pd_bindings::console_bindings {} {

    bind .console.text <$::var(modifierKey)-Key-x> "tk_textCut .console.text"
    bind .console.text <$::var(modifierKey)-Key-c> "tk_textCopy .console.text"
    bind .console.text <$::var(modifierKey)-Key-v> "tk_textPaste .console.text"

    # Select All doesn't seem to work unless its applied to the whole window
    bind .console <$::var(modifierKey)-Key-a> ".console.text tag add sel 1.0 end"
    # the "; break" part stops executing another binds, like from the Text class

    # these don't do anything in the Pd window, so alert the user, then break
    # so no more bindings run
    bind .console <$::var(modifierKey)-Key-s> "bell; break"
    bind .console <$::var(modifierKey)-Key-p> "bell; break"

    # ways of hiding/closing the Pd window
    if {[tk windowingsystem] eq "aqua"} {
        # on Mac OS X, you can close the Pd window, since the menubar is there
        bind .console <$::var(modifierKey)-Key-w>   "wm withdraw .console"
        wm protocol .console WM_DELETE_WINDOW "wm withdraw .console"
    } else {
        # TODO should it possible to close the Pd window and keep Pd open?
        bind .console <$::var(modifierKey)-Key-w>   "wm iconify .console"
        wm protocol .console WM_DELETE_WINDOW "::pd_connect::pdsend \"pd verifyquit\""
    }
}

# binding by class is not recursive, so its useful for window events
proc ::pd_bindings::initialize {} {
    # and the Pd window is in a class to itself
    bind PdConsole <FocusIn>                "::pd_bindings::window_focusin %W"
    # bind to all the windows dedicated to patch canvases
    bind PatchWindow <FocusIn>                "::pd_bindings::window_focusin %W"
    bind PatchWindow <Map>                    "::pd_bindings::map %W"
    bind PatchWindow <Unmap>                  "::pd_bindings::unmap %W"
    bind PatchWindow <Configure> "::pd_bindings::patch_configure %W %w %h %x %y"
    # dialog panel windows bindings, which behave differently than PatchWindows
    bind DialogWindow <Configure>              "::pd_bindings::dialog_configure %W"
    bind DialogWindow <FocusIn>                "::pd_bindings::dialog_focusin %W"

    # we use 'bind all' everywhere to get as much of Tk's automatic binding
    # behaviors as possible, things like not sending an event for 'O' when
    # 'Control-O' is pressed.
    bind all <$::var(modifierKey)-Key-a>      {::pd_commands::menu_send %W selectall}
    bind all <$::var(modifierKey)-Key-c>      {::pd_commands::menu_send %W copy}
    bind all <$::var(modifierKey)-Key-d>      {::pd_commands::menu_send %W duplicate}
    bind all <$::var(modifierKey)-Key-e>      {::pd_commands::menu_toggle_editmode}
    bind all <$::var(modifierKey)-Key-g>      {::pd_commands::menu_send %W findagain}
    bind all <$::var(modifierKey)-Key-n>      {::pd_commands::menu_new}
    bind all <$::var(modifierKey)-Key-o>      {::pd_commands::menu_open}
    bind all <$::var(modifierKey)-Key-p>      {::pd_commands::menu_print $::var(windowFocused)}
    bind all <$::var(modifierKey)-Key-q>      {::pd_connect::pdsend "pd verifyquit"}
    bind all <$::var(modifierKey)-Key-r>      {::pd_commands::menu_raise_console}
    bind all <$::var(modifierKey)-Key-s>      {::pd_commands::menu_send %W menusave}
    bind all <$::var(modifierKey)-Key-v>      {::pd_commands::menu_send %W paste}
    bind all <$::var(modifierKey)-Key-w>      {::pd_commands::menu_send_float %W menuclose 0}
    bind all <$::var(modifierKey)-Key-x>      {::pd_commands::menu_send %W cut}
    bind all <$::var(modifierKey)-Key-z>      {}
    bind all <$::var(modifierKey)-Key-1>      {::pd_commands::menu_send_float %W obj 0}
    bind all <$::var(modifierKey)-Key-2>      {::pd_commands::menu_send_float %W msg 0}
    bind all <$::var(modifierKey)-Key-3>      {::pd_commands::menu_send_float %W floatatom 0}
    bind all <$::var(modifierKey)-Key-4>      {::pd_commands::menu_send_float %W symbolatom 0}
    bind all <$::var(modifierKey)-Key-5>      {::pd_commands::menu_send_float %W text 0}
    bind all <$::var(modifierKey)-Key-slash>  {::pd_connect::pdsend "pd dsp 1"}
    bind all <$::var(modifierKey)-Key-period> {::pd_connect::pdsend "pd dsp 0"}
    bind all <$::var(modifierKey)-greater>    {::pd_commands::menu_raisenextwindow}
    bind all <$::var(modifierKey)-less>       {::pd_commands::menu_raisepreviouswindow}

    # annoying, but Tk's bind needs uppercase letter to get the Shift
    bind all <$::var(modifierKey)-Shift-Key-B> {::pd_commands::menu_send %W bng}
    bind all <$::var(modifierKey)-Shift-Key-C> {::pd_commands::menu_send %W mycnv}
    bind all <$::var(modifierKey)-Shift-Key-D> {::pd_commands::menu_send %W vradio}
    bind all <$::var(modifierKey)-Shift-Key-H> {::pd_commands::menu_send %W hslider}
    bind all <$::var(modifierKey)-Shift-Key-I> {::pd_commands::menu_send %W hradio}
    bind all <$::var(modifierKey)-Shift-Key-N> {::pd_commands::menu_send %W numbox}
    bind all <$::var(modifierKey)-Shift-Key-Q> {::pd_connect::pdsend "pd quit"}
    bind all <$::var(modifierKey)-Shift-Key-S> {::pd_commands::menu_send %W menusaveas}
    bind all <$::var(modifierKey)-Shift-Key-T> {::pd_commands::menu_send %W toggle}
    bind all <$::var(modifierKey)-Shift-Key-U> {::pd_commands::menu_send %W vumeter}
    bind all <$::var(modifierKey)-Shift-Key-V> {::pd_commands::menu_send %W vslider}
    bind all <$::var(modifierKey)-Shift-Key-W> {::pd_commands::menu_send_float %W menuclose 1}
    bind all <$::var(modifierKey)-Shift-Key-Z> {}

    # OS-specific bindings
    if {[tk windowingsystem] eq "aqua"} {
        # Cmd-m = Minimize and Cmd-t = Font on Mac OS X for all apps
        bind all <$::var(modifierKey)-Key-m>       {::pd_commands::menu_minimize %W}
        bind all <$::var(modifierKey)-quoteleft>   {::pd_commands::menu_raisenextwindow}
    } else {
        #bind all <$::var(modifierKey)-Key-t>       {::pd_commands::menu_texteditor}
        bind all <$::var(modifierKey)-Next>        {::pd_commands::menu_raisenextwindow}    ;# PgUp
        bind all <$::var(modifierKey)-Prior>       {::pd_commands::menu_raisepreviouswindow};# PageDown
    }

    bind all <KeyPress>         {::pd_bindings::sendkey %W 1 %K %A 0}
    bind all <KeyRelease>       {::pd_bindings::sendkey %W 0 %K %A 0}
    bind all <Shift-KeyPress>   {::pd_bindings::sendkey %W 1 %K %A 1}
    bind all <Shift-KeyRelease> {::pd_bindings::sendkey %W 0 %K %A 1}
}

# this is for the dialogs: gatom properties, array
# properties, iemgui properties, canvas properties, data structures
# properties, Audio setup, and MIDI setup
proc ::pd_bindings::dialog_bindings {mytoplevel dialogname} {
    variable modifier

    bind $mytoplevel <KeyPress-Escape> "dialog_${dialogname}::cancel $mytoplevel"
    bind $mytoplevel <KeyPress-Return> "dialog_${dialogname}::ok $mytoplevel"
    bind $mytoplevel <$::var(modifierKey)-Key-w> "dialog_${dialogname}::cancel $mytoplevel"
    # these aren't supported in the dialog, so alert the user, then break so
    # that no other key bindings are run
    bind $mytoplevel <$::var(modifierKey)-Key-s>       {bell; break}
    bind $mytoplevel <$::var(modifierKey)-Shift-Key-S> {bell; break}
    bind $mytoplevel <$::var(modifierKey)-Key-p>       {bell; break}

    wm protocol $mytoplevel WM_DELETE_WINDOW "dialog_${dialogname}::cancel $mytoplevel"
}

proc ::pd_bindings::patch_bindings {mytoplevel} {
    variable modifier
    set tkcanvas [tkcanvas_name $mytoplevel]

    # TODO move mouse bindings to global and bind to 'all'

    # mouse bindings -----------------------------------------------------------
    # these need to be bound to $tkcanvas because %W will return $mytoplevel for
    # events over the window frame and $tkcanvas for events over the canvas
    bind $tkcanvas <Motion>                   "pdtk_canvas_motion %W %x %y 0"
    bind $tkcanvas <$::var(modifierKey)-Motion>         "pdtk_canvas_motion %W %x %y 2"
    bind $tkcanvas <ButtonPress-1>            "pdtk_canvas_mouse %W %x %y %b 0"
    bind $tkcanvas <ButtonRelease-1>          "pdtk_canvas_mouseup %W %x %y %b"
    bind $tkcanvas <$::var(modifierKey)-ButtonPress-1>  "pdtk_canvas_mouse %W %x %y %b 2"
    bind $tkcanvas <Shift-ButtonPress-1>        "pdtk_canvas_mouse %W %x %y %b 1"

    if {[tk windowingsystem] eq "x11"} {
        # from http://wiki.tcl.tk/3893
        bind all <Button-4> \
            {event generate [focus -displayof %W] <MouseWheel> -delta  1}
        bind all <Button-5> \
            {event generate [focus -displayof %W] <MouseWheel> -delta -1}
        bind all <Shift-Button-4> \
            {event generate [focus -displayof %W] <Shift-MouseWheel> -delta  1}
        bind all <Shift-Button-5> \
            {event generate [focus -displayof %W] <Shift-MouseWheel> -delta -1}
    }
    bind $tkcanvas <MouseWheel>       {::pd_canvas::scroll %W y %D}
    bind $tkcanvas <Shift-MouseWheel> {::pd_canvas::scroll %W x %D}

    # "right clicks" are defined differently on each platform
    switch -- [tk windowingsystem] { 
        "aqua" {
            bind $tkcanvas <ButtonPress-2>      "pdtk_canvas_rightclick %W %x %y %b"
            # on Mac OS X, make a rightclick with Ctrl-click for 1 button mice
            bind $tkcanvas <Control-Button-1> "pdtk_canvas_rightclick %W %x %y %b"
            bind $tkcanvas <Option-ButtonPress-1> "pdtk_canvas_mouse %W %x %y %b 3"    
        } "x11" {
            bind $tkcanvas <ButtonPress-3>    "pdtk_canvas_rightclick %W %x %y %b"
            # on X11, button 2 "pastes" from the X windows clipboard
            bind $tkcanvas <ButtonPress-2>   "pdtk_canvas_clickpaste %W %x %y %b"
            bind $tkcanvas <Alt-ButtonPress-1> "pdtk_canvas_mouse %W %x %y %b 3"
        } "win32" {
            bind $tkcanvas <ButtonPress-3>   "pdtk_canvas_rightclick %W %x %y %b"
            bind $tkcanvas <Alt-ButtonPress-1> "pdtk_canvas_mouse %W %x %y %b 3"
        }
    }

    # window protocol bindings
    wm protocol $mytoplevel WM_DELETE_WINDOW "::pd_connect::pdsend \"$mytoplevel menuclose 0\""
    bind $tkcanvas <Destroy> "::pd_bindings::window_destroy %W"
}


#------------------------------------------------------------------------------#
# event handlers

proc ::pd_bindings::patch_configure {mytoplevel width height x y} {
    # for some reason, when we create a window, we get an event with a
    # widthXheight of 1x1 first, then we get the right values, so filter it out
    if {$width == 1 && $height == 1} {return}
    ::pd_canvas::pdtk_canvas_getscroll [tkcanvas_name $mytoplevel]
    # send the size/location of the window and canvas to 'pd' in the form of:
    #    left top right bottom
    ::pd_connect::pdsend "$mytoplevel setbounds $x $y [expr $x + $width] [expr $y + $height]"
}
    
proc ::pd_bindings::window_destroy {window} {
    set mytoplevel [winfo toplevel $window]
    unset ::patch_isEditmode($mytoplevel)
    unset ::patch_isEditing($mytoplevel)
    # unset my entries all of the window data tracking arrays
    array unset ::patch_name $mytoplevel
    array unset ::patch_parents $mytoplevel
    array unset ::patch_childs $mytoplevel
}

# do tasks when changing focus (Window menu, scrollbars, etc.)
proc ::pd_bindings::window_focusin {mytoplevel} {
    # ::var(windowFocused) is used throughout for sending bindings, menu commands,
    # etc. to the correct patch receiver symbol.  MSP took out a line that
    # confusingly redirected the "find" window which might be in mid search
    set ::var(windowFocused) $mytoplevel
    ::pd_commands::set_filenewdir $mytoplevel
    if {$mytoplevel eq ".console"} {
        ::pd_menus::configureForConsole 
    } else {
        ::pd_menus::configure_for_canvas $mytoplevel
    }
    # if {[winfo exists .font]} {wm transient .font $::var(windowFocused)}
    # if we regain focus from another app, make sure to editmode cursor is right
    if {$::patch_isEditmode($mytoplevel)} {
        $mytoplevel configure -cursor hand2
    }
    # TODO handle enabling/disabling the Cut/Copy/Paste menu items in Edit
}

proc ::pd_bindings::dialog_configure {mytoplevel} {
}

proc ::pd_bindings::dialog_focusin {mytoplevel} {
    # TODO disable things on the menus that don't work for dialogs
    ::pd_menus::configure_for_dialog $mytoplevel
}

# "map" event tells us when the canvas becomes visible, and "unmap",
# invisible.  Invisibility means the Window Manager has minimized us.  We
# don't get a final "unmap" event when we destroy the window.
proc ::pd_bindings::map {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel map 1"
    ::pd_canvas::finished_loading_file $mytoplevel
}

proc ::pd_bindings::unmap {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel map 0"
}


#------------------------------------------------------------------------------#
# key usage

# canvas_key() expects to receive the patch's mytoplevel because key messages
# are local to each patch.  Therefore, key messages are not send for the
# dialog panels, the Pd window, help browser, etc. so we need to filter those
# events out.
proc ::pd_bindings::sendkey {window state key iso shift} {
    # TODO canvas_key on the C side should be refactored with this proc as well
    switch -- $key {
        "BackSpace" { set iso ""; set key 8    }
        "Tab"       { set iso ""; set key 9 }
        "Return"    { set iso ""; set key 10 }
        "Escape"    { set iso ""; set key 27 }
        "Space"     { set iso ""; set key 32 }
        "Delete"    { set iso ""; set key 127 }
        "KP_Delete" { set iso ""; set key 127 }
    }
    if {$iso ne ""} {
        scan $iso %c key
    }
    # some pop-up panels also bind to keys like the enter, but then disappear,
    # so ignore their events.  The inputbox in the Startup dialog does this.
    if {! [winfo exists $window]} {return}
    #$window might be a toplevel or canvas, [winfo toplevel] does the right thing
    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PatchWindow"} {
        ::pd_connect::pdsend "$mytoplevel key $state $key $shift"
    } else {
    ::pd_connect::pdsend "pd key $state $key $shift"
    }
}
