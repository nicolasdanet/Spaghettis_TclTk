
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

#define IEM_DIAL_DEFAULT_DIGITS     5
#define IEM_DIAL_DEFAULT_STEPS      127
#define IEM_DIAL_DEFAULT_SIZE       40
#define IEM_DIAL_DEFAULT_MINIMUM    0
#define IEM_DIAL_DEFAULT_MAXIMUM    127

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DIAL_MAXIMUM_STEPS      (1024 * 1024)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_set    (t_dial *x, t_float f);
static void dial_motion (t_dial *x, t_float deltaX, t_float deltaY);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior dial_widgetBehavior;

static t_class *dial_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int dial_getWidthDigits (t_dial *x)
{
    return (int)(x->x_digitsFontSize * x->x_digitsNumber * (3.0 / 5.0));        /* Empirical ratio. */
}

static int dial_getWidth (t_dial *x)
{
    return PD_MAX (dial_getWidthDigits (x), x->x_gui.iem_height);
}

static void dial_setString (t_dial *x)
{
    int size = PD_MIN (x->x_digitsNumber + 1, IEM_DIAL_BUFFER_LENGTH);
    t_error err = string_sprintf (x->x_t, size, "%f", x->x_floatValue);
    PD_ASSERT (!err); 
}

static int dial_getNeedleColor (t_dial *x)
{
    if (x->x_gui.iem_height < dial_getWidth (x)) { return x->x_gui.iem_colorBackground; }
    else {
        return x->x_gui.iem_colorForeground;
    }
}

static int dial_getNeedleAngle (t_dial *x)
{
    int k = (int)(((double)x->x_position / x->x_steps) * 360.0);
    k = PD_CLAMP (k, 0, 359);
    return -k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_drawUpdate (t_dial *x, t_glist *glist)
{
    if (glist_isvisible (glist)) {
    //
    dial_setString (x);
    
    sys_vGui (".x%lx.c itemconfigure %lxNEEDLE -extent %d\n",
                glist_getcanvas (glist),
                x,
                dial_getNeedleAngle (x));
                
    sys_vGui (".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s}\n",
                glist_getcanvas (glist),
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorForeground,
                x->x_t);
    //
    }
}

static void dial_drawMove (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);
    
    int a     = text_xpix (cast_object (x), glist);
    int b     = text_ypix (cast_object (x), glist);
    int k     = x->x_gui.iem_height - (x->x_digitsFontSize / 2);
    int h     = x->x_digitsFontSize;
    int width = dial_getWidth (x);
    
    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                canvas,
                x,
                a, 
                b,
                a + width, 
                b + x->x_gui.iem_height);
    sys_vGui (".x%lx.c coords %lxNEEDLE %d %d %d %d\n",
                canvas,
                x,
                a + 1 + (h / 2),
                b + 1,
                a - 1 + width - (h / 2),
                b - 1 + x->x_gui.iem_height - h);
    sys_vGui (".x%lx.c coords %lxNUMBER %d %d\n",
                canvas,
                x,
                a + 1 + (width / 2),
                b + k);
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                canvas,
                x,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY);
}

