
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

/* Note that the grip size has been kept for compatibility with legacy patches only. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_PANEL_DEFAULT_WIDTH     250
#define IEM_PANEL_DEFAULT_HEIGHT    45

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define IEM_PANEL_MINIMUM_SIZE      1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_behaviorGetRectangle (t_gobj *, t_glist *, t_rectangle *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *panel_class;                                   /* Shared. */

static t_widgetbehavior panel_widgetBehavior =          /* Shared. */
    {
        panel_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        NULL
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void panel_drawMove (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));

    gui_vAdd ("%s.c coords %lxPANEL %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a,
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight);
    gui_vAdd ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a, 
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight);
}

void panel_drawNew (t_panel *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));

    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxPANEL\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground,
                    x);
    gui_vAdd ("%s.c create rectangle %d %d %d %d -outline #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    a + x->x_panelWidth,
                    b + x->x_panelHeight,
                    x->x_gui.iem_colorBackground,
                    x);
}

void panel_drawSelect (t_panel* x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorBackground);
}

void panel_drawErase (t_panel* x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxPANEL\n",
                    glist_getTagAsString (view),
                    x);
}

void panel_drawConfig (t_panel* x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxPANEL -fill #%06x -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorBackground);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void panel_draw (t_panel *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_MOVE      : panel_drawMove (x, glist);    break;
        case IEM_DRAW_NEW       : panel_drawNew (x, glist);     break;
        case IEM_DRAW_SELECT    : panel_drawSelect (x, glist);  break;
        case IEM_DRAW_ERASE     : panel_drawErase (x, glist);   break;
        case IEM_DRAW_CONFIG    : panel_drawConfig (x, glist);  break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_size (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int i = (int)atom_getFloatAtIndex (0, argc, argv);

    x->x_panelWidth = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    
    if (argc > 1) { 
        i = (int)atom_getFloatAtIndex (1, argc, argv); 
    }
    
    x->x_panelHeight = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_MOVE);
    //
    }
}

static void panel_getPosition (t_panel *x)
{
    if (x->x_gui.iem_canSend && pd_hasThing (x->x_gui.iem_send)) {
        t_float a = glist_getPixelX (x->x_gui.iem_owner, cast_object (x));
        t_float b = glist_getPixelY (x->x_gui.iem_owner, cast_object (x));
        SET_FLOAT (&x->x_t[0], a);
        SET_FLOAT (&x->x_t[1], b);
        pd_list (pd_getThing (x->x_gui.iem_send), 2, x->x_t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void panel_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_panel *x = (t_panel *)z;
    
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + x->x_panelWidth;
    int d = b + x->x_panelHeight;
    
    rectangle_set (r, a, b, c, d);
}

static void panel_functionSave (t_gobj *z, t_buffer *b)
{
    t_panel *x = (t_panel *)z;
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_vAppend (b, "ssiisiiisssiiiiss;",
        sym___hash__X,
        sym_obj,
        object_getX (cast_object (z)),
        object_getY (cast_object (z)),
        sym_cnv,
        x->x_gui.iem_width,
        x->x_panelWidth,
        x->x_panelHeight,
        names.n_unexpandedSend,
        names.n_unexpandedReceive,
        names.n_unexpandedLabel,
        x->x_gui.iem_labelX,
        x->x_gui.iem_labelY,
        iemgui_serializeFontStyle (cast_iem (z)),
        x->x_gui.iem_fontSize,
        colors.c_symColorBackground,
        colors.c_symColorLabel);
}

static void panel_functionProperties (t_gobj *z, t_glist *owner)
{
    t_panel *x = (t_panel *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Panel"
            " 0 0 $::var(nil) 0 0 $::var(nil)"      // --
            " %d {Panel Width} %d {Panel Height}"   // --
            " -1 $::var(nil) $::var(nil)"           // --
            " -1"
            " -1 -1 $::var(nil)"                    // --
            " %s %s"
            " %d %d"
            " -1\n",
            x->x_panelWidth, x->x_panelHeight,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground);
            
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void panel_fromDialog (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0 = x->x_panelWidth;
    int t1 = x->x_panelHeight;
    
    {
    //
    int panelWidth  = (int)atom_getFloatAtIndex (2, argc, argv);
    int panelHeight = (int)atom_getFloatAtIndex (3, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);

    x->x_panelWidth     = PD_MAX (panelWidth,  IEM_PANEL_MINIMUM_SIZE);
    x->x_panelHeight    = PD_MAX (panelHeight, IEM_PANEL_MINIMUM_SIZE);
    //
    }
    
    isDirty |= (t0 != x->x_panelWidth);
    isDirty |= (t1 != x->x_panelHeight);
    
    if (isDirty) { iemgui_boxChanged ((void *)x); glist_setDirty (cast_iem (x)->iem_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *panel_new (t_symbol *s, int argc, t_atom *argv)
{
    t_panel *x = (t_panel *)pd_new (panel_class);
    
    int gripSize        = IEM_DEFAULT_SIZE;
    int panelWidth      = IEM_PANEL_DEFAULT_WIDTH;
    int panelHeight     = IEM_PANEL_DEFAULT_HEIGHT;
    int labelX          = 0;
    int labelY          = 0;
    int labelFontSize   = IEM_DEFAULT_FONT;
        
    if (argc < 12) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    gripSize        = (int)atom_getFloatAtIndex (0, argc, argv);
    panelWidth      = (int)atom_getFloatAtIndex (1, argc, argv);
    panelHeight     = (int)atom_getFloatAtIndex (2, argc, argv);
    labelX          = (int)atom_getFloatAtIndex (6, argc, argv);
    labelY          = (int)atom_getFloatAtIndex (7, argc, argv);
    labelFontSize   = (int)atom_getFloatAtIndex (9, argc, argv);
    
    iemgui_deserializeNames (cast_iem (x), 3, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (8, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 10, NULL, argv + 11);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)panel_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;

    x->x_gui.iem_width      = PD_MAX (gripSize, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_height     = PD_MAX (gripSize, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = labelFontSize;
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    x->x_panelWidth  = PD_MAX (panelWidth,  IEM_PANEL_MINIMUM_SIZE);
    x->x_panelHeight = PD_MAX (panelHeight, IEM_PANEL_MINIMUM_SIZE);

    SET_FLOAT (&x->x_t[0], (t_float)0.0);
    SET_FLOAT (&x->x_t[1], (t_float)0.0);
    
    return x;
}

static void panel_free (t_panel *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void panel_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_cnv, 
            (t_newmethod)panel_new,
            (t_method)panel_free,
            sizeof (t_panel), 
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
        
    class_addMethod (c, (t_method)panel_fromDialog,             sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_size,                   sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_move,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_position,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_getPosition,            sym_getposition,        A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)iemgui_movePosition,          sym_delta,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setPosition,           sym_pos,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_dummy,                 sym_color,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelPosition,      sym_label_pos,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabelFont,          sym_label_font,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setLabel,              sym_label,              A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)panel_size,                   sym_vis_size,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_getPosition,            sym_get_pos,            A_NULL);
        
    class_addCreator ((t_newmethod)panel_new, sym_my_canvas, A_GIMME, A_NULL);
        
    #endif
    
    class_setWidgetBehavior (c, &panel_widgetBehavior);
    class_setSaveFunction (c, panel_functionSave);
    class_setPropertiesFunction (c, panel_functionProperties);
    
    panel_class = c;
}

void panel_destroy (void)
{
    class_free (panel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
