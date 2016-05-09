
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
#include "g_canvas.h"
#include "g_iem.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_RADIO_DEFAULT_BUTTONS   8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void radio_buttonsNumber (t_radio *x, t_float numberOfButtons);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior radio_widgetBehavior;       /* Shared. */

static t_class *radio_class;                        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void radio_drawMoveVertical (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int n = x->x_numberOfButtons;
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    int k = x->x_gui.iem_height / 4;
    
    int i, t = b;
    
    for (i = 0; i < n; i++, t += x->x_gui.iem_height) {
    //
    sys_vGui (".x%lx.c coords %lxBASE%d %d %d %d %d\n",
                canvas,
                x,
                i,
                a,
                t,
                a + x->x_gui.iem_width,
                t + x->x_gui.iem_height);
    sys_vGui (".x%lx.c coords %lxBUTTON%d %d %d %d %d\n",
                canvas, 
                x, 
                i, 
                a + k,
                t + k,
                a + x->x_gui.iem_width - k,
                t + x->x_gui.iem_height - k);
    //
    }
    
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                canvas,
                x, 
                a + x->x_gui.iem_labelX, 
                b + x->x_gui.iem_labelY);
}

void radio_drawNewVertical (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int n = x->x_numberOfButtons;
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    int k = x->x_gui.iem_height / 4;
    
    int i, t = b;
    
    for (i = 0; i < n; i++, t += x->x_gui.iem_height) {
    //
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE%d\n",
                canvas,
                a,
                t,
                a + x->x_gui.iem_width,
                t + x->x_gui.iem_height,
                x->x_gui.iem_colorBackground,
                x,
                i);
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxBUTTON%d\n",
                canvas,
                a + k,
                t + k,
                a + x->x_gui.iem_width - k,
                t + x->x_gui.iem_height - k,
                (x->x_state == i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                (x->x_state == i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                x,
                i);
    //
    }
    
    sys_vGui (".x%lx.c create text %d %d -text {%s}"    // --
                " -anchor w"
                " -font [::getFont %d]"     // --
                " -fill #%06x"
                " -tags %lxLABEL\n",
                canvas,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "",
                x->x_gui.iem_fontSize,
                x->x_gui.iem_colorLabel,
                x);

    x->x_stateDrawn = x->x_state;
}

void radio_drawMoveHorizontal (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int n = x->x_numberOfButtons;
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    int k = x->x_gui.iem_width / 4;
    
    int i, t = a;
    
    for (i = 0; i < n; i++, t += x->x_gui.iem_width) {
    //
    sys_vGui (".x%lx.c coords %lxBASE%d %d %d %d %d\n",
                canvas, 
                x, 
                i,
                t,
                b,
                t + x->x_gui.iem_width,
                b + x->x_gui.iem_height);
    sys_vGui (".x%lx.c coords %lxBUTTON%d %d %d %d %d\n",
                canvas, 
                x, 
                i, 
                t + k, 
                b + k, 
                t + x->x_gui.iem_width - k, 
                b + x->x_gui.iem_height - k);
    //
    }
    
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                canvas, 
                x, 
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY);
}

void radio_drawNewHorizontal (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);

    int n = x->x_numberOfButtons;
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    int k = x->x_gui.iem_width / 4;
    
    int i, t = a;

    for (i = 0; i < n; i++, t += x->x_gui.iem_width) {
    //
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -tags %lxBASE%d\n",
                canvas, 
                t, 
                b, 
                t + x->x_gui.iem_width, 
                b + x->x_gui.iem_height,
                x->x_gui.iem_colorBackground,
                x,
                i);
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxBUTTON%d\n",
                canvas,
                t + k,
                b + k,
                t + x->x_gui.iem_width - k, 
                b + x->x_gui.iem_height - k,
                (x->x_state == i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                (x->x_state == i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                x,
                i);
    //
    }
    
    sys_vGui (".x%lx.c create text %d %d -text {%s}"    // --
                " -anchor w"
                " -font [::getFont %d]"     // --
                " -fill #%06x"
                " -tags %lxLABEL\n",
                canvas,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "",
                x->x_gui.iem_fontSize,
                x->x_gui.iem_colorLabel,
                x);
    
    x->x_stateDrawn = x->x_state;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void radio_drawUpdate (t_radio *x, t_glist *glist)
{
    if (canvas_isVisible (glist)) {
    //
    t_glist *canvas = canvas_getView (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBUTTON%d -fill #%06x -outline #%06x\n",
                canvas, 
                x, 
                x->x_stateDrawn,
                x->x_gui.iem_colorBackground,
                x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxBUTTON%d -fill #%06x -outline #%06x\n",
                canvas, 
                x, 
                x->x_state,
                x->x_gui.iem_colorForeground,
                x->x_gui.iem_colorForeground);
                
    x->x_stateDrawn = x->x_state;
    //
    }
}