static void dial_drawNew (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);
    
    int a     = text_xpix (cast_object (x), glist);
    int b     = text_ypix (cast_object (x), glist);
    int k     = x->x_gui.iem_height - (x->x_digitsFontSize / 2);
    int h     = x->x_digitsFontSize;
    int width = dial_getWidth (x);
    
    dial_setString (x);
    
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxBASE\n",
                canvas,
                a,
                b,
                a + width,
                b + x->x_gui.iem_height,
                x->x_gui.iem_colorBackground,
                x->x_gui.iem_colorBackground,
                x);
    sys_vGui (".x%lx.c create arc %d %d %d %d"
                " -start -90"
                " -extent %d"
                " -fill #%6.6x"
                " -outline #%6.6x"
                " -tags %lxNEEDLE\n",
                canvas,
                a + 1 + (h / 2),
                b + 1,
                a - 1 + width - (h / 2),
                b - 1 + x->x_gui.iem_height - h,
                dial_getNeedleAngle (x),
                x->x_gui.iem_colorBackground,
                dial_getNeedleColor (x),
                x);
    sys_vGui (".x%lx.c create text %d %d -text {%s}"
                " -anchor center"
                " -font [::getFont %d]"
                " -fill #%6.6x"
                " -tags %lxNUMBER\n",
                canvas,
                a + 1 + (width / 2),
                b + k,
                x->x_t, 
                x->x_digitsFontSize,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorForeground,
                x);
    sys_vGui (".x%lx.c create text %d %d -text {%s}"
                " -anchor w"
                " -font [::getFont %d]"
                " -fill #%6.6x"
                " -tags %lxLABEL\n",
                canvas,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "",
                x->x_gui.iem_fontSize,
                x->x_gui.iem_colorLabel,
                x);
}

