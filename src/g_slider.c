
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_HSLIDER_DEFAULT_WIDTH       128
#define IEM_VSLIDER_DEFAULT_WIDTH       15
#define IEM_HSLIDER_DEFAULT_HEIGHT      15
#define IEM_VSLIDER_DEFAULT_HEIGHT      128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_SLIDER_STEPS_PER_PIXEL      100

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_APPLE

#define IEM_SLIDER_PIXEL                1

#else

#define IEM_SLIDER_PIXEL                0

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int slider_stepsToPixels (int n)
{
    return (int)((n / (double)IEM_SLIDER_STEPS_PER_PIXEL) + 0.5);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_set                  (t_slider *, t_float );
static void slider_motion               (t_slider *, t_float, t_float, t_float);
static void slider_behaviorGetRectangle (t_gobj *, t_glist *, int *, int *, int *, int *);
static int  slider_behaviorMouse        (t_gobj *, t_glist *, int, int, int, int, int, int, int);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *slider_class;                           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior slider_widgetBehavior =         /* Shared. */
    {
        slider_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        slider_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void slider_drawUpdateVertical (t_slider *x, t_glist *glist)
{
    if (canvas_isMapped (glist)) {
    //
    t_glist *canvas = canvas_getView (glist);
    
    int a = text_getPixelX (cast_object (x), glist);
    int k = text_getPixelY (cast_object (x), glist) + x->x_gui.iem_height - slider_stepsToPixels (x->x_position);
        
    sys_vGui (".x%lx.c coords %lxKNOB %d %d %d %d\n",
                    canvas, 
                    x, 
                    a + 1,
                    k,
                    a + x->x_gui.iem_width - IEM_SLIDER_PIXEL, 
                    k);
    //
    }
}

static void slider_drawUpdateHorizontal (t_slider *x, t_glist *glist)
{
    if (canvas_isMapped (glist)) {
    //
    t_glist *canvas = canvas_getView (glist);
    
    int k = text_getPixelX (cast_object (x), glist) + slider_stepsToPixels (x->x_position);
    int b = text_getPixelY (cast_object (x), glist);
        
    sys_vGui (".x%lx.c coords %lxKNOB %d %d %d %d\n",
                    canvas, 
                    x, 
                    k,
                    b + 1,
                    k, 
                    b + x->x_gui.iem_height - IEM_SLIDER_PIXEL);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_drawJob (t_gobj *z, t_glist *glist)
{
    t_slider *x = (t_slider *)z;
    
    if (x->x_isVertical) { slider_drawUpdateVertical (x, glist); }
    else {
        slider_drawUpdateHorizontal (x, glist);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_drawUpdate (t_slider *x, t_glist *glist)
{
    interface_guiQueueAddIfNotAlreadyThere ((void *)x, glist, slider_drawJob);
}

static void slider_drawMove (t_slider *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);

    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                    canvas,
                    x,
                    a, 
                    b,
                    a + x->x_gui.iem_width, 
                    b + x->x_gui.iem_height);
                
    slider_drawUpdate (x, glist);
    
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                    canvas,
                    x, 
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY);
}

static void slider_drawNew (t_slider *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);
    
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    canvas,
                    a, 
                    b,
                    a + x->x_gui.iem_width, 
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
    
    if (x->x_isVertical) {
    //
    int k = b + x->x_gui.iem_height - slider_stepsToPixels (x->x_position);
    sys_vGui (".x%lx.c create line %d %d %d %d -width 3 -fill #%06x -tags %lxKNOB\n",
                    canvas,
                    a + 1,
                    k, 
                    a + x->x_gui.iem_width - IEM_SLIDER_PIXEL,
                    k,
                    x->x_gui.iem_colorForeground,
                    x);
    //
    } else {
    //
    int k = a + slider_stepsToPixels (x->x_position);
    sys_vGui (".x%lx.c create line %d %d %d %d -width 3 -fill #%06x -tags %lxKNOB\n",
                    canvas,
                    k,
                    b + 1, 
                    k,
                    b + x->x_gui.iem_height - IEM_SLIDER_PIXEL,
                    x->x_gui.iem_colorForeground,
                    x);
    //
    }
    
    sys_vGui (".x%lx.c create text %d %d -text {%s}"
                    " -anchor w"
                    " -font [::getFont %d]"
                    " -fill #%06x"
                    " -tags %lxLABEL\n",
                    canvas,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY,
                    (x->x_gui.iem_label != utils_empty()) ? x->x_gui.iem_label->s_name : "",
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_colorLabel,
                    x);
}

static void slider_drawSelect (t_slider *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -outline #%06x\n", 
                    canvas, 
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
                
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%06x\n", 
                    canvas, 
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

static void slider_drawErase (t_slider *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    sys_vGui (".x%lx.c delete %lxBASE\n",
                    canvas, 
                    x);
    sys_vGui (".x%lx.c delete %lxKNOB\n",
                    canvas,
                    x);
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                    canvas,
                    x);
}

static void slider_drawConfig (t_slider *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -fill #%06x\n",
                    canvas,
                    x, 
                    x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxKNOB -fill #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",
                    canvas,
                    x,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel,
                    (x->x_gui.iem_label != utils_empty()) ? x->x_gui.iem_label->s_name : "");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void slider_draw (t_toggle *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : slider_drawUpdate (x, glist); break;
        case IEM_DRAW_MOVE      : slider_drawMove (x, glist);   break;
        case IEM_DRAW_NEW       : slider_drawNew (x, glist);    break;
        case IEM_DRAW_SELECT    : slider_drawSelect (x, glist); break;
        case IEM_DRAW_ERASE     : slider_drawErase (x, glist);  break;
        case IEM_DRAW_CONFIG    : slider_drawConfig (x, glist); break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int slider_getNumberOfSteps (t_slider *x)
{
    if (x->x_isVertical) { 
        return ((x->x_gui.iem_height - 1) * IEM_SLIDER_STEPS_PER_PIXEL);
    } else {
        return ((x->x_gui.iem_width - 1) * IEM_SLIDER_STEPS_PER_PIXEL);
    }
}

static double slider_getStepValue (t_slider *x)
{
    if (x->x_isLogarithmic) {
        return (log (x->x_maximum / x->x_minimum) / (double)slider_getNumberOfSteps (x));
    } else {
        return ((x->x_maximum - x->x_minimum) / (double)slider_getNumberOfSteps (x));
    }
}

static t_float slider_getValue (t_slider *x)
{
    double f, t = slider_getStepValue (x) * (double)x->x_position;
    
    if (x->x_isLogarithmic) { 
        f = x->x_minimum * exp (t); 
    } else {
        f = x->x_minimum + t;
    }
    
    if ((f < 1.0e-10) && (f > -1.0e-10)) { f = 0.0; }
    
    return (t_float)f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_setRange (t_slider *x, double minimum, double maximum)
{
    t_error err = PD_ERROR_NONE;
    
    if (x->x_isLogarithmic) {
        err |= (minimum == 0.0);
        err |= (maximum * minimum < 0.0);
    }
    
    if (err) { 
        x->x_isLogarithmic = 0;
        error_invalid (sym_slider, sym_range);
    } else {
        x->x_minimum = minimum;
        x->x_maximum = maximum;
    }
}

static void slider_setWidth (t_slider *x, int width)
{
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    
    if ((x->x_isVertical == 0) && x->x_position > slider_getNumberOfSteps (x)) {
        x->x_position = slider_getNumberOfSteps (x);
    }
}

static void slider_setHeight (t_slider *x, int height)
{
    x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    
    if ((x->x_isVertical == 1) && x->x_position > slider_getNumberOfSteps (x)) {
        x->x_position = slider_getNumberOfSteps (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_out (t_slider *x)
{
    outlet_float (x->x_outlet, x->x_floatValue);

    if (x->x_gui.iem_canSend && pd_isThing (x->x_gui.iem_send)) {
        pd_float (pd_getThing (x->x_gui.iem_send), x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_bang (t_slider *x)
{
    slider_out (x);
}

static void slider_float (t_slider *x, t_float f)
{
    slider_set (x, f);
    
    if (x->x_gui.iem_goThrough) { slider_out (x); }
}

static void slider_click (t_slider *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    t_float t;
    
    if (x->x_isVertical) {
        t = text_getPixelY (cast_object (x), x->x_gui.iem_owner) + x->x_gui.iem_height - b;
    } else {
        t = a - text_getPixelX (cast_object (x), x->x_gui.iem_owner);
    }
    
    t *= IEM_SLIDER_STEPS_PER_PIXEL;
    
    if (!x->x_isSteadyOnClick) {
        int numberOfSteps = slider_getNumberOfSteps (x);
        x->x_position = PD_CLAMP ((int)t, 0, numberOfSteps);
        x->x_floatValue = slider_getValue (x);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    }
    
    canvas_setMotionFunction (x->x_gui.iem_owner, cast_gobj (x), (t_motionfn)slider_motion, a, b);
    
    slider_out (x);
}

static void slider_motion (t_slider *x, t_float deltaX, t_float deltaY, t_float modifier)
{
    int old = x->x_position;
    int t = old;
    int numberOfSteps = slider_getNumberOfSteps (x);
    int shift = (int)modifier & MODIFIER_SHIFT;
    
    if (shift) { t += (int)(x->x_isVertical ? -deltaY : deltaX); }
    else {
        t += (int)(x->x_isVertical ? -deltaY : deltaX) * IEM_SLIDER_STEPS_PER_PIXEL;
    }
    
    t = PD_CLAMP (t, 0, numberOfSteps);
    
    if (t != old) {
        x->x_position   = t;
        x->x_floatValue = slider_getValue (x);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
        slider_out (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_loadbang (t_slider *x)
{
    if (x->x_gui.iem_loadbang) { slider_out (x); }
}

static void slider_initialize (t_slider *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void slider_size (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    slider_setWidth (x, (int)atom_getFloatAtIndex (0, argc, argv));
    if (argc > 1) { slider_setHeight (x, (int)atom_getFloatAtIndex (1, argc, argv)); }
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

static void slider_range (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    double minimum = (double)atom_getFloatAtIndex (0, argc, argv);
    double maximum = (double)atom_getFloatAtIndex (1, argc, argv);
    
    slider_setRange (x, minimum, maximum);
    
    x->x_floatValue = slider_getValue (x);
}

static void slider_set (t_slider *x, t_float f)
{
    int old = x->x_position;
    
    x->x_floatValue = f;
    
    if (x->x_minimum > x->x_maximum) { f = PD_CLAMP (f, x->x_maximum, x->x_minimum); }
    else {
        f = PD_CLAMP (f, x->x_minimum, x->x_maximum);
    }
    
    if (x->x_isLogarithmic) { 
        x->x_position = (int)(log (f / x->x_minimum) / slider_getStepValue (x));
    } else {
        x->x_position = (int)((f - x->x_minimum) / slider_getStepValue (x));
    }
        
    if (x->x_position != old) { (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
}

static void slider_steady (t_slider *x, t_float f)
{
    x->x_isSteadyOnClick = (f != 0.0);
}

static void slider_logarithmic (t_slider *x)
{
    x->x_isLogarithmic = 1; 
    
    slider_setRange (x, x->x_minimum, x->x_maximum);
    
    x->x_floatValue = slider_getValue (x);
}

static void slider_linear (t_slider *x)
{
    x->x_isLogarithmic = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_slider *x = (t_slider *)z;
    
    *a = text_getPixelX (cast_object (z), glist);
    *b = text_getPixelY (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static int slider_behaviorMouse (t_gobj *z, t_glist *glist,
    int a,
    int b,
    int shift,
    int ctrl,
    int alt,
    int dbl,
    int clicked)
{
    if (clicked) {
        t_slider *x = (t_slider *)z;
        slider_click (x, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt);
    }
    
    return 1;
}

static void slider_functionSave (t_gobj *z, t_buffer *b)
{
    t_slider *x = (t_slider *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiiffiisssiiiisssii", 
        sym___hash__X,
        sym_obj,
        (int)cast_object (z)->te_xCoordinate, 
        (int)cast_object (z)->te_yCoordinate,
        x->x_isVertical ? sym_vslider : sym_hslider, 
        x->x_gui.iem_width,                                             // Width.
        x->x_gui.iem_height,                                            // Height.
        (t_float)x->x_minimum,                                          // Range minimum.
        (t_float)x->x_maximum,                                          // Range maximum.
        x->x_isLogarithmic,                                             // Is logarithmic.
        iemgui_serializeLoadbang (&x->x_gui),                           // Loadbang.
        names.n_unexpandedSend,                                         // Send.
        names.n_unexpandedReceive,                                      // Receive.
        names.n_unexpandedLabel,                                        // Label.
        x->x_gui.iem_labelX,                                            // Label X.
        x->x_gui.iem_labelY,                                            // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                          // Label font.
        x->x_gui.iem_fontSize,                                          // label font size.
        colors.c_symColorBackground,                                    // Background color.
        colors.c_symColorForeground,                                    // Foreground color.
        colors.c_symColorLabel,                                         // Label color.
        x->x_position,                                                  // Position.
        x->x_isSteadyOnClick);                                          // Is steady.
        
    buffer_appendSemicolon (b);
}

static void slider_functionProperties (t_gobj *z, t_glist *owner)
{
    t_slider *x = (t_slider *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);

    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Slider"
            " %d %d {Slider Width} %d %d {Slider Height}"
            " %g {Value %s}"
            " %g {Value %s}"
            " %d Linear Logarithmic"
            " %d"
            " -1 -1 $::var(nil)"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " %d\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH, x->x_gui.iem_height, IEM_MINIMUM_HEIGHT,
            x->x_minimum, x->x_isVertical ? "Bottom" : "Left",
            x->x_maximum, x->x_isVertical ? "Top" : "Right",
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel,
            x->x_isSteadyOnClick);
    
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

static void slider_fromDialog (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int width         = (int)atom_getFloatAtIndex (0, argc, argv);
    int height        = (int)atom_getFloatAtIndex (1, argc, argv);
    double minimum    = (double)atom_getFloatAtIndex (2, argc, argv);
    double maximum    = (double)atom_getFloatAtIndex (3, argc, argv);
    int isLogarithmic = (int)atom_getFloatAtIndex (4, argc, argv);
    int isSteady      = (int)atom_getFloatAtIndex (16, argc, argv);

    iemgui_fromDialog (&x->x_gui, argc, argv);
    
    x->x_isLogarithmic   = (isLogarithmic != 0);
    x->x_isSteadyOnClick = (isSteady != 0);
    
    slider_setHeight (x, height);               /* Must be set at last. */
    slider_setWidth (x, width);                 /* Ditto. */
    slider_setRange (x, minimum, maximum);      /* Ditto. */
    
    x->x_floatValue = slider_getValue (x);
    
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *slider_new (t_symbol *s, int argc, t_atom *argv)
{
    t_slider *x = (t_slider *)pd_new (slider_class);
    
    if (s == sym_vslider) { x->x_isVertical = 1; }
    if (s == sym_vsl)     { x->x_isVertical = 1; }

    {
    //
    int width           = x->x_isVertical ? IEM_VSLIDER_DEFAULT_WIDTH  : IEM_HSLIDER_DEFAULT_WIDTH;
    int height          = x->x_isVertical ? IEM_VSLIDER_DEFAULT_HEIGHT : IEM_HSLIDER_DEFAULT_HEIGHT;
    int isLogarithmic   = 0;
    int labelX          = x->x_isVertical ? IEM_DEFAULT_LABELX_NEXT : IEM_DEFAULT_LABELX_TOP;
    int labelY          = x->x_isVertical ? IEM_DEFAULT_LABELY_NEXT : IEM_DEFAULT_LABELY_TOP;
    int isSteady        = 0;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    double minimum      = 0.0;
    double maximum      = (double)(x->x_isVertical ? (height - 1) : (width - 1));
    t_float position    = 0.0;

    if (argc >= 17
            && IS_FLOAT (argv + 0)                                      // Width.
            && IS_FLOAT (argv + 1)                                      // Height.
            && IS_FLOAT (argv + 2)                                      // Range minimum.
            && IS_FLOAT (argv + 3)                                      // Range maximum.
            && IS_FLOAT (argv + 4)                                      // Is logarithmic.
            && IS_FLOAT (argv + 5)                                      // Loadbang.
            && IS_SYMBOL_OR_FLOAT (argv + 6)                            // Send.
            && IS_SYMBOL_OR_FLOAT (argv + 7)                            // Receive.
            && IS_SYMBOL_OR_FLOAT (argv + 8)                            // Label.
            && IS_FLOAT (argv + 9)                                      // Label X.
            && IS_FLOAT (argv + 10)                                     // Label Y.
            && IS_FLOAT (argv + 11)                                     // Label font.
            && IS_FLOAT (argv + 12)                                     // Label font size.
            && IS_SYMBOL_OR_FLOAT (argv + 13)                           // Background color.
            && IS_SYMBOL_OR_FLOAT (argv + 14)                           // Foreground color.
            && IS_SYMBOL_OR_FLOAT (argv + 15)                           // Label color.
            && IS_FLOAT (argv + 16))                                    // Position.
    {
        width                       = (int)atom_getFloatAtIndex (0,  argc, argv);
        height                      = (int)atom_getFloatAtIndex (1,  argc, argv);
        minimum                     = (double)atom_getFloatAtIndex (2, argc, argv);
        maximum                     = (double)atom_getFloatAtIndex (3, argc, argv);
        isLogarithmic               = (int)atom_getFloatAtIndex (4,  argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (9,  argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (10, argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (12, argc, argv);
        position                    = atom_getFloatAtIndex (16, argc, argv);
        
        if (argc == 18 && IS_FLOAT (argv + 17)) {
            isSteady = (int)atom_getFloatAtIndex (17, argc, argv);
        }
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (5, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 6, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (11, argc, argv));
        iemgui_deserializeColors (&x->x_gui, argv + 13, argv + 14, argv + 15);
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 6, NULL);
        iemgui_deserializeColors (&x->x_gui, NULL, NULL, NULL);
    }
    
    x->x_gui.iem_owner      = canvas_getCurrent();
    x->x_gui.iem_draw       = (t_iemfn)slider_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == utils_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == utils_empty()) ? 0 : 1;
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);

    slider_setHeight (x, height);
    slider_setWidth (x, width);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    if (x->x_gui.iem_loadbang) { x->x_position = (int)position; }
    else {
        x->x_position = 0;
    }
    
    x->x_isLogarithmic   = (isLogarithmic != 0);
    x->x_isSteadyOnClick = (isSteady != 0);
    
    slider_setRange (x, minimum, maximum);
        
    x->x_floatValue = slider_getValue (x);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    //
    }
    
    return x;
}

static void slider_free (t_slider *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
        
    guistub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void slider_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_hslider,
            (t_newmethod)slider_new,
            (t_method)slider_free,
            sizeof (t_slider),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);

    class_addCreator ((t_newmethod)slider_new, sym_vslider, A_GIMME, A_NULL);

    class_addBang (c, (t_method)slider_bang);
    class_addFloat (c, (t_method)slider_float);
    class_addClick (c, (t_method)slider_click);
    class_addMotion (c, (t_method)slider_motion);
    
    class_addMethod (c, (t_method)slider_loadbang,          sym_loadbang,               A_NULL);
    class_addMethod (c, (t_method)slider_initialize,        sym_initialize,             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_fromDialog,        sym__iemdialog,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_size,              sym_size,                   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             sym_move,                   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         sym_position,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        sym_labelfont,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    sym_labelposition,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_backgroundColor,  sym_backgroundcolor,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_foregroundColor,  sym_foregroundcolor,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelColor,       sym_labelcolor,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_range,             sym_range,                  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_set,               sym_set,                    A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_steady,            sym_steady,                 A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_logarithmic,       sym_logarithmic,            A_NULL);
    class_addMethod (c, (t_method)slider_linear,            sym_linear,                 A_NULL);
    class_addMethod (c, (t_method)iemjump_send,             sym_send,                   A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_receive,          sym_receive,                A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_label,            sym_label,                  A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)slider_initialize,        sym_init,                   A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             sym_delta,                  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         sym_pos,                    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            sym_color,                  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    sym_label_pos,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        sym_label_font,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_logarithmic,       sym_log,                    A_NULL);
    class_addMethod (c, (t_method)slider_linear,            sym_lin,                    A_NULL);
    
    class_addCreator ((t_newmethod)slider_new, sym_hsl, A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)slider_new, sym_vsl, A_GIMME, A_NULL);
    
    #endif

    class_setWidgetBehavior (c, &slider_widgetBehavior);
    class_setHelpName (c, sym_slider);
    class_setSaveFunction (c, slider_functionSave);
    class_setPropertiesFunction (c, slider_functionProperties);
    
    slider_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
