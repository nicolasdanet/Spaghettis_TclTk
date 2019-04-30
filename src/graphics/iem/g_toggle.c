
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

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

static void toggle_drawJob (t_gobj *z, t_glist *glist)
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

static void toggle_drawUpdate (t_toggle *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, toggle_drawJob);
}

static void toggle_drawMove (t_toggle *x, t_glist *glist)
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

static void toggle_drawNew (t_toggle *x, t_glist *glist)
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

static void toggle_drawSelect (t_toggle *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view), 
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
}

static void toggle_drawErase (t_toggle *x, t_glist *glist)
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

static void toggle_drawConfig (t_toggle *x, t_glist *glist)
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

static void toggle_draw (t_toggle *x, t_glist *glist, int mode)
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
    toggle_set (x, (x->x_state == 0.0) ? x->x_nonZero : 0.0);
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void toggle_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_toggle *x = (t_toggle *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (z)));
    buffer_appendFloat (b,  object_getY (cast_object (z)));
    buffer_appendSymbol (b, sym_tgl);
    buffer_appendFloat (b,  x->x_gui.iem_width);
    buffer_appendFloat (b,  iemgui_serializeLoadbang (cast_iem (z)));
    buffer_appendSymbol (b, names.n_unexpandedSend);
    buffer_appendSymbol (b, names.n_unexpandedReceive);
    buffer_appendSymbol (b, names.n_unexpandedLabel);
    buffer_appendFloat (b,  x->x_gui.iem_labelX);
    buffer_appendFloat (b,  x->x_gui.iem_labelY);
    buffer_appendFloat (b,  iemgui_serializeFontStyle (cast_iem (z)));
    buffer_appendFloat (b,  x->x_gui.iem_fontSize);
    buffer_appendSymbol (b, colors.c_symColorBackground);
    buffer_appendSymbol (b, colors.c_symColorForeground);
    buffer_appendSymbol (b, colors.c_symColorLabel);
    buffer_appendFloat (b,  x->x_state);
    buffer_appendFloat (b,  x->x_nonZero);
    if (SAVED_DEEP (flags)) { buffer_appendFloat (b, 1.0); }
    buffer_appendSemicolon (b);
    
    gobj_saveUniques (z, b, flags);
}

static t_buffer *toggle_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_buffer *b = buffer_new();

    buffer_appendSymbol (b, sym__restore);
    
    return b;
    //
    }
    
    return NULL;
}

/* Fake dialog message from interpreter. */

static void toggle_functionUndo (t_gobj *z, t_buffer *b)
{
    t_toggle *x = (t_toggle *)z;
    
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    buffer_appendSymbol (b, sym__iemdialog);
    buffer_appendFloat (b,  x->x_gui.iem_width);                /* Width. */
    buffer_appendFloat (b,  -1.0);                              /* Height. */
    buffer_appendFloat (b,  x->x_nonZero);                      /* Option1. */
    buffer_appendFloat (b,  -1.0);                              /* Option2. */
    buffer_appendFloat (b,  -1.0);                              /* Check. */
    buffer_appendFloat (b,  x->x_gui.iem_loadbang);             /* Loadbang. */
    buffer_appendFloat (b,  -1.0);                              /* Extra. */
    buffer_appendSymbol (b, names.n_unexpandedSend);            /* Send. */
    buffer_appendSymbol (b, names.n_unexpandedReceive);         /* Receive. */
    buffer_appendFloat (b,  x->x_gui.iem_colorBackground);      /* Background color. */
    buffer_appendFloat (b,  x->x_gui.iem_colorForeground);      /* Foreground color. */
    buffer_appendFloat (b,  -1.0);                              /* Steady. */
    buffer_appendFloat (b,  -1.0);                              /* Save. */
}

static void toggle_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_toggle *x = (t_toggle *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s Toggle"
            " %d %d Size -1 -1 $::var(nil)"                     // --
            " %.9g {Non-Zero Value} -1 $::var(nil)"             // --
            " -1 $::var(nil) $::var(nil)"                       // --
            " %d"
            " -1 -1 $::var(nil)"                                // --
            " %s %s"
            " %d %d"
            " -1"
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
    int isDirty  = 0;
    int undoable = glist_undoIsOk (cast_iem (x)->iem_owner);
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0     = x->x_gui.iem_width;
    t_float t1 = x->x_nonZero;
    
    t_undosnippet *snippet = NULL;
    
    if (undoable) { snippet = undosnippet_newProperties (cast_gobj (x), cast_iem (x)->iem_owner); }
    
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
    
    iemgui_dirty (cast_iem (x), isDirty, undoable, snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void toggle_restore (t_toggle *x)
{
    t_toggle *old = (t_toggle *)instance_pendingFetch (cast_gobj (x));
    
    if (old) {
    //
    iemgui_restore (cast_gobj (x), cast_gobj (old));
    
    toggle_set (x, old->x_state);
    //
    }
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
    t_float state       = 0.0;
    t_float nonZero     = (t_float)1.0;
    int deep            = 0;

    if (argc < 13) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    size            = (int)atom_getFloatAtIndex (0, argc,  argv);
    labelX          = (int)atom_getFloatAtIndex (5, argc,  argv);
    labelY          = (int)atom_getFloatAtIndex (6, argc,  argv);
    labelFontSize   = (int)atom_getFloatAtIndex (8, argc,  argv);
    state           = (t_float)atom_getFloatAtIndex (12, argc, argv);
    nonZero         = (argc > 13) ? atom_getFloatAtIndex (13, argc, argv) : (t_float)1.0;
    deep            = (argc > 14);
    
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
    
    if (deep || x->x_gui.iem_loadbang) { x->x_state = (state != 0.0) ? nonZero : 0.0; }
    else {
        x->x_state = 0.0;
    }

    x->x_outlet = outlet_newFloat (cast_object (x));
    
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
    class_addLoadbang (c, (t_method)toggle_loadbang);
    
    class_addMethod (c, (t_method)toggle_initialize,            sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_fromDialog,            sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_size,                  sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_set,                   sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_nonZero,               sym_nonzero,            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)toggle_restore,               sym__restore,           A_NULL);

    class_setWidgetBehavior (c, &toggle_widgetBehavior);
    class_setSaveFunction (c, toggle_functionSave);
    class_setDataFunction (c, toggle_functionData);
    class_setUndoFunction (c, toggle_functionUndo);
    class_setPropertiesFunction (c, toggle_functionProperties);
    class_requirePending (c);
    
    toggle_class = c;
}

void toggle_destroy (void)
{
    class_free (toggle_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
