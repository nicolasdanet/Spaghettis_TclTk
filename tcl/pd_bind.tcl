
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Event loop binding and handling.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_bind 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_bind:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable opt "Alt"
variable mod "Control"

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc initialize {} {
    
    variable opt
    variable mod
    
    if {[tk windowingsystem] eq "aqua"} { set opt "Option"; set mod "Command" }

    # Virtual events.
    
    event add <<Copy>>                      <$mod-Key-c>
    event add <<Duplicate>>                 <$mod-Key-d>
    event add <<SelectAll>>                 <$mod-Key-a>
    event add <<EditMode>>                  <$mod-Key-e>
    event add <<NewFile>>                   <$mod-Key-n>
    event add <<OpenFile>>                  <$mod-Key-o>
    event add <<Save>>                      <$mod-Key-s>
    event add <<SaveAs>>                    <Shift-$mod-Key-s>
    event add <<Close>>                     <$mod-Key-w>
    event add <<Quit>>                      <$mod-Key-q>
    
    event add <<NewObject>>                 <$mod-ampersand>
    event add <<NewMessage>>                <$mod-eacute>
    event add <<NewFloat>>                  <$mod-quotedbl>
    event add <<NewSymbol>>                 <$mod-quoteright>
    event add <<NewComment>>                <$mod-parenleft>
    
    event add <<NewObject>>                 <Shift-$mod-Key-1>
    event add <<NewMessage>>                <Shift-$mod-Key-2>
    event add <<NewFloat>>                  <Shift-$mod-Key-3>
    event add <<NewSymbol>>                 <Shift-$mod-Key-4>
    event add <<NewComment>>                <Shift-$mod-Key-5>
        
    event add <<RunDSP>>                    <$mod-Key-r>
    
    event add <<Motion1>>                   <Motion>
    event add <<Motion2>>                   <$mod-Motion>
    event add <<ClickLeft1>>                <ButtonPress-1>
    event add <<ClickLeft2>>                <Shift-ButtonPress-1>
    event add <<ClickLeft3>>                <$mod-ButtonPress-1>
    event add <<ClickLeft4>>                <$opt-ButtonPress-1>
    event add <<PopupMenu>>                 <ButtonPress-2>
    event add <<PopupMenu>>                 <ButtonPress-3>
    event add <<ClickRelease>>              <ButtonRelease-1>
    
    # Class.
    
    bind PdConsole  <FocusIn>               { ::pd_bind::_focusIn %W             }
    bind PdDialog   <FocusIn>               { ::pd_bind::_focusIn %W             }
    bind PdText     <FocusIn>               { ::pd_bind::_focusIn %W             }
    bind PdPatch    <FocusIn>               { ::pd_bind::_focusIn %W             }
    
    bind PdPatch    <Configure>             { ::pd_bind::_resized %W %w %h %x %y }
    bind PdPatch    <Map>                   { ::pd_bind::_mapped %W              }
    bind PdPatch    <Unmap>                 { ::pd_bind::_unmapped %W            }

    # All.
    
    bind all <<Cut>>                        { .menubar.edit     invoke "Cut"        }
    bind all <<Copy>>                       { .menubar.edit     invoke "Copy"       }
    bind all <<Paste>>                      { .menubar.edit     invoke "Paste"      }
    bind all <<Duplicate>>                  { .menubar.edit     invoke "Duplicate"  }
    bind all <<SelectAll>>                  { .menubar.edit     invoke "Select All" }
    bind all <<EditMode>>                   { .menubar.edit     invoke "Edit Mode"  }
    bind all <<NewFile>>                    { .menubar.file     invoke "New Patch"  }
    bind all <<OpenFile>>                   { .menubar.file     invoke "Open..."    }
    bind all <<Save>>                       { .menubar.file     invoke "Save"       }
    bind all <<SaveAs>>                     { .menubar.file     invoke "Save As..." }
    bind all <<Close>>                      { .menubar.file     invoke "Close"      }
    
    bind all <<NewObject>>                  { .menubar.object   invoke "Object"     }
    bind all <<NewMessage>>                 { .menubar.object   invoke "Message"    }
    bind all <<NewFloat>>                   { .menubar.object   invoke "Float"      }
    bind all <<NewSymbol>>                  { .menubar.object   invoke "Symbol"     }
    bind all <<NewComment>>                 { .menubar.object   invoke "Comment"    }

    bind all <<RunDSP>>                     { .menubar.media    invoke "Run DSP"    }
    
    bind all <KeyPress>                     { ::pd_bind::_key %W %K %A 1 0  }
    bind all <KeyRelease>                   { ::pd_bind::_key %W %K %A 0 0  }
    bind all <Shift-KeyPress>               { ::pd_bind::_key %W %K %A 1 1  }
    bind all <Shift-KeyRelease>             { ::pd_bind::_key %W %K %A 0 1  }
    
    bind all <<Quit>>                       { ::pd_connect::pdsend "pd verifyquit" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc bindPatch {top} {

    bind $top.c <<Motion1>>                 { ::pd_bind::_motion %W %x %y 0   }
    bind $top.c <<Motion2>>                 { ::pd_bind::_motion %W %x %y 2   }
    bind $top.c <<ClickLeft1>>              { ::pd_bind::_mouse %W %x %y %b 0 }
    bind $top.c <<ClickLeft2>>              { ::pd_bind::_mouse %W %x %y %b 1 }
    bind $top.c <<ClickLeft3>>              { ::pd_bind::_mouse %W %x %y %b 2 }
    bind $top.c <<ClickLeft4>>              { ::pd_bind::_mouse %W %x %y %b 3 }
    bind $top.c <<PopupMenu>>               { ::pd_bind::_mouse %W %x %y %b 8 }
    bind $top.c <<ClickRelease>>            { ::pd_bind::_mouseUp %W %x %y %b }
    bind $top.c <MouseWheel>                { ::pd_patch::scroll %W y %D      }
    bind $top.c <Destroy>                   { ::pd_patch::closed [winfo toplevel %W] }
        
    wm protocol $top WM_DELETE_WINDOW       "::pd_patch::willClose $top"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _focusIn {top} {

    set ::var(windowFocused) $top
    
    switch -- [winfo class $top] {
        "PdPatch"   {
            ::pd_menu::configureForPatch
            ::pd_patch::setEditMode $top
        }
        "PdConsole" {
            ::pd_menu::configureForConsole
            ::pd_menu::disableEditing
            set ::var(isEditMode) 0
        }
        "PdDialog"  { 
            ::pd_menu::configureForDialog
            ::pd_menu::disableEditing
            set ::var(isEditMode) 0
        }
        "PdText"    {
            ::pd_menu::configureForText
            ::pd_menu::disableEditing
            set ::var(isEditMode) 0
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _motion {c x y m} {

    set top [winfo toplevel $c]
    ::pd_connect::pdsend "$top motion [$c canvasx $x] [$c canvasy $y] $m"
}

proc _mouse {c x y b f} {

    set top [winfo toplevel $c]
    ::pd_connect::pdsend "$top mouse [$c canvasx $x] [$c canvasy $y] $b $f"
}

proc _mouseUp {c x y b} {

    set top [winfo toplevel $c]
    ::pd_connect::pdsend "$top mouseup [$c canvasx $x] [$c canvasy $y] $b"
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
    
    if {[winfo class $top] eq "PdPatch"} { set selector "$top" } else { set selector "pd" }
    
    ::pd_connect::pdsend "$selector key $isPress $k $isShift" 
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _resized {top width height x y} {

    if {$width > 1 || $height > 1} { 
        ::pd_patch::updateScrollRegion $top.c
        ::pd_connect::pdsend "$top setbounds $x $y [expr {$x + $width}] [expr {$y + $height}]"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _mapped {top} {

    ::pd_connect::pdsend "$top map 1"
}

proc _unmapped {top} {

    ::pd_connect::pdsend "$top map 0"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
