
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

static void toggle_set                  (t_toggle *, t_float);
static void toggle_nonZero              (t_toggle *, t_float);
static void toggle_behaviorGetRectangle (t_gobj *, t_glist *, t_rectangle *);
static int  toggle_behaviorMouse        (t_gobj *, t_glist *, t_mouse *);
    
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
// MARK: -

void toggle_drawJob (t_gobj *z, t_glist *glist)
{
    t_toggle *x   = (t_toggle *)z;
    t_glist *view = glist_getView (glist);
    
    gui_vAdd ("%s.c itemconfigure %lxCROSS1 -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxCROSS2 -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void toggle_drawUpdate (t_toggle *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, toggle_drawJob);
}

void toggle_drawMove (t_toggle *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    
    int thickness = (int)((x->x_gui.iem_width / 30.0) + 0.5);
        
    gui_vAdd ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
    gui_vAdd ("%s.c itemconfigure %lxCROSS1 -width %d\n", 
                    glist_getTagAsString (view), 
                    x, 
                    thickness);
    gui_vAdd ("%s.c itemconfigure %lxCROSS2 -width %d\n",
                    glist_getTagAsString (view),
                    x,
                    thickness);
    gui_vAdd ("%s.c coords %lxCROSS1 %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + thickness + 1,
                    b + thickness + 1,
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + x->x_gui.iem_height - thickness - 1);
    gui_vAdd ("%s.c coords %lxCROSS2 %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + thickness + 1,
                    b + x->x_gui.iem_height - thickness - 1,
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + thickness + 1);
}

void toggle_drawNew (t_toggle *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    
    int thickness = (int)((x->x_gui.iem_width / 30.0) + 0.5);

    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create line %d %d %d %d -width %d -fill #%06x -tags %lxCROSS1\n",
                    glist_getTagAsString (view),
                    a + thickness + 1,
                    b + thickness + 1, 
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + x->x_gui.iem_height - thickness - 1,
                    thickness,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create line %d %d %d %d -width %d -fill #%06x -tags %lxCROSS2\n",
                    glist_getTagAsString (view),
                    a + thickness + 1,
                    b + x->x_gui.iem_height - thickness - 1,
                    a + x->x_gui.iem_width  - thickness - 1,
                    b + thickness + 1,
                    thickness,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    x);
}

void toggle_drawSelect (t_toggle *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view), 
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
}

void toggle_drawErase (t_toggle *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxCROSS1\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxCROSS2\n",
                    glist_getTagAsString (view),
                    x);
}

