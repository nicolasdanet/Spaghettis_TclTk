
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_MENUBUTTON_MARGIN_X_LEFT        2
#define IEM_MENUBUTTON_MARGIN_X_BOTH        8
#define IEM_MENUBUTTON_MARGIN_Y_TOP         2
#define IEM_MENUBUTTON_MARGIN_Y_BOTH        4

#define IEM_MENUBUTTON_DELTA                2
#define IEM_MENUBUTTON_DEFAULT_WIDTH        8
#define IEM_MENUBUTTON_MINIMUM_WIDTH        1
#define IEM_MENUBUTTON_MAXIMUM_WIDTH        127

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_next                 (t_menubutton *x);
static void menubutton_previous             (t_menubutton *x);
static void menubutton_motion               (t_menubutton *, t_float, t_float, t_float);
static void menubutton_behaviorGetRectangle (t_gobj *, t_glist *, t_rectangle *);
static int  menubutton_behaviorMouse        (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *menubutton_class;                                  /* Shared. */

static t_widgetbehavior menubutton_widgetBehavior =         /* Shared. */
    {
        menubutton_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        menubutton_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float menubutton_delta;                            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int menubutton_drawGetWidth (t_menubutton *x, t_glist *glist)
{
    return x->x_gui.iem_width * (int)font_getWidth (glist_getFontSize (glist));
}

static int menubutton_drawGetHeight (t_menubutton *x, t_glist *glist)
{
    return (int)font_getHeight (glist_getFontSize (glist));
}

static t_error menubutton_drawGetContents (t_menubutton *x, int index, int width, t_heapstring *h)
{
    t_error err = PD_ERROR_NONE;
    
    t_atom key;
    
    buffer_clear (x->x_cachedBuffer);
    SET_FLOAT (&key, index);
    slots_get (x->x_slots, &key, x->x_cachedBuffer);
    
    {
    
    char *t = buffer_toString (x->x_cachedBuffer);
    if (width > 0) { err = heapstring_append (h, t, width); }
    else {
        err = heapstring_add (h, t);
    }
    PD_MEMORY_FREE (t);
    
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_drawJob (t_gobj *z, t_glist *glist)
{
    t_menubutton *x = (t_menubutton *)z;
    
    heapstring_clear (x->x_cachedString);

    menubutton_drawGetContents (x, x->x_index, x->x_gui.iem_width, x->x_cachedString);

    gui_vAdd ("%s.c itemconfigure %lxTEXT -text {%s}\n",                // --
                    glist_getTagAsString (glist_getView (glist)),
                    x,
                    heapstring_getRaw (x->x_cachedString));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_drawUpdate (t_menubutton *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, menubutton_drawJob);
}

static void menubutton_drawMove (t_menubutton *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int c = a + menubutton_drawGetWidth (x,  glist) + IEM_MENUBUTTON_MARGIN_X_BOTH;
    int d = b + menubutton_drawGetHeight (x, glist) + IEM_MENUBUTTON_MARGIN_Y_BOTH;
    
    gui_vAdd ("%s.c coords %lxBASE %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a,
                    b,
                    c,
                    d);
    gui_vAdd ("%s.c coords %lxTEXT %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    a + IEM_MENUBUTTON_MARGIN_X_LEFT,
                    b + IEM_MENUBUTTON_MARGIN_Y_TOP);
}

static void menubutton_drawNew (t_menubutton *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int c = a + menubutton_drawGetWidth  (x, glist) + IEM_MENUBUTTON_MARGIN_X_BOTH;
    int d = b + menubutton_drawGetHeight (x, glist) + IEM_MENUBUTTON_MARGIN_Y_BOTH;
    
    heapstring_clear (x->x_cachedString);
    
    menubutton_drawGetContents (x, x->x_index, x->x_gui.iem_width, x->x_cachedString);

    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxBASE\n",
                    glist_getTagAsString (view),
                    a,
                    b,
                    c,
                    d,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorForeground,
                    x);
    gui_vAdd ("%s.c create text %d %d"
                    " -anchor nw"
                    " -fill #%06x"
                    " -font [::getFont %d]"     // --
                    " -text {%s}"               // --
                    " -tags %lxTEXT\n",
                    glist_getTagAsString (view),
                    a + IEM_MENUBUTTON_MARGIN_X_LEFT,
                    b + IEM_MENUBUTTON_MARGIN_Y_TOP,
                    x->x_gui.iem_colorForeground,
                    glist_getFontSize (glist),
                    heapstring_getRaw (x->x_cachedString),
                    x);
}

static void menubutton_drawSelect (t_menubutton *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : x->x_gui.iem_colorForeground);
}

static void menubutton_drawErase (t_menubutton *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c delete %lxBASE\n",
                    glist_getTagAsString (view),
                    x);
    gui_vAdd ("%s.c delete %lxTEXT\n",
                    glist_getTagAsString (view),
                    x);
}

static void menubutton_drawConfig (t_menubutton *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    gui_vAdd ("%s.c itemconfigure %lxBASE -fill #%06x -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorBackground,
                    x->x_gui.iem_colorForeground);
    gui_vAdd ("%s.c itemconfigure %lxTEXT -fill #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    x->x_gui.iem_colorForeground);
    
    menubutton_drawSelect (x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_draw (t_menubutton *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : menubutton_drawUpdate (x, glist);  break;
        case IEM_DRAW_MOVE      : menubutton_drawMove (x, glist);    break;
        case IEM_DRAW_NEW       : menubutton_drawNew (x, glist);     break;
        case IEM_DRAW_SELECT    : menubutton_drawSelect (x, glist);  break;
        case IEM_DRAW_ERASE     : menubutton_drawErase (x, glist);   break;
        case IEM_DRAW_CONFIG    : menubutton_drawConfig (x, glist);  break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_out (t_menubutton *x)
{
    if (!slots_isEmpty (x->x_slots)) {
    //
    t_atom key;
    
    buffer_clear (x->x_cachedBuffer);
    SET_FLOAT (&key, x->x_index);
    slots_get (x->x_slots, &key, x->x_cachedBuffer);
    
    outlet_float (x->x_outletRight, x->x_index);
    outlet_list (x->x_outletLeft, buffer_getSize (x->x_cachedBuffer), buffer_getAtoms (x->x_cachedBuffer));
    
    if (x->x_gui.iem_canSend && symbol_hasThing (x->x_gui.iem_send)) {
        
        pd_list (symbol_getThing (x->x_gui.iem_send),
            buffer_getSize (x->x_cachedBuffer),
            buffer_getAtoms (x->x_cachedBuffer));
    }
    //
    }
}

static void menubutton_index (t_menubutton *x, int n)
{
    int redraw = (x->x_index != n);
    
    x->x_index = n;
    
    if (redraw) { IEMGUI_UPDATE (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_bang (t_menubutton *x)
{
    menubutton_out (x);
}

static void menubutton_float (t_menubutton *x, t_float f)
{
    int n = (int)f; int size = slots_getSize (x->x_slots);
    
    if (n >= 0 && n < size) { menubutton_index (x, n); if (x->x_gui.iem_goThrough) { menubutton_out (x); } }
    else {
        outlet_bang (x->x_outletMiddle);
    }
}

static void menubutton_list (t_menubutton *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_FLOAT (argv)) {
    //
    int size = slots_getSize (x->x_slots);
    int n = (int)GET_FLOAT (argv);
    int i = PD_CLAMP (n, 0, size);
    int redraw = (i == x->x_index);
   
    buffer_clear (x->x_cachedBuffer);
    
    SET_FLOAT (argv, i); buffer_append (x->x_cachedBuffer, argc - 1, argv + 1);
    
    slots_set (x->x_slots, argv, x->x_cachedBuffer);
    
    if (x->x_saveWithParent) { glist_setDirty (cast_iem (x)->iem_owner, 1); }
    
    if (redraw) { IEMGUI_UPDATE (x); }
    //
    }
}

static void menubutton_click (t_menubutton *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_isScrollable) {
    //
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    
    menubutton_delta = 0.0;
        
    glist_setMotion (x->x_gui.iem_owner, cast_gobj (x), (t_motionfn)menubutton_motion, a, b);
    //
    }
    
    menubutton_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_set (t_menubutton *x, t_float f)
{
    if (!slots_isEmpty (x->x_slots)) {
    //
    int size = slots_getSize (x->x_slots);
    int n = PD_CLAMP ((int)f, 0, size - 1);

    menubutton_index (x, n);
    //
    }
}

static void menubutton_clear (t_menubutton *x)
{
    if (!slots_isEmpty (x->x_slots)) {
    //
    slots_clear (x->x_slots);
    
    x->x_index = 0;
    
    if (x->x_saveWithParent) { glist_setDirty (cast_iem (x)->iem_owner, 1); }
    
    IEMGUI_UPDATE (x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_motion (t_menubutton *x, t_float deltaX, t_float deltaY, t_float modifier)
{
    menubutton_delta += deltaY;
    
    if (menubutton_delta < -IEM_MENUBUTTON_DELTA) { menubutton_delta = 0.0; menubutton_previous (x); }
    if (menubutton_delta > IEM_MENUBUTTON_DELTA)  { menubutton_delta = 0.0; menubutton_next (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_loadbang (t_menubutton *x)
{
    if (x->x_gui.iem_loadbang) { menubutton_out (x); }
}

static void menubutton_initialize (t_menubutton *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void menubutton_width (t_menubutton *x, t_float f)
{
    x->x_gui.iem_width = PD_CLAMP ((int)f, IEM_MENUBUTTON_MINIMUM_WIDTH, IEM_MENUBUTTON_MAXIMUM_WIDTH);
    
    iemgui_boxChanged ((void *)x);
}

static void menubutton_scroll (t_menubutton *x, t_float f)
{
    x->x_isScrollable = (f != 0.0);
}

static void menubutton_next (t_menubutton *x)
{
    menubutton_float (x, x->x_index + 1);
}

static void menubutton_previous (t_menubutton *x)
{
    menubutton_float (x, x->x_index - 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_menubutton *x = (t_menubutton *)z;
    
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c = a + menubutton_drawGetWidth (x,  glist) + IEM_MENUBUTTON_MARGIN_X_BOTH;
    int d = b + menubutton_drawGetHeight (x, glist) + IEM_MENUBUTTON_MARGIN_Y_BOTH;
    
    rectangle_set (r, a, b, c, d);
}

static int menubutton_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { menubutton_click ((t_menubutton *)z, NULL, mouse_argc (m), mouse_argv (m)); }
    
    return 1;
}

static void menubutton_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_menubutton *x = (t_menubutton *)z;
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (z)));
    buffer_appendFloat (b,  object_getY (cast_object (z)));
    buffer_appendSymbol (b, sym_menubutton);
    buffer_appendFloat (b,  x->x_gui.iem_width);
    buffer_appendFloat (b,  x->x_saveWithParent);
    buffer_appendFloat (b,  x->x_isScrollable);
    buffer_appendFloat (b,  iemgui_serializeLoadbang (cast_iem (z)));
    buffer_appendSymbol (b, names.n_unexpandedSend);
    buffer_appendSymbol (b, names.n_unexpandedReceive);
    buffer_appendSymbol (b, names.n_unexpandedLabel);
    buffer_appendSymbol (b, colors.c_symColorBackground);
    buffer_appendSymbol (b, colors.c_symColorForeground);
    buffer_appendSymbol (b, colors.c_symColorLabel);
    buffer_appendFloat (b,  (SAVED_DEEP (flags) || x->x_gui.iem_loadbang) ? x->x_index : 0.0);
    buffer_appendSemicolon (b);
    
    if (flags & SAVE_UNDO) { gobj_serializeUnique (z, sym__tagobject, b); }
}

static t_buffer *menubutton_functionData (t_gobj *z, int flags)
{
    t_menubutton *x = (t_menubutton *)z;

    if (SAVED_DEEP (flags) || x->x_saveWithParent) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_serialize (b, slots_getRaw (x->x_slots));
    
    return b;
    //
    }
    
    return NULL;
}

static void menubutton_functionValue (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_menubutton *x = (t_menubutton *)z;
    t_error err = PD_ERROR_NONE;
    int i, n = slots_getSize (x->x_slots);
    
    heapstring_clear (x->x_cachedString);
    
    err = heapstring_addSprintf (x->x_cachedString,
            "::ui_value::show %%s menu menu %d",    // --
            x->x_index);
    
    
    for (i = 0; i < n; i++) {
    //
    err |= heapstring_add (x->x_cachedString, " {");
    err |= menubutton_drawGetContents (x, i, 0, x->x_cachedString);
    err |= heapstring_add (x->x_cachedString, "}");
    //
    }
    
    err |= heapstring_add (x->x_cachedString, "\n");

    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, heapstring_getRaw (x->x_cachedString));
}

static void menubutton_functionUndo (t_gobj *z, t_buffer *b)
{
    t_menubutton *x = (t_menubutton *)z;
    
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    buffer_appendSymbol (b, sym__iemdialog);
    buffer_appendFloat (b,  x->x_gui.iem_width);                /* Width. */
    buffer_appendFloat (b,  -1.0);                              /* Height. */
    buffer_appendFloat (b,  -1.0);                              /* Option1. */
    buffer_appendFloat (b,  -1.0);                              /* Option2. */
    buffer_appendFloat (b,  x->x_isScrollable);                 /* Check. */
    buffer_appendFloat (b,  x->x_gui.iem_loadbang);             /* Loadbang. */
    buffer_appendFloat (b,  -1.0);                              /* Extra. */
    buffer_appendSymbol (b, names.n_unexpandedSend);            /* Send. */
    buffer_appendSymbol (b, names.n_unexpandedReceive);         /* Receive. */
    buffer_appendFloat (b,  x->x_gui.iem_colorBackground);      /* Background color. */
    buffer_appendFloat (b,  x->x_gui.iem_colorForeground);      /* Foreground color. */
    buffer_appendFloat (b,  -1.0);                              /* Steady. */
    buffer_appendFloat (b,  x->x_saveWithParent);               /* Save. */
}

static void menubutton_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_menubutton *x = (t_menubutton *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s {Menu Button}"    // --
            " %d %d Width -1 -1 $::var(nil)"                                    // --
            " -1 $::var(nil) -1 $::var(nil)"                                    // --
            " %d Hold Scroll"                                                   // --
            " %d"
            " -1 -1 $::var(nil)"                                                // --
            " %s %s"
            " %d %d"
            " -1"
            " %d\n",
            x->x_gui.iem_width, IEM_MENUBUTTON_MINIMUM_WIDTH,
            x->x_isScrollable,
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground,
            x->x_saveWithParent);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void menubutton_fromValue (t_menubutton *x, t_symbol *s, int argc, t_atom *argv)
{
    menubutton_float (x, atom_getFloatAtIndex (0, argc, argv));
}

static void menubutton_fromDialog (t_menubutton *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty  = 0;
    int undoable = glist_undoIsOk (cast_iem (x)->iem_owner);
    
    int t0 = x->x_gui.iem_width;
    int t1 = x->x_saveWithParent;
    int t2 = x->x_isScrollable;
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    t_undosnippet *snippet = NULL;
    
    if (undoable) { snippet = undosnippet_newProperties (cast_gobj (x), cast_iem (x)->iem_owner); }

    {
    //
    int width          = (int)atom_getFloatAtIndex (0,  argc, argv);
    int isScrollable   = (int)atom_getFloatAtIndex (4,  argc, argv);
    int saveWithParent = (int)atom_getFloatAtIndex (12, argc, argv);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);

    x->x_gui.iem_width  = PD_CLAMP (width, IEM_MENUBUTTON_MINIMUM_WIDTH, IEM_MENUBUTTON_MAXIMUM_WIDTH);
    x->x_saveWithParent = saveWithParent;
    x->x_isScrollable   = isScrollable;
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_width);
    isDirty |= (t1 != x->x_saveWithParent);
    isDirty |= (t2 != x->x_isScrollable);

    iemgui_dirty (cast_iem (x), isDirty, undoable, snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void menubutton_menu (t_menubutton *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    
    menubutton_clear (x);
    
    for (i = 0; i < argc; i++) {
    //
    t_atom a;
    
    buffer_clear (x->x_cachedBuffer); buffer_append (x->x_cachedBuffer, 1, argv + i);
    
    SET_FLOAT (&a, i);
    
    slots_set (x->x_slots, &a, x->x_cachedBuffer);
    //
    }
    
    if (x->x_saveWithParent) { glist_setDirty (cast_iem (x)->iem_owner, 1); }
    
    IEMGUI_UPDATE (x);
}

static void menubutton_restore (t_menubutton *x, t_symbol *s, int argc, t_atom *argv)
{
    menubutton_clear (x); buffer_deserialize (slots_getRaw (x->x_slots), argc, argv);
    
    IEMGUI_UPDATE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *menubutton_new (t_symbol *s, int argc, t_atom *argv)
{
    t_menubutton *x = (t_menubutton *)pd_new (menubutton_class);
    
    int width          = IEM_MENUBUTTON_DEFAULT_WIDTH;
    int isScrollable   = 0;                                       /* Default is scrollable. */
    int index          = 0;
    int saveWithParent = 0;
    
    if (argc < 11) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    width          = (int)atom_getFloatAtIndex (0,  argc, argv);
    saveWithParent = (int)atom_getFloatAtIndex (1, argc, argv);
    isScrollable   = (int)atom_getFloatAtIndex (2,  argc, argv);
    index          = (int)atom_getFloatAtIndex (10,  argc, argv);
    
    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (3, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 4, argv);
    iemgui_deserializeColors (cast_iem (x), argv + 7, argv + 8, argv + 9);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)menubutton_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = PD_CLAMP (width, IEM_MENUBUTTON_MINIMUM_WIDTH, IEM_MENUBUTTON_MAXIMUM_WIDTH);
    x->x_saveWithParent     = saveWithParent;
    x->x_isScrollable       = isScrollable;
    x->x_index              = index;
    x->x_cachedBuffer       = buffer_new();
    x->x_cachedString       = heapstring_new (0);
    x->x_slots              = slots_new();
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    x->x_outletLeft   = outlet_newList (cast_object (x));
    x->x_outletMiddle = outlet_newBang (cast_object (x));
    x->x_outletRight  = outlet_newFloat (cast_object (x));
    
    return x;
}

static void menubutton_free (t_menubutton *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    stub_destroyWithKey ((void *)x);
    
    heapstring_free (x->x_cachedString);
    buffer_free (x->x_cachedBuffer);
    slots_free (x->x_slots);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void menubutton_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_menubutton,
            (t_newmethod)menubutton_new,
            (t_method)menubutton_free,
            sizeof (t_menubutton),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)menubutton_bang);
    class_addFloat (c, (t_method)menubutton_float);
    class_addList (c, (t_method)menubutton_list);
    class_addClick (c, (t_method)menubutton_click);
    class_addLoadbang (c, (t_method)menubutton_loadbang);
    
    class_addMethod (c, (t_method)menubutton_initialize,        sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)menubutton_fromValue,         sym__valuedialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)menubutton_fromDialog,        sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)menubutton_width,             sym_width,              A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)menubutton_scroll,            sym_scroll,             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)menubutton_set,               sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)menubutton_next,              sym_next,               A_NULL);
    class_addMethod (c, (t_method)menubutton_previous,          sym_previous,           A_NULL);
    class_addMethod (c, (t_method)menubutton_clear,             sym_clear,              A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)menubutton_menu,              sym_menu,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)menubutton_restore,           sym__restore,           A_GIMME, A_NULL);

    class_setWidgetBehavior (c, &menubutton_widgetBehavior);
    class_setSaveFunction (c, menubutton_functionSave);
    class_setDataFunction (c, menubutton_functionData);
    class_setValueFunction (c, menubutton_functionValue);
    class_setUndoFunction (c, menubutton_functionUndo);
    class_setPropertiesFunction (c, menubutton_functionProperties);
    
    menubutton_class = c;
}

void menubutton_destroy (void)
{
    class_free (menubutton_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
