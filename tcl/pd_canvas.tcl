
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Canvas properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_canvas 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_canvas:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  canvasScaleX
variable  canvasScaleY
variable  canvasVisible
variable  canvasHide
variable  canvasLowX
variable  canvasLowY
variable  canvasHighX
variable  canvasHighY
variable  canvasWidth
variable  canvasHeight
variable  canvasX
variable  canvasY

array set canvasScaleX  {}
array set canvasScaleY  {}
array set canvasVisible {}
array set canvasHide    {}
array set canvasLowX    {}
array set canvasLowY    {}
array set canvasHighX   {}
array set canvasHighY   {}
array set canvasWidth   {}
array set canvasHeight  {}
array set canvasX       {}
array set canvasY       {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top scaleX scaleY flags lowX lowY highX highY width height x y} {
    
    ::pd_canvas::_create $top $scaleX $scaleY $flags $lowX $lowY $highX $highY $width $height $x $y
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top scaleX scaleY flags lowX lowY highX highY width height x y} {

    variable canvasScaleX
    variable canvasScaleY
    variable canvasVisible
    variable canvasHide
    variable canvasLowX
    variable canvasLowY
    variable canvasHighX
    variable canvasHighY
    variable canvasWidth
    variable canvasHeight
    variable canvasX
    variable canvasY

    toplevel $top -class PdDialog
    wm title $top [_ "Canvas"]
    wm group $top .
    
    wm resizable $top 0 0
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    set canvasScaleX($top)          $scaleX
    set canvasScaleY($top)          $scaleY
    set canvasLowX($top)            $lowX
    set canvasLowY($top)            $lowY
    set canvasHighX($top)           $highX
    set canvasHighY($top)           $highY
    set canvasWidth($top)           $width
    set canvasHeight($top)          $height
    set canvasX($top)               $x
    set canvasY($top)               $y
    
    set canvasScaleX(${top}.old)    $scaleX
    set canvasScaleY(${top}.old)    $scaleY
    set canvasLowX(${top}.old)      $lowX
    set canvasLowY(${top}.old)      $lowY
    set canvasHighX(${top}.old)     $highX
    set canvasHighY(${top}.old)     $highY
    set canvasWidth(${top}.old)     $width
    set canvasHeight(${top}.old)    $height
    set canvasX(${top}.old)         $x
    set canvasY(${top}.old)         $y
    
    switch -- $flags {
        "0" {
            set canvasVisible($top) 0
            set canvasHide($top)    0
        } 
        "1" {
            set canvasVisible($top) 1
            set canvasHide($top)    0
        } 
        "2" {
            set canvasVisible($top) 0
            set canvasHide($top)    1
        } 
        "3" {
            set canvasVisible($top) 1
            set canvasHide($top)    1
        }
    }
    
    label $top.scaleXLabel      -text [_ "Scale Horizontal"]
    entry $top.scaleX           -textvariable ::pd_canvas::canvasScaleX($top)
    
    label $top.scaleYLabel      -text [_ "Scale Vertical"]
    entry $top.scaleY           -textvariable ::pd_canvas::canvasScaleY($top)

    checkbutton $top.visible    -text [_ "Visible in Parent"] \
                                -variable ::pd_canvas::canvasVisible($top) \
                                -takefocus 0
    
    checkbutton $top.hide       -text [_ "Hide Arguments"] \
                                -variable ::pd_canvas::canvasHide($top) \
                                -takefocus 0
    
    label $top.lowXLabel        -text [_ "X Minimum"]
    entry $top.lowX             -textvariable ::pd_canvas::canvasLowX($top)

    label $top.highXLabel       -text [_ "X Maximum"]
    entry $top.highX            -textvariable ::pd_canvas::canvasHighX($top)
    
    label $top.lowYLabel        -text [_ "Y Minimum"]
    entry $top.lowY             -textvariable ::pd_canvas::canvasLowY($top)

    label $top.highYLabel       -text [_ "Y Maximum"]
    entry $top.highY            -textvariable ::pd_canvas::canvasHighY($top)
    
    label $top.xLabel           -text [_ "X Origin"]
    entry $top.x                -textvariable ::pd_canvas::canvasX($top)

    label $top.yLabel           -text [_ "Y Origin"]
    entry $top.y                -textvariable ::pd_canvas::canvasY($top)
    
    label $top.widthLabel       -text [_ "Width"]
    entry $top.width            -textvariable ::pd_canvas::canvasWidth($top)

    label $top.heightLabel      -text [_ "Height"]
    entry $top.height           -textvariable ::pd_canvas::canvasHeight($top)
    
    pack  $top.scaleXLabel      -side top -anchor w
    pack  $top.scaleX           -side top -anchor w
    pack  $top.scaleYLabel      -side top -anchor w
    pack  $top.scaleY           -side top -anchor w
    pack  $top.visible          -side top -anchor w
    pack  $top.hide             -side top -anchor w
    pack  $top.lowXLabel        -side top -anchor w
    pack  $top.lowX             -side top -anchor w
    pack  $top.highXLabel       -side top -anchor w
    pack  $top.highX            -side top -anchor w
    pack  $top.lowYLabel        -side top -anchor w
    pack  $top.lowY             -side top -anchor w
    pack  $top.highYLabel       -side top -anchor w
    pack  $top.highY            -side top -anchor w
    pack  $top.xLabel           -side top -anchor w
    pack  $top.x                -side top -anchor w
    pack  $top.yLabel           -side top -anchor w
    pack  $top.y                -side top -anchor w
    pack  $top.widthLabel       -side top -anchor w
    pack  $top.width            -side top -anchor w
    pack  $top.heightLabel      -side top -anchor w
    pack  $top.height           -side top -anchor w
    
    bind  $top.scaleX   <Return> { ::nextEntry %W }
    bind  $top.scaleY   <Return> { ::nextEntry %W }
    bind  $top.lowX     <Return> { ::nextEntry %W }
    bind  $top.highX    <Return> { ::nextEntry %W }
    bind  $top.lowY     <Return> { ::nextEntry %W }
    bind  $top.highY    <Return> { ::nextEntry %W }
    bind  $top.x        <Return> { ::nextEntry %W }
    bind  $top.y        <Return> { ::nextEntry %W }
    bind  $top.width    <Return> { ::nextEntry %W }
    bind  $top.height   <Return> { ::nextEntry %W }

    focus $top.scaleX
    
    $top.scaleX selection range 0 end
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_canvas::_closed $top"
    
    ::pd_canvas::_check $top
}

