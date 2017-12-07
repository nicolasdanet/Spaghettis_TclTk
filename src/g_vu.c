
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

#define IEM_VUMETER_THICKNESS               3
#define IEM_VUMETER_THICKNESS_MINIMUM       2
#define IEM_VUMETER_THICKNESS_MAXIMUM       5
#define IEM_VUMETER_STEPS                   40

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int vu_colors[41] =
    {
        0x000000,
        0x14e814,   // Green.
        0x14e814,       
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0xe8e828,   // Yellow.
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xfcac44,   // Orange.
        0xfcac44,
        0xfcac44,
        0xfcac44,
        0xfcac44,
        0xfc2828,   // Red.
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xf430f0,   // Violet.
        0x000000
    };

static int vu_decibelToStep[226] =
    {
        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
        4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
        5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
        6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
        7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
        8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
        9,  9,  9,  9,  9,  10, 10, 10, 10, 10,
        11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
        13, 13, 13, 13, 14, 14, 14, 14, 15, 15,
        15, 15, 16, 16, 16, 16, 17, 17, 17, 18,
        18, 18, 19, 19, 19, 20, 20, 20, 21, 21,
        22, 22, 23, 23, 24, 24, 25, 26, 27, 28,
        29, 30, 31, 32, 33, 33, 34, 34, 35, 35,
        36, 36, 37, 37, 37, 38, 38, 38, 39, 39,
        39, 39, 39, 39, 39, 39
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline int vu_stepWithDecibels (t_float f)
{
    int i = (int)(2.0 * f); return vu_decibelToStep[PD_CLAMP (i, 0, 225)];
}

static inline int vu_offsetWithStep (t_vu *x, int step)
{
    return (x->x_thickness * (IEM_VUMETER_STEPS - step)) - (x->x_thickness / 2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void vu_behaviorGetRectangle (t_gobj *, t_glist *, t_rectangle *r);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *vu_class;                           /* Shared. */

static t_widgetbehavior vu_widgetBehavior =         /* Shared. */
    {
        vu_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        NULL
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void vu_drawJob (t_gobj *z, t_glist *glist)
{
    t_vu *x = (t_vu *)z;
    t_glist *view = glist_getView (glist);

    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int h = vu_offsetWithStep (x, x->x_decibel) + (x->x_thickness / 2);
    
    gui_vAdd ("%s.c coords %lxCOVER %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + 1,
                    b + 1,
                    a + x->x_gui.iem_width - 1,
                    b + PD_CLAMP (h, 1, (x->x_gui.iem_height - 1)));
                
    if (x->x_peak) {
    //
    h = vu_offsetWithStep (x, x->x_peak);

    gui_vAdd ("%s.c coords %lxPEAK %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + 1,
                    b + h,
                    a + x->x_gui.iem_width,
                    b + h);
    gui_vAdd ("%s.c itemconfigure %lxPEAK -fill #%06x\n",
                    glist_getTagAsString (view), 
                    x,
                    vu_colors[x->x_peak]);
    //
    } else {
    //
    h = vu_offsetWithStep (x, IEM_VUMETER_STEPS - 1);
    
    gui_vAdd ("%s.c coords %lxPEAK %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x, 
                    a + 1,
                    b + h,
                    a + x->x_gui.iem_width,
                    b + h);
    gui_vAdd ("%s.c itemconfigure %lxPEAK -fill #%06x\n",
                    glist_getTagAsString (view), 
                    x, 
                    x->x_gui.iem_colorBackground);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void vu_drawUpdate (t_vu *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, vu_drawJob);
}

static void vu_drawMove (t_vu *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int h, i;

    gui_vAdd ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x, 
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
             
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    h = vu_offsetWithStep (x, i);
    
    gui_vAdd ("%s.c coords %lxLED%d %d %d %d %d\n",
                    glist_getTagAsString (view), 
                    x,
                    i,
                    a + 3,
                    b + h + x->x_thickness,
                    a + x->x_gui.iem_width - 2,
                    b + h + x->x_thickness);
    //
    }

    gui_vAdd ("%s.c coords %lxLABEL %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY);
             
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void vu_drawNew (t_vu *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int h, i;

    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
             
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    h = vu_offsetWithStep (x, i);

    gui_vAdd ("%s.c create line %d %d %d %d -width %d -fill #%06x -tags %lxLED%d\n",
                    glist_getTagAsString (view),
                    a + 3,
                    b + h + x->x_thickness,
                    a + x->x_gui.iem_width - 2,
                    b + h + x->x_thickness,
                    x->x_thickness - 1,
                    vu_colors[i],
                    x,
                    i);
    //
    }

    h = vu_offsetWithStep (x, IEM_VUMETER_STEPS - 1);
    
    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxCOVER\n",
                    glist_getTagAsString (view),
                    a + 1, 
                    b + 1, 
                    a + x->x_gui.iem_width - 1,
                    b + x->x_gui.iem_height - 1,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create line %d %d %d %d -width %d -fill #%06x -tags %lxPEAK\n",
                    glist_getTagAsString (view),
                    a + 1,
                    b + h,
                    a + x->x_gui.iem_width,
                    b + h,
                    x->x_thickness - 1,
                    x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create text %d %d -text {%s} -anchor w"                 // --
                    " -font [::getFont %d] -fill #%06x -tags %lxLABEL\n",   // --
                    glist_getTagAsString (view),
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY,
                    symbol_isNil (x->x_gui.iem_label) ? "" : x->x_gui.iem_label->s_name,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_colorLabel,
                    x);

    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void vu_drawSelect (t_vu *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    gui_vAdd ("%s.c itemconfigure %lxLABEL -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

static void vu_drawErase (t_vu *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    int i;
    
    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    gui_vAdd ("%s.c delete %lxLED%d\n",
                    glist_getTagAsString (view),
                    x,
                    i);
    //
    }

    gui_vAdd ("%s.c delete %lxPEAK\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxCOVER\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxLABEL\n",
                    glist_getTagAsString (view),
                    x);
}

static void vu_drawConfig (t_vu *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    int i;
        
    gui_vAdd ("%s.c itemconfigure %lxBASE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground);
                
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    gui_vAdd ("%s.c itemconfigure %lxLED%d -width %d\n",
                    glist_getTagAsString (view),
                    x,
                    i,
                    x->x_thickness - 1);
    //
    }

    gui_vAdd ("%s.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",   // --
                    glist_getTagAsString (view),
                    x,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel,
                    symbol_isNil (x->x_gui.iem_label) ? "" : x->x_gui.iem_label->s_name);

    gui_vAdd ("%s.c itemconfigure %lxCOVER -fill #%06x -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxPEAK -width %d\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_thickness - 1);
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void vu_draw (t_vu *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : vu_drawUpdate (x, glist); break;
        case IEM_DRAW_MOVE      : vu_drawMove (x, glist);   break;
        case IEM_DRAW_NEW       : vu_drawNew (x, glist);    break;
        case IEM_DRAW_SELECT    : vu_drawSelect (x, glist); break;
        case IEM_DRAW_ERASE     : vu_drawErase (x, glist);  break;
        case IEM_DRAW_CONFIG    : vu_drawConfig (x, glist); break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void vu_setHeight (t_vu *x, int height)
{
    int n = (int)(height / IEM_VUMETER_STEPS);

    x->x_thickness      = PD_CLAMP (n, IEM_VUMETER_THICKNESS_MINIMUM, IEM_VUMETER_THICKNESS_MAXIMUM);
    x->x_gui.iem_height = IEM_VUMETER_STEPS * x->x_thickness;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void vu_bang (t_vu *x)
{
    outlet_float (x->x_outletRight, x->x_peakValue);
    outlet_float (x->x_outletLeft, x->x_decibelValue);
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void vu_floatPeak (t_vu *x, t_float peak)
{
    int old = x->x_peak;
    
    x->x_peakValue = peak;
    x->x_peak = vu_stepWithDecibels (peak);
    
    if (x->x_peak != old) { (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
        
    outlet_float (x->x_outletRight, peak);
}

static void vu_float (t_vu *x, t_float decibel)
{
    int old = x->x_decibel;
    
    x->x_decibelValue = decibel;
    x->x_decibel = vu_stepWithDecibels (decibel);
    
    if (x->x_decibel != old) { (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
    
    if (x->x_decibelValue > x->x_peakValue) { vu_floatPeak (x, decibel); }
    
    outlet_float (x->x_outletLeft, decibel);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void vu_size (t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int width = atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    if (argc > 1) { vu_setHeight (x, (int)atom_getFloatAtIndex (1, argc, argv)); }
    iemgui_boxChanged ((void *)x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void vu_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + cast_iem (z)->iem_width;
    int d = b + cast_iem (z)->iem_height;
    
    rectangle_set (r, a, b, c, d);
}

static void vu_functionSave (t_gobj *z, t_buffer *b)
{
    t_vu *x = (t_vu *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_vAppend (b, "ssiisiissiiiissii;",
        sym___hash__X,
        sym_obj,
        object_getX (cast_object (x)),
        object_getY (cast_object (x)),
        sym_vu,
        x->x_gui.iem_width,
        x->x_gui.iem_height,
        names.n_unexpandedReceive,
        names.n_unexpandedLabel,
        x->x_gui.iem_labelX,
        x->x_gui.iem_labelY,
        iemgui_serializeFontStyle (cast_iem (z)),
        x->x_gui.iem_fontSize,
        colors.c_symColorBackground,
        colors.c_symColorLabel,
        x->x_hasScale,
        0);
}

static void vu_functionProperties (t_gobj *z, t_glist *owner)
{
    t_vu *x = (t_vu *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s VU"
            " %d %d {Meter Width}"          // --
            " %d %d {Led Thickness}"        // --
            " 0 $::var(nil) 0 $::var(nil)"  // --
            " 0 $::var(nil) $::var(nil)"    // --
            " -1"
            " -1 -1 $::var(nil)"            // --
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            (x->x_gui.iem_height / IEM_VUMETER_STEPS) - 1, IEM_VUMETER_THICKNESS_MINIMUM - 1,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel);

    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void vu_fromDialog (t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0 = x->x_gui.iem_width;
    int t1 = x->x_gui.iem_height;
    
    {
    //
    int width     = (int)atom_getFloatAtIndex (0, argc, argv);
    int thickness = (int)atom_getFloatAtIndex (1, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);
    
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    
    vu_setHeight (x, (thickness + 1) * IEM_VUMETER_STEPS);
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_width);
    isDirty |= (t1 != x->x_gui.iem_height);
    
    if (isDirty) { iemgui_boxChanged ((void *)x); glist_setDirty (cast_iem (x)->iem_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *vu_new (t_symbol *s, int argc, t_atom *argv)
{
    t_vu *x = (t_vu *)pd_new (vu_class);

    int width           = IEM_DEFAULT_SIZE;
    int height          = IEM_VUMETER_STEPS * IEM_VUMETER_THICKNESS;
    int labelX          = IEM_DEFAULT_LABELX_NEXT;
    int labelY          = IEM_DEFAULT_LABELY_NEXT;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    int hasScale        = 0;

    if (argc < 11) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    width           = (int)atom_getFloatAtIndex (0,  argc, argv);
    height          = (int)atom_getFloatAtIndex (1,  argc, argv);
    labelX          = (int)atom_getFloatAtIndex (4,  argc, argv);
    labelY          = (int)atom_getFloatAtIndex (5,  argc, argv);
    labelFontSize   = (int)atom_getFloatAtIndex (7,  argc, argv);
    hasScale        = (int)atom_getFloatAtIndex (10, argc, argv);
    
    /* Note that the value of height is pitiably attribute to the send symbol. */
    /* Must be kept for backward compatibility. */
    
    iemgui_deserializeNames (cast_iem (x), 1, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (6, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 8, NULL, argv + 9);
    //
    }

    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)vu_draw;
    x->x_gui.iem_canSend    = 0;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (width, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
        
    vu_setHeight (x, height);
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_hasScale = (hasScale != 0);
    
    inlet_new2 (x, &s_float);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void vu_free (t_vu *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
        
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void vu_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_vu,
            (t_newmethod)vu_new,
            (t_method)vu_free,
            sizeof (t_vu),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    class_addBang (c, (t_method)vu_bang);
    class_addFloat (c, (t_method)vu_float);
    
    class_addMethod (c, (t_method)vu_floatPeak,                 sym__inlet2,            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vu_fromDialog,                sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_size,                      sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_labelfont,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_labelposition,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelColor,         sym_labelcolor,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabel,              sym_label,              A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_scale,              A_GIMME, A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &vu_widgetBehavior);
    class_setSaveFunction (c, vu_functionSave);
    class_setPropertiesFunction (c, vu_functionProperties);
    
    vu_class = c;
}

void vu_destroy (void)
{
    class_free (vu_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