static void dial_drawSelect (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

static void dial_drawErase (t_dial* x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas(glist);

    sys_vGui (".x%lx.c delete %lxBASE\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxNEEDLE\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxNUMBER\n",
                canvas,
                x);
}

static void dial_drawConfig (t_dial* x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxNEEDLE -fill #%6.6x -outline #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_colorBackground, 
                dial_getNeedleColor (x));
    sys_vGui (".x%lx.c itemconfigure %lxNUMBER -font [::getFont %d] -fill #%6.6x \n",
                canvas,
                x, 
                x->x_digitsFontSize,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s}\n",
                canvas,
                x,
                x->x_gui.iem_fontSize,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dial_draw (t_toggle *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : dial_drawUpdate (x, glist);   break;
        case IEM_DRAW_MOVE      : dial_drawMove (x, glist);     break;
        case IEM_DRAW_NEW       : dial_drawNew (x, glist);      break;
        case IEM_DRAW_SELECT    : dial_drawSelect (x, glist);   break;
        case IEM_DRAW_ERASE     : dial_drawErase (x, glist);    break;
        case IEM_DRAW_CONFIG    : dial_drawConfig (x, glist);   break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static double dial_getStepValue (t_dial *x)
{
    if (x->x_isLogarithmic) {
        return (log (x->x_maximum / x->x_minimum) / (double)x->x_steps);
    } else {
        return ((x->x_maximum - x->x_minimum) / (double)x->x_steps);
    }
}

static t_float dial_getValue (t_dial *x)
{
    double f, t = dial_getStepValue (x) * (double)x->x_position;
    
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

static void dial_setRange (t_dial *x, double minimum, double maximum)
{
    t_error err = PD_ERROR_NONE;
    
    err |= minimum > maximum;
    
    if (x->x_isLogarithmic) {
        err |= (minimum == 0.0);
        err |= (maximum * minimum < 0.0);
    }
    
    if (err) { 
        x->x_isLogarithmic = 0;
        post_error (PD_TRANSLATE ("dial: invalid range"));   // --
        
    } else {
        x->x_minimum = minimum;
        x->x_maximum = maximum;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_out (t_dial *x)
{
    outlet_float (cast_object (x)->te_outlet, x->x_floatValue);
    
    if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing) {
        pd_float(x->x_gui.iem_send->s_thing, x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_bang (t_dial *x)
{
    dial_out (x);
}

static void dial_float (t_dial *x, t_float f)
{
    dial_set (x, f);
    
    if (x->x_gui.iem_goThrough) { dial_out (x); }
}

static void dial_list (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_FLOAT (argv)) { dial_float (x, atom_getFloatAtIndex (0, argc, argv)); }
}

static void dial_click (t_dial *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    glist_grab (x->x_gui.iem_glist, cast_gobj (x), (t_glistmotionfn)dial_motion, NULL, a, b);
}

static void dial_motion (t_dial *x, t_float deltaX, t_float deltaY)
{
    int old = x->x_position;
    int t = old;
    int k = (int)(-deltaY);
    
    if (!x->x_isAccurateMoving) { k *= PD_MAX (1, (int)(x->x_steps * 0.05)); }
    
    t += k;
    
    t = PD_CLAMP (t, 0, x->x_steps);
    
    if (t != old) {
        x->x_position   = t;
        x->x_floatValue = dial_getValue (x);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        dial_out (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_loadbang (t_dial *x)
{
    if (x->x_gui.iem_loadbang) { dial_out (x); }
}

static void dial_initialize (t_dial *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void dial_dialog (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int digits          = (int)atom_getFloatAtIndex (0, argc, argv);
    int height          = (int)atom_getFloatAtIndex (1, argc, argv);
    double minimum      = (double)atom_getFloatAtIndex (2, argc, argv);
    double maximum      = (double)atom_getFloatAtIndex (3, argc, argv);
    int isLogarithmic   = (int)atom_getFloatAtIndex (4, argc, argv);
    int steps           = (int)atom_getFloatAtIndex (6, argc, argv);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);
    
    x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    x->x_isLogarithmic  = (isLogarithmic != 0);
    x->x_digitsNumber   = PD_MAX (digits, 1);
    x->x_steps          = PD_MAX (steps, 1);
    x->x_position       = PD_MIN (x->x_position, x->x_steps);
    
    dial_setRange (x, minimum, maximum);
    
    x->x_floatValue = dial_getValue (x);
        
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    
    canvas_fixlines (x->x_gui.iem_glist, cast_object (x));
    //
    }
}

static void dial_size (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int digits = (int)atom_getFloatAtIndex (0, argc, argv);

    x->x_digitsNumber = PD_MAX (digits, 1);
    
    if (argc > 1) {
        int height = (int)atom_getFloatAtIndex (1, argc, argv);
        x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    }
    
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

static void dial_move (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_movePosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void dial_position (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setPosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void dial_labelFont (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelFont ((void *)x, &x->x_gui, s, argc, argv);
}

static void dial_labelPosition (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelPosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void dial_range (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    double minimum = (double)atom_getFloatAtIndex (0, argc, argv);
    double maximum = (double)atom_getFloatAtIndex (1, argc, argv);
    
    dial_setRange (x, minimum, maximum);
    
    x->x_floatValue = dial_getValue (x);
}

static void dial_set (t_dial *x, t_float f)
{
    t_float old = x->x_floatValue;
    
    f = PD_CLAMP (f, x->x_minimum, x->x_maximum);
    
    if (x->x_isLogarithmic) { 
        x->x_position = (int)(log (f / x->x_minimum) / dial_getStepValue (x));
    } else {
        x->x_position = (int)((f - x->x_minimum) / dial_getStepValue (x));
    }
    
    x->x_floatValue = dial_getValue (x);
    
    if (x->x_floatValue != old) { (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE); }
}

static void dial_steps (t_dial *x, t_float f)
{
    x->x_steps    = PD_MAX ((int)f, 1);
    x->x_position = PD_MIN (x->x_position, x->x_steps);
    
    x->x_floatValue = dial_getValue (x);
}

static void dial_logarithmic (t_dial *x)
{
    x->x_isLogarithmic = 1;
    
    dial_setRange (x, x->x_minimum, x->x_maximum);
    
    x->x_floatValue = dial_getValue (x);
}

static void dial_linear (t_dial *x)
{
    x->x_isLogarithmic = 0;
}

static void dial_send (t_dial *x, t_symbol *s)
{
    iemgui_setSend ((void *)x, &x->x_gui, s);
}

static void dial_receive (t_dial *x, t_symbol *s)
{
    iemgui_setReceive ((void *)x, &x->x_gui, s);
}

static void dial_label (t_dial *x, t_symbol *s)
{
    iemgui_setLabel ((void *)x, &x->x_gui, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_dial *x = (t_dial *)z;
    
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    *c = *a + dial_getWidth (x);
    *d = *b + cast_iem (z)->iem_height;
}

static int dial_behaviorClick (t_gobj *z, t_glist *glist, int a, int b, int shift, int alt, int dbl, int k)
{
    if (k) {
        t_dial *x = (t_dial *)z;
        x->x_isAccurateMoving = (shift != 0);
        dial_click (x, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt);
    }
    
    return 1;
}

static void dial_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_dial *x = (t_dial *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);

    buffer_vAppend (b, "ssiisiiffiisssiiiiiiifi",
        gensym ("#X"),
        gensym ("obj"),
        (int)cast_object (z)->te_xCoordinate,
        (int)cast_object (z)->te_yCoordinate,
        gensym ("nbx"),
        x->x_digitsNumber,                                                      // Number of digits.
        x->x_gui.iem_height,                                                    // Height.
        (t_float)x->x_minimum,                                                  // Range minimum.
        (t_float)x->x_maximum,                                                  // Range maximum.
        x->x_isLogarithmic,                                                     // Is logarithmic.
        iemgui_serializeLoadbang (&x->x_gui),                                   // Loadbang.
        names.n_unexpandedSend,                                                 // Send.
        names.n_unexpandedReceive,                                              // Receive.
        names.n_unexpandedLabel,                                                // Label.
        x->x_gui.iem_labelX,                                                    // Label X.
        x->x_gui.iem_labelY,                                                    // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                                  // Label font.
        x->x_gui.iem_fontSize,                                                  // Label font size.
        colors.c_colorBackground,                                               // Background color.
        colors.c_colorForeground,                                               // Foreground color.
        colors.c_colorLabel,                                                    // Label color.
        x->x_floatValue,                                                        // Value.
        x->x_steps);                                                            // Steps.
        
    buffer_vAppend (b, ";");
}

static void dial_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_dial *x = (t_dial *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);

    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Dial"
            " %d %d Digits %d %d Size"
            " %g {Value Low} %g {Value High}"
            " %d Linear Logarithmic"
            " %d"
            " %d %d {Steps}"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_digitsNumber, 1, x->x_gui.iem_height, IEM_MINIMUM_HEIGHT,
            x->x_minimum, x->x_maximum,
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            x->x_steps, IEM_DIAL_MAXIMUM_STEPS,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel);
    
    PD_ASSERT (!err);
    
    gfxstub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_dummy (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *dial_new (t_symbol *s, int argc, t_atom *argv)
{
    t_dial *x = (t_dial *)pd_new (dial_class);
    
    int digits          = IEM_DIAL_DEFAULT_DIGITS;
    int height          = IEM_DIAL_DEFAULT_SIZE;
    int isLogarithmic   = 0;
    int labelX          = IEM_DEFAULT_LABELX_TOP;
    int labelY          = IEM_DEFAULT_LABELY_TOP;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    int steps           = IEM_DIAL_DEFAULT_STEPS;
    double minimum      = IEM_DIAL_DEFAULT_MINIMUM;
    double maximum      = IEM_DIAL_DEFAULT_MAXIMUM;
    double value        = IEM_DIAL_DEFAULT_MINIMUM;
    t_iemcolors colors  = IEM_DEFAULT_COLORS;
    
    if (argc >= 17                                                              // --
            && IS_FLOAT (argv + 0)                                              // Number of digits.
            && IS_FLOAT (argv + 1)                                              // Height.
            && IS_FLOAT (argv + 2)                                              // Range minimum.
            && IS_FLOAT (argv + 3)                                              // Range maximum.
            && IS_FLOAT (argv + 4)                                              // Is logarithmic.
            && IS_FLOAT (argv + 5)                                              // Loadbang.
            && (IS_SYMBOL (argv + 6) || IS_FLOAT (argv + 6))                    // Send.
            && (IS_SYMBOL (argv + 7) || IS_FLOAT (argv + 7))                    // Receive.
            && (IS_SYMBOL (argv + 8) || IS_FLOAT (argv + 8))                    // Label.
            && IS_FLOAT (argv + 9)                                              // Label X.
            && IS_FLOAT (argv + 10)                                             // Label Y.
            && IS_FLOAT (argv + 11)                                             // Label font.
            && IS_FLOAT (argv + 12)                                             // Label font size.
            && IS_FLOAT (argv + 13)                                             // Background color.
            && IS_FLOAT (argv + 14)                                             // Foreground color.
            && IS_FLOAT (argv + 15)                                             // Label color.
            && IS_FLOAT (argv + 16))                                            // Value.
    {
        digits                      = (int)atom_getFloatAtIndex (0,  argc, argv);
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
        value                       = atom_getFloatAtIndex (16, argc, argv);
        
        if (argc == 18 && IS_FLOAT (argv + 17)) {
            steps = (int)atom_getFloatAtIndex (17, argc, argv);
        }
    
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (5, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 6, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (11, argc, argv));
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 6, NULL);
    }
    
    x->x_gui.iem_glist      = (t_glist *)canvas_getcurrent();
    x->x_gui.iem_draw       = (t_iemfn)dial_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_width      = 0;
    x->x_gui.iem_height     = PD_MAX (height, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    x->x_digitsFontSize     = IEM_DEFAULT_FONTSIZE;
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    iemgui_deserializeColors (&x->x_gui, &colors);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_isLogarithmic  = (isLogarithmic != 0);
    x->x_steps          = PD_MAX (steps, 1);
    x->x_digitsNumber   = PD_MAX (digits, 1);
    
    dial_setRange (x, minimum, maximum);
    
    if (x->x_gui.iem_loadbang) { dial_set (x, value); }
    else {
        dial_set (x, 0.0);
    }

    outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void dial_free (t_dial *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
    
    gfxstub_deleteforkey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dial_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("nbx"),
        (t_newmethod)dial_new,
        (t_method)dial_free,
        sizeof (t_dial),
        CLASS_DEFAULT,
        A_GIMME,
        A_NULL);
        
    class_addBang (c, dial_bang);
    class_addFloat (c, dial_float);
    class_addList (c, dial_list);
    class_addClick (c, dial_click);
    class_addMotion (c, dial_motion);

    class_addMethod (c, (t_method)dial_loadbang,        gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)dial_initialize,      gensym ("initialize"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_dialog,          gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_size,            gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_move,            gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_position,        gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_labelFont,       gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_labelPosition,   gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_range,           gensym ("range"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_set,             gensym ("set"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_steps,           gensym ("steps"),           A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_logarithmic,     gensym ("logarithmic"),     A_NULL);
    class_addMethod (c, (t_method)dial_linear,          gensym ("linear"),          A_NULL);
    class_addMethod (c, (t_method)dial_send,            gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)dial_receive,         gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)dial_label,           gensym ("label"),           A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)dial_initialize,      gensym ("init"),            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_move,            gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_position,        gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_dummy,           gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_labelPosition,   gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_labelFont,       gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_steps,           gensym ("log_height"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_logarithmic,     gensym ("log"),             A_NULL);
    class_addMethod (c, (t_method)dial_linear,          gensym ("lin"),             A_NULL);
    
    class_addCreator ((t_newmethod)dial_new, gensym ("my_numbox"), A_GIMME, A_NULL);
    
    #endif
    
    dial_widgetBehavior.w_getrectfn  = dial_behaviorGetRectangle;
    dial_widgetBehavior.w_displacefn = iemgui_behaviorDisplace;
    dial_widgetBehavior.w_selectfn   = iemgui_behaviorSelected;
    dial_widgetBehavior.w_activatefn = NULL;
    dial_widgetBehavior.w_deletefn   = iemgui_behaviorDeleted;
    dial_widgetBehavior.w_visfn      = iemgui_behaviorVisible;
    dial_widgetBehavior.w_clickfn    = dial_behaviorClick;
    
    class_setWidgetBehavior (c, &dial_widgetBehavior);
    class_setHelpName (c, gensym ("nbx"));
    class_setSaveFunction (c, dial_behaviorSave);
    class_setPropertiesFunction (c, dial_behaviorProperties);
    
    dial_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
