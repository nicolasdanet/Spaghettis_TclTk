
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2018 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Application styles for Mac OS X.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ( http://wiki.tcl-lang.org/14796 ).
# ( http://wiki.tcl-lang.org/37973 ).
# ( http://www.tkdocs.com/tutorial/styles.html ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

ttk::style configure Application.TFrame
ttk::style configure Application.TLabelframe
ttk::style configure Application.TEntry
ttk::style configure Application.TMenubutton
ttk::style configure Application.TLabel
ttk::style configure Application.TCheckbutton   -padding 3
ttk::style configure Application.TRadiobutton

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

ttk::style map Application.TEntry -foreground [list disabled DarkGrey]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

array set width {
    tiny    3
    small   6
    medium  12
    large   18
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc styleFontText {}           { return TkTextFont }
proc styleFontConsole {}        { return TkTextFont }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc styleFrame {}              { return "-style Application.TFrame         -padding 10"    }
proc styleLabelFrame {}         { return "-style Application.TLabelframe    -padding 7"     }
proc styleEntry {}              { return "-style Application.TEntry"                        }
proc styleEntryNumber {}        { return "-style Application.TEntry         -justify right" }
proc styleMenuButton {}         { return "-style Application.TMenubutton"  }
proc styleLabel {}              { return "-style Application.TLabel"       }
proc styleCheckButton {}        { return "-style Application.TCheckbutton" }
proc styleRadioButton {}        { return "-style Application.TRadiobutton" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc packMain {}                { return "-side top -fill both -expand 1" }
proc packCategory {}            { return "-side top -fill both -expand 1" }
proc packCategoryNext {}        { return "-side top -fill both -expand 1 -pady {5 0}" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc styleMinimumSize {}        { return "300 150" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