proc _closed {top} {

    variable canvasScaleX
    variable canvasScaleY
    variable canvasVisible
    variable canvasHide
    variable canvasLowX
    variable canvasLowY
    variable canvasHighX
    variable canvasHighY
    variable canvasWidth
    variable canvasHeight
    variable canvasX
    variable canvasY
    
    ::pd_canvas::_apply $top
    
    unset canvasScaleX($top)
    unset canvasScaleY($top)
    unset canvasVisible($top)
    unset canvasHide($top)
    unset canvasLowX($top)
    unset canvasLowY($top)
    unset canvasHighX($top)
    unset canvasHighY($top)
    unset canvasWidth($top)
    unset canvasHeight($top)
    unset canvasX($top)
    unset canvasY($top)
    
    unset canvasScaleX(${top}.old)
    unset canvasScaleY(${top}.old)
    unset canvasLowX(${top}.old)
    unset canvasLowY(${top}.old)
    unset canvasHighX(${top}.old)
    unset canvasHighY(${top}.old)
    unset canvasWidth(${top}.old)
    unset canvasHeight(${top}.old)
    unset canvasX(${top}.old)
    unset canvasY(${top}.old)
    
    ::pd_canvas::_cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    if {0} {
    
    ::pd_connect::pdsend "$top donecanvasdialog \
            [$top.scale.x.entry get] \
            [$top.scale.y.entry get] \
            [expr $::graphme_button($top) + 2 * $::hidetext_button($top)] \
            [$top.range.x.from_entry get] \
            [$top.range.y.from_entry get] \
            [$top.range.x.to_entry get] \
            [$top.range.y.to_entry get] \
            [$top.range.x.size_entry get] \
            [$top.range.y.size_entry get] \
            [$top.range.x.margin_entry get] \
            [$top.range.y.margin_entry get]"
    
    }
}

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _check {top} {

    if {0} {
    
    if { $::graphme_button($top) != 0 } {
        $top.scale.x.entry configure -state disabled
        $top.scale.y.entry configure -state disabled
        $top.parent.hidetext configure -state normal
        $top.range.x.from_entry configure -state normal
        $top.range.x.to_entry configure -state normal
        $top.range.x.size_entry configure -state normal
        $top.range.x.margin_entry configure -state normal
        $top.range.y.from_entry configure -state normal
        $top.range.y.to_entry configure -state normal
        $top.range.y.size_entry configure -state normal
        $top.range.y.margin_entry configure -state normal
        if { [$top.range.x.from_entry get] == 0 \
                 && [$top.range.y.from_entry get] == 0 \
                 && [$top.range.x.to_entry get] == 0 \
                 && [$top.range.y.to_entry get] == 0 } {
            $top.range.y.to_entry insert 0 1
            $top.range.y.to_entry insert 0 1
        }
        if { [$top.range.x.size_entry get] == 0 } {
            $top.range.x.size_entry delete 0 end
            $top.range.x.margin_entry delete 0 end
            $top.range.x.size_entry insert 0 85
            $top.range.x.margin_entry insert 0 100
        }
        if { [$top.range.y.size_entry get] == 0 } {
            $top.range.y.size_entry delete 0 end
            $top.range.y.margin_entry delete 0 end
            $top.range.y.size_entry insert 0 60
            $top.range.y.margin_entry insert 0 100
       }
    } else {
        $top.scale.x.entry configure -state normal
        $top.scale.y.entry configure -state normal
        $top.parent.hidetext configure -state disabled
        $top.range.x.from_entry configure -state disabled
        $top.range.x.to_entry configure -state disabled
        $top.range.x.size_entry configure -state disabled
        $top.range.x.margin_entry configure -state disabled
        $top.range.y.from_entry configure -state disabled
        $top.range.y.to_entry configure -state disabled
        $top.range.y.size_entry configure -state disabled
        $top.range.y.margin_entry configure -state disabled
        if { [$top.scale.x.entry get] == 0 } {
            $top.scale.x.entry delete 0 end
            $top.scale.x.entry insert 0 1
        }
        if { [$top.scale.y.entry get] == 0 } {
            $top.scale.y.entry delete 0 end
            $top.scale.y.entry insert 0 1
        }
    }
    
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
