
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

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_BANG_DEFAULT_HOLD       250
#define IEM_BANG_DEFAULT_BREAK      50

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_BANG_MINIMUM_HOLD       10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_behaviorGetRectangle    (t_gobj *, t_glist *, t_rectangle *);
static int  bng_behaviorMouse           (t_gobj *, t_glist *, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *bng_class;                          /* Shared. */

static t_widgetbehavior bng_widgetBehavior =        /* Shared. */
    {
        bng_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        bng_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void bng_taskFlash (t_bng *x)
{
    x->x_flashed = 0;
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_drawJob (t_gobj *z, t_glist *glist)
{
    t_bng *x = (t_bng *)z;
    
    sys_vGui ("%s.c itemconfigure %lxBUTTON -fill #%06x\n", 
                    glist_getTagAsString (glist_getView (glist)),
                    x,
                    x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bng_drawUpdate (t_bng *x, t_glist *glist)
{
    defer_addJob ((void *)x, glist, bng_drawJob);
}

void bng_drawMove (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
        
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    
    sys_vGui ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
    sys_vGui ("%s.c coords %lxBUTTON %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + 1,
                    b + 1,
                    a + x->x_gui.iem_width - 1,
                    b + x->x_gui.iem_height - 1);
    sys_vGui ("%s.c coords %lxLABEL %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY);
}

void bng_drawNew (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    
    sys_vGui ("%s.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_gui.iem_width,
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x);
    sys_vGui ("%s.c create oval %d %d %d %d -fill #%06x -outline #%06x -tags %lxBUTTON\n",
                    glist_getTagAsString (view),
                    a + 1,
                    b + 1,
                    a + x->x_gui.iem_width - 1,
                    b + x->x_gui.iem_height - 1,
                    x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    COLOR_NORMAL,
                    x);
    sys_vGui ("%s.c create text %d %d -text {%s}"       // --
                    " -anchor w"                                                 
                    " -font [::getFont %d]"             // --
                    " -fill #%06x"
                    " -tags %lxLABEL\n",
                    glist_getTagAsString (view), 
                    a + x->x_gui.iem_labelX,
                    b + x->x_gui.iem_labelY,
                    utils_isNil (x->x_gui.iem_label) ? "" : x->x_gui.iem_label->s_name,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_colorLabel,
                    x);
}

void bng_drawSelect (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    sys_vGui ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    sys_vGui ("%s.c itemconfigure %lxBUTTON -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    sys_vGui ("%s.c itemconfigure %lxLABEL -fill #%06x\n",
                    glist_getTagAsString (view),
                    x, 
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

void bng_drawErase (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    sys_vGui ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    sys_vGui ("%s.c delete %lxBUTTON\n",
                    glist_getTagAsString (view),
                    x);
    sys_vGui ("%s.c delete %lxLABEL\n",
                    glist_getTagAsString (view),
                    x);
}

void bng_drawConfig (t_bng *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    sys_vGui ("%s.c itemconfigure %lxBASE -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground);
    sys_vGui ("%s.c itemconfigure %lxBUTTON -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    sys_vGui ("%s.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",   // --
                    glist_getTagAsString (view),
                    x,
                    font_getHostFontSize (x->x_gui.iem_fontSize),
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorLabel,
                    utils_isNil (x->x_gui.iem_label) ? "" : x->x_gui.iem_label->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bng_draw (t_bng *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : bng_drawUpdate (x, glist);    break;
        case IEM_DRAW_MOVE      : bng_drawMove (x, glist);      break;
        case IEM_DRAW_NEW       : bng_drawNew (x, glist);       break;
        case IEM_DRAW_SELECT    : bng_drawSelect (x, glist);    break;
        case IEM_DRAW_ERASE     : bng_drawErase (x, glist);     break;
        case IEM_DRAW_CONFIG    : bng_drawConfig (x, glist);    break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_updateFlash (t_bng *x)
{
    if (!x->x_flashed) {
        x->x_flashed = 1; (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    }
    
    clock_delay (x->x_clock, x->x_flashTime);
}

static void bng_out (t_bng *x)
{
    outlet_bang (x->x_outlet);
    
    if (x->x_gui.iem_canSend && pd_hasThing (x->x_gui.iem_send)) {
        pd_bang (pd_getThing (x->x_gui.iem_send));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_bang (t_bng *x)
{
    bng_updateFlash (x);
    
    if (x->x_gui.iem_goThrough) { 
        bng_out (x);
    }
}

static void bng_float (t_bng *x, t_float f)
{
    bng_bang (x);
}

static void bng_symbol (t_bng *x, t_symbol *s)
{
    bng_bang (x);
}

static void bng_pointer (t_bng *x, t_gpointer *gp)
{
    bng_bang (x);
}

static void bng_list (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    bng_bang (x);
}

static void bng_anything (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    bng_bang (x);
}

static void bng_click (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    bng_updateFlash (x);
    bng_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_loadbang (t_bng *x)
{
    if (x->x_gui.iem_loadbang) { bng_bang (x); }
}

static void bng_initialize (t_bng *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void bng_size (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int size = atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    iemgui_boxChanged ((void *)x);
    //
    }
}

static void bng_flashtime (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    int n = x->x_flashTime;
    
    if (argc == 1) { n = (int)atom_getFloatAtIndex (0, argc, argv); }
    if (argc == 2) { n = (int)atom_getFloatAtIndex (1, argc, argv); }
    
    x->x_flashTime = PD_MAX (n, IEM_BANG_MINIMUM_HOLD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void bng_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + cast_iem (z)->iem_width;
    int d = b + cast_iem (z)->iem_height;
    
    rectangle_set (r, a, b, c, d);
}

static int bng_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { bng_click ((t_bng *)z, NULL, 0, NULL); }
    
    return 1;
}

static void bng_functionSave (t_gobj *z, t_buffer *b)
{
    t_bng *x = (t_bng *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_vAppend (b, "ssiisiiiisssiiiisss;",
        sym___hash__X,
        sym_obj,
        object_getX (cast_object (z)),
        object_getY (cast_object (z)),
        sym_bng,
        x->x_gui.iem_width,
        x->x_flashTime,
        x->x_flashTimeBreak,
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
        colors.c_symColorLabel);
}

static void bng_functionProperties (t_gobj *z, t_glist *owner)
{
    t_bng *x = (t_bng *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s Bang"
            " %d %d Size 0 0 $::var(nil)"           // --
            " %d {Flash Time} -1 $::var(nil)"       // --
            " -1 $::var(nil) $::var(nil)"           // --
            " %d"
            " -1 -1 $::var(nil)"                    // --
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_flashTime,
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel);

    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void bng_fromDialog (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    int t0 = x->x_gui.iem_width;
    int t1 = x->x_flashTime;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    {
    //
    int size      = (int)atom_getFloatAtIndex (0, argc, argv);
    int flashTime = (int)atom_getFloatAtIndex (2, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);

    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_flashTime      = PD_MAX (flashTime, IEM_BANG_MINIMUM_HOLD);
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_width);
    isDirty |= (t1 != x->x_flashTime);
    
    if (isDirty) { iemgui_boxChanged ((void *)x); glist_setDirty (cast_iem (x)->iem_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *bng_new (t_symbol *s, int argc, t_atom *argv)
{
    t_bng *x = (t_bng *)pd_new (bng_class);
    
    int size            = IEM_DEFAULT_SIZE;
    int flashHold       = IEM_BANG_DEFAULT_HOLD;
    int flashBreak      = IEM_BANG_DEFAULT_BREAK;
    int labelX          = IEM_DEFAULT_LABELX_NEXT;
    int labelY          = IEM_DEFAULT_LABELY_NEXT;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    
    if (argc != 14) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    size            = (int)atom_getFloatAtIndex (0,  argc, argv);
    flashHold       = (int)atom_getFloatAtIndex (1,  argc, argv);
    flashBreak      = (int)atom_getFloatAtIndex (2,  argc, argv);
    labelX          = (int)atom_getFloatAtIndex (7,  argc, argv);
    labelY          = (int)atom_getFloatAtIndex (8,  argc, argv);
    labelFontSize   = (int)atom_getFloatAtIndex (10, argc, argv);
    
    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (3, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 4, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (9, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 11, argv + 12, argv + 13);
    //
    }

    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)bng_draw;
    x->x_gui.iem_canSend    = utils_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = utils_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    x->x_flashTimeBreak     = flashBreak;
    x->x_flashTime          = PD_MAX (flashHold, IEM_BANG_MINIMUM_HOLD);
    
    x->x_outlet = outlet_new (cast_object (x), &s_bang);
    x->x_clock  = clock_new ((void *)x, (t_method)bng_taskFlash);
    
    return x;
}

static void bng_free (t_bng *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    clock_free (x->x_clock);
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bng_setup (void) 
{
    t_class *c = NULL;
    
    c = class_new (sym_bng, 
            (t_newmethod)bng_new, 
            (t_method)bng_free, 
            sizeof (t_bng), 
            CLASS_DEFAULT,
            A_GIMME, 
            A_NULL);
    
    class_addBang (c, (t_method)bng_bang);
    class_addFloat (c, (t_method)bng_float);
    class_addSymbol (c, (t_method)bng_symbol);
    class_addPointer (c, (t_method)bng_pointer);
    class_addList (c, (t_method)bng_list);
    class_addAnything (c, (t_method)bng_anything);
    class_addClick (c, (t_method)bng_click);
    
    class_addMethod (c, (t_method)bng_loadbang,                 sym_loadbang,           A_NULL);
    class_addMethod (c, (t_method)bng_initialize,               sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bng_fromDialog,               sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_size,                     sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_flashtime,                sym_flashtime,          A_GIMME, A_NULL);
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
    
    class_addMethod (c, (t_method)bng_initialize,               sym_init,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &bng_widgetBehavior);
    class_setSaveFunction (c, bng_functionSave);
    class_setPropertiesFunction (c, bng_functionProperties);
    
    bng_class = c;
}

void bng_destroy (void)
{
    class_free (bng_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
