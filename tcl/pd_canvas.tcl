
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
variable  canvasStart
variable  canvasUp
variable  canvasEnd
variable  canvasDown
variable  canvasWidth
variable  canvasHeight
variable  canvasX
variable  canvasY

array set canvasScaleX  {}
array set canvasScaleY  {}
array set canvasVisible {}
array set canvasHide    {}
array set canvasStart   {}
array set canvasUp      {}
array set canvasEnd     {}
array set canvasDown    {}
array set canvasWidth   {}
array set canvasHeight  {}
array set canvasX       {}
array set canvasY       {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top scaleX scaleY flags start up end down width height x y} {
    
    ::pd_canvas::_create $top $scaleX $scaleY $flags $start $up $end $down $width $height $x $y
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top scaleX scaleY flags start up end down width height x y} {

    variable canvasScaleX
    variable canvasScaleY
    variable canvasVisible
    variable canvasHide
    variable canvasStart
    variable canvasUp
    variable canvasEnd
    variable canvasDown
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
    set canvasStart($top)           $start
    set canvasUp($top)              $up
    set canvasEnd($top)             $end
    set canvasDown($top)            $down
    set canvasWidth($top)           $width
    set canvasHeight($top)          $height
    set canvasX($top)               $x
    set canvasY($top)               $y
    
    set canvasScaleX(${top}.old)    $scaleX
    set canvasScaleY(${top}.old)    $scaleY
    set canvasStart(${top}.old)     $start
    set canvasUp(${top}.old)        $up
    set canvasEnd(${top}.old)       $end
    set canvasDown(${top}.old)      $down
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
    
    ::pd_canvas::_forceVisible $top
    
    label $top.scaleXLabel      -text [_ "Scale Horizontal"]
    entry $top.scaleX           -textvariable ::pd_canvas::canvasScaleX($top)
    
    label $top.scaleYLabel      -text [_ "Scale Vertical"]
    entry $top.scaleY           -textvariable ::pd_canvas::canvasScaleY($top)

    checkbutton $top.visible    -text [_ "Visible in Parent"] \
                                -variable ::pd_canvas::canvasVisible($top) \
                                -takefocus 0
    
    checkbutton $top.hide       -text [_ "Hide Text"] \
                                -variable ::pd_canvas::canvasHide($top) \
                                -takefocus 0
    
    label $top.startLabel       -text [_ "Start"]
    entry $top.start            -textvariable ::pd_canvas::canvasStart($top)

    label $top.endLabel         -text [_ "End"]
    entry $top.end              -textvariable ::pd_canvas::canvasEnd($top)
    
    label $top.upLabel          -text [_ "Up"]
    entry $top.up               -textvariable ::pd_canvas::canvasUp($top)

    label $top.downLabel        -text [_ "Down"]
    entry $top.down             -textvariable ::pd_canvas::canvasDown($top)
    
    label $top.xLabel           -text [_ "Origin X"]
    entry $top.x                -textvariable ::pd_canvas::canvasX($top)

    label $top.yLabel           -text [_ "Origin Y"]
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
    pack  $top.startLabel       -side top -anchor w
    pack  $top.start            -side top -anchor w
    pack  $top.endLabel         -side top -anchor w
    pack  $top.end              -side top -anchor w
    pack  $top.upLabel          -side top -anchor w
    pack  $top.up               -side top -anchor w
    pack  $top.downLabel        -side top -anchor w
    pack  $top.down             -side top -anchor w
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
    bind  $top.start    <Return> { ::nextEntry %W }
    bind  $top.end      <Return> { ::nextEntry %W }
    bind  $top.up       <Return> { ::nextEntry %W }
    bind  $top.down     <Return> { ::nextEntry %W }
    bind  $top.x        <Return> { ::nextEntry %W }
    bind  $top.y        <Return> { ::nextEntry %W }
    bind  $top.width    <Return> { ::nextEntry %W }
    bind  $top.height   <Return> { ::nextEntry %W }

    focus $top.scaleX
    
    $top.scaleX selection range 0 end
    
    wm protocol $top WM_DELETE_WINDOW   "::pd_canvas::_closed $top"
}

