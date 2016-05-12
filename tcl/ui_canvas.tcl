
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2016 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Canvas properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_canvas 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_canvas:: {

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
    
    ::ui_canvas::_create $top $scaleX $scaleY $flags $start $up $end $down $width $height $x $y
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
    wm minsize   $top {*}[::styleMinimumSize]
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
    
    ::ui_canvas::_forceVisible $top
    
    ttk::frame      $top.f                      {*}[::styleFrame]
    ttk::labelframe $top.f.table                {*}[::styleLabelFrame]  -text [_ "Array"]
    ttk::labelframe $top.f.onParent             {*}[::styleLabelFrame]  -text [_ "Patch"]
    ttk::labelframe $top.f.graph                {*}[::styleLabelFrame]  -text [_ "Drawing"]
        
    pack $top.f                                 {*}[::packMain]
    pack $top.f.table                           {*}[::packCategory]
    pack $top.f.onParent                        {*}[::packCategoryNext]
    pack $top.f.graph                           {*}[::packCategoryNext]
    
    ttk::label $top.f.table.startLabel          {*}[::styleLabel] \
                                                    -text [_ "Index Start"]
    ttk::entry $top.f.table.start               {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasStart($top) \
                                                    -width $::width(small)

    ttk::label $top.f.table.endLabel            {*}[::styleLabel] \
                                                    -text [_ "Index End"]
    ttk::entry $top.f.table.end                 {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasEnd($top) \
                                                    -width $::width(small)
    
    ttk::label $top.f.table.downLabel           {*}[::styleLabel] \
                                                    -text [_ "Value Bottom"]
    ttk::entry $top.f.table.down                {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasDown($top) \
                                                    -width $::width(small)
                                                        
    ttk::label $top.f.table.upLabel             {*}[::styleLabel] \
                                                    -text [_ "Value Top"]
    ttk::entry $top.f.table.up                  {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasUp($top) \
                                                    -width $::width(small)

    ttk::label $top.f.onParent.visibleLabel     {*}[::styleLabel] \
                                                    -text [_ "Graph On Parent"]
    ttk::checkbutton $top.f.onParent.visible    {*}[::styleCheckButton] \
                                                    -variable ::ui_canvas::canvasVisible($top) \
                                                    -takefocus 0 \
                                                    -command "::ui_canvas::_setVisible $top"
    
    ttk::label $top.f.onParent.hideLabel        {*}[::styleLabel] \
                                                    -text [_ "Hide Title"]
    ttk::checkbutton $top.f.onParent.hide       {*}[::styleCheckButton] \
                                                    -variable ::ui_canvas::canvasHide($top) \
                                                    -takefocus 0

    ttk::label $top.f.onParent.xLabel           {*}[::styleLabel] \
                                                    -text [_ "View X"]
    ttk::entry $top.f.onParent.x                {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasX($top) \
                                                    -width $::width(small)

    ttk::label $top.f.onParent.yLabel           {*}[::styleLabel] \
                                                    -text [_ "View Y"]
    ttk::entry $top.f.onParent.y                {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasY($top) \
                                                    -width $::width(small)
    
    ttk::label $top.f.onParent.widthLabel       {*}[::styleLabel] \
                                                    -text [_ "View Width"]
    ttk::entry $top.f.onParent.width            {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasWidth($top) \
                                                    -width $::width(small)

    ttk::label $top.f.onParent.heightLabel      {*}[::styleLabel] \
                                                    -text [_ "View Height"]
    ttk::entry $top.f.onParent.height           {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasHeight($top) \
                                                    -width $::width(small)
    
    ttk::label $top.f.graph.scaleXLabel         {*}[::styleLabel] \
                                                    -text [_ "Scale Horizontal"]
    ttk::entry $top.f.graph.scaleX              {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasScaleX($top) \
                                                    -width $::width(small)
    
    ttk::label $top.f.graph.scaleYLabel         {*}[::styleLabel] \
                                                    -text [_ "Scale Vertical"]
    ttk::entry $top.f.graph.scaleY              {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasScaleY($top) \
                                                    -width $::width(small)
                                                    
    grid $top.f.table.startLabel                -row 0 -column 0 -sticky ew
    grid $top.f.table.start                     -row 0 -column 1 -sticky ew
    grid $top.f.table.endLabel                  -row 1 -column 0 -sticky ew
    grid $top.f.table.end                       -row 1 -column 1 -sticky ew
    grid $top.f.table.downLabel                 -row 2 -column 0 -sticky ew
    grid $top.f.table.down                      -row 2 -column 1 -sticky ew
    grid $top.f.table.upLabel                   -row 3 -column 0 -sticky ew
    grid $top.f.table.up                        -row 3 -column 1 -sticky ew
    
    grid $top.f.onParent.visibleLabel           -row 0 -column 0 -sticky ew
    grid $top.f.onParent.visible                -row 0 -column 1 -sticky ew
    grid $top.f.onParent.hideLabel              -row 1 -column 0 -sticky ew
    grid $top.f.onParent.hide                   -row 1 -column 1 -sticky ew
    grid $top.f.onParent.xLabel                 -row 2 -column 0 -sticky ew
    grid $top.f.onParent.x                      -row 2 -column 1 -sticky ew
    grid $top.f.onParent.yLabel                 -row 3 -column 0 -sticky ew
    grid $top.f.onParent.y                      -row 3 -column 1 -sticky ew
    grid $top.f.onParent.widthLabel             -row 4 -column 0 -sticky ew
    grid $top.f.onParent.width                  -row 4 -column 1 -sticky ew
    grid $top.f.onParent.heightLabel            -row 5 -column 0 -sticky ew
    grid $top.f.onParent.height                 -row 5 -column 1 -sticky ew
    
    grid $top.f.graph.scaleXLabel               -row 0 -column 0 -sticky ew
    grid $top.f.graph.scaleX                    -row 0 -column 1 -sticky ew
    grid $top.f.graph.scaleYLabel               -row 1 -column 0 -sticky ew
    grid $top.f.graph.scaleY                    -row 1 -column 1 -sticky ew
    
    grid columnconfigure $top.f.table           0 -weight 1
    grid columnconfigure $top.f.onParent        0 -weight 1
    grid columnconfigure $top.f.graph           0 -weight 1
    
    bind $top.f.table.start     <Return>        { ::nextEntry %W }
    bind $top.f.table.end       <Return>        { ::nextEntry %W }
    bind $top.f.table.up        <Return>        { ::nextEntry %W }
    bind $top.f.table.down      <Return>        { ::nextEntry %W }
    
    bind $top.f.onParent.x      <Return>        { ::nextEntry %W }
    bind $top.f.onParent.y      <Return>        { ::nextEntry %W }
    bind $top.f.onParent.width  <Return>        { ::nextEntry %W }
    bind $top.f.onParent.height <Return>        { ::nextEntry %W }
    
    bind $top.f.graph.scaleX    <Return>        { ::nextEntry %W }
    bind $top.f.graph.scaleY    <Return>        { ::nextEntry %W }
    
    focus $top.f.table.start
    
    after 250 "$top.f.table.start selection range 0 end"
    
    ::ui_canvas::_setVisible $top
        
    wm protocol $top WM_DELETE_WINDOW   "::ui_canvas::closed $top"
}

proc closed {top} {

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
    
    ::ui_canvas::_apply $top
    
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
    
    ::cancel $top
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
    
    ::ui_canvas::_forceScales  $top
    ::ui_canvas::_forceLimits  $top
    ::ui_canvas::_forceVisible $top

    ::ui_interface::pdsend "$top _canvasdialog \
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
    set canvasWidth($top)  [::ifNonZero $canvasWidth($top)  200]
    set canvasHeight($top) [::ifNonZero $canvasHeight($top) 140]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _setVisible {top} {

    variable canvasVisible
    
    set state "disabled"
    
    if {$canvasVisible($top)} { set state "!disabled" }
    
    $top.f.onParent.hide    state $state
    $top.f.onParent.x       state $state
    $top.f.onParent.y       state $state
    $top.f.onParent.width   state $state
    $top.f.onParent.height  state $state
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
