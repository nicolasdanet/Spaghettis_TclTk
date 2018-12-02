
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2019 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Scalar properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_scalar 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_scalar:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  scalarType
variable  scalarIndex
variable  scalarArray
variable  scalarTemplate
variable  scalarKeys
variable  scalarValues

array set scalarType     {}
array set scalarIndex    {}
array set scalarArray    {}
array set scalarTemplate {}
array set scalarKeys     {}
array set scalarValues   {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top type index field template args} {
    
    variable scalarType
    variable scalarIndex
    variable scalarArray
    variable scalarTemplate
    variable scalarKeys
    variable scalarValues
    
    set scalarType($top)     $type
    set scalarIndex($top)    $index
    set scalarArray($top)    $field
    set scalarTemplate($top) $template
    
    foreach {key value} $args {
        
    lappend scalarKeys($top) $key
    
    set scalarValues($top$key) [::hashToDollar $value]
        
    }
    
    ::ui_scalar::_create $top $type $index $field $template
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top type index field template} {
    
    variable scalarKeys
    variable scalarValues
    
    toplevel $top -class PdDialog
    wm title $top [_ [string totitle $type]]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSizeScalar]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]

    ttk::frame      $top.f                      {*}[::styleFrame]
    ttk::labelframe $top.f.properties           {*}[::styleLabelFrame] -text [_ $template]
    
    pack $top.f                                 {*}[::packMain]
    pack $top.f.properties                      {*}[::packCategory]
    
    set row -1
    set anchor center
    
    foreach key $scalarKeys($top) {
        if {[string length $key] != 1} { set anchor w; break }
    }
    
    foreach key $scalarKeys($top) {
    
    set label [format "%s.f.properties.%sLabel" $top $key]
    set entry [format "%s.f.properties.%s"      $top $key]
    
    ttk::label $label                           {*}[::styleLabel] \
                                                    -text $key \
                                                    -anchor $anchor
    ttk::entry $entry                           {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_scalar::scalarValues($top$key) \
                                                    -width $::width(medium)
    
    grid $label                                 -row [incr row] -column 0 -sticky ew -padx {0 5}
    grid $entry                                 -row $row       -column 1 -sticky ew
    
    bind $entry                                 <Return> { ::nextEntry %W }
    
    if {$row == 0} { focus $entry; after idle "$entry selection range 0 end" }
    
    }

    grid columnconfigure $top.f.properties      0 -weight 1
    
    wm protocol $top WM_DELETE_WINDOW  "::ui_scalar::closed $top"
}

proc closed {top} {
    
    ::ui_scalar::_apply $top
    ::cancel $top
}

proc released {top} {

    variable scalarType
    variable scalarIndex
    variable scalarArray
    variable scalarTemplate
    variable scalarKeys
    variable scalarValues
    
    foreach key $scalarKeys($top) { unset scalarValues($top$key) }
    
    unset scalarType($top)
    unset scalarIndex($top)
    unset scalarArray($top)
    unset scalarTemplate($top)
    unset scalarKeys($top)
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable scalarType
    variable scalarIndex
    variable scalarArray
    variable scalarTemplate
    variable scalarKeys
    variable scalarValues
    
    set command [list $top _scalardialog \
            $scalarType($top) \
            $scalarIndex($top) \
            $scalarArray($top) \
            $scalarTemplate($top)]
    
    foreach key $scalarKeys($top) {
    
        lappend command "field"
        lappend command $key
        
        if {$scalarValues($top$key) ne ""} {
            lappend command "value"
            lappend command [::sanitized [::dollarToHash $scalarValues($top$key)]]
        }
    }
    
    ::ui_interface::pdsend $command
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
