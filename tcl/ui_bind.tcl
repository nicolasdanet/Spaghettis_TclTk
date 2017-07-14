
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2017 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Event loop binding and handling.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_bind 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_bind:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable opt "Alt"
variable mod "Control"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Filter annoying mouse interactions resizing windows. 

variable isResizing 0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {
    
    variable opt
    variable mod
    
    if {[tk windowingsystem] eq "aqua"} { set opt "Option"; set mod "Command" }

    # ( https://wiki.tcl-lang.org/3893 ).

    if {[tk windowingsystem] eq "x11"}  {

    bind all <Button-4> { event generate [focus -displayof %W] <MouseWheel> -delta  1 }
    bind all <Button-5> { event generate [focus -displayof %W] <MouseWheel> -delta -1 }

    bind all <Shift-Button-4> { event generate [focus -displayof %W] <Shift-MouseWheel> -delta  1 }
    bind all <Shift-Button-5> { event generate [focus -displayof %W] <Shift-MouseWheel> -delta -1 }

    }

    if {[tk windowingsystem] eq "aqua"} {
    
    event add <<BringFront>>                <Shift-$mod-Key-f>
    event add <<SendBack>>                  <Shift-$mod-Key-b>
    event add <<SaveAs>>                    <Shift-$mod-Key-s>
    
    }
    
    if {[tk windowingsystem] eq "x11"}  {
    
    event add <<BringFront>>                <Shift-$mod-Key-F>
    event add <<SendBack>>                  <Shift-$mod-Key-B>
    event add <<SaveAs>>                    <Shift-$mod-Key-S>
    event delete <<Paste>>                  <$mod-Key-y>
    
    }
    
    event add <<Copy>>                      <$mod-Key-c>
    event add <<Duplicate>>                 <$mod-Key-d>
    event add <<SelectAll>>                 <$mod-Key-a>
    event add <<EditMode>>                  <$mod-Key-e>
    event add <<Snap>>                      <$mod-Key-y>
    event add <<SnapToGrid>>                <Alt-$mod-Key-g>
    event add <<NewFile>>                   <$mod-Key-n>
    event add <<OpenFile>>                  <$mod-Key-o>
    event add <<Save>>                      <$mod-Key-s>
    event add <<Close>>                     <$mod-Key-w>
    event add <<Quit>>                      <$mod-Key-q>
    
    # ( http://wiki.tcl.tk/1435 ).
    
    event add <<NewObject>>                 <$mod-ampersand>
    event add <<NewMessage>>                <$mod-eacute>
    event add <<NewAtom>>                   <$mod-quotedbl>
    event add <<NewSymbol>>                 <$mod-quoteright>
    event add <<NewComment>>                <$mod-parenleft>
    event add <<NewBang>>                   <$mod-section>
    event add <<NewToggle>>                 <$mod-egrave>
    event add <<NewDial>>                   <$mod-exclam>
    event add <<NewArray>>                  <$mod-ccedilla>
     
    event add <<NewObject>>                 <Shift-$mod-Key-1>
    event add <<NewMessage>>                <Shift-$mod-Key-2>
    event add <<NewAtom>>                   <Shift-$mod-Key-3>
    event add <<NewSymbol>>                 <Shift-$mod-Key-4>
    event add <<NewComment>>                <Shift-$mod-Key-5>
    event add <<NewBang>>                   <Shift-$mod-Key-6>
    event add <<NewToggle>>                 <Shift-$mod-Key-7>
    event add <<NewDial>>                   <Shift-$mod-Key-8>
    event add <<NewArray>>                  <Shift-$mod-Key-9>
        
    event add <<RunDSP>>                    <$mod-Key-r>
    
    event add <<Motion1>>                   <Motion>
    event add <<Motion2>>                   <Shift-Motion>
    event add <<Motion3>>                   <$mod-Motion>
    event add <<Motion4>>                   <$opt-Motion>
    event add <<ClickLeft1>>                <ButtonPress-1>
    event add <<ClickLeft2>>                <Shift-ButtonPress-1>
    event add <<ClickLeft3>>                <$mod-ButtonPress-1>
    event add <<ClickLeft4>>                <$opt-ButtonPress-1>
    event add <<ClickLeft5>>                <Double-Button-1>
    event add <<ClickDummy>>                <Triple-Button-1>
    event add <<ClickDummy>>                <Quadruple-Button-1>
    event add <<PopupMenu>>                 <ButtonPress-2>
    event add <<PopupMenu>>                 <ButtonPress-3>
    event add <<ClickRelease>>              <ButtonRelease-1>
    
    bind PdConsole  <FocusIn>               { ::ui_bind::_focusIn %W }
    bind PdDialog   <FocusIn>               { ::ui_bind::_focusIn %W }
    bind PdPatch    <FocusIn>               { ::ui_bind::_focusIn %W }
    bind PdText     <FocusIn>               { ::ui_bind::_focusIn %W }
    bind PdTool     <FocusIn>               { ::ui_bind::_focusIn %W }
    
    bind PdPatch    <Configure>             { ::ui_bind::_resized %W %w %h %x %y }
    bind PdPatch    <Map>                   { ::ui_bind::_mapped %W   }
    bind PdPatch    <Unmap>                 { ::ui_bind::_unmapped %W }

    bind all <Escape>                       { ::cancel %W }
     
    bind all <<Cut>>                        { .menubar.edit     invoke "Cut"            }
    bind all <<Copy>>                       { .menubar.edit     invoke "Copy"           }
    bind all <<Paste>>                      { .menubar.edit     invoke "Paste"          }
    bind all <<Duplicate>>                  { .menubar.edit     invoke "Duplicate"      }
    bind all <<SelectAll>>                  { .menubar.edit     invoke "Select All"     }
    bind all <<EditMode>>                   { .menubar.edit     invoke "Edit Mode"      }
    bind all <<BringFront>>                 { .menubar.arrange  invoke "Bring to Front" }
    bind all <<SendBack>>                   { .menubar.arrange  invoke "Send to Back"   }
    bind all <<Snap>>                       { .menubar.arrange  invoke "Snap"           }
    bind all <<SnapToGrid>>                 { .menubar.arrange  invoke "Snap to Grid"   }
    bind all <<NewFile>>                    { .menubar.file     invoke "New Patch"      }
    bind all <<OpenFile>>                   { .menubar.file     invoke "Open..."        }
    bind all <<Save>>                       { .menubar.file     invoke "Save"           }
    bind all <<SaveAs>>                     { .menubar.file     invoke "Save As..."     }
    bind all <<Close>>                      { .menubar.file     invoke "Close"          }
    
    bind all <<NewObject>>                  { .menubar.object   invoke "Object"         }
    bind all <<NewMessage>>                 { .menubar.object   invoke "Message"        }
    bind all <<NewAtom>>                    { .menubar.object   invoke "Atom"           }
    bind all <<NewSymbol>>                  { .menubar.object   invoke "Symbol"         }
    bind all <<NewComment>>                 { .menubar.object   invoke "Comment"        }
    bind all <<NewBang>>                    { .menubar.object   invoke "Bang"           }
    bind all <<NewToggle>>                  { .menubar.object   invoke "Toggle"         }
    bind all <<NewDial>>                    { .menubar.object   invoke "Dial"           }
    bind all <<NewArray>>                   { .menubar.object   invoke "Array"          }
    
    bind all <<RunDSP>>                     { .menubar.media    invoke "Run DSP"        }
    
    bind all <KeyPress>                     { ::ui_bind::_key %W %K %A 1 }
    bind all <KeyRelease>                   { ::ui_bind::_key %W %K %A 0 }
    
    bind all <<Quit>>                       { ::ui_interface::pdsend "pd _quit" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc bindPatch {top} {

    bind $top.c <<Motion1>>                 { ::ui_bind::_motion %W %x %y 0      }
    bind $top.c <<Motion2>>                 { ::ui_bind::_motion %W %x %y 1      }
    bind $top.c <<Motion3>>                 { ::ui_bind::_motion %W %x %y 2      }
    bind $top.c <<Motion4>>                 { ::ui_bind::_motion %W %x %y 4      }
    bind $top.c <<ClickLeft1>>              { ::ui_bind::_mouseDown %W %x %y 0   }
    bind $top.c <<ClickLeft2>>              { ::ui_bind::_mouseDown %W %x %y 1   }
    bind $top.c <<ClickLeft3>>              { ::ui_bind::_mouseDown %W %x %y 2   }
    bind $top.c <<ClickLeft4>>              { ::ui_bind::_mouseDown %W %x %y 4   }
    bind $top.c <<ClickLeft5>>              { ::ui_bind::_mouseDown %W %x %y 16  }
    bind $top.c <<PopupMenu>>               { ::ui_bind::_mouseDown %W %x %y 8   }

    bind $top.c <<ClickRelease>>            { ::ui_bind::_mouseUp %W %x %y       }
    bind $top.c <<ClickDummy>>              { ::ui_interface::pdsend "pd _dummy" }
    
    bind $top.c <MouseWheel>                { ::ui_patch::scroll %W y %D }
    bind $top.c <Shift-MouseWheel>          { ::ui_patch::scroll %W x %D }
    bind $top.c <Destroy>                   { ::ui_patch::closed [winfo toplevel %W] }
    
    wm protocol $top WM_DELETE_WINDOW       "::ui_patch::willClose $top"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pasteText {top} {

    # Fake typing contents of the clipboard if any. 
    
    if {[catch { clipboard get } contents]} {
        #
    } else {
        foreach c [split $contents ""] { ui_bind::_key $top "" $c 1 }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _focusIn {top} {

    set ::var(windowFocused) $top
    
    switch -regexp -- [winfo class $top] {
        "PdConsole"         {
            ::ui_menu::configureForConsole
            ::ui_menu::disableEditing
            set ::var(isEditMode) 0
        }
        "PdPatch"           {
            ::ui_menu::configureForPatch
            ::ui_patch::setEditMode $top
        }
        "PdText"            {
            ::ui_menu::configureForText
            ::ui_menu::disableEditing
            set ::var(isEditMode) 0
        }
        "PdTool|PdDialog"   { 
            ::ui_menu::configureForDialog
            ::ui_menu::disableEditing
            set ::var(isEditMode) 0
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _motion {c x y m} {

    variable isResizing
    
    set top [winfo toplevel $c]
    ::ui_interface::pdsend "$top _motion [$c canvasx $x] [$c canvasy $y] $m"
    
    set isResizing 0
}

proc _mouseDown {c x y f} {

    variable isResizing
    
    if {$isResizing == 0} {
    
        set top [winfo toplevel $c]
        ::ui_interface::pdsend "$top _mousedown [$c canvasx $x] [$c canvasy $y] $f"
    }
}

proc _mouseUp {c x y} {

    variable isResizing
    
    if {$isResizing == 0} {
    
        set top [winfo toplevel $c]
        ::ui_interface::pdsend "$top _mouseup [$c canvasx $x] [$c canvasy $y]"
    }
    
    set isResizing 0
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _key {w keysym iso isPress} {

    set k ""
    
    switch -- $keysym {
        "Alt_L"     { set k "Option"    }
        "BackSpace" { set k "BackSpace" }
        "Caps_Lock" { set k "CapsLock"  }
        "Meta_L"    { set k "Command"   }
        "Control_L" { set k "Control"   }
        "Delete"    { set k "Delete"    }
        "Down"      { set k "Down"      }
        "Escape"    { set k "Escape"    }
        "Left"      { set k "Left"      }
        "KP_Delete" { set k "Delete"    }
        "KP_Enter"  { set k "Enter"     }
        "Return"    { set k "Return"    }
        "Right"     { set k "Right"     }
        "Shift_L"   { set k "Shift"     }
        "Super_L"   { set k "Super"     }
        "Tab"       { set k "Tab"       }
        "Up"        { set k "Up"        } 
    }

    if {$k eq ""} { set k [scan $iso %c] }
    
    set top [winfo toplevel $w]
    
    if {[winfo class $top] eq "PdPatch"} { set selector "$top" } else { set selector "pd" }
    
    ::ui_interface::pdsend "$selector _key $isPress $k" 
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _resized {top width height x y} {

    variable isResizing
    
    # Conditional below aims to filter annoying bad values generated at initialization time.
    
    if {$width > 1 && $height > 1} { 
    
        set isResizing 1
        
        ::ui_interface::pdsend "$top _window $x $y [expr {$x + $width}] [expr {$y + $height}]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _mapped {top} {

    ::ui_interface::pdsend "$top _map 1"
}

proc _unmapped {top} {

    ::ui_interface::pdsend "$top _map 0"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
