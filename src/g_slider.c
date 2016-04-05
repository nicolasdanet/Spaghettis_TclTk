
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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
#include "g_canvas.h"
#include "g_iem.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_HSLIDER_DEFAULT_WIDTH       128
#define IEM_HSLIDER_DEFAULT_HEIGHT      15

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_SLIDER_STEPS_PER_PIXEL      100

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

static void slider_set      (t_slider *x, t_float f);
static void slider_motion   (t_slider *x, t_float deltaX, t_float deltaY);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior slider_widgetBehavior;

static t_class *slider_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void slider_drawUpdate (t_slider *x, t_glist *glist)
{
    if (glist_isvisible (glist)) {
    //
    t_glist *canvas = glist_getcanvas (glist);
    
    int k = text_xpix (cast_object (x), glist) + slider_stepsToPixels (x->x_position);
    int b = text_ypix (cast_object (x), glist);
        
    sys_vGui (".x%lx.c coords %lxKNOB %d %d %d %d\n",
                canvas, 
                x, 
                k,
                b + 1,
                k, 
                b + x->x_gui.iem_height - 1);
    //
    }
}

static void slider_drawMove (t_slider *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);
    
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    int k = a + slider_stepsToPixels (x->x_position);

    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                canvas,
                x,
                a, 
                b,
                a + x->x_gui.iem_width, 
                b + x->x_gui.iem_height);
    sys_vGui (".x%lx.c coords %lxKNOB %d %d %d %d\n",
                canvas, 
                x, 
                k, 
                b + 1,
                k, 
                b + x->x_gui.iem_height - 1);
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                canvas,
                x, 
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY);
}

static void slider_drawNew (t_slider *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);
    
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    int k = a + slider_stepsToPixels (x->x_position);

    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
                canvas,
                a, 
                b,
                a + x->x_gui.iem_width, 
                b + x->x_gui.iem_height,
                x->x_gui.iem_colorBackground,
                x);
    sys_vGui (".x%lx.c create line %d %d %d %d -width 3 -fill #%6.6x -tags %lxKNOB\n",
                canvas,
                k,
                b + 1, 
                k,
                b + x->x_gui.iem_height - 1,
                x->x_gui.iem_colorForeground,
                x);
    sys_vGui (".x%lx.c create text %d %d -text {%s} -anchor w"                              // --
                " -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",    // --
                canvas,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "",
                x->x_gui.iem_fontSize,
                x->x_gui.iem_colorLabel,
                x);
}

static void slider_drawSelect (t_slider *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", 
                canvas, 
                x, 
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : IEM_COLOR_NORMAL);
                
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", 
                canvas, 
                x, 
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

static void slider_drawErase (t_slider *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

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
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s}\n",  // --
                canvas,
                x,
                x->x_gui.iem_fontSize,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "");
    sys_vGui (".x%lx.c itemconfigure %lxKNOB -fill #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n",
                canvas,
                x, 
                x->x_gui.iem_colorBackground);
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
    return ((x->x_gui.iem_width - 1) * IEM_SLIDER_STEPS_PER_PIXEL);
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
        post_error (PD_TRANSLATE ("slider: invalid logarithmic range"));   // --
    } else {
        x->x_minimum = minimum;
        x->x_maximum = maximum;
    }
}

