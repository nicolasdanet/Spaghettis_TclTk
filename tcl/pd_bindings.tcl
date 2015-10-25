
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_bindings 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_commands
package require pd_connect

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_bindings:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export initialize

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable modifier "Control"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {
    
    variable modifier
    
    if {[tk windowingsystem] eq "aqua"} { set modifier "Command" }
    
    # Virtual events.
    
    event add <<Close>>         <$modifier-w>
    event add <<Copy>>          <$modifier-c>
    event add <<Duplicate>>     <$modifier-d>
    event add <<EditMode>>      <$modifier-e>
    event add <<NewFile>>       <$modifier-n>
    event add <<OpenFile>>      <$modifier-o>
    event add <<Quit>>          <$modifier-q>
    event add <<Save>>          <$modifier-s>
    event add <<SaveAs>>        <Shift-$modifier-s>
    event add <<SelectAll>>     <$modifier-a>
    
    # Widget binding.
    
    wm protocol .console WM_DELETE_WINDOW   { ::pd_connect::pdsend "pd verifyquit" }
    
    # Class binding.
    
    bind PdConsole  <FocusIn>               { ::pd_bindings::window_focusin %W }
    
    bind PdPatch    <Configure>             { ::pd_bindings::patch_configure %W %w %h %x %y }
    bind PdPatch    <FocusIn>               { ::pd_bindings::window_focusin %W }
    bind PdPatch    <Map>                   { ::pd_bindings::map %W }
    bind PdPatch    <Unmap>                 { ::pd_bindings::unmap %W }

    bind PdDialog   <Configure>             { ::pd_bindings::dialog_configure %W }
    bind PdDialog   <FocusIn>               { ::pd_bindings::dialog_focusin %W }

    # All binding.
    
    bind all <KeyPress>                     { ::pd_bindings::sendkey %W 1 %K %A 0 }
    bind all <KeyRelease>                   { ::pd_bindings::sendkey %W 0 %K %A 0 }
    bind all <Shift-KeyPress>               { ::pd_bindings::sendkey %W 1 %K %A 1 }
    bind all <Shift-KeyRelease>             { ::pd_bindings::sendkey %W 0 %K %A 1 }
    
    bind all <<Close>>                      { ::pd_commands::menu_send_float %W menuclose 0 }
    bind all <<Copy>>                       { ::pd_commands::menu_send %W copy }
    bind all <<Cut>>                        { ::pd_commands::menu_send %W cut }
    bind all <<Duplicate>>                  { ::pd_commands::menu_send %W duplicate }
    bind all <<EditMode>>                   { }
    bind all <<NewFile>>                    { ::pd_commands::menu_new }
    bind all <<OpenFile>>                   { ::pd_commands::menu_open }
    bind all <<Paste>>                      { ::pd_commands::menu_send %W paste }
    bind all <<Quit>>                       { ::pd_connect::pdsend "pd verifyquit" }
    bind all <<Save>>                       { ::pd_commands::menu_send %W menusave }
    bind all <<SaveAs>>                     { ::pd_commands::menu_send %W menusaveas }
    bind all <<SelectAll>>                  { ::pd_commands::menu_send %W selectall }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc patch_bindings {mytoplevel} {
    set tkcanvas [tkcanvas_name $mytoplevel]

    # TODO move mouse bindings to global and bind to 'all'

    # mouse bindings -----------------------------------------------------------
    # these need to be bound to $tkcanvas because %W will return $mytoplevel for
    # events over the window frame and $tkcanvas for events over the canvas
    bind $tkcanvas <Motion>                   "pdtk_canvas_motion %W %x %y 0"
    #bind $tkcanvas <$::var(modifierKey)-Motion>         "pdtk_canvas_motion %W %x %y 2"
    bind $tkcanvas <ButtonPress-1>            "pdtk_canvas_mouse %W %x %y %b 0"
    bind $tkcanvas <ButtonRelease-1>          "pdtk_canvas_mouseup %W %x %y %b"
    #bind $tkcanvas <$::var(modifierKey)-ButtonPress-1>  "pdtk_canvas_mouse %W %x %y %b 2"
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

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------


#------------------------------------------------------------------------------#
# event handlers

proc patch_configure {mytoplevel width height x y} {
    # for some reason, when we create a window, we get an event with a
    # widthXheight of 1x1 first, then we get the right values, so filter it out
    if {$width == 1 && $height == 1} {return}
    ::pd_canvas::pdtk_canvas_getscroll [tkcanvas_name $mytoplevel]
    # send the size/location of the window and canvas to 'pd' in the form of:
    #    left top right bottom
    ::pd_connect::pdsend "$mytoplevel setbounds $x $y [expr $x + $width] [expr $y + $height]"
}
    
proc window_destroy {window} {
    set mytoplevel [winfo toplevel $window]
    unset ::patch_isEditMode($mytoplevel)
    unset ::patch_isEditing($mytoplevel)
    # unset my entries all of the window data tracking arrays
    array unset ::patch_name $mytoplevel
    array unset ::patch_parents $mytoplevel
    array unset ::patch_childs $mytoplevel
}

# do tasks when changing focus (Window menu, scrollbars, etc.)
proc window_focusin {mytoplevel} {
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
    if {$::patch_isEditMode($mytoplevel)} {
        $mytoplevel configure -cursor hand2
    }
    # TODO handle enabling/disabling the Cut/Copy/Paste menu items in Edit
}

proc dialog_configure {mytoplevel} {
}

proc dialog_focusin {mytoplevel} {
    # TODO disable things on the menus that don't work for dialogs
    ::pd_menus::configure_for_dialog $mytoplevel
}

# "map" event tells us when the canvas becomes visible, and "unmap",
# invisible.  Invisibility means the Window Manager has minimized us.  We
# don't get a final "unmap" event when we destroy the window.
proc map {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel map 1"
    ::pd_canvas::finished_loading_file $mytoplevel
}

proc unmap {mytoplevel} {
    ::pd_connect::pdsend "$mytoplevel map 0"
}


#------------------------------------------------------------------------------#
# key usage

# canvas_key() expects to receive the patch's mytoplevel because key messages
# are local to each patch.  Therefore, key messages are not send for the
# dialog panels, the Pd window, help browser, etc. so we need to filter those
# events out.
proc sendkey {window state key iso shift} {
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
    if {[winfo class $mytoplevel] eq "PdPatch"} {
        ::pd_connect::pdsend "$mytoplevel key $state $key $shift"
    } else {
    ::pd_connect::pdsend "pd key $state $key $shift"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
