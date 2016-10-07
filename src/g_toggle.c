
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

static void toggle_set                  (t_toggle *, t_float);
static void toggle_nonZero              (t_toggle *, t_float);
static void toggle_behaviorGetRectangle (t_gobj *, t_glist *, int *, int *, int *, int *);
static int  toggle_behaviorMouse        (t_gobj *, t_glist *, int, int, int, int, int, int, int);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *toggle_class;                           /* Shared. */

static t_widgetbehavior toggle_widgetBehavior =         /* Shared. */
    {
        toggle_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        toggle_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void toggle_drawJob (t_gobj *z, t_glist *glist)
{
    t_toggle *x = (t_toggle *)z;
    
    if (canvas_isMapped (glist)) {
    //
    t_glist *canvas = canvas_getView (glist);
    
    sys_vGui (".x%lx.c itemconfigure %lxCROSS1 -fill #%06x\n",
                    canvas,
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxCROSS2 -fill #%06x\n",
                    canvas,
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void toggle_drawUpdate (t_toggle *x, t_glist *glist)
{
    interface_guiQueueAddIfNotAlreadyThere ((void *)x, glist, toggle_drawJob);
}

void toggle_drawMove (t_toggle *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);
    
    int thickness = (int)((x->x_gui.iem_width / 30.0) + 0.5);
        
    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                    canvas,
                    x,
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
    sys_vGui (".x%lx.c itemconfigure %lxCROSS1 -width %d\n", 
                    canvas, 
                    x, 
                    thickness);
    sys_vGui (".x%lx.c itemconfigure %lxCROSS2 -width %d\n",
                    canvas,
                    x,
                    thickness);
    sys_vGui (".x%lx.c coords %lxCROSS1 %d %d %d %d\n",
                    canvas,
                    x,
                    a + thickness + 1,
                    b + thickness + 1,
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + x->x_gui.iem_height - thickness - 1);
    sys_vGui (".x%lx.c coords %lxCROSS2 %d %d %d %d\n",
                    canvas,
                    x,
                    a + thickness + 1,
                    b + x->x_gui.iem_height - thickness - 1,
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + thickness + 1);
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                    canvas,
                    x,
                    a + x->x_gui.iem_labelX, 
                    b + x->x_gui.iem_labelY);
}

void toggle_drawNew (t_toggle *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int a = text_getPixelX (cast_object (x), glist);
    int b = text_getPixelY (cast_object (x), glist);
    
    int thickness = (int)((x->x_gui.iem_width / 30.0) + 0.5);

    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    canvas,
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -fill #%06x -tags %lxCROSS1\n",
                    canvas,
                    a + thickness + 1,
                    b + thickness + 1, 
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + x->x_gui.iem_height - thickness - 1,
                    thickness,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    x);
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -fill #%06x -tags %lxCROSS2\n",
                    canvas,
                    a + thickness + 1,
                    b + x->x_gui.iem_height - thickness - 1,
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + thickness + 1,
                    thickness,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    x);
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

void toggle_drawSelect (t_toggle *x, t_glist *glist)
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

void toggle_drawErase (t_toggle *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    sys_vGui (".x%lx.c delete %lxBASE\n",
                    canvas,
                    x);
    sys_vGui (".x%lx.c delete %lxCROSS1\n",
                    canvas,
                    x);
    sys_vGui (".x%lx.c delete %lxCROSS2\n",
                    canvas,
                    x);
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                    canvas,
                    x);
}

