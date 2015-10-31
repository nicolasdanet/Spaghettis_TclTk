
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Override methods specific to Mac OS X application.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_apple 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require pd_connect
package require pd_file

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set ::tk::mac::CGAntialiasLimit 0
set ::tk::mac::antialiasedtext  1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ::tk::mac::OpenDocument {args} {

    foreach filename $args {
        if {$::var(isInitialized)} {
            ::pd_file::openFile $filename
        } else {
            lappend ::var(filesOpenPended) $filename
        }
    }
}

proc ::tk::mac::Quit {args} {

    ::pd_connect::pdsend "pd verifyquit"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
