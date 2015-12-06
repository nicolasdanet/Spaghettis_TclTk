
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# PureData styles.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# ( http://wiki.tcl-lang.org/14796 ).
# ( http://wiki.tcl-lang.org/37973 ).
# ( http://www.tkdocs.com/tutorial/styles.html ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

ttk::style configure PureData.TFrame
ttk::style configure PureData.TLabelframe
ttk::style configure PureData.TEntry
ttk::style configure PureData.TMenubutton
ttk::style configure PureData.TLabel
ttk::style configure PureData.TCheckbutton

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc styleFontText {}           { return TkTextFont }
proc styleFontConsole {}        { return TkTextFont }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc styleFrame {}              { return "-style PureData.TFrame        -padding 10" }
proc styleLabelFrame {}         { return "-style PureData.TLabelframe   -padding 10" }
proc styleEntry {}              { return "-style PureData.TEntry"       }
proc styleMenuButton {}         { return "-style PureData.TMenubutton"  }
proc styleLabel {}              { return "-style PureData.TLabel"       }
proc styleCheckButton {}        { return "-style PureData.TCheckbutton" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc packMain {}                { return "-side top -fill both -expand 1" }
proc packCategory {}            { return "-side top -fill both -expand 1" }
proc packNextCategory {}        { return "-side top -fill both -expand 1 -pady {5 0}" }

proc gridPadRight {}            { return "-padx {0 20}" }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
