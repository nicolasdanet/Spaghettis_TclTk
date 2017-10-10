
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2017 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Patch properties.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_canvas 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_canvas:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable  canvasX
variable  canvasY
variable  canvasWidth
variable  canvasHeight
variable  canvasGOP
variable  canvasScaleX
variable  canvasScaleY

array set canvasX       {}
array set canvasY       {}
array set canvasWidth   {}
array set canvasHeight  {}
array set canvasGOP     {}
array set canvasScaleX  {}
array set canvasScaleY  {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc show {top x y width height isGOP scaleX scaleY} {
    
    ::ui_canvas::_create $top $x $y $width $height $isGOP $scaleX $scaleY
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _create {top x y width height isGOP scaleX scaleY} {

    variable canvasX
    variable canvasY
    variable canvasWidth
    variable canvasHeight
    variable canvasGOP
    variable canvasScaleX
    variable canvasScaleY
    
    toplevel $top -class PdDialog
    wm title $top [_ "Patch"]
    wm group $top .
    
    wm resizable $top 0 0
    wm minsize   $top {*}[::styleMinimumSize]
    wm geometry  $top [::rightNextTo $::var(windowFocused)]
    
    set canvasX($top)               $x
    set canvasY($top)               $y
    set canvasWidth($top)           $width
    set canvasHeight($top)          $height
    set canvasGOP($top)             $isGOP
    set canvasScaleX($top)          $scaleX
    set canvasScaleY($top)          $scaleY

    set canvasX(${top}.old)         $x
    set canvasY(${top}.old)         $y
    set canvasWidth(${top}.old)     $width
    set canvasHeight(${top}.old)    $height
    set canvasScaleX(${top}.old)    $scaleX
    set canvasScaleY(${top}.old)    $scaleY
    
    ::ui_canvas::_forceGOP $top
    
    ttk::frame      $top.f                      {*}[::styleFrame]
    ttk::labelframe $top.f.onParent             {*}[::styleLabelFrame]  -text [_ "Properties"]
    ttk::labelframe $top.f.graph                {*}[::styleLabelFrame]  -text [_ "Scalars"]
        
    pack $top.f                                 {*}[::packMain]
    pack $top.f.onParent                        {*}[::packCategory]
    pack $top.f.graph                           {*}[::packCategoryNext]
    
    ttk::label $top.f.onParent.gopLabel         {*}[::styleLabel] \
                                                    -text [_ "Graph On Parent"]
    ttk::checkbutton $top.f.onParent.gop        {*}[::styleCheckButton] \
                                                    -variable ::ui_canvas::canvasGOP($top) \
                                                    -takefocus 0 \
                                                    -command "::ui_canvas::_setGOP $top"

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
                                                    -text [_ "Value Per Pixel X"]
    ttk::entry $top.f.graph.scaleX              {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasScaleX($top) \
                                                    -width $::width(small)
    
    ttk::label $top.f.graph.scaleYLabel         {*}[::styleLabel] \
                                                    -text [_ "Value Per Pixel Y"]
    ttk::entry $top.f.graph.scaleY              {*}[::styleEntryNumber] \
                                                    -textvariable ::ui_canvas::canvasScaleY($top) \
                                                    -width $::width(small)
                                                    
    grid $top.f.onParent.gopLabel               -row 0 -column 0 -sticky ew
    grid $top.f.onParent.gop                    -row 0 -column 1 -sticky ew
    grid $top.f.onParent.xLabel                 -row 1 -column 0 -sticky ew
    grid $top.f.onParent.x                      -row 1 -column 1 -sticky ew
    grid $top.f.onParent.yLabel                 -row 2 -column 0 -sticky ew
    grid $top.f.onParent.y                      -row 2 -column 1 -sticky ew
    grid $top.f.onParent.widthLabel             -row 3 -column 0 -sticky ew
    grid $top.f.onParent.width                  -row 3 -column 1 -sticky ew
    grid $top.f.onParent.heightLabel            -row 4 -column 0 -sticky ew
    grid $top.f.onParent.height                 -row 4 -column 1 -sticky ew
    
    grid $top.f.graph.scaleXLabel               -row 0 -column 0 -sticky ew
    grid $top.f.graph.scaleX                    -row 0 -column 1 -sticky ew
    grid $top.f.graph.scaleYLabel               -row 1 -column 0 -sticky ew
    grid $top.f.graph.scaleY                    -row 1 -column 1 -sticky ew
    
    grid columnconfigure $top.f.onParent        0 -weight 1
    grid columnconfigure $top.f.graph           0 -weight 1
    
    bind $top.f.onParent.x      <Return>        { ::nextEntry %W }
    bind $top.f.onParent.y      <Return>        { ::nextEntry %W }
    bind $top.f.onParent.width  <Return>        { ::nextEntry %W }
    bind $top.f.onParent.height <Return>        { ::nextEntry %W }
    
    bind $top.f.graph.scaleX    <Return>        { ::nextEntry %W }
    bind $top.f.graph.scaleY    <Return>        { ::nextEntry %W }
    
    focus $top.f.graph.scaleX
    
    after 250 "$top.f.graph.scaleX selection range 0 end"
    
    ::ui_canvas::_setGOP $top
        
    wm protocol $top WM_DELETE_WINDOW   "::ui_canvas::closed $top"
}

proc closed {top} {

    variable canvasX
    variable canvasY
    variable canvasWidth
    variable canvasHeight
    variable canvasGOP
    variable canvasScaleX
    variable canvasScaleY
    
    ::ui_canvas::_apply $top
    
    unset canvasX($top)
    unset canvasY($top)
    unset canvasWidth($top)
    unset canvasHeight($top)
    unset canvasGOP($top)
    unset canvasScaleX($top)
    unset canvasScaleY($top)
    
    unset canvasX(${top}.old)
    unset canvasY(${top}.old)
    unset canvasWidth(${top}.old)
    unset canvasHeight(${top}.old)
    unset canvasScaleX(${top}.old)
    unset canvasScaleY(${top}.old)
    
    ::cancel $top
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _apply {top} {

    variable canvasScaleX
    variable canvasScaleY
    variable canvasGOP
    variable canvasWidth
    variable canvasHeight
    variable canvasX
    variable canvasY
    
    ::ui_canvas::_forceScales  $top
    ::ui_canvas::_forceGOP     $top

    ::ui_interface::pdsend "$top _canvasdialog \
            $canvasX($top) \
            $canvasY($top) \
            $canvasWidth($top) \
            $canvasHeight($top) \
            $canvasGOP($top) \
            $canvasScaleX($top) \
            $canvasScaleY($top)"
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _forceScales {top} {

    variable canvasScaleX
    variable canvasScaleY
    
    set canvasScaleX($top) [::ifNumber  $canvasScaleX($top) $canvasScaleX(${top}.old)]
    set canvasScaleX($top) [::ifNotZero $canvasScaleX($top) $canvasScaleX(${top}.old)]
    set canvasScaleX($top) [::ifNotZero $canvasScaleX($top) 1.0)]
    
    set canvasScaleY($top) [::ifNumber  $canvasScaleY($top) $canvasScaleY(${top}.old)]
    set canvasScaleY($top) [::ifNotZero $canvasScaleY($top) $canvasScaleY(${top}.old)]
    set canvasScaleY($top) [::ifNotZero $canvasScaleY($top) 1.0)]
}

proc _forceGOP {top} {

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
    
    set canvasWidth($top)  [::ifNotZero $canvasWidth($top)  $canvasWidth(${top}.old)]
    set canvasHeight($top) [::ifNotZero $canvasHeight($top) $canvasHeight(${top}.old)]
    set canvasWidth($top)  [::ifNotZero $canvasWidth($top)  200]
    set canvasHeight($top) [::ifNotZero $canvasHeight($top) 140]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _setGOP {top} {

    variable canvasGOP
    
    set state "disabled"
    
    if {$canvasGOP($top)} { set state "!disabled" }
    
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