proc _closed {top} {

    variable canvasScaleX
    variable canvasScaleY
    variable canvasVisible
    variable canvasHide
    variable canvasStart
    variable canvasUp
    variable canvasEnd
    variable canvasDown
    variable canvasWidth
    variable canvasHeight
    variable canvasX
    variable canvasY
    
    ::pd_canvas::_apply $top
    
    unset canvasScaleX($top)
    unset canvasScaleY($top)
    unset canvasVisible($top)
    unset canvasHide($top)
    unset canvasStart($top)
    unset canvasUp($top)
    unset canvasEnd($top)
    unset canvasDown($top)
    unset canvasWidth($top)
    unset canvasHeight($top)
    unset canvasX($top)
    unset canvasY($top)
    
    unset canvasScaleX(${top}.old)
    unset canvasScaleY(${top}.old)
    unset canvasStart(${top}.old)
    unset canvasUp(${top}.old)
    unset canvasEnd(${top}.old)
    unset canvasDown(${top}.old)
    unset canvasWidth(${top}.old)
    unset canvasHeight(${top}.old)
    unset canvasX(${top}.old)
    unset canvasY(${top}.old)
    
    ::pd_canvas::_cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable canvasScaleX
    variable canvasScaleY
    variable canvasVisible
    variable canvasHide
    variable canvasStart
    variable canvasUp
    variable canvasEnd
    variable canvasDown
    variable canvasWidth
    variable canvasHeight
    variable canvasX
    variable canvasY
    
    ::pd_canvas::_forceScales  $top
    ::pd_canvas::_forceLimits  $top
    ::pd_canvas::_forceVisible $top

    ::pd_connect::pdsend "$top donecanvasdialog \
            $canvasScaleX($top) \
            $canvasScaleY($top) \
            [expr {$canvasVisible($top) + 2 * $canvasHide($top)}] \
            $canvasStart($top) \
            $canvasUp($top) \
            $canvasEnd($top) \
            $canvasDown($top) \
            $canvasWidth($top) \
            $canvasHeight($top) \
            $canvasX($top) \
            $canvasY($top)"
}

proc _cancel {top} {

    ::pd_connect::pdsend "$top cancel"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceScales {top} {

    variable canvasScaleX
    variable canvasScaleY
    
    set canvasScaleX($top) [::ifNumber  $canvasScaleX($top) $canvasScaleX(${top}.old)]
    set canvasScaleX($top) [::ifNonZero $canvasScaleX($top) $canvasScaleX(${top}.old)]
    set canvasScaleX($top) [::ifNonZero $canvasScaleX($top) 1.0)]
    
    set canvasScaleY($top) [::ifNumber  $canvasScaleY($top) $canvasScaleY(${top}.old)]
    set canvasScaleY($top) [::ifNonZero $canvasScaleY($top) $canvasScaleY(${top}.old)]
    set canvasScaleY($top) [::ifNonZero $canvasScaleY($top) 1.0)]
}

proc _forceLimits {top} {

    variable canvasStart
    variable canvasUp
    variable canvasEnd
    variable canvasDown

    set canvasStart($top) [::ifInteger $canvasStart($top) $canvasStart(${top}.old)]
    set canvasEnd($top)   [::ifInteger $canvasEnd($top)   $canvasEnd(${top}.old)]
    set canvasUp($top)    [::ifNumber  $canvasUp($top)    $canvasUp(${top}.old)]
    set canvasDown($top)  [::ifNumber  $canvasDown($top)  $canvasDown(${top}.old)]
    
    set canvasStart($top) [::tcl::mathfunc::max $canvasStart($top) 0]
    set canvasEnd($top)   [::tcl::mathfunc::max $canvasEnd($top)   0]
    
    if {$canvasStart($top) == $canvasEnd($top)} {
        set canvasStart($top) $canvasStart(${top}.old); set canvasEnd($top) $canvasEnd(${top}.old)
    }
    
    if {$canvasUp($top) == $canvasDown($top)} { 
        set canvasUp($top) $canvasUp(${top}.old); set canvasDown($top) $canvasDown(${top}.old)
    }
}

proc _forceVisible {top} {

    variable canvasWidth
    variable canvasHeight
    variable canvasX
    variable canvasY
    
    set canvasWidth($top)  [::ifInteger $canvasWidth($top)  $canvasWidth(${top}.old)]
    set canvasHeight($top) [::ifInteger $canvasHeight($top) $canvasHeight(${top}.old)]
    set canvasX($top)      [::ifInteger $canvasX($top)      $canvasX(${top}.old)]
    set canvasY($top)      [::ifInteger $canvasY($top)      $canvasY(${top}.old)]
    
    set canvasWidth($top)  [::tcl::mathfunc::max $canvasWidth($top)  0]
    set canvasHeight($top) [::tcl::mathfunc::max $canvasHeight($top) 0]
    set canvasX($top)      [::tcl::mathfunc::max $canvasX($top)      0]
    set canvasY($top)      [::tcl::mathfunc::max $canvasY($top)      0]
    
    set canvasWidth($top)  [::ifNonZero $canvasWidth($top)  $canvasWidth(${top}.old)]
    set canvasHeight($top) [::ifNonZero $canvasHeight($top) $canvasHeight(${top}.old)]
    set canvasWidth($top)  [::ifNonZero $canvasWidth($top)  85]
    set canvasHeight($top) [::ifNonZero $canvasHeight($top) 60]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
