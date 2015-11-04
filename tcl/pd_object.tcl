
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_object 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_object:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc newText {c tags x y text fontSize color} {

    $c create text $x $y    -tags $tags \
                            -text $text \
                            -fill $color \
                            -anchor nw \
                            -font [getFont $fontSize]

}

proc setText {c tag text} {

    $c itemconfig $tag -text $text
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# paste into an existing text box by literally "typing" the contents of the
# clipboard, i.e. send the contents one character at a time via 'pd key'
proc pdtk_pastetext {tkcanvas} {
    if { [catch {set pdtk_pastebuffer [clipboard get]}] } {
        # no selection... do nothing
    } else {
        for {set i 0} {$i < [string length $pdtk_pastebuffer]} {incr i 1} {
            set cha [string index $pdtk_pastebuffer $i]
            scan $cha %c keynum
            ::pd_connect::pdsend "[winfo toplevel $tkcanvas] key 1 $keynum 0"
        }
    }
}

# select all of the text in an existing text box
proc pdtk_text_selectall {tkcanvas mytag} {
    if {$::patch_isEditMode([winfo toplevel $tkcanvas])} {
        $tkcanvas select from $mytag 0
        $tkcanvas select to $mytag end
    }
}

# de/activate a text box for editing based on $editing flag
proc pdtk_text_editing {mytoplevel tag editing} {
    set tkcanvas $mytoplevel.c
    if {$editing == 0} {selection clear $tkcanvas}
    $tkcanvas focus $tag
    set ::patch_isEditing($mytoplevel) $editing
}
