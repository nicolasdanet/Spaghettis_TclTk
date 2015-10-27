
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_bind 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_commands
package require pd_connect
package require pd_menu
package require pd_patch

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_bind:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace export initialize
namespace export patch

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
    
    event add <<Motion1>>       <Motion>
    event add <<Motion2>>       <$modifier-Motion>
    event add <<ClickLeft1>>    <ButtonPress-1>
    event add <<ClickLeft2>>    <Shift-ButtonPress-1>
    event add <<ClickLeft3>>    <$modifier-ButtonPress-1>
    event add <<ClickLeft4>>    <Alt-ButtonPress-1>
    event add <<ClickRight>>    <ButtonPress-2>
    event add <<ClickRight>>    <ButtonPress-3>
    event add <<ClickRelease>>  <ButtonRelease-1>
    
    # Widget.
    
    wm protocol .console WM_DELETE_WINDOW   { ::pd_connect::pdsend "pd verifyquit" }
    
    # Class.
    
    bind PdConsole  <FocusIn>               { ::pd_bind::_focusIn %W }
    bind PdDialog   <FocusIn>               { ::pd_bind::_focusIn %W }
    bind PdPatch    <FocusIn>               { ::pd_bind::_focusIn %W }
    bind PdPatch    <Configure>             { ::pd_bind::_resized %W %w %h %x %y }
    bind PdPatch    <Map>                   { ::pd_bind::_map %W    }
    bind PdPatch    <Unmap>                 { ::pd_bind::_unmap %W  }

    # All.
    
    bind all <KeyPress>                     { ::pd_bind::_key %W %K %A 1 0 }
    bind all <KeyRelease>                   { ::pd_bind::_key %W %K %A 0 0 }
    bind all <Shift-KeyPress>               { ::pd_bind::_key %W %K %A 1 1 }
    bind all <Shift-KeyRelease>             { ::pd_bind::_key %W %K %A 0 1 }
    
    bind all <<Close>>                      { ::pd_commands::menu_send_float %W menuclose 0 }
    bind all <<Copy>>                       { ::pd_commands::menu_send %W copy }
    bind all <<Cut>>                        { ::pd_commands::menu_send %W cut  }
    bind all <<Duplicate>>                  { ::pd_commands::menu_send %W duplicate }
    bind all <<EditMode>>                   { .menubar.edit invoke "Edit Mode" }
    bind all <<NewFile>>                    { ::pd_commands::menu_new  }
    bind all <<OpenFile>>                   { ::pd_commands::menu_open }
    bind all <<Paste>>                      { ::pd_commands::menu_send %W paste      }
    bind all <<Quit>>                       { ::pd_connect::pdsend "pd verifyquit"   }
    bind all <<Save>>                       { ::pd_commands::menu_send %W menusave   }
    bind all <<SaveAs>>                     { ::pd_commands::menu_send %W menusaveas }
    bind all <<SelectAll>>                  { ::pd_commands::menu_send %W selectall  }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc patch {top} {

    bind $top.c <<Motion1>>                 { pdtk_canvas_motion %W %x %y 0 }
    bind $top.c <<Motion2>>                 { pdtk_canvas_motion %W %x %y 2 }
    bind $top.c <<ClickLeft1>>              { pdtk_canvas_mouse %W %x %y %b 0 }
    bind $top.c <<ClickLeft2>>              { pdtk_canvas_mouse %W %x %y %b 1 }
    bind $top.c <<ClickLeft3>>              { pdtk_canvas_mouse %W %x %y %b 2 }
    bind $top.c <<ClickLeft4>>              { pdtk_canvas_mouse %W %x %y %b 3 }
    bind $top.c <<ClickRelease>>            { pdtk_canvas_mouseup %W %x %y %b }
    bind $top.c <<ClickRight>>              { pdtk_canvas_rightclick %W %x %y %b }
    
    bind $top.c <MouseWheel>                { ::pd_patch::scroll %W y %D }
    bind $top.c <Destroy>                   { ::pd_bind::_closed [winfo toplevel %W] }
        
    wm protocol $top WM_DELETE_WINDOW       [list ::pd_connect::pdsend "$top menuclose 0"]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _focusIn {top} {

    set ::var(windowFocused) $top
    
    switch -- [winfo class $top] {
        "PdPatch"   {
            ::pd_commands::set_filenewdir $top
            ::pd_menu::configureForPatch $top
            if {$::patch_isEditMode($top)} { $top configure -cursor $::var(cursorEditNothing) }
        }
        "PdConsole" {
            ::pd_menu::configureForConsole
        }
        "PdDialog"  { 
            ::pd_menu::configureForDialog $top
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _map {top} {
    ::pd_connect::pdsend "$top map 1"
    ::pd_patch::finished_loading_file $top
}

proc _unmap {top} {
    ::pd_connect::pdsend "$top map 0"
}

proc _resized {top width height x y} {

    if {$width > 1 || $height > 1} { 
        ::pd_patch::pdtk_canvas_getscroll $top.c
        ::pd_connect::pdsend "$top setbounds $x $y [expr $x + $width] [expr $y + $height]"
    }
}

proc _closed {top} {

    unset ::patch_isEditMode($top)
    unset ::patch_isEditing($top)
    
    array unset ::patch_name $top
    array unset ::patch_parents $top
    array unset ::patch_childs $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _key {w keysym iso isPress isShift} {

    set k ""
    
    switch -- $keysym {
        "BackSpace" { set k 8   }
        "Tab"       { set k 9   }
        "Return"    { set k 10  }
        "Escape"    { set k 27  }
        "Space"     { set k 32  }
        "Delete"    { set k 127 }
        "KP_Delete" { set k 127 }
    }
    
    if {$k eq ""} { set k [scan $iso %c] }
    
    set top [winfo toplevel $w]
    
    if {[winfo class $top] eq "PdPatch"} { 
        ::pd_connect::pdsend "$top key $isPress $k $isShift" 
    } else {
        ::pd_connect::pdsend "pd key $isPress $k $isShift"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
