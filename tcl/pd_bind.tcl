
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_bind 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

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
    
    if {[tk windowingsystem] eq "aqua"} {
    
    event add <<NewArray>>                  <Shift-$mod-Key-a>
    event add <<NewBang>>                   <Shift-$mod-Key-b>
    event add <<NewToggle>>                 <Shift-$mod-Key-t>
    event add <<NewPanel>>                  <Shift-$mod-Key-p>
    event add <<NewNumber>>                 <Shift-$mod-Key-n>
    event add <<NewMeter>>                  <Shift-$mod-Key-u>
    event add <<NewVSlider>>                <Shift-$mod-Key-v>  
    event add <<NewVRadioButton>>           <Shift-$mod-Key-d>
    event add <<NewHSlider>>                <Shift-$mod-Key-h>
    event add <<NewHRadioButton>>           <Shift-$mod-Key-i>
    
    } else {
    
    event add <<NewArray>>                  <Shift-$mod-Key-A>
    event add <<NewBang>>                   <Shift-$mod-Key-B>
    event add <<NewToggle>>                 <Shift-$mod-Key-T>
    event add <<NewPanel>>                  <Shift-$mod-Key-P>
    event add <<NewNumber>>                 <Shift-$mod-Key-N>
    event add <<NewMeter>>                  <Shift-$mod-Key-U>
    event add <<NewVSlider>>                <Shift-$mod-Key-V>  
    event add <<NewVRadioButton>>           <Shift-$mod-Key-D>
    event add <<NewHSlider>>                <Shift-$mod-Key-H>
    event add <<NewHRadioButton>>           <Shift-$mod-Key-I>
    
    }
    
    event add <<SearchPath>>                <$opt-$mod-Key-p>
    event add <<Libraries>>                 <$opt-$mod-Key-l>
    event add <<MidiSettings>>              <$opt-$mod-Key-m>
    event add <<AudioSettings>>             <$opt-$mod-Key-a>
    event add <<RunDSP>>                    <$mod-Key-p>
    event add <<NextWindow>>                <$mod-Key-Down>
    event add <<PreviousWindow>>            <$mod-Key-Up>
    event add <<PdWindow>>                  <$mod-Key-r>
    
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
    
    bind PdPatch    <FocusIn>               { ::pd_bind::_focusIn %W             }
    bind PdPatch    <Configure>             { ::pd_bind::_resized %W %w %h %x %y }
    bind PdPatch    <Map>                   { ::pd_bind::_map %W                 }
    bind PdPatch    <Unmap>                 { ::pd_bind::_unmap %W               }

    # All.
    
    bind all <<Cut>>                        { .menubar.edit invoke "Cut"         }
    bind all <<Copy>>                       { .menubar.edit invoke "Copy"        }
    bind all <<Paste>>                      { .menubar.edit invoke "Paste"       }
    bind all <<Duplicate>>                  { .menubar.edit invoke "Duplicate"   }
    bind all <<SelectAll>>                  { .menubar.edit invoke "Select All"  }
    bind all <<EditMode>>                   { .menubar.edit invoke "Edit Mode"   }
    bind all <<NewFile>>                    { .menubar.file invoke "New Patch"   }
    bind all <<OpenFile>>                   { .menubar.file invoke "Open..."     }
    bind all <<Save>>                       { .menubar.file invoke "Save"        }
    bind all <<SaveAs>>                     { .menubar.file invoke "Save As..."  }
    bind all <<Close>>                      { .menubar.file invoke "Close"       }
    
    bind all <<NewObject>>                  { .menubar.object            invoke "Object"        }
    bind all <<NewMessage>>                 { .menubar.object            invoke "Message"       }
    bind all <<NewFloat>>                   { .menubar.object            invoke "Float"         }
    bind all <<NewSymbol>>                  { .menubar.object            invoke "Symbol"        }
    bind all <<NewComment>>                 { .menubar.object            invoke "Comment"       }
    bind all <<NewArray>>                   { .menubar.object            invoke "Array"         }
    bind all <<NewBang>>                    { .menubar.object            invoke "Bang"          }
    bind all <<NewToggle>>                  { .menubar.object            invoke "Toggle"        }
    bind all <<NewPanel>>                   { .menubar.object            invoke "Panel"         }
    bind all <<NewNumber>>                  { .menubar.object            invoke "Number"        }
    bind all <<NewMeter>>                   { .menubar.object            invoke "VU Meter"      }
    bind all <<NewVSlider>>                 { .menubar.object.vertical   invoke "Slider"        } 
    bind all <<NewVRadioButton>>            { .menubar.object.vertical   invoke "RadioButton"   }
    bind all <<NewHSlider>>                 { .menubar.object.horizontal invoke "Slider"        }
    bind all <<NewHRadioButton>>            { .menubar.object.horizontal invoke "RadioButton"   }
    
    bind all <<SearchPath>>                 { .menubar.media  invoke "Path..."      }
    bind all <<Libraries>>                  { .menubar.media  invoke "Libraries..." }
    bind all <<MidiSettings>>               { .menubar.media  invoke "MIDI..."      }
    bind all <<AudioSettings>>              { .menubar.media  invoke "Audio..."     }
    bind all <<RunDSP>>                     { .menubar.media  invoke "Run DSP"      }
    bind all <<NextWindow>>                 { .menubar.window invoke "Next"         }
    bind all <<PreviousWindow>>             { .menubar.window invoke "Previous"     }
    bind all <<PdWindow>>                   { .menubar.window invoke "PureData"     }
    
    bind all <KeyPress>                     { ::pd_bind::_key %W %K %A 1 0  }
    bind all <KeyRelease>                   { ::pd_bind::_key %W %K %A 0 0  }
    bind all <Shift-KeyPress>               { ::pd_bind::_key %W %K %A 1 1  }
    bind all <Shift-KeyRelease>             { ::pd_bind::_key %W %K %A 0 1  }
    
    bind all <<Quit>>                       { ::pd_connect::pdsend "pd verifyquit" }
    
    wm protocol .console WM_DELETE_WINDOW   { ::pd_connect::pdsend "pd verifyquit" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc patch {top} {

    bind $top.c <<Motion1>>                 { pdtk_canvas_motion %W %x %y 0          }
    bind $top.c <<Motion2>>                 { pdtk_canvas_motion %W %x %y 2          }
    bind $top.c <<ClickLeft1>>              { pdtk_canvas_mouse %W %x %y %b 0        }
    bind $top.c <<ClickLeft2>>              { pdtk_canvas_mouse %W %x %y %b 1        }
    bind $top.c <<ClickLeft3>>              { pdtk_canvas_mouse %W %x %y %b 2        }
    bind $top.c <<ClickLeft4>>              { pdtk_canvas_mouse %W %x %y %b 3        }
    bind $top.c <<ClickRelease>>            { pdtk_canvas_mouseup %W %x %y %b        }
    bind $top.c <<PopupMenu>>               { pdtk_canvas_rightclick %W %x %y %b     }
    
    bind $top.c <MouseWheel>                { ::pd_patch::scroll %W y %D             }
    bind $top.c <Destroy>                   { ::pd_bind::_closed [winfo toplevel %W] }
        
    wm protocol $top WM_DELETE_WINDOW       [list ::pd_connect::pdsend "$top menuclose 0"]
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

proc _focusIn {top} {

    set ::var(windowFocused) $top
    
    switch -- [winfo class $top] {
        "PdPatch"   {
            ::pd_patch::pdtk_canvas_editmode $top $::patch_isEditMode($top)
            if {$::patch_isEditMode($top)} { $top configure -cursor $::var(cursorEditNothing) }
        }
        "PdConsole" {
            ::pd_patch::pdtk_canvas_editmode .console 0
        }
        "PdDialog"  { 
            ::pd_patch::pdtk_canvas_editmode $top 0
        }
    }
}

proc _resized {top width height x y} {

    if {$width > 1 || $height > 1} { 
        ::pd_patch::pdtk_canvas_getscroll $top.c
        ::pd_connect::pdsend "$top setbounds $x $y [expr $x + $width] [expr $y + $height]"
    }
}

proc _map {top} {
    ::pd_connect::pdsend "$top map 1"
    ::pd_patch::finished_loading_file $top
}

proc _unmap {top} {
    ::pd_connect::pdsend "$top map 0"
}

proc _closed {top} {
    unset ::patch_isEditMode($top)
    unset ::patch_isEditing($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
