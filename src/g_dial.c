
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
#include "s_system.h"
#include "g_graphics.h"

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

#define IEM_DIAL_ANGULAR_RANGE      300
#define IEM_DIAL_ANGULAR_OFFSET     (90 + ((360 - IEM_DIAL_ANGULAR_RANGE) / 2))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DIAL_MAXIMUM_STEPS      (1024 * 1024)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DIAL_SHOW_KNOB          30

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_set                    (t_dial *, t_float);
static void dial_motion                 (t_dial *, t_float, t_float, t_float);
static void dial_behaviorGetRectangle   (t_gobj *, t_glist *, t_rectangle *);
static int  dial_behaviorMouse          (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *dial_class;                         /* Shared. */

static t_widgetbehavior dial_widgetBehavior =       /* Shared. */
    {
        dial_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        dial_behaviorMouse,
    };
    
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

static double dial_getNeedleAngle (t_dial *x)
{
    int degrees    = (int)(((double)x->x_position / x->x_steps) * IEM_DIAL_ANGULAR_RANGE);
    double radians = PD_TO_RADIANS (PD_CLAMP (degrees, 0, IEM_DIAL_ANGULAR_RANGE) + IEM_DIAL_ANGULAR_OFFSET);
    return radians;
}

static int dial_getNeedleTopX (t_dial *x, int m, double distance)
{
    return (int)(m + PD_MAX (distance, 1.0) * cos (dial_getNeedleAngle (x)));
}

static int dial_getNeedleTopY (t_dial *x, int n, double distance)
{
    return (int)(n + PD_MAX (distance, 1.0) * sin (dial_getNeedleAngle (x)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_setString (t_dial *x)
{
    t_error err = string_sprintf (x->x_t, IEM_DIGITS_SIZE, "%f", x->x_floatValue);
    PD_ASSERT (!err);
    PD_ASSERT (x->x_digitsNumber < IEM_DIGITS_SIZE);
    x->x_t[x->x_digitsNumber] = 0;
}

static int dial_hasKnob (t_dial *x, t_glist *glist)
{
    int t = (x->x_gui.iem_height >= IEM_DIAL_SHOW_KNOB);
    
    if (x->x_hasKnob != t) {
    
        t_glist *canvas = glist_getView (glist);
        
        if (t) {
        
            sys_vGui (".x%lx.c itemconfigure %lxARC -state normal\n",
                            canvas,
                            x);
            sys_vGui (".x%lx.c itemconfigure %lxNEEDLE -state normal\n",
                            canvas,
                            x);
        } else {
            
            sys_vGui (".x%lx.c itemconfigure %lxARC -state hidden\n",
                            canvas,
                            x);
            sys_vGui (".x%lx.c itemconfigure %lxNEEDLE -state hidden\n",
                            canvas,
                            x);
        }
        
        x->x_hasKnob = t;
    }
    
    return x->x_hasKnob;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_drawJob (t_gobj *z, t_glist *glist)
{
    t_dial *x = (t_dial *)z;
    
    t_glist *canvas = glist_getView (glist);
    
    int a = object_getPixelX (cast_object (x), glist);
    int b = object_getPixelY (cast_object (x), glist);
    int h = x->x_digitsFontSize;
    int w = dial_getWidth (x);
    int m = a + (w / 2);
    int n = b + ((x->x_gui.iem_height - h) / 2);
    
    dial_setString (x);

    sys_vGui (".x%lx.c coords %lxNEEDLE %d %d %d %d\n",
                    canvas,
                    x,
                    m,
                    n,
                    dial_getNeedleTopX (x, m, ((w - h) / 2.0) + 2),
                    dial_getNeedleTopY (x, n, ((w - h) / 2.0) + 2));
    
    sys_vGui (".x%lx.c itemconfigure %lxNUMBER -fill #%06x -text {%s}\n",   // --
                    canvas,
                    x,
                    x->x_gui.iem_colorForeground,
                    x->x_t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_drawUpdate (t_dial *x, t_glist *glist)
{
    defer_addJob ((void *)x, glist, dial_drawJob);
}

static void dial_drawMove (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getView (glist);
    
    int a = object_getPixelX (cast_object (x), glist);
    int b = object_getPixelY (cast_object (x), glist);
    int k = x->x_gui.iem_height - (x->x_digitsFontSize / 2);
    int h = x->x_digitsFontSize;
    int w = dial_getWidth (x);
    int m = a + (w / 2);
    int n = b + ((x->x_gui.iem_height - h) / 2);
    
    sys_vGui (".x%lx.c coords %lxARC %d %d %d %d\n",
                    canvas,
                    x,
                    a + 2 + (h / 2),
                    b + 2,
                    a - 2 + w - (h / 2),
                    b - 2 + x->x_gui.iem_height - h);
    sys_vGui (".x%lx.c coords %lxNEEDLE %d %d %d %d\n",
                    canvas,
                    x,
                    m,
                    n,
                    dial_getNeedleTopX (x, m, ((w - h) / 2.0) + 2),
                    dial_getNeedleTopY (x, n, ((w - h) / 2.0) + 2));
    sys_vGui (".x%lx.c coords %lxNUMBER %d %d\n",
                    canvas,
                    x,
                    a + 1 + (w / 2),
                    b + k);
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                    canvas,
                    x,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY);
}

static void dial_drawNew (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getView (glist);
    
    int a = object_getPixelX (cast_object (x), glist);
    int b = object_getPixelY (cast_object (x), glist);
    int k = x->x_gui.iem_height - (x->x_digitsFontSize / 2);
    int h = x->x_digitsFontSize;
    int w = dial_getWidth (x);
    int m = a + (w / 2);
    int n = b + ((x->x_gui.iem_height - h) / 2);
    
    dial_setString (x);
    
    sys_vGui (".x%lx.c create arc %d %d %d %d"
                    " -width 2"
                    " -start %d"
                    " -extent %d"
                    " -outline #%06x"
                    " -style arc"
                    " -tags %lxARC\n",
                    canvas,
                    a + 2 + (h / 2),
                    b + 2,
                    a - 2 + w - (h / 2),
                    b - 2 + x->x_gui.iem_height - h,
                    -IEM_DIAL_ANGULAR_OFFSET,
                    -IEM_DIAL_ANGULAR_RANGE,
                    x->x_gui.iem_colorForeground,
                    x);
    sys_vGui (".x%lx.c create line %d %d %d %d"
                    " -width 2"
                    " -fill #%06x"
                    " -tags %lxNEEDLE\n",
                    canvas,
                    m,
                    n,
                    dial_getNeedleTopX (x, m, ((w - h) / 2.0) + 2),
                    dial_getNeedleTopY (x, n, ((w - h) / 2.0) + 2),
                    x->x_gui.iem_colorForeground,
                    x);
    sys_vGui (".x%lx.c create text %d %d -text {%s}"    // --
                    " -anchor center"
                    " -font [::getFont %d]"             // --
                    " -fill #%06x"
                    " -tags %lxNUMBER\n",
                    canvas,
                    a + 1 + (w / 2),
                    b + k,
                    x->x_t, 
                    x->x_digitsFontSize,
                    x->x_gui.iem_colorForeground,
                    x);
    sys_vGui (".x%lx.c create text %d %d -text {%s}"    // --
                    " -anchor w"
                    " -font [::getFont %d]"             // --
                    " -fill #%06x"
                    " -tags %lxLABEL\n",
                    canvas,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY,
                    (x->x_gui.iem_label != utils_empty()) ? x->x_gui.iem_label->s_name : "",
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_colorLabel,
                    x);
    
    dial_hasKnob (x, glist);
}

static void dial_drawSelect (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getView (glist);
    
    sys_vGui (".x%lx.c itemconfigure %lxARC -outline #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxNEEDLE -fill #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxNUMBER -fill #%06x\n",
                    canvas,
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

static void dial_drawErase (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getView (glist);
    
    sys_vGui (".x%lx.c delete %lxARC\n",
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

static void dial_drawConfig (t_dial *x, t_glist *glist)
{
    t_glist *canvas = glist_getView (glist);

    sys_vGui (".x%lx.c itemconfigure %lxARC -outline #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxNEEDLE -fill #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxNUMBER -font [::getFont %d] -fill #%06x\n",             // --
                    canvas,
                    x, 
                    x->x_digitsFontSize,
                    x->x_gui.iem_colorForeground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",   // --
                    canvas,
                    x,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel,
                    (x->x_gui.iem_label != utils_empty()) ? x->x_gui.iem_label->s_name : "");
    
    dial_hasKnob (x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dial_draw (t_dial *x, t_glist *glist, int mode)
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
        error_invalid (sym_nbx, sym_range);
        
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
    outlet_float (x->x_outlet, x->x_floatValue);
    
    if (x->x_gui.iem_canSend && pd_isThing (x->x_gui.iem_send)) {
        pd_float (pd_getThing (x->x_gui.iem_send), x->x_floatValue);
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

static void dial_click (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    
    canvas_setMotionFunction (x->x_gui.iem_owner, cast_gobj (x), (t_motionfn)dial_motion, a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_motion (t_dial *x, t_float deltaX, t_float deltaY, t_float modifier)
{
    int old = x->x_position;
    int t = old;
    int k = (int)(-deltaY);
    int shift = (int)modifier & MODIFIER_SHIFT;
    
    if (!shift) { k *= PD_MAX (1, (int)(x->x_steps * 0.05)); }
    
    t += k;
    
    t = PD_CLAMP (t, 0, x->x_steps);
    
    if (t != old) {
        x->x_position   = t;
        x->x_floatValue = dial_getValue (x);
        (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
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

static void dial_size (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int digits = (int)atom_getFloatAtIndex (0, argc, argv);

    x->x_digitsNumber = PD_CLAMP (digits, 1, IEM_DIGITS_SIZE - 1);
    
    if (argc > 1) {
        int height = (int)atom_getFloatAtIndex (1, argc, argv);
        x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    }
    
    iemgui_boxChanged ((void *)x);
    //
    }
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
    
    f = (t_float)PD_CLAMP (f, x->x_minimum, x->x_maximum);
    
    if (x->x_isLogarithmic) { 
        x->x_position = (int)(log (f / x->x_minimum) / dial_getStepValue (x));
    } else {
        x->x_position = (int)((f - x->x_minimum) / dial_getStepValue (x));
    }
    
    x->x_floatValue = dial_getValue (x);
    
    if (x->x_floatValue != old) { (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_dial *x = (t_dial *)z;
    
    int a = object_getPixelX (cast_object (z), glist);
    int b = object_getPixelY (cast_object (z), glist);
    int c = a + dial_getWidth (x);
    int d = b + cast_iem (z)->iem_height;
    
    rectangle_set (r, a, b, c, d);
}

static int dial_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { dial_click ((t_dial *)z, NULL, mouse_argc (m), mouse_argv (m)); }
    
    return 1;
}

static void dial_functionSave (t_gobj *z, t_buffer *b)
{
    t_dial *x = (t_dial *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);

    buffer_vAppend (b, "ssiisiiffiisssiiiisssfi;",
        sym___hash__X,
        sym_obj,
        object_getX (cast_object (z)),
        object_getY (cast_object (z)),
        sym_nbx,
        x->x_digitsNumber,                                                      // Number of digits.
        x->x_gui.iem_height,                                                    // Height.
        (t_float)x->x_minimum,                                                  // Range minimum.
        (t_float)x->x_maximum,                                                  // Range maximum.
        x->x_isLogarithmic,                                                     // Is logarithmic.
        iemgui_serializeLoadbang (cast_iem (z)),                                // Loadbang.
        names.n_unexpandedSend,                                                 // Send.
        names.n_unexpandedReceive,                                              // Receive.
        names.n_unexpandedLabel,                                                // Label.
        x->x_gui.iem_labelX,                                                    // Label X.
        x->x_gui.iem_labelY,                                                    // Label Y.
        iemgui_serializeFontStyle (cast_iem (z)),                               // Label font.
        x->x_gui.iem_fontSize,                                                  // Label font size.
        colors.c_symColorBackground,                                            // Background color.
        colors.c_symColorForeground,                                            // Foreground color.
        colors.c_symColorLabel,                                                 // Label color.
        x->x_floatValue,                                                        // Value.
        x->x_steps);                                                            // Steps.
}

static void dial_functionProperties (t_gobj *z, t_glist *owner)
{
    t_dial *x = (t_dial *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);

    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Dial"
            " %d %d Digits %d %d Size"
            " %g {Value Low} %g {Value High}"   // --
            " %d Linear Logarithmic"
            " %d"
            " %d %d {Steps}"                    // --
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
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void dial_fromDialog (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int digits          = (int)atom_getFloatAtIndex (0, argc, argv);
    int height          = (int)atom_getFloatAtIndex (1, argc, argv);
    double minimum      = (double)atom_getFloatAtIndex (2, argc, argv);
    double maximum      = (double)atom_getFloatAtIndex (3, argc, argv);
    int isLogarithmic   = (int)atom_getFloatAtIndex (4, argc, argv);
    int steps           = (int)atom_getFloatAtIndex (6, argc, argv);
    
    iemgui_fromDialog (cast_iem (x), argc, argv);
    
    x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    x->x_isLogarithmic  = (isLogarithmic != 0);
    x->x_digitsNumber   = PD_CLAMP (digits, 1, IEM_DIGITS_SIZE - 1);
    x->x_steps          = PD_MAX (steps, 1);
    x->x_position       = PD_MIN (x->x_position, x->x_steps);
    
    dial_setRange (x, minimum, maximum);
    
    x->x_floatValue = dial_getValue (x);
        
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_CONFIG);
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_MOVE);
    
    canvas_updateLinesByObject (x->x_gui.iem_owner, cast_object (x));
    //
    }
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
    
    if (argc >= 17                                                              // --
            && IS_FLOAT (argv + 0)                                              // Number of digits.
            && IS_FLOAT (argv + 1)                                              // Height.
            && IS_FLOAT (argv + 2)                                              // Range minimum.
            && IS_FLOAT (argv + 3)                                              // Range maximum.
            && IS_FLOAT (argv + 4)                                              // Is logarithmic.
            && IS_FLOAT (argv + 5)                                              // Loadbang.
            && IS_SYMBOL_OR_FLOAT (argv + 6)                                    // Send.
            && IS_SYMBOL_OR_FLOAT (argv + 7)                                    // Receive.
            && IS_SYMBOL_OR_FLOAT (argv + 8)                                    // Label.
            && IS_FLOAT (argv + 9)                                              // Label X.
            && IS_FLOAT (argv + 10)                                             // Label Y.
            && IS_FLOAT (argv + 11)                                             // Label font.
            && IS_FLOAT (argv + 12)                                             // Label font size.
            && IS_SYMBOL_OR_FLOAT (argv + 13)                                   // Background color.
            && IS_SYMBOL_OR_FLOAT (argv + 14)                                   // Foreground color.
            && IS_SYMBOL_OR_FLOAT (argv + 15)                                   // Label color.
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
        value                       = atom_getFloatAtIndex (16, argc, argv);
        
        if (argc == 18 && IS_FLOAT (argv + 17)) {
            steps = (int)atom_getFloatAtIndex (17, argc, argv);
        }
    
        iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (5, argc, argv));
        iemgui_deserializeNames (cast_iem (x), 6, argv);
        iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (11, argc, argv));
        iemgui_deserializeColors (cast_iem (x), argv + 13, argv + 14, argv + 15);
        
    } else {
        iemgui_deserializeNames (cast_iem (x), 6, NULL);
        iemgui_deserializeColors (cast_iem (x), NULL, NULL, NULL);
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)dial_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == utils_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == utils_empty()) ? 0 : 1;
    x->x_gui.iem_width      = 0;
    x->x_gui.iem_height     = PD_MAX (height, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    x->x_hasKnob            = -1;
    x->x_digitsFontSize     = IEM_DEFAULT_FONTSIZE;
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_isLogarithmic  = (isLogarithmic != 0);
    x->x_steps          = PD_MAX (steps, 1);
    x->x_digitsNumber   = PD_CLAMP (digits, 1, IEM_DIGITS_SIZE - 1);
    
    dial_setRange (x, minimum, maximum);
    
    if (x->x_gui.iem_loadbang) { dial_set (x, (t_float)value); }
    else {
        dial_set (x, (t_float)0.0);
    }

    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void dial_free (t_dial *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dial_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_nbx,
            (t_newmethod)dial_new,
            (t_method)dial_free,
            sizeof (t_dial),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    class_addBang (c, (t_method)dial_bang);
    class_addFloat (c, (t_method)dial_float);
    class_addList (c, (t_method)dial_list);
    class_addClick (c, (t_method)dial_click);

    class_addMethod (c, (t_method)dial_loadbang,                sym_loadbang,           A_NULL);
    class_addMethod (c, (t_method)dial_initialize,              sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_fromDialog,              sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_size,                    sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_labelfont,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_labelposition,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelColor,         sym_labelcolor,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_range,                   sym_range,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_set,                     sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_steps,                   sym_steps,              A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_logarithmic,             sym_logarithmic,        A_NULL);
    class_addMethod (c, (t_method)dial_linear,                  sym_linear,             A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabel,              sym_label,              A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)dial_initialize,              sym_init,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_steps,                   sym_log_height,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_logarithmic,             sym_log,                A_NULL);
    class_addMethod (c, (t_method)dial_linear,                  sym_lin,                A_NULL);
    
    class_addCreator ((t_newmethod)dial_new, sym_my_numbox, A_GIMME, A_NULL);
    
    #endif

    class_setWidgetBehavior (c, &dial_widgetBehavior);
    class_setSaveFunction (c, dial_functionSave);
    class_setPropertiesFunction (c, dial_functionProperties);
    
    dial_class = c;
}

void dial_destroy (void)
{
    CLASS_FREE (dial_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
