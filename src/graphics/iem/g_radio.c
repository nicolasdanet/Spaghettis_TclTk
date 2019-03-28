
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

#define IEM_RADIO_BUTTONS_DEFAULT       8
#define IEM_RADIO_BUTTONS_MAXIMUM       32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int  radio_getStateAt            (t_radio *x, int);
static void radio_buttonsNumber         (t_radio *, t_float);
static void radio_behaviorGetRectangle  (t_gobj *, t_glist *, t_rectangle *);
static int  radio_behaviorMouse         (t_gobj *, t_glist *, t_mouse *);
static void radio_drawConfig            (t_radio *, t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *radio_class;                            /* Shared. */

static t_widgetbehavior radio_widgetBehavior =          /* Shared. */
    {
        radio_behaviorGetRectangle,
        iemgui_behaviorDisplaced,
        iemgui_behaviorSelected,
        NULL,
        iemgui_behaviorDeleted,
        iemgui_behaviorVisibilityChanged,
        radio_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_drawMoveVertical (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int n = x->x_numberOfButtons;
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int k = x->x_gui.iem_height / 4;
    
    int i, t = b;
    
    for (i = 0; i < n; i++, t += x->x_gui.iem_height) {
    //
    gui_vAdd ("%s.c coords %lxBASE%d %d %d %d %d\n",
                    glist_getTagAsString (view),
                    x,
                    i,
                    a,
                    t,
                    a + x->x_gui.iem_width,
                    t + x->x_gui.iem_height);
    gui_vAdd ("%s.c coords %lxBUTTON%d %d %d %d %d\n",
                    glist_getTagAsString (view), 
                    x, 
                    i, 
                    a + k,
                    t + k,
                    a + x->x_gui.iem_width - k,
                    t + x->x_gui.iem_height - k);
    //
    }
}

static void radio_drawNewVertical (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int n = x->x_numberOfButtons;
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int k = x->x_gui.iem_height / 4;
    
    int i, t = b;
    
    for (i = 0; i < n; i++, t += x->x_gui.iem_height) {
    //
    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE%d\n",
                    glist_getTagAsString (view),
                    a,
                    t,
                    a + x->x_gui.iem_width,
                    t + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x,
                    i);
    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxBUTTON%d\n",
                    glist_getTagAsString (view),
                    a + k,
                    t + k,
                    a + x->x_gui.iem_width - k,
                    t + x->x_gui.iem_height - k,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    x,
                    i);
    //
    }
}

static void radio_drawMoveHorizontal (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int n = x->x_numberOfButtons;
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int k = x->x_gui.iem_width / 4;
    
    int i, t = a;
    
    for (i = 0; i < n; i++, t += x->x_gui.iem_width) {
    //
    gui_vAdd ("%s.c coords %lxBASE%d %d %d %d %d\n",
                    glist_getTagAsString (view), 
                    x, 
                    i,
                    t,
                    b,
                    t + x->x_gui.iem_width,
                    b + x->x_gui.iem_height);
    gui_vAdd ("%s.c coords %lxBUTTON%d %d %d %d %d\n",
                    glist_getTagAsString (view), 
                    x, 
                    i, 
                    t + k, 
                    b + k, 
                    t + x->x_gui.iem_width - k, 
                    b + x->x_gui.iem_height - k);
    //
    }
}

