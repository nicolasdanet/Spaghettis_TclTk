
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_DIAL_DEFAULT_DIGITS         5
#define IEM_DIAL_DEFAULT_STEPS          127
#define IEM_DIAL_DEFAULT_SIZE           40
#define IEM_DIAL_DEFAULT_MINIMUM        0
#define IEM_DIAL_DEFAULT_MAXIMUM        127
       
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IEM_DIAL_ANGULAR_RANGE          300
#define IEM_DIAL_ANGULAR_OFFSET         (90 + ((360 - IEM_DIAL_ANGULAR_RANGE) / 2))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IEM_DIAL_MAXIMUM_STEPS          (1024 * 1024)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IEM_DIAL_SHOW_KNOB              30

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void dial_set                    (t_dial *, t_float);
static void dial_motion                 (t_dial *, t_float, t_float, t_float);
static void dial_behaviorGetRectangle   (t_gobj *, t_glist *, t_rectangle *);
static int  dial_behaviorMouse          (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void dial_setString (t_dial *x)
{
    t_error err = string_sprintf (x->x_t, IEM_DIGITS_SIZE, "%f", x->x_floatValue);
    PD_ASSERT (!err);
    PD_ASSERT (x->x_digitsNumber < IEM_DIGITS_SIZE);
    PD_UNUSED (err);
    x->x_t[x->x_digitsNumber] = 0;
}

static int dial_hasKnob (t_dial *x, t_glist *glist)
{
    int k = dial_getWidthDigits (x);
    int t = (x->x_gui.iem_height >= PD_MAX (IEM_DIAL_SHOW_KNOB, k));
    
    if (x->x_hasKnob != t) {
    
        t_glist *view = glist_getView (glist);
        
        if (t) {
        
            gui_vAdd ("%s.c itemconfigure %lxARC -state normal\n",
                            glist_getTagAsString (view),
                            x);
            gui_vAdd ("%s.c itemconfigure %lxNEEDLE -state normal\n",
                            glist_getTagAsString (view),
                            x);
        } else {
            
            gui_vAdd ("%s.c itemconfigure %lxARC -state hidden\n",
                            glist_getTagAsString (view),
                            x);
            gui_vAdd ("%s.c itemconfigure %lxNEEDLE -state hidden\n",
                            glist_getTagAsString (view),
                            x);
        }
        
        x->x_hasKnob = t;
    }
    
    return x->x_hasKnob;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void dial_drawJob (t_gobj *z, t_glist *glist)
{
    t_dial *x = (t_dial *)z;
    
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int h = x->x_digitsFontSize;
    int w = dial_getWidth (x);
    int m = a + (w / 2);
    int n = b + ((x->x_gui.iem_height - h) / 2);
    
    dial_setString (x);

    gui_vAdd ("%s.c coords %lxNEEDLE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    m,
                    n,
                    dial_getNeedleTopX (x, m, ((w - h) / 2.0) + 2),
                    dial_getNeedleTopY (x, n, ((w - h) / 2.0) + 2));
    
    gui_vAdd ("%s.c itemconfigure %lxNUMBER -fill #%06x -text {%s}\n",   // --
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorForeground,
                    x->x_t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void dial_drawUpdate (t_dial *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, dial_drawJob);
}

static void dial_drawMove (t_dial *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int k = x->x_gui.iem_height - (x->x_digitsFontSize / 2);
    int h = x->x_digitsFontSize;
    int w = dial_getWidth (x);
    int m = a + (w / 2);
    int n = b + ((x->x_gui.iem_height - h) / 2);
    
    gui_vAdd ("%s.c coords %lxARC %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + 2 + (h / 2),
                    b + 2,
                    a - 2 + w - (h / 2),
                    b - 2 + x->x_gui.iem_height - h);
    gui_vAdd ("%s.c coords %lxNEEDLE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    m,
                    n,
                    dial_getNeedleTopX (x, m, ((w - h) / 2.0) + 2),
                    dial_getNeedleTopY (x, n, ((w - h) / 2.0) + 2));
    gui_vAdd ("%s.c coords %lxNUMBER %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + 1 + (w / 2),
                    b + k);
}

static void dial_drawNew (t_dial *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int k = x->x_gui.iem_height - (x->x_digitsFontSize / 2);
    int h = x->x_digitsFontSize;
    int w = dial_getWidth (x);
    int m = a + (w / 2);
    int n = b + ((x->x_gui.iem_height - h) / 2);
    
    dial_setString (x);
    
    gui_vAdd ("%s.c create arc %d %d %d %d"
                    " -width 2"
                    " -start %d"
                    " -extent %d"
                    " -outline #%06x"
                    " -style arc"
                    " -tags %lxARC\n",
                    glist_getTagAsString (view),
                    a + 2 + (h / 2),
                    b + 2,
                    a - 2 + w - (h / 2),
                    b - 2 + x->x_gui.iem_height - h,
                    -IEM_DIAL_ANGULAR_OFFSET,
                    -IEM_DIAL_ANGULAR_RANGE,
                    x->x_gui.iem_colorForeground,
                    x);
    gui_vAdd ("%s.c create line %d %d %d %d"
                    " -width 2"
                    " -fill #%06x"
                    " -tags %lxNEEDLE\n",
                    glist_getTagAsString (view),
                    m,
                    n,
                    dial_getNeedleTopX (x, m, ((w - h) / 2.0) + 2),
                    dial_getNeedleTopY (x, n, ((w - h) / 2.0) + 2),
                    x->x_gui.iem_colorForeground,
                    x);
    gui_vAdd ("%s.c create text %d %d -text {%s}"    // --
                    " -anchor center"
                    " -font [::getFont %d]"             // --
                    " -fill #%06x"
                    " -tags %lxNUMBER\n",
                    glist_getTagAsString (view),
                    a + 1 + (w / 2),
                    b + k,
                    x->x_t, 
                    x->x_digitsFontSize,
                    x->x_gui.iem_colorForeground,
                    x);
    
    dial_hasKnob (x, glist);
}

static void dial_drawSelect (t_dial *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    gui_vAdd ("%s.c itemconfigure %lxARC -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
    gui_vAdd ("%s.c itemconfigure %lxNEEDLE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
    gui_vAdd ("%s.c itemconfigure %lxNUMBER -fill #%06x\n",
                    glist_getTagAsString (view),
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
}

static void dial_drawErase (t_dial *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    gui_vAdd ("%s.c delete %lxARC\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxNEEDLE\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxNUMBER\n",
                    glist_getTagAsString (view),
                    x);
}

static void dial_drawConfig (t_dial *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxARC -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorForeground);
    gui_vAdd ("%s.c itemconfigure %lxNEEDLE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorForeground);
    gui_vAdd ("%s.c itemconfigure %lxNUMBER -font [::getFont %d] -fill #%06x\n",             // --
                    glist_getTagAsString (view),
                    x, 
                    x->x_digitsFontSize,
                    x->x_gui.iem_colorForeground);
    
    dial_hasKnob (x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
// MARK: -

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
// MARK: -

static void dial_out (t_dial *x)
{
    outlet_float (x->x_outlet, x->x_floatValue);
    
    if (x->x_gui.iem_canSend && symbol_hasThing (x->x_gui.iem_send)) {
        pd_float (symbol_getThing (x->x_gui.iem_send), x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    glist_setMotion (x->x_gui.iem_owner, cast_gobj (x), (t_motionfn)dial_motion, a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
// MARK: -

static void dial_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_dial *x = (t_dial *)z;
    
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
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
        x->x_digitsNumber,
        x->x_gui.iem_height,
        (t_float)x->x_minimum,
        (t_float)x->x_maximum,
        x->x_isLogarithmic,
        iemgui_serializeLoadbang (cast_iem (z)),
        names.n_unexpandedSend,
        names.n_unexpandedReceive,
        names.n_unexpandedLabel,
        x->x_gui.iem_labelX,
        x->x_gui.iem_labelY,
        iemgui_serializeFontStyle (cast_iem (z)),
        x->x_gui.iem_fontSize,
        colors.c_symColorBackground,
        colors.c_symColorForeground,
        colors.c_symColorLabel,
        x->x_floatValue,
        x->x_steps);
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
            " %d %d"
            " -1\n",
            x->x_digitsNumber, 1, x->x_gui.iem_height, IEM_MINIMUM_HEIGHT,
            x->x_minimum, x->x_maximum,
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            x->x_steps, IEM_DIAL_MAXIMUM_STEPS,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void dial_fromDialog (t_dial *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0    = x->x_gui.iem_height;
    int t1    = x->x_isLogarithmic;
    int t2    = x->x_digitsNumber;
    int t3    = x->x_steps;
    double t4 = x->x_minimum;
    double t5 = x->x_maximum;
    
    {
    //
    int digits          = (int)atom_getFloatAtIndex (0, argc, argv);
    int height          = (int)atom_getFloatAtIndex (1, argc, argv);
    double minimum      = (double)atom_getFloatAtIndex (2, argc, argv);
    double maximum      = (double)atom_getFloatAtIndex (3, argc, argv);
    int isLogarithmic   = (int)atom_getFloatAtIndex (4, argc, argv);
    int steps           = (int)atom_getFloatAtIndex (6, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);
    
    x->x_gui.iem_height = PD_MAX (height, IEM_MINIMUM_HEIGHT);
    x->x_isLogarithmic  = (isLogarithmic != 0);
    x->x_digitsNumber   = PD_CLAMP (digits, 1, IEM_DIGITS_SIZE - 1);
    x->x_steps          = PD_MAX (steps, 1);
    x->x_position       = PD_MIN (x->x_position, x->x_steps);
    
    dial_setRange (x, minimum, maximum);
    
    x->x_floatValue = dial_getValue (x);
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_height);
    isDirty |= (t1 != x->x_isLogarithmic);
    isDirty |= (t2 != x->x_digitsNumber);
    isDirty |= (t3 != x->x_steps);
    isDirty |= (t4 != x->x_minimum);
    isDirty |= (t5 != x->x_maximum);
    
    if (isDirty) { iemgui_boxChanged ((void *)x); glist_setDirty (cast_iem (x)->iem_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dial_new (t_symbol *s, int argc, t_atom *argv)
{
    t_dial *x = (t_dial *)pd_new (dial_class);
    
    int digits          = IEM_DIAL_DEFAULT_DIGITS;
    int height          = IEM_DIAL_DEFAULT_SIZE;
    int isLogarithmic   = 0;
    int labelX          = 0;
    int labelY          = 0;
    int labelFontSize   = IEM_DEFAULT_FONT;
    int steps           = IEM_DIAL_DEFAULT_STEPS;
    double minimum      = IEM_DIAL_DEFAULT_MINIMUM;
    double maximum      = IEM_DIAL_DEFAULT_MAXIMUM;
    double value        = IEM_DIAL_DEFAULT_MINIMUM;
    
    if (argc < 17) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    digits          = (int)atom_getFloatAtIndex (0,  argc, argv);
    height          = (int)atom_getFloatAtIndex (1,  argc, argv);
    minimum         = (double)atom_getFloatAtIndex (2, argc, argv);
    maximum         = (double)atom_getFloatAtIndex (3, argc, argv);
    isLogarithmic   = (int)atom_getFloatAtIndex (4,  argc, argv);
    labelX          = (int)atom_getFloatAtIndex (9,  argc, argv);
    labelY          = (int)atom_getFloatAtIndex (10, argc, argv);
    labelFontSize   = (int)atom_getFloatAtIndex (12, argc, argv);
    value           = atom_getFloatAtIndex (16, argc, argv);
    
    if (argc == 18) { steps = (int)atom_getFloatAtIndex (17, argc, argv); }

    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (5, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 6, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (11, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 13, argv + 14, argv + 15);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)dial_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = 0;
    x->x_gui.iem_height     = PD_MAX (height, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = labelFontSize;
    x->x_hasKnob            = -1;
    x->x_digitsFontSize     = IEM_DEFAULT_FONT;
    
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
// MARK: -

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
    
    class_addCreator ((t_newmethod)dial_new, sym_dial, A_GIMME, A_NULL);
    
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
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_range,                   sym_range,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)dial_set,                     sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_steps,                   sym_steps,              A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)dial_logarithmic,             sym_logarithmic,        A_NULL);
    class_addMethod (c, (t_method)dial_linear,                  sym_linear,             A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)dial_initialize,              sym_init,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabel,              sym_label,              A_DEFSYMBOL, A_NULL);
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
    class_free (dial_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