static void slider_setWidth (t_slider *x, int width)
{
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    
    if (x->x_position > slider_getNumberOfSteps (x)) {
        x->x_position = slider_getNumberOfSteps (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_out (t_slider *x)
{
    outlet_float (cast_object (x)->te_outlet, x->x_floatValue);

    if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing) {
        pd_float (x->x_gui.iem_send->s_thing, x->x_floatValue);
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
    t_float t = (a - text_xpix (cast_object (x), x->x_gui.iem_glist)) * IEM_SLIDER_STEPS_PER_PIXEL;
    
    if (!x->x_isSteadyOnClick) {
        int numberOfSteps = slider_getNumberOfSteps (x);
        x->x_position = PD_CLAMP ((int)t, 0, numberOfSteps);
        x->x_floatValue = slider_getValue (x);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    }
    
    slider_out (x);
    glist_grab (x->x_gui.iem_glist, cast_gobj (x), (t_glistmotionfn)slider_motion, NULL, a, b);
}

static void slider_motion (t_slider *x, t_float deltaX, t_float deltaY)
{
    int old = x->x_position;
    int t = old;
    int numberOfSteps = slider_getNumberOfSteps (x);
    
    if (x->x_isAccurateMoving) { t += (int)deltaX; }
    else {
        t += (int)deltaX * IEM_SLIDER_STEPS_PER_PIXEL;
    }
    
    t = PD_CLAMP (t, 0, numberOfSteps);
    
    if (t != old) {
        x->x_position   = t;
        x->x_floatValue = slider_getValue (x);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        slider_out (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_loadbang (t_slider *x)
{
    if (x->x_gui.iem_loadbang) {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        slider_out (x);
    }
}

static void slider_initialize (t_slider *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void slider_dialog (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int width         = (int)atom_getFloatAtIndex (0, argc, argv);
    int height        = (int)atom_getFloatAtIndex (1, argc, argv);
    double minimum    = (double)atom_getFloatAtIndex (2, argc, argv);
    double maximum    = (double)atom_getFloatAtIndex (3, argc, argv);
    int isLogarithmic = (int)atom_getFloatAtIndex (4, argc, argv);
    int isSteady      = (int)atom_getFloatAtIndex (16, argc, argv);

    x->x_gui.iem_height  = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);
    
    x->x_isLogarithmic   = (isLogarithmic != 0);
    x->x_isSteadyOnClick = (isSteady != 0);
    
    slider_setWidth (x, width);                 /* Must be set at last. */
    slider_setRange (x, minimum, maximum);      /* Ditto. */
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    
    canvas_fixlines (x->x_gui.iem_glist, cast_object (x));
    //
    }
}

static void slider_size (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    slider_setWidth (x, (int)atom_getFloatAtIndex (0, argc, argv));
    
    if (argc > 1) {
        int height = atom_getFloatAtIndex (1, argc, argv);
        x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    }
    
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

static void slider_move (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_movePosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void slider_position (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setPosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void slider_labelFont (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{ 
    iemgui_setLabelFont ((void *)x, &x->x_gui, s, argc, argv);
}

static void slider_labelPosition (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelPosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void slider_range (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    double minimum = (double)atom_getFloatAtIndex (0, argc, argv);
    double maximum = (double)atom_getFloatAtIndex (1, argc, argv);
    
    slider_setRange (x, minimum, maximum);
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
        
    if (x->x_position != old) { (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE); }
}

static void slider_steady (t_slider *x, t_float f)
{
    x->x_isSteadyOnClick = (f != 0.0);
}

static void slider_logarithmic (t_slider *x)
{
    x->x_isLogarithmic = 1; slider_setRange (x, x->x_minimum, x->x_maximum);
}

static void slider_linear (t_slider *x)
{
    x->x_isLogarithmic = 0;
}

static void slider_send (t_slider *x, t_symbol *s)
{
    iemgui_setSend ((void *)x, &x->x_gui, s);
}

static void slider_receive (t_slider *x, t_symbol *s)
{ 
    iemgui_setReceive ((void *)x, &x->x_gui, s);
}

static void slider_label (t_slider *x, t_symbol *s)
{
    iemgui_setLabel ((void *)x, &x->x_gui, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_slider *x = (t_slider*)z;
    
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static int slider_behaviorClick (t_gobj *z, t_glist *glist, int a, int b, int shift, int alt, int dbl, int k)
{
    if (k) {
        t_slider *x = (t_slider *)z;
        x->x_isAccurateMoving = (shift != 0);
        slider_click (x, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt);
    }
    
    return 1;
}

static void slider_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_slider *x = (t_slider *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiiffiisssiiiiiiiii", 
        gensym ("#X"),
        gensym ("obj"),
        (int)cast_object (z)->te_xCoordinate, 
        (int)cast_object (z)->te_yCoordinate,
        gensym ("hsl"), 
        x->x_gui.iem_width,                                         // Width.
        x->x_gui.iem_height,                                        // Height.
        (t_float)x->x_minimum,                                      // Range minimum.
        (t_float)x->x_maximum,                                      // Range maximum.
        x->x_isLogarithmic,                                         // Is logarithmic.
        iemgui_serializeLoadbang (&x->x_gui),                       // Loadbang.
        names.n_unexpandedSend,                                     // Send.
        names.n_unexpandedReceive,                                  // Receive.
        names.n_unexpandedLabel,                                    // Label.
        x->x_gui.iem_labelX,                                        // Label X.
        x->x_gui.iem_labelY,                                        // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                      // Label font.
        x->x_gui.iem_fontSize,                                      // label font size.
        colors.c_colorBackground,                                   // Background color.
        colors.c_colorForeground,                                   // Foreground color.
        colors.c_colorLabel,                                        // Label color.
        x->x_position,                                              // Position.
        x->x_isSteadyOnClick);                                      // Is steady.
        
    buffer_vAppend (b, ";");
}

static void slider_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_slider *x = (t_slider *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING];
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Slider"
            " %d %d {Slider Width} %d %d {Slider Height}"               // --
            " %g {Value Left} %g {Value Right}"                         // --
            " %d Linear Logarithmic"
            " %d"
            " -1 -1 empty"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " %d\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH, x->x_gui.iem_height, IEM_MINIMUM_HEIGHT,
            x->x_minimum, x->x_maximum,
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel,
            x->x_isSteadyOnClick);
    
    PD_ASSERT (!err);
    
    gfxstub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void slider_dummy (t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *slider_new (t_symbol *s, int argc, t_atom *argv)
{
    t_slider *x = (t_slider *)pd_new (slider_class);
    
    int width           = IEM_HSLIDER_DEFAULT_WIDTH;
    int height          = IEM_HSLIDER_DEFAULT_HEIGHT;
    int isLogarithmic   = 0;
    int labelX          = IEM_DEFAULT_LABELX;
    int labelY          = IEM_DEFAULT_LABELY;
    int isSteady        = 0;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    double minimum      = 0.0;
    double maximum      = (double)(IEM_HSLIDER_DEFAULT_WIDTH - 1);
    t_float position  = 0.0;
    t_iemcolors colors  = IEM_COLORS_DEFAULT;

    if (argc >= 17                                                  // --
            && IS_FLOAT (argv + 0)                                  // Width.
            && IS_FLOAT (argv + 1)                                  // Height.
            && IS_FLOAT (argv + 2)                                  // Range minimum.
            && IS_FLOAT (argv + 3)                                  // Range maximum.
            && IS_FLOAT (argv + 4)                                  // Is logarithmic.
            && IS_FLOAT (argv + 5)                                  // Loadbang.
            && (IS_SYMBOL (argv + 6) || IS_FLOAT (argv + 6))        // Send.
            && (IS_SYMBOL (argv + 7) || IS_FLOAT (argv + 7))        // Receive.
            && (IS_SYMBOL (argv + 8) || IS_FLOAT (argv + 8))        // Label.
            && IS_FLOAT (argv + 9)                                  // Label X.
            && IS_FLOAT (argv + 10)                                 // Label Y.
            && IS_FLOAT (argv + 11)                                 // Label font.
            && IS_FLOAT (argv + 12)                                 // Label font size.
            && IS_FLOAT (argv + 13)                                 // Background color.
            && IS_FLOAT (argv + 14)                                 // Foreground color.
            && IS_FLOAT (argv + 15)                                 // Label color.
            && IS_FLOAT (argv + 16))                                // Position.
    {
        width                       = (int)atom_getFloatAtIndex (0,  argc, argv);
        height                      = (int)atom_getFloatAtIndex (1,  argc, argv);
        minimum                     = (double)atom_getFloatAtIndex (2, argc, argv);
        maximum                     = (double)atom_getFloatAtIndex (3, argc, argv);
        isLogarithmic               = (int)atom_getFloatAtIndex (4,  argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (9,  argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (10, argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (12, argc, argv);
        colors.c_colorBackground    = (int)atom_getFloatAtIndex (13, argc, argv);
        colors.c_colorForeground    = (int)atom_getFloatAtIndex (14, argc, argv);
        colors.c_colorLabel         = (int)atom_getFloatAtIndex (15, argc, argv);
        position                    = atom_getFloatAtIndex (16, argc, argv);
        
        if (argc == 18 && IS_FLOAT (argv + 17)) {
            isSteady = (int)atom_getFloatAtIndex (17, argc, argv);
        }
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (5, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 6, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (11, argc, argv));
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 6, NULL);
    }
    
    x->x_gui.iem_glist      = (t_glist *)canvas_getcurrent();
    x->x_gui.iem_draw       = (t_iemfn)slider_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_height     = PD_MAX (height, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);

    slider_setWidth (x, width);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    iemgui_deserializeColors (&x->x_gui, &colors);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    if (s == gensym ("vsl"))     { x->x_isVertical = 1; }
    if (s == gensym ("vslider")) { x->x_isVertical = 1; }
    
    if (x->x_gui.iem_loadbang) { x->x_position = (int)position; }
    else {
        x->x_position = 0;
    }
    
    x->x_isLogarithmic   = (isLogarithmic != 0);
    x->x_isSteadyOnClick = (isSteady != 0);
    
    slider_setRange (x, minimum, maximum);
        
    x->x_floatValue = slider_getValue (x);
    
    outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void slider_free (t_slider *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
        
    gfxstub_deleteforkey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void slider_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("hsl"),
        (t_newmethod)slider_new,
        (t_method)slider_free,
        sizeof (t_slider),
        CLASS_DEFAULT,
        A_GIMME,
        A_NULL);
        
    class_addCreator ((t_newmethod)slider_new, gensym ("hslider"), A_GIMME, A_NULL);

    class_addBang (c, slider_bang);
    class_addFloat (c, slider_float);
    class_addClick (c, slider_click);
    class_addMotion (c, slider_motion);
    
    class_addMethod (c, (t_method)slider_loadbang,      gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)slider_initialize,    gensym ("initialize"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_dialog,        gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_size,          gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_move,          gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_position,      gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_labelFont,     gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_labelPosition, gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_range,         gensym ("range"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_set,           gensym ("set"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_steady,        gensym ("steady"),          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_logarithmic,   gensym ("logarithmic"),     A_NULL);
    class_addMethod (c, (t_method)slider_linear,        gensym ("linear"),          A_NULL);
    class_addMethod (c, (t_method)slider_send,          gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)slider_receive,       gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)slider_label,         gensym ("label"),           A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)slider_initialize,    gensym ("init"),            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)slider_move,          gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_position,      gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_dummy,         gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_labelPosition, gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_labelFont,     gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)slider_logarithmic,   gensym ("log"),             A_NULL);
    class_addMethod (c, (t_method)slider_linear,        gensym ("lin"),             A_NULL);
    
    #endif
    
    slider_widgetBehavior.w_getrectfn   = slider_behaviorGetRectangle;
    slider_widgetBehavior.w_displacefn  = iemgui_behaviorDisplace;
    slider_widgetBehavior.w_selectfn    = iemgui_behaviorSelected;
    slider_widgetBehavior.w_activatefn  = NULL;
    slider_widgetBehavior.w_deletefn    = iemgui_behaviorDeleted;
    slider_widgetBehavior.w_visfn       = iemgui_behaviorVisible;
    slider_widgetBehavior.w_clickfn     = slider_behaviorClick;
    
    class_setWidgetBehavior (c, &slider_widgetBehavior);
    class_setHelpName (c, gensym ("slider"));
    class_setSaveFunction (c, slider_behaviorSave);
    class_setPropertiesFunction (c, slider_behaviorProperties);
    
    slider_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