static void radio_drawNewHorizontal (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);

    int n = x->x_numberOfButtons;
    int a = glist_getPixelX (glist, cast_object (x));
    int b = glist_getPixelY (glist, cast_object (x));
    int k = x->x_gui.iem_width / 4;
    
    int i, t = a;

    for (i = 0; i < n; i++, t += x->x_gui.iem_width) {
    //
    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE%d\n",
                    glist_getTagAsString (view), 
                    t, 
                    b, 
                    t + x->x_gui.iem_width, 
                    b + x->x_gui.iem_height,
                    x->x_gui.iem_colorBackground,
                    x,
                    i);
    gui_vAdd ("%s.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxBUTTON%d\n",
                    glist_getTagAsString (view),
                    t + k,
                    b + k,
                    t + x->x_gui.iem_width - k, 
                    b + x->x_gui.iem_height - k,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    x,
                    i);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_drawJob (t_gobj *z, t_glist *glist)
{
    t_radio *x = (t_radio *)z;
    t_glist *view = glist_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    gui_vAdd ("%s.c itemconfigure %lxBUTTON%d -fill #%06x -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    i,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_drawUpdate (t_radio *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, radio_drawJob);
}

static void radio_drawMove (t_radio *x, t_glist *glist)
{
    if (x->x_isVertical) { radio_drawMoveVertical (x, glist); }
    else {
        radio_drawMoveHorizontal (x, glist);
    }
}

static void radio_drawNew (t_radio *x, t_glist *glist)
{
    if (x->x_isVertical) { radio_drawNewVertical (x, glist); }
    else {
        radio_drawNewHorizontal (x, glist);
    }
}

static void radio_drawSelect (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    gui_vAdd ("%s.c itemconfigure %lxBASE%d -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    i,
                    x->x_gui.iem_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    //
    }
}

static void radio_drawErase (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    gui_vAdd ("%s.c delete %lxBASE%d\n",
                    glist_getTagAsString (view),
                    x,
                    i);
    gui_vAdd ("%s.c delete %lxBUTTON%d\n",
                    glist_getTagAsString (view),
                    x,
                    i);
    //
    }
}