void radio_drawMove (t_radio *x, t_glist *glist)
{
    if (x->x_isVertical) { radio_drawMoveVertical (x, glist); }
    else {
        radio_drawMoveHorizontal (x, glist);
    }
}

void radio_drawNew (t_radio *x, t_glist *glist)
{
    if (x->x_isVertical) { radio_drawNewVertical (x, glist); }
    else {
        radio_drawNewHorizontal (x, glist);
    }
}

void radio_drawSelect (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    sys_vGui (".x%lx.c itemconfigure %lxBASE%d -outline #%06x\n",
                canvas,
                x,
                i,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : IEM_COLOR_NORMAL);
    //
    }

    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%06x\n",
                canvas,
                x, 
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

void radio_drawErase (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    sys_vGui (".x%lx.c delete %lxBASE%d\n",
                canvas,
                x,
                i);
    sys_vGui (".x%lx.c delete %lxBUTTON%d\n",
                canvas,
                x,
                i);
    //
    }
    
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                canvas,
                x);
}

void radio_drawConfig (t_radio *x, t_glist *glist)
{
    t_glist *canvas = canvas_getView (glist);
    
    int i;

    for (i = 0; i < x->x_numberOfButtons; i++) {
    //
    sys_vGui (".x%lx.c itemconfigure %lxBASE%d -fill #%06x\n", 
                canvas,
                x,
                i,
                x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxBUTTON%d -fill #%06x -outline #%06x\n",
                canvas,
                x,
                i,
                (x->x_state == i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                (x->x_state == i) ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    //
    }
    
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",  // --
                canvas, 
                x, 
                x->x_gui.iem_fontSize,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void radio_draw (t_toggle *x, t_glist *glist, int mode)
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
#pragma mark -

static void radio_out (t_radio *x)
{
    outlet_float (cast_object (x)->te_outlet, x->x_floatValue);
    
    if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing) {
        pd_float (x->x_gui.iem_send->s_thing, x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void radio_bang (t_radio *x)
{
    radio_out (x);
}

static void radio_float (t_radio *x, t_float f)
{
    x->x_state = PD_CLAMP ((int)f, 0, x->x_numberOfButtons - 1);
    x->x_floatValue = f;
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
    
    if (x->x_gui.iem_goThrough) {
        radio_out (x); 
    }
}

static void radio_click (t_radio *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    t_float f;
    
    if (x->x_isVertical) { 
        f = ((b - text_ypix (cast_object (x), x->x_gui.iem_owner)) / x->x_gui.iem_height);
    } else {
        f = ((a - text_xpix (cast_object (x), x->x_gui.iem_owner)) / x->x_gui.iem_width);
    }

    x->x_state = PD_CLAMP ((int)f, 0, x->x_numberOfButtons - 1);
    x->x_floatValue = x->x_state;
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
        
    radio_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void radio_loadbang (t_radio *x)
{
    if (x->x_gui.iem_loadbang) { radio_bang (x); }
}

static void radio_initialize (t_radio *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void radio_dialog (t_radio *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int size            = (int)atom_getFloatAtIndex (0, argc, argv);
    int changed         = (int)atom_getFloatAtIndex (4, argc, argv);
    int numberOfButtons = (int)atom_getFloatAtIndex (6, argc, argv);

    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);
    
    numberOfButtons = PD_CLAMP (numberOfButtons, 1, IEM_MAXIMUM_BUTTONS);
    
    x->x_changed = (changed != 0);

    if (x->x_numberOfButtons != numberOfButtons) { radio_buttonsNumber (x, (t_float)numberOfButtons); } 
    else {
        iemgui_boxChanged ((void *)x, &x->x_gui);
    }
    //
    }
}

static void radio_size (t_radio *x, t_symbol *s, int argc, t_atom *argv)
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

static void radio_set (t_radio *x, t_float f)
{
    x->x_state = PD_CLAMP ((int)f, 0, x->x_numberOfButtons - 1);
    x->x_floatValue = f;
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_UPDATE);
}

static void radio_buttonsNumber (t_radio *x, t_float numberOfButtons)
{
    int n = PD_CLAMP ((int)numberOfButtons, 1, IEM_MAXIMUM_BUTTONS);

    if (n != x->x_numberOfButtons) {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_ERASE);
        x->x_numberOfButtons = numberOfButtons;
        x->x_state = PD_MIN (x->x_state, x->x_numberOfButtons - 1);
        x->x_floatValue = x->x_state;
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_owner, IEM_DRAW_NEW);
        canvas_updateLinesByObject (x->x_gui.iem_owner, cast_object (x));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void radio_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_radio *x = (t_radio *)z;
    
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    
    if (x->x_isVertical) {
        *c = *a + cast_iem (z)->iem_width;
        *d = *b + cast_iem (z)->iem_height * x->x_numberOfButtons;
    } else {
        *c = *a + cast_iem (z)->iem_width * x->x_numberOfButtons;
        *d = *b + cast_iem (z)->iem_height;
    }
}

static int radio_behaviorClick (t_gobj *z, t_glist *glist, int a, int b, int shift, int alt, int dbl, int k)
{
    if (k) {
        radio_click ((t_radio *)z, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt);
    }
    
    return 1;
}

static void radio_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_radio *x = (t_radio *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiiiisssiiiisssf", 
        sym__X,
        sym_obj,
        (int)cast_object (z)->te_xCoordinate,
        (int)cast_object (z)->te_yCoordinate,
        x->x_isVertical ? sym_vradio : sym_hradio,
        x->x_gui.iem_width,                                         // Size.
        x->x_changed,                                               // Dummy.
        iemgui_serializeLoadbang (&x->x_gui),                       // Loadbang.
        x->x_numberOfButtons,                                       // Number of buttons.
        names.n_unexpandedSend,                                     // Send.
        names.n_unexpandedReceive,                                  // Receive.
        names.n_unexpandedLabel,                                    // Label.
        x->x_gui.iem_labelX,                                        // Label X.
        x->x_gui.iem_labelY,                                        // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                      // Label font.
        x->x_gui.iem_fontSize,                                      // Label font size.
        colors.c_symColorBackground,                                // Background color.
        colors.c_symColorForeground,                                // Foreground color.
        colors.c_symColorLabel,                                     // Label color.
        x->x_floatValue);                                           // Float value.
        
    buffer_vAppend (b, ";");
}

static void radio_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_radio *x = (t_radio *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;
    
    iemgui_serializeNames (&x->x_gui, &names);

    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s {Radio Button}"   // --
            " %d %d Size 0 0 empty"
            " 0 empty 0 empty"
            " -1 empty empty"
            " %d"
            " %d 256 {Number Of Buttons}"           // --
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_gui.iem_loadbang,
            x->x_numberOfButtons,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel);
            
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *radio_new (t_symbol *s, int argc, t_atom *argv)
{
    t_radio *x = (t_radio *)pd_new (radio_class);
    
    if (s == sym_vradio)  { x->x_isVertical = 1; }
    
    {
    //
    int size            = IEM_DEFAULT_SIZE;
    int state           = 0;
    int labelX          = x->x_isVertical ? IEM_DEFAULT_LABELX_NEXT : IEM_DEFAULT_LABELX_TOP;
    int labelY          = x->x_isVertical ? IEM_DEFAULT_LABELY_NEXT : IEM_DEFAULT_LABELY_TOP;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    int changed         = 1;
    int numberOfButtons = IEM_RADIO_DEFAULT_BUTTONS;
    t_float floatValue  = 0.0;
    
    if (argc == 15                                                  // --
            && IS_FLOAT (argv + 0)                                  // Size.
            && IS_FLOAT (argv + 1)                                  // Dummy.
            && IS_FLOAT (argv + 2)                                  // Loadbang.
            && IS_FLOAT (argv + 3)                                  // Number of buttons.
            && IS_SYMBOLORFLOAT (argv + 4)                          // Send.
            && IS_SYMBOLORFLOAT (argv + 5)                          // Receive.
            && IS_SYMBOLORFLOAT (argv + 6)                          // Label.
            && IS_FLOAT (argv + 7)                                  // Label X.
            && IS_FLOAT (argv + 8)                                  // Label Y.
            && IS_FLOAT (argv + 9)                                  // Label font.
            && IS_FLOAT (argv + 10)                                 // Label font size.
            && IS_SYMBOLORFLOAT (argv + 11)                         // Background color.
            && IS_SYMBOLORFLOAT (argv + 12)                         // Foreground color.
            && IS_SYMBOLORFLOAT (argv + 13)                         // Label color.
            && IS_FLOAT (argv + 14))                                // Float value.
    {
        size                        = (int)atom_getFloatAtIndex (0, argc,  argv);
        changed                     = (int)atom_getFloatAtIndex (1, argc,  argv);
        numberOfButtons             = (int)atom_getFloatAtIndex (3, argc,  argv);
        labelX                      = (int)atom_getFloatAtIndex (7, argc,  argv);
        labelY                      = (int)atom_getFloatAtIndex (8, argc,  argv);
        labelFontSize               = (int)atom_getFloatAtIndex (10, argc, argv);
        floatValue                  = atom_getFloatAtIndex (14, argc, argv);
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (2, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 4, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (9, argc, argv));
        iemgui_deserializeColors (&x->x_gui, argv + 11, argv + 12, argv + 13);
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 4, NULL);
        iemgui_deserializeColors (&x->x_gui, NULL, NULL, NULL);
    }
    
    x->x_gui.iem_owner      = (t_glist *)canvas_getCurrent();
    x->x_gui.iem_draw       = (t_iemfn)radio_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_changed = (changed != 0);
    x->x_numberOfButtons = PD_CLAMP (numberOfButtons, 1, IEM_MAXIMUM_BUTTONS);
    x->x_floatValue = floatValue;
    
    if (x->x_gui.iem_loadbang) {
        x->x_state = PD_CLAMP ((int)floatValue, 0, x->x_numberOfButtons - 1);
    } else {
        x->x_state = 0;
    }

    outlet_new (cast_object (x), &s_list);
    //
    }
    
    return x;
}

static void radio_free (t_radio *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
    
    guistub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
        
    class_addBang (c, radio_bang);
    class_addFloat (c, radio_float);
    class_addClick (c, radio_click);
    
    class_addMethod (c, (t_method)radio_loadbang,           sym_loadbang,               A_NULL);
    class_addMethod (c, (t_method)radio_initialize,         gensym ("initialize"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)radio_dialog,             gensym ("_iemdialog"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)radio_size,               gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    gensym ("labelpostion"),    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_backgroundColor,  gensym ("backgroundcolor"), A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_foregroundColor,  gensym ("foregroundcolor"), A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelColor,       gensym ("labelcolor"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)radio_set,                gensym ("set"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)radio_buttonsNumber,      gensym ("buttonsnumber"),   A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemjump_send,             gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_receive,          gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_label,            gensym ("label"),           A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)radio_initialize,         gensym ("init"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             gensym ("delta"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         gensym ("pos"),              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            gensym ("color"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    gensym ("label_pos"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        gensym ("label_font"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)radio_buttonsNumber,      gensym ("number"),           A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            gensym ("single_change"),    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            gensym ("double_change"),    A_GIMME, A_NULL);
    
    #endif
    
    radio_widgetBehavior.w_fnGetRectangle   = radio_behaviorGetRectangle;
    radio_widgetBehavior.w_fnDisplace       = iemgui_behaviorDisplace;
    radio_widgetBehavior.w_fnSelect         = iemgui_behaviorSelected;
    radio_widgetBehavior.w_fnActivate       = NULL;
    radio_widgetBehavior.w_fnDelete         = iemgui_behaviorDeleted;
    radio_widgetBehavior.w_fnVisible        = iemgui_behaviorVisible;
    radio_widgetBehavior.w_fnClick          = radio_behaviorClick;
    
    class_setWidgetBehavior (c, &radio_widgetBehavior);
    class_setHelpName (c, gensym ("radio"));
    class_setSaveFunction (c, radio_behaviorSave);
    class_setPropertiesFunction (c, radio_behaviorProperties);
    
    radio_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
