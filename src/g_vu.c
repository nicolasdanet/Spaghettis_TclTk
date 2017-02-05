
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
#include "m_macros.h"
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
#pragma mark -

int vu_colors[41] =
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
    int i = (int)(2.0 * (f + 100.0)); return vu_decibelToStep[PD_CLAMP (i, 0, 225)];
}

static inline int vu_offsetWithStep (t_vu *x, int step)
{
    return (x->x_thickness * (IEM_VUMETER_STEPS - step)) - (x->x_thickness / 2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_behaviorGetRectangle (t_gobj *, t_glist *, int *, int *, int *, int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    if (canvas_isMapped (glist)) {
    //
    t_glist *canvas = canvas_getView (glist);

    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);
    int h = vu_offsetWithStep (x, x->x_rms) + (x->x_thickness / 2);
    
    sys_vGui (".x%lx.c coords %lxCOVER %d %d %d %d\n",
                    canvas,
                    x,
                    a + 1,
                    b + 1,
                    a + x->x_gui.iem_width - 1,
                    b + PD_CLAMP (h, 1, (x->x_gui.iem_height - 1)));
                
    if (x->x_peak) {
    //
    h = vu_offsetWithStep (x, x->x_peak);

    sys_vGui (".x%lx.c coords %lxPEAK %d %d %d %d\n",
                    canvas,
                    x,
                    a + 1,
                    b + h,
                    a + x->x_gui.iem_width,
                    b + h);
    sys_vGui (".x%lx.c itemconfigure %lxPEAK -fill #%06x\n",
                    canvas, 
                    x,
                    vu_colors[x->x_peak]);
    //
    } else {
    //
    h = vu_offsetWithStep (x, IEM_VUMETER_STEPS - 1);
    
    sys_vGui (".x%lx.c coords %lxPEAK %d %d %d %d\n",
                    canvas,
                    x, 
                    a + 1,
                    b + h,
                    a + x->x_gui.iem_width,
                    b + h);
    sys_vGui (".x%lx.c itemconfigure %lxPEAK -fill #%06x\n",
                    canvas, 
                    x, 
                    x->x_gui.iem_colorBackground);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_drawUpdate (t_vu *x, t_glist *glist)
{
    defer_addTask ((void *)x, glist, vu_drawJob);
}

static void vu_drawMove (t_vu *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);
    int h, i;

    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                    canvas,
                    x, 
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
             
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    h = vu_offsetWithStep (x, i);
    
    sys_vGui (".x%lx.c coords %lxLED%d %d %d %d %d\n",
                    canvas, 
                    x,
                    i,
                    a + 3,
                    b + h + x->x_thickness,
                    a + x->x_gui.iem_width - 2,
                    b + h + x->x_thickness);
    //
    }

    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                    canvas,
                    x,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY);
             
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void vu_drawNew (t_vu *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);
    int h, i;

    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    canvas,
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
             
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    h = vu_offsetWithStep (x, i);

    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -fill #%06x -tags %lxLED%d\n",
                    canvas,
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
    
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxCOVER\n",
                    canvas,
                    a + 1, 
                    b + 1, 
                    a + x->x_gui.iem_width - 1,
                    b + x->x_gui.iem_height - 1,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground,
                    x);
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -fill #%06x -tags %lxPEAK\n",
                    canvas,
                    a + 1,
                    b + h,
                    a + x->x_gui.iem_width,
                    b + h,
                    x->x_thickness - 1,
                    x->x_gui.iem_colorBackground,
                    x);
    sys_vGui (".x%lx.c create text %d %d -text {%s} -anchor w"              // --
                    " -font [::getFont %d] -fill #%06x -tags %lxLABEL\n",   // --
                    canvas,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY,
                    (x->x_gui.iem_label != utils_empty()) ? x->x_gui.iem_label->s_name : "",
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_colorLabel,
                    x);

    (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void vu_drawSelect (t_vu *x, t_glist *glist)
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

static void vu_drawErase (t_vu *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    int i;
    
    sys_vGui (".x%lx.c delete %lxBASE\n",
                    canvas,
                    x);
    
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    sys_vGui (".x%lx.c delete %lxLED%d\n",
                    canvas,
                    x,
                    i);
    //
    }

    sys_vGui (".x%lx.c delete %lxPEAK\n",
                    canvas,
                    x);
    sys_vGui (".x%lx.c delete %lxCOVER\n",
                    canvas,
                    x);
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                    canvas,
                    x);
}