static void radio_drawConfig (t_radio *x, t_glist *glist)
{
    t_glist *view = glist_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    gui_vAdd ("%s.c itemconfigure %lxBASE%d -fill #%06x\n", 
                    glist_getTagAsString (view),
                    x,
                    i,
                    x->x_gui.iem_colorBackground);
    gui_vAdd ("%s.c itemconfigure %lxBUTTON%d -fill #%06x -outline #%06x\n",
                    glist_getTagAsString (view),
                    x,
                    i,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                    radio_getStateAt (x, i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_draw (t_radio *x, t_glist *glist, int mode)
{
    switch (mode) {
        case IEM_DRAW_UPDATE    : radio_drawUpdate (x, glist);  break;
        case IEM_DRAW_MOVE      : radio_drawMove (x, glist);    break;
        case IEM_DRAW_NEW       : radio_drawNew (x, glist);     break;
        case IEM_DRAW_SELECT    : radio_drawSelect (x, glist);  break;
        case IEM_DRAW_ERASE     : radio_drawErase (x, glist);   break;
        case IEM_DRAW_CONFIG    : radio_drawConfig (x, glist);  break;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_setState (t_radio *x, int64_t n)
{
    if (x->x_mode == sym_multiple) { x->x_state = n & (((int64_t)1 << x->x_numberOfButtons) - 1); }
    else {
        x->x_state = PD_CLAMP (n, 0, (x->x_numberOfButtons - 1));
    }
}

static void radio_setStateAt (t_radio *x, int position)
{
    if (x->x_mode != sym_multiple) { radio_setState (x, (int64_t)position); }
    else {
    //
    int64_t n = x->x_state;
    int64_t i = 1 << position;
    
    if (n & i) { n &= ~(i); } else { n |= i; }
    
    radio_setState (x, n);
    //
    }
    
    x->x_floatValue = x->x_state;
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static int radio_getStateAt (t_radio *x, int position)
{
    if (x->x_mode != sym_multiple) { return (x->x_state == position); }
    else {
    //
    return ((x->x_state & (1 << position)) != 0);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_out (t_radio *x)
{
    outlet_float (x->x_outlet, x->x_floatValue);
    
    if (x->x_gui.iem_canSend && symbol_hasThing (x->x_gui.iem_send)) {
        pd_float (symbol_getThing (x->x_gui.iem_send), x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_bang (t_radio *x)
{
    radio_out (x);
}

static void radio_float (t_radio *x, t_float f)
{
    radio_setState (x, (int64_t)f); x->x_floatValue = f;
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    
    if (x->x_gui.iem_goThrough) {
        radio_out (x); 
    }
}

static void radio_click (t_radio *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    int f;
    
    if (x->x_isVertical) { 
        f = (int)((b - glist_getPixelY (x->x_gui.iem_owner, cast_object (x))) / x->x_gui.iem_height);
    } else {
        f = (int)((a - glist_getPixelX (x->x_gui.iem_owner, cast_object (x))) / x->x_gui.iem_width);
    }

    radio_setStateAt (x, f); radio_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_loadbang (t_radio *x)
{
    if (x->x_gui.iem_loadbang) { radio_bang (x); }
}

static void radio_initialize (t_radio *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void radio_size (t_radio *x, t_symbol *s, int argc, t_atom *argv)
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

static void radio_set (t_radio *x, t_float f)
{
    radio_setState (x, (int64_t)f); x->x_floatValue = f;
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void radio_buttonsNumber (t_radio *x, t_float numberOfButtons)
{
    int n = PD_CLAMP ((int)numberOfButtons, 1, IEM_RADIO_BUTTONS_MAXIMUM);

    if (n != x->x_numberOfButtons) {
        (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_ERASE);
        x->x_numberOfButtons = numberOfButtons;
        radio_setState (x, x->x_state);
        (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_NEW);
        glist_updateLinesForObject (x->x_gui.iem_owner, cast_object (x));
    }
}

static void radio_mode (t_radio *x, t_symbol *s)
{
    t_symbol *old = x->x_mode;
    
    x->x_mode = (s == sym_multiple) ? sym_multiple : sym_single;
    
    if (old != x->x_mode) {
    //
    radio_setState (x, x->x_state);
    
    glist_setDirty (cast_iem (x)->iem_owner, 1);
    
    (*(cast_iem (x)->iem_fnDraw)) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void radio_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_radio *x = (t_radio *)z;
    
    int a = glist_getPixelX (glist, cast_object (z));
    int b = glist_getPixelY (glist, cast_object (z));
    int c;
    int d;
    
    if (x->x_isVertical) {
        c = a + cast_iem (z)->iem_width;
        d = b + cast_iem (z)->iem_height * x->x_numberOfButtons;
        
    } else {
        c = a + cast_iem (z)->iem_width * x->x_numberOfButtons;
        d = b + cast_iem (z)->iem_height;
    }
    
    rectangle_set (r, a, b, c, d);
}

static int radio_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    if (m->m_clicked) { radio_click ((t_radio *)z, NULL, mouse_argc (m), mouse_argv (m)); }
    
    return 1;
}

/* To keep compatibility with legacy the multiple mode is saved only if activated. */

static void radio_functionSave (t_gobj *z, t_buffer *b, int flags)
{
    t_radio *x = (t_radio *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (z)));
    buffer_appendFloat (b,  object_getY (cast_object (z)));
    buffer_appendSymbol (b, x->x_isVertical ? sym_vradio : sym_hradio);
    buffer_appendFloat (b,  x->x_gui.iem_width);
    if (x->x_mode == sym_multiple) { buffer_appendSymbol (b, x->x_mode); }
    else {
        buffer_appendFloat (b,  x->x_changed);
    }
    buffer_appendFloat (b,  iemgui_serializeLoadbang (cast_iem (z)));
    buffer_appendFloat (b,  x->x_numberOfButtons);
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
    buffer_appendFloat (b,  x->x_floatValue);
    if (SAVED_DEEP (flags)) { buffer_appendFloat (b, 1.0); }
    buffer_appendSemicolon (b);
    
    if (flags & SAVE_UNDO) { gobj_serializeUnique (z, sym__tagobject, b); }
}

static void radio_functionValue (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_radio *x = (t_radio *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    
    err = string_sprintf (t, PD_STRING,
            "::ui_value::show %%s radio float %.9g\n",      // --
            x->x_floatValue);

    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

/* Fake dialog message from interpreter. */

static void radio_functionUndo (t_gobj *z, t_buffer *b)
{
    t_radio *x = (t_radio *)z;
    
    t_iemnames names;

    iemgui_serializeNames (cast_iem (z), &names);
    
    buffer_appendSymbol (b, sym__iemdialog);
    buffer_appendFloat (b,  x->x_gui.iem_width);                /* Width. */
    buffer_appendFloat (b,  -1.0);                              /* Height. */
    buffer_appendFloat (b,  -1.0);                              /* Option1. */
    buffer_appendFloat (b,  -1.0);                              /* Option2. */
    buffer_appendFloat (b,  (x->x_mode == sym_multiple));       /* Check. */
    buffer_appendFloat (b,  x->x_gui.iem_loadbang);             /* Loadbang. */
    buffer_appendFloat (b,  x->x_numberOfButtons);              /* Extra. */
    buffer_appendSymbol (b, names.n_unexpandedSend);            /* Send. */
    buffer_appendSymbol (b, names.n_unexpandedReceive);         /* Receive. */
    buffer_appendFloat (b,  x->x_gui.iem_colorBackground);      /* Background color. */
    buffer_appendFloat (b,  x->x_gui.iem_colorForeground);      /* Foreground color. */
    buffer_appendFloat (b,  -1.0);                              /* Steady. */
    buffer_appendFloat (b,  -1.0);                              /* Save. */
}

static void radio_functionProperties (t_gobj *z, t_glist *owner, t_mouse *dummy)
{
    t_radio *x = (t_radio *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;
    
    iemgui_serializeNames (cast_iem (z), &names);

    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s {Radio Button}"               // --
            " %d %d Size -1 -1 $::var(nil)"                     // --
            " -1 $::var(nil) -1 $::var(nil)"                    // --
            " %d Single Multiple"                               // --
            " %d"
            " %d 256 {Number Of Buttons}"                       // --
            " %s %s"
            " %d %d"
            " -1"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_mode == sym_multiple,
            x->x_gui.iem_loadbang,
            x->x_numberOfButtons,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground);
            
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void radio_fromValue (t_radio *x, t_symbol *s, int argc, t_atom *argv)
{
    radio_float (x, atom_getFloatAtIndex (0, argc, argv));
}

static void radio_fromDialog (t_radio *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty  = 0;
    int undoable = glist_undoIsOk (cast_iem (x)->iem_owner);
    
    PD_ASSERT (argc == IEM_DIALOG_SIZE);
    
    int t0 = x->x_gui.iem_width;
    int t1 = x->x_numberOfButtons;
    t_symbol *t2 = x->x_mode;
    
    t_undosnippet *snippet = NULL;
    
    if (undoable) { snippet = undosnippet_newProperties (cast_gobj (x), cast_iem (x)->iem_owner); }

    {
    //
    int size            = (int)atom_getFloatAtIndex (0, argc, argv);
    int isMultiple      = (int)atom_getFloatAtIndex (4, argc, argv);
    int numberOfButtons = (int)atom_getFloatAtIndex (6, argc, argv);
    
    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    
    isDirty = iemgui_fromDialog (cast_iem (x), argc, argv);
    
    numberOfButtons = PD_CLAMP (numberOfButtons, 1, IEM_RADIO_BUTTONS_MAXIMUM);
    
    if (x->x_numberOfButtons != numberOfButtons) { radio_buttonsNumber (x, (t_float)numberOfButtons); }

    radio_mode (x, isMultiple ? sym_multiple : sym_single);
    //
    }
    
    isDirty |= (t0 != x->x_gui.iem_width);
    isDirty |= (t1 != x->x_numberOfButtons);
    isDirty |= (t2 != x->x_mode);
    
    iemgui_dirty (cast_iem (x), isDirty, undoable, snippet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *radio_new (t_symbol *s, int argc, t_atom *argv)
{
    t_radio *x = (t_radio *)pd_new (radio_class);
    
    if (s == sym_vradio)  { x->x_isVertical = 1; }
    
    {
    //
    int size            = IEM_DEFAULT_SIZE;
    int labelX          = 0;
    int labelY          = 0;
    int labelFontSize   = IEM_DEFAULT_FONT;
    int changed         = 1;
    int numberOfButtons = IEM_RADIO_BUTTONS_DEFAULT;
    t_float floatValue  = 0.0;
    t_symbol *mode      = NULL;
    int deep            = 0;
    
    if (argc < 15) { iemgui_deserializeDefault (cast_iem (x)); }
    else {
    //
    size            = (int)atom_getFloatAtIndex (0, argc,  argv);
    changed         = (int)atom_getFloatAtIndex (1, argc,  argv);
    numberOfButtons = (int)atom_getFloatAtIndex (3, argc,  argv);
    labelX          = (int)atom_getFloatAtIndex (7, argc,  argv);
    labelY          = (int)atom_getFloatAtIndex (8, argc,  argv);
    labelFontSize   = (int)atom_getFloatAtIndex (10, argc, argv);
    floatValue      = atom_getFloatAtIndex (14, argc, argv);
    mode            = atom_getSymbolAtIndex (1, argc, argv);
    deep            = (argc > 15);
    
    iemgui_deserializeLoadbang (cast_iem (x), (int)atom_getFloatAtIndex (2, argc, argv));
    iemgui_deserializeNames (cast_iem (x), 4, argv);
    iemgui_deserializeFontStyle (cast_iem (x), (int)atom_getFloatAtIndex (9, argc, argv));
    iemgui_deserializeColors (cast_iem (x), argv + 11, argv + 12, argv + 13);
    //
    }
    
    x->x_gui.iem_owner      = instance_contextGetCurrent();
    x->x_gui.iem_fnDraw     = (t_iemfn)radio_draw;
    x->x_gui.iem_canSend    = symbol_isNil (x->x_gui.iem_send) ? 0 : 1;
    x->x_gui.iem_canReceive = symbol_isNil (x->x_gui.iem_receive) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = labelFontSize;
    
    iemgui_checkSendReceiveLoop (cast_iem (x));
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_changed = (changed != 0);
    x->x_numberOfButtons = PD_CLAMP (numberOfButtons, 1, IEM_RADIO_BUTTONS_MAXIMUM);
    x->x_floatValue = (deep || x->x_gui.iem_loadbang) ? floatValue : 0;
    x->x_mode = (mode == sym_multiple) ? sym_multiple : sym_single;
    
    radio_setState (x, (int64_t)(x->x_floatValue));
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    //
    }
    
    return x;
}

static void radio_free (t_radio *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void radio_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_hradio, 
            (t_newmethod)radio_new,
            (t_method)radio_free,
            sizeof (t_radio),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addCreator ((t_newmethod)radio_new, sym_vradio, A_GIMME, A_NULL);
        
    class_addBang (c, (t_method)radio_bang);
    class_addFloat (c, (t_method)radio_float);
    class_addClick (c, (t_method)radio_click);
    class_addLoadbang (c, (t_method)radio_loadbang);
    
    class_addMethod (c, (t_method)radio_initialize,             sym_initialize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)radio_fromValue,              sym__valuedialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)radio_fromDialog,             sym__iemdialog,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)radio_size,                   sym_size,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setBackgroundColor,    sym_backgroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemgui_setForegroundColor,    sym_foregroundcolor,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)radio_set,                    sym_set,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)radio_buttonsNumber,          sym_buttonsnumber,      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemgui_setSend,               sym_send,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemgui_setReceive,            sym_receive,            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)radio_mode,                   sym_mode,               A_DEFSYMBOL, A_NULL);
    
    class_setWidgetBehavior (c, &radio_widgetBehavior);
    class_setHelpName (c, sym_radio);
    class_setSaveFunction (c, radio_functionSave);
    class_setValueFunction (c, radio_functionValue);
    class_setUndoFunction (c, radio_functionUndo);
    class_setPropertiesFunction (c, radio_functionProperties);
    
    radio_class = c;
}

void radio_destroy (void)
{
    class_free (radio_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