void toggle_drawConfig (t_toggle *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxCROSS1 -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxCROSS2 -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    (x->x_state != 0.0) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void toggle_out (t_toggle *x)
{
    outlet_float (x->x_outlet, x->x_state);
    
    if (x->x_gui.iem_canSend && symbol_hasThing (x->x_gui.iem_send)) { 
        pd_float (symbol_getThing (x->x_gui.iem_send), x->x_state); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void toggle_bang (t_toggle *x)
{
    toggle_set (x, (x->x_state == 0.0) ? x->x_nonZero : (t_float)0.0);
    toggle_out (x);
}

static void toggle_float (t_toggle *x, t_float f)
{
    toggle_set (x, f); if (x->x_gui.iem_goThrough) { toggle_out (x); }
}

static void toggle_click (t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    toggle_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    iemgui_boxChanged ((void *)x);
    //
    }
}

static void toggle_set (t_toggle *x, t_float f)
{
    int draw = ((x->x_state != 0.0) != (f != 0.0));
    
    x->x_state = f;

    if (draw) { (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE); }
}

static void toggle_nonZero (t_toggle *x, t_float f)
{
    if (f != 0.0) { x->x_nonZero = f; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void toggle_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + cast_iem (z)->iem_width;
    int d = b + cast_iem (z)->iem_height;
    
    rectangle_set (r, a, b, c, d);
}

static int toggle_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { toggle_click ((t_toggle *)z, NULL, 0, NULL); }
    
    return 1;
}

static void toggle_functionSave (t_gobj *z, t_buffer *b)
{
    t_toggle *x = (t_toggle *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_vAppend (b, "ssiisiisssiiiisssff;", 
        sym___hash__X,
        sym_obj,
        object_getX (cast_object (z)),
        object_getY (cast_object (z)),
        sym_tgl, 
        x->x_gui.iem_width,
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
        x->x_state,
        x->x_nonZero);
}

static void toggle_functionProperties (t_gobj *z, t_glist *owner)
{
    t_toggle *x = (t_toggle *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s Toggle"
            " %d %d Size 0 0 $::var(nil)"           // --
            " %g {Non-Zero Value} 0 $::var(nil)"    // --
            " -1 $::var(nil) $::var(nil)"           // --
            " %d"
            " -1 -1 $::var(nil)"                    // --
            " %s %s"
            " %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_nonZero,
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void toggle_fromDialog (t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0     = x->x_gui.iem_width;
    t_float t1 = x->x_nonZero;
    
    {
    //
    int size = (int)atom_getFloatAtIndex (0, argc, argv);
    t_float nonZero = atom_getFloatAtIndex (2, argc, argv);
    
    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);
        
    toggle_nonZero (x, nonZero);
    
    if (x->x_state != 0.0) { 
        toggle_set (x, x->x_nonZero); 
    }
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_width);
    isDirty |= (t1 != x->x_nonZero);
    
    if (isDirty) { iemgui_boxChanged ((void *)x); glist_setDirty (cast_iem (x)->iem_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *toggle_new (t_symbol *s, int argc, t_atom *argv)
{
    t_toggle *x = (t_toggle *)pd_new (toggle_class);
    
    int size            = IEM_DEFAULT_SIZE;
    int labelX          = 0;
    int labelY          = 0;
    int labelFontSize   = IEM_DEFAULT_FONT;
    t_float state       = (t_float)0.0;
    t_float nonZero     = (t_float)1.0;

    if (argc < 13) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    size            = (int)atom_getFloatAtIndex (0, argc,  argv);
    labelX          = (int)atom_getFloatAtIndex (5, argc,  argv);
    labelY          = (int)atom_getFloatAtIndex (6, argc,  argv);
    labelFontSize   = (int)atom_getFloatAtIndex (8, argc,  argv);
    state           = (t_float)atom_getFloatAtIndex (12, argc, argv);
    nonZero         = (argc == 14) ? atom_getFloatAtIndex (13, argc, argv) : (t_float)1.0;
    
    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (1, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 2, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (7, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 9, argv + 10, argv + 11);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)toggle_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = labelFontSize;
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_nonZero = (nonZero != 0.0) ? nonZero : (t_float)1.0;
    
    if (x->x_gui.iem_loadbang) { x->x_state = (state != 0.0) ? nonZero : (t_float)0.0; }
    else {
        x->x_state = (t_float)0.0;
    }

    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void toggle_free (t_toggle *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    class_addBang (c, (t_method)toggle_bang);
    class_addFloat (c, (t_method)toggle_float);
    class_addClick (c, (t_method)toggle_click);
    
    class_addMethod (c, (t_method)toggle_loadbang,              sym_loadbang,           A_NULL);
    class_addMethod (c, (t_method)toggle_initialize,            sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_fromDialog,            sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_size,                  sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_set,                   sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_nonZero,               sym_nonzero,            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)toggle_initialize,            sym_init,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabel,              sym_label,              A_DEFSYMBOL, A_NULL);

    class_addCreator ((t_newmethod)toggle_new, sym_toggle, A_GIMME, A_NULL);
        
    #endif
    
    class_setWidgetBehavior (c, &toggle_widgetBehavior);
    class_setSaveFunction (c, toggle_functionSave);
    class_setPropertiesFunction (c, toggle_functionProperties);
    
    toggle_class = c;
}

void toggle_destroy (void)
{
    class_free (toggle_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