static void vu_drawConfig (t_vu *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    int i;
        
    sys_vGui (".x%lx.c itemconfigure %lxBASE -fill #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_colorBackground);
                
    for (i = 1; i <= IEM_VUMETER_STEPS; i++) {
    //
    sys_vGui (".x%lx.c itemconfigure %lxLED%d -width %d\n",
                    canvas,
                    x,
                    i,
                    x->x_thickness - 1);
    //
    }

    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",   // --
                    canvas,
                    x,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel,
                    (x->x_gui.iem_label != utils_empty()) ? x->x_gui.iem_label->s_name : "");

    sys_vGui (".x%lx.c itemconfigure %lxCOVER -fill #%06x -outline #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxPEAK -width %d\n",
                    canvas,
                    x,
                    x->x_thickness - 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

void vu_setHeight (t_vu *x, int height)
{
    int n = (int)(height / IEM_VUMETER_STEPS);

    x->x_thickness      = PD_CLAMP (n, IEM_VUMETER_THICKNESS_MINIMUM, IEM_VUMETER_THICKNESS_MAXIMUM);
    x->x_gui.iem_height = IEM_VUMETER_STEPS * x->x_thickness;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_bang (t_vu *x)
{
    outlet_float (x->x_outletRight, x->x_peakValue);
    outlet_float (x->x_outletLeft, x->x_rmsValue);
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void vu_float (t_vu *x, t_float rms)
{
    int old = x->x_rms;
    
    x->x_rmsValue = rms;
    x->x_rms = vu_stepWithDecibels (rms);
    
    if (x->x_rms != old) { (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
    
    outlet_float (x->x_outletLeft, rms);
}

static void vu_floatPeak (t_vu *x, t_float peak)
{
    int old = x->x_peak;
    
    x->x_peakValue = peak;
    x->x_peak = vu_stepWithDecibels (peak);
    
    if (x->x_peak != old) { (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
        
    outlet_float (x->x_outletRight, peak);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_size (t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int width = atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    if (argc > 1) { vu_setHeight (x, (int)atom_getFloatAtIndex (1, argc, argv)); }
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    *a = text_getPixelX (cast_object (z), glist);
    *b = text_getPixelY (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static void vu_functionSave (t_gobj *z, t_buffer *b)
{
    t_vu *x = (t_vu *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiissiiiissii",
        sym___hash__X,
        sym_obj,
        cast_object (x)->te_xCoordinate,
        cast_object (x)->te_yCoordinate,
        sym_vu,
        x->x_gui.iem_width,                                         // Width.
        x->x_gui.iem_height,                                        // Height.
        names.n_unexpandedReceive,                                  // Receive.
        names.n_unexpandedLabel,                                    // Label.
        x->x_gui.iem_labelX,                                        // Label X.
        x->x_gui.iem_labelY,                                        // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                      // Label font.
        x->x_gui.iem_fontSize,                                      // Label font size.
        colors.c_symColorBackground,                                // Background color.
        colors.c_symColorLabel,                                     // Label color.
        x->x_hasScale,                                              // Dummy.
        0);                                                         // Dummy.
        
    buffer_appendSemicolon (b);
}

static void vu_functionProperties (t_gobj *z, t_glist *owner)
{
    t_vu *x = (t_vu *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);
    
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
            "nosndno", names.n_unexpandedReceive->s_name,                               /* No send. */
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, -1, x->x_gui.iem_colorLabel);                 /* No foreground. */
            
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

static void vu_fromDialog (t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int width     = (int)atom_getFloatAtIndex (0, argc, argv);
    int thickness = (int)atom_getFloatAtIndex (1, argc, argv);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);
    
    x->x_gui.iem_canSend = 0;    /* Force values that could be misguidedly set. */
    
    x->x_gui.iem_width = PD_MAX (width, IEM_MINIMUM_WIDTH);
    
    vu_setHeight (x, (thickness + 1) * IEM_VUMETER_STEPS);
    
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vu_new (t_symbol *s, int argc, t_atom *argv)
{
    t_vu *x = (t_vu *)pd_new (vu_class);

    int width           = IEM_DEFAULT_SIZE;
    int height          = IEM_VUMETER_STEPS * IEM_VUMETER_THICKNESS;
    int labelX          = IEM_DEFAULT_LABELX_NEXT;
    int labelY          = IEM_DEFAULT_LABELY_NEXT;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    int hasScale        = 0;

    if (argc >= 11                                                  // --
            && IS_FLOAT (argv + 0)                                  // Width.
            && IS_FLOAT (argv + 1)                                  // Height.
            && IS_SYMBOL_OR_FLOAT (argv + 2)                        // Receive.
            && IS_SYMBOL_OR_FLOAT (argv + 3)                        // Label.
            && IS_FLOAT (argv + 4)                                  // Label X.
            && IS_FLOAT (argv + 5)                                  // Label Y.
            && IS_FLOAT (argv + 6)                                  // Label font.
            && IS_FLOAT (argv + 7)                                  // Label font size.
            && IS_SYMBOL_OR_FLOAT (argv + 8)                        // Background color.
            && IS_SYMBOL_OR_FLOAT (argv + 9)                        // Label color.
            && IS_FLOAT (argv + 10))                                // Dummy.
    {
        width                       = (int)atom_getFloatAtIndex (0,  argc, argv);
        height                      = (int)atom_getFloatAtIndex (1,  argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (4,  argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (5,  argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (7,  argc, argv);
        hasScale                    = (int)atom_getFloatAtIndex (10, argc, argv);
        
        /* Note that a fake float value is pitiably attribute to the send symbol. */
        
        iemgui_deserializeNamesByIndex (&x->x_gui, 1, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (6, argc, argv));
        iemgui_deserializeColors (&x->x_gui, argv + 8, NULL, argv + 9);
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 1, NULL);
        iemgui_deserializeColors (&x->x_gui, NULL, NULL, NULL);
    }

    x->x_gui.iem_owner      = canvas_getCurrent();
    x->x_gui.iem_draw       = (t_iemfn)vu_draw;
    x->x_gui.iem_canSend    = 0;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == utils_empty()) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (width, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
        
    vu_setHeight (x, height);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_hasScale    = (hasScale != 0);
    x->x_peakValue   = (t_float)-101.0;
    x->x_rmsValue    = (t_float)-101.0;
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, sym_inlet2);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void vu_free (t_vu *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
        
    guistub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    class_addMethod (c, (t_method)vu_floatPeak,             sym_inlet2,             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vu_fromDialog,            sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_size,                  sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        sym_labelfont,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    sym_labelposition,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_backgroundColor,  sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelColor,       sym_labelcolor,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_receive,          sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_label,            sym_label,              A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)iemjump_move,             sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            sym_scale,              A_GIMME, A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &vu_widgetBehavior);
    class_setSaveFunction (c, vu_functionSave);
    class_setPropertiesFunction (c, vu_functionProperties);
    
    vu_class = c;
}

void vu_destroy (void)
{
    CLASS_FREE (vu_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
