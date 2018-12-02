
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2019 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Text in object boxes.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_box 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_box:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc newText {c tag x y text fontSize color} {
    
    $c create text $x $y    -tags $tag \
                            -text $text \
                            -fill $color \
                            -anchor nw \
                            -font [::getFont $fontSize]
}

proc setText {c tag text} {

    # On macOS with ActiveTcl 8.6.7 the cursor isn't properly erased while deleting.
    # A workaround is to add a space to the text in the box.
    
    $c itemconfig $tag -text "$text "
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
