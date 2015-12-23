
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Global handy tutti frutti.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getFont {size} { 

    if {[lsearch -exact $::var(fontSizes) $size] > -1} { 
        return [format "::var(font%s)" $size] 
        
    } else {
        set next [lindex $::var(fontSizes) end]
        foreach f $::var(fontSizes) { if {$f > $size} { set next $f; break } }
        return [format "::var(font%s)" $next]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc getTitle {top} { 
    
    if {[winfo class $top] eq "PdPatch"} { return [::ui_patch::getTitle $top] }
    
    return [wm title $top]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc nextEntry {w} {

    set next [tk_focusNext $w]
    
    focus $next
    
    if {[string match "*Entry" [winfo class $next]]} { $next selection range 0 end }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc rightNextTo {top} {

    set offset [expr {[incr ::var(windowStagger) 50] + 50}]
    
    set x [expr {[winfo rootx $top] + $offset}]
    set y [expr {[winfo rooty $top] + $offset}]
    
    after 1000 { set ::var(windowStagger) 0 }
    
    return [format "+%d+%d" $x $y]
}

proc bringToFront {top} {

    wm deiconify $top; raise $top; focus $top
}

proc removeFromScreen {top} {

    wm withdraw $top; focus [lindex [wm stackorder .] end]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Escaping and quoting.

proc encoded   {x} { concat +[string map {" " "+_" "$" "+d" ";" "+s" "," "+c" "+" "++"} $x] }
proc escaped   {x} { string map {"," "\\," ";" "\\;" " " "\\ " "$" "\\$"} $x }
proc sanitized {x} { concat [string map {" " "_" ";" "" "," "" "{" "" "}" "" "\\" ""} $x] }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc parseEmpty {x} { if {$x eq "empty"} { return "" } else { return $x } }
proc withEmpty  {x} { if {$x eq ""} { return "empty" } else { return $x } }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc parseDash  {x} {

    if {[string equal -length 1 $x "-"]} { return [string replace $x 0 0 ""] }
    
    return [::rauteToDollar $x]
}

proc withDash   {x} {

    if {$x eq ""} { return "-" }
    if {[string equal -length 1 $x "-"]} { return [string replace $x 0 0 "--"] }
    
    return [::dollarToRaute $x]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc rauteToDollar {x} { return [string map {"#" "$"} $x] }
proc dollarToRaute {x} { return [string map {"$" "#"} $x] }    

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ifInteger {new old} {

    if {[string is integer -strict $new]} { return $new } else { return $old }
}

proc ifNumber  {new old} {

    if {[string is double -strict $new]}  { return $new } else { return $old }
}

proc ifNonZero {new old} {

    if {$new != 0.0} { return $new } else { return $old }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ifAqua {a b} {

    if {[tk windowingsystem] eq "aqua"} { return $a } else { return $b }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc ping {} {

    ::ui_connect::pdsend "pd ping"
}

proc watchdog {} {

    ::ui_connect::pdsend "pd watchdog"; after 2000 { ::watchdog }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc cancel {w} {

    set top [winfo toplevel $w]
    set class [winfo class $top]
    
    if {$class eq "PdDialog" || $class eq "PdData"} { ::ui_connect::pdsend "$top cancel" }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# For future msgcat ( https://www.gnu.org/software/gettext/manual/html_node/Tcl.html ).

proc _ {s} { return $s }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Could use ::msgcat::max later.

proc measure {words} {

    set m 0
    
    foreach e $words {
        set m [::tcl::mathfunc::max $m [string length $e]]
    }
    
    return $m
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Note that the name parameter must be fully qualified.

proc createMenuByIndex {top values name args} {

    upvar $name v
    
    ttk::menubutton $top            {*}[::styleMenuButton] \
                                        -text [lindex $values $v] \
                                        -takefocus 0 \
                                        {*}$args
    
    menu $top.menu
    $top configure                  -menu $top.menu
    
    set i 0
    
    foreach e $values {
        $top.menu add radiobutton   -label "$e" \
                                    -variable $name \
                                    -value $i \
                                    -command [list $top configure -text [lindex $values $i]]
        incr i
    }
}

proc createMenuByValue {top values name args} {

    upvar $name v
    
    ttk::menubutton $top            {*}[::styleMenuButton] \
                                        -text $v \
                                        -takefocus 0 \
                                        {*}$args
    
    menu $top.menu
    $top configure                  -menu $top.menu
    
    foreach e $values {
        $top.menu add radiobutton   -label "$e" \
                                    -variable $name \
                                    -value $e \
                                    -command [list $top configure -text "$e"]
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc colorToInteger {color} {

    set hex [string replace $color 0 0 "0x"]
    return  [expr {$hex}]
}

proc integerToColor {integer} {

    return [format "#%6.6x" $integer]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc chooseColor {label initial title} {

    # On Mac OS X the initial color parameter seems broken.
    
    set color [tk_chooseColor -title $title -parent [winfo toplevel $label]]
    
    if {$color ne ""} {
        $label configure -background $color
        return [::colorToInteger $color]
    }
    
    return $initial
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