void toggle_drawConfig (t_toggle *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -fill #%06x\n",
                    canvas,
                    x,
                    x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxCROSS1 -fill #%06x\n",
                    canvas,
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxCROSS2 -fill #%06x\n",
                    canvas,
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
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

void toggle_draw (t_toggle *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : toggle_drawUpdate (x, glist); break;
        case IEM_DRAW_MOVE      : toggle_drawMove (x, glist);   break;
        case IEM_DRAW_NEW       : toggle_drawNew (x, glist);    break;
        case IEM_DRAW_SELECT    : toggle_drawSelect (x, glist); break;
        case IEM_DRAW_ERASE     : toggle_drawErase (x, glist);  break;
        case IEM_DRAW_CONFIG    : toggle_drawConfig (x, glist); break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void toggle_out (t_toggle *x)
{
    outlet_float (x->x_outlet, x->x_state);
    
    if (x->x_gui.iem_canSend && pd_isThing (x->x_gui.iem_send)) { 
        pd_float (pd_getThing (x->x_gui.iem_send), x->x_state); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void toggle_bang (t_toggle *x)
{
    toggle_set (x, (x->x_state == 0.0) ? x->x_nonZero : 0.0);
    toggle_out (x);
}

static void toggle_float (t_toggle *x, t_float f)
{
    toggle_set (x, f); if (x->x_gui.iem_goThrough) { toggle_out (x); }
}

static void toggle_click (t_toggle *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    toggle_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void toggle_loadbang (t_toggle *x)
{
    if (x->x_gui.iem_loadbang) {
        toggle_set (x, x->x_state);
        toggle_out (x);
    }
}

static void toggle_initialize (t_toggle *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void toggle_size (t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int width = atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width  = PD_MAX (width, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (width, IEM_MINIMUM_WIDTH);
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

static void toggle_set (t_toggle *x, t_float f)
{
    int draw = ((x->x_state != 0.0) != (f != 0.0));
    
    x->x_state = f;

    if (draw) { (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
}

static void toggle_nonZero (t_toggle *x, t_float f)
{
    if (f != 0.0) { x->x_nonZero = f; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void toggle_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    *a = text_getPixelX (cast_object (z), glist);
    *b = text_getPixelY (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static int toggle_behaviorMouse (t_gobj *z, t_glist *glist,
    int a,
    int b,
    int shift,
    int ctrl,
    int alt,
    int dbl,
    int clicked)
{
    if (clicked) { 
        toggle_click ((t_toggle *)z, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt); 
    }
    
    return 1;
}

static void toggle_functionSave (t_gobj *z, t_buffer *b)
{
    t_toggle *x = (t_toggle *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiisssiiiisssff", 
        &s__X,
        sym_obj,
        (int)cast_object (z)->te_xCoordinate,
        (int)cast_object (z)->te_yCoordinate,
        sym_tgl, 
        x->x_gui.iem_width,                                 // Size.
        iemgui_serializeLoadbang (&x->x_gui),               // Loadbang.
        names.n_unexpandedSend,                             // Send.
        names.n_unexpandedReceive,                          // Receive.
        names.n_unexpandedLabel,                            // Label.
        x->x_gui.iem_labelX,                                // Label X.
        x->x_gui.iem_labelY,                                // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),              // Label font.
        x->x_gui.iem_fontSize,                              // Label font size.
        colors.c_symColorBackground,                        // Backround color.
        colors.c_symColorForeground,                        // Foreground color.
        colors.c_symColorLabel,                             // Label color.
        x->x_state,                                         // Toggle state.
        x->x_nonZero);                                      // Non-zero value.
        
    buffer_appendSemicolon (b);
}

static void toggle_functionProperties (t_gobj *z, t_glist *owner)
{
    t_toggle *x = (t_toggle *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);
    
    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s Toggle"
            " %d %d Size 0 0 $::var(nil)"
            " %g {Non-Zero Value} 0 $::var(nil)"
            " -1 $::var(nil) $::var(nil)"
            " %d"
            " -1 -1 $::var(nil)"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_nonZero,
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel);
    
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

static void toggle_fromDialog (t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int size = (int)atom_getFloatAtIndex (0, argc, argv);
    t_float nonZero = atom_getFloatAtIndex (2, argc, argv);
    
    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);
        
    toggle_nonZero (x, nonZero);
    
    if (x->x_state != 0.0) { 
        toggle_set (x, x->x_nonZero); 
    }
    
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *toggle_new (t_symbol *s, int argc, t_atom *argv)
{
    t_toggle *x = (t_toggle *)pd_new (toggle_class);
    
    int size            = IEM_DEFAULT_SIZE;
    int labelX          = IEM_DEFAULT_LABELX_NEXT;
    int labelY          = IEM_DEFAULT_LABELY_NEXT;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    t_float state       = 0.0;
    t_float nonZero     = 1.0;

    if (argc >= 13
            && IS_FLOAT (argv)                                  // Size.
            && IS_FLOAT (argv + 1)                              // Loadbang.
            && IS_SYMBOL_OR_FLOAT (argv + 2)                    // Send.
            && IS_SYMBOL_OR_FLOAT (argv + 3)                    // Receive.
            && IS_SYMBOL_OR_FLOAT (argv + 4)                    // Label.
            && IS_FLOAT (argv + 5)                              // Label X.
            && IS_FLOAT (argv + 6)                              // Label Y.
            && IS_FLOAT (argv + 7)                              // Label font.
            && IS_FLOAT (argv + 8)                              // Label font size.
            && IS_SYMBOL_OR_FLOAT (argv + 9)                    // Background color.
            && IS_SYMBOL_OR_FLOAT (argv + 10)                   // Foreground color.
            && IS_SYMBOL_OR_FLOAT (argv + 11)                   // Label color.
            && IS_FLOAT (argv + 12))                            // Toggle state.
    {
        size                        = (int)atom_getFloatAtIndex (0, argc,  argv);
        labelX                      = (int)atom_getFloatAtIndex (5, argc,  argv);
        labelY                      = (int)atom_getFloatAtIndex (6, argc,  argv);
        labelFontSize               = (int)atom_getFloatAtIndex (8, argc,  argv);
        state                       = (t_float)atom_getFloatAtIndex (12, argc, argv);
        nonZero                     = (argc == 14) ? atom_getFloatAtIndex (13, argc, argv) : 1.0;
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (1, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 2, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (7, argc, argv));
        iemgui_deserializeColors (&x->x_gui, argv + 9, argv + 10, argv + 11);
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 2, NULL);
        iemgui_deserializeColors (&x->x_gui, NULL, NULL, NULL);
    }
    
    x->x_gui.iem_owner      = canvas_getCurrent();
    x->x_gui.iem_draw       = (t_iemfn)toggle_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == utils_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == utils_empty()) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_nonZero = (nonZero != 0.0) ? nonZero : 1.0;
    
    if (x->x_gui.iem_loadbang) { x->x_state = (state != 0.0) ? nonZero : 0.0; }
    else {
        x->x_state = 0.0;
    }

    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void toggle_free (t_toggle *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    guistub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void toggle_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tgl,
            (t_newmethod)toggle_new,
            (t_method)toggle_free,
            sizeof (t_toggle),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, toggle_bang);
    class_addFloat (c, toggle_float);
    class_addClick (c, toggle_click);
    
    class_addMethod (c, (t_method)toggle_loadbang,          sym_loadbang,           A_NULL);
    class_addMethod (c, (t_method)toggle_initialize,        sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_fromDialog,        sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_size,              sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        sym_labelfont,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    sym_labelposition,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_backgroundColor,  sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_foregroundColor,  sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelColor,       sym_labelcolor,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_set,               sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_nonZero,           sym_nonzero,            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemjump_send,             sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_receive,          sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_label,            sym_label,              A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)toggle_initialize,        sym_init,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        sym_label_font,         A_GIMME, A_NULL);
    
    class_addCreator ((t_newmethod)toggle_new, sym_toggle, A_GIMME, A_NULL);
        
    #endif
    
    class_setWidgetBehavior (c, &toggle_widgetBehavior);
    class_setSaveFunction (c, toggle_functionSave);
    class_setPropertiesFunction (c, toggle_functionProperties);
    
    toggle_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
