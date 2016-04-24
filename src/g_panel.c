
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

#define IEM_PANEL_DEFAULT_WIDTH     100
#define IEM_PANEL_DEFAULT_HEIGHT    60

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_PANEL_MINIMUM_SIZE      1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior panel_widgetBehavior;       /* Shared. */

static t_class *panel_class;                        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void panel_drawMove (t_panel *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);
    
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);

    sys_vGui (".x%lx.c coords %lxPANEL %d %d %d %d\n",
                canvas,
                x,
                a,
                b,
                a + x->x_panelWidth,
                b + x->x_panelHeight);
    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                canvas,
                x,
                a, 
                b,
                a + x->x_gui.iem_width,
                b + x->x_gui.iem_height);
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                canvas,
                x,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY);
}

void panel_drawNew (t_panel *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);
    
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);

    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%06x -outline #%06x -tags %lxPANEL\n",
                canvas,
                a,
                b,
                a + x->x_panelWidth,
                b + x->x_panelHeight,
                x->x_gui.iem_colorBackground,
                x->x_gui.iem_colorBackground,
                x);
    sys_vGui (".x%lx.c create rectangle %d %d %d %d -outline #%06x -tags %lxBASE\n",
                canvas,
                a,
                b,
                a + x->x_gui.iem_width,
                b + x->x_gui.iem_height,
                x->x_gui.iem_colorBackground,
                x);
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
}

void panel_drawSelect (t_panel* x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -outline #%06x\n",
                canvas,
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorBackground);
}

void panel_drawErase (t_panel* x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c delete %lxBASE\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxPANEL\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                canvas,
                x);
}

void panel_drawConfig (t_panel* x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxPANEL -fill #%06x -outline #%06x\n",
                canvas,
                x,
                x->x_gui.iem_colorBackground,
                x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxBASE -outline #%06x\n",
                canvas,
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%06x -text {%s}\n",  // --
                canvas,
                x,
                x->x_gui.iem_fontSize,
                x->x_gui.iem_colorLabel,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

static void panel_dialog (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int gripSize    = (int)atom_getFloatAtIndex (0, argc, argv);
    int panelWidth  = (int)atom_getFloatAtIndex (2, argc, argv);
    int panelHeight = (int)atom_getFloatAtIndex (3, argc, argv);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);

    x->x_gui.iem_width  = PD_MAX (gripSize,    IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_height = PD_MAX (gripSize,    IEM_PANEL_MINIMUM_SIZE);
    x->x_panelWidth     = PD_MAX (panelWidth,  IEM_PANEL_MINIMUM_SIZE);
    x->x_panelHeight    = PD_MAX (panelHeight, IEM_PANEL_MINIMUM_SIZE);
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    //
    }
}

static void panel_gripSize (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int i = (int)atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width  = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_height = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

static void panel_panelSize (t_panel *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int i = (int)atom_getFloatAtIndex (0, argc, argv);

    x->x_panelWidth = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    
    if (argc > 1) { 
        i = (int)atom_getFloatAtIndex (1, argc, argv); 
    }
    
    x->x_panelHeight = PD_MAX (i, IEM_PANEL_MINIMUM_SIZE);
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    //
    }
}

static void panel_getPosition (t_panel *x)
{
    if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing) {
        t_float a = text_xpix (cast_object (x), x->x_gui.iem_glist);
        t_float b = text_ypix (cast_object (x), x->x_gui.iem_glist);
        SET_FLOAT (&x->x_t[0], a);
        SET_FLOAT (&x->x_t[1], b);
        pd_list (x->x_gui.iem_send->s_thing, 2, x->x_t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void panel_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static void panel_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_panel *x = (t_panel *)z;
    t_error err = PD_ERROR_NONE;
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiiisssiiiiss",
        gensym ("#X"),
        gensym ("obj"),
        (int)cast_object (z)->te_xCoordinate,
        (int)cast_object (z)->te_yCoordinate,
        gensym ("cnv"),
        x->x_gui.iem_width,                                                     // Grip width.
        x->x_panelWidth,                                                        // Panel width.
        x->x_panelHeight,                                                       // Panel height.
        names.n_unexpandedSend,                                                 // Send.
        names.n_unexpandedReceive,                                              // Receive.
        names.n_unexpandedLabel,                                                // Label.
        x->x_gui.iem_labelX,                                                    // Label X.
        x->x_gui.iem_labelY,                                                    // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                                  // Label font.
        x->x_gui.iem_fontSize,                                                  // Label font size.
        colors.c_symColorBackground,                                            // Background color.
        colors.c_symColorLabel);                                                // Label color.
        
    buffer_vAppend (b, ";");
}

static void panel_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_panel *x = (t_panel *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s Panel"
            " %d %d {Grip Size} 0 0 empty"          // --
            " %d {Panel Width} %d {Panel Height}"   // --
            " -1 empty empty"
            " -1"
            " -1 -1 empty"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_PANEL_MINIMUM_SIZE,
            x->x_panelWidth, x->x_panelHeight,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, -1, x->x_gui.iem_colorLabel);     /* No foreground color. */
            
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *panel_new (t_symbol *s, int argc, t_atom *argv)
{
    t_panel *x = (t_panel *)pd_new (panel_class);
    
    int gripSize        = IEM_DEFAULT_SIZE;
    int panelWidth      = IEM_PANEL_DEFAULT_WIDTH;
    int panelHeight     = IEM_PANEL_DEFAULT_HEIGHT;
    int labelX          = IEM_DEFAULT_LABELX_TOP;
    int labelY          = IEM_DEFAULT_LABELY_TOP;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
        
    if (argc >= 12                                                              // --
            && IS_FLOAT (argv + 0)                                              // Grip width.
            && IS_FLOAT (argv + 1)                                              // Panel width.
            && IS_FLOAT (argv + 2)                                              // Panel Height.
            && IS_SYMBOLORFLOAT (argv + 3)                                      // Send.
            && IS_SYMBOLORFLOAT (argv + 4)                                      // Receive.
            && IS_SYMBOLORFLOAT (argv + 5)                                      // Label.
            && IS_FLOAT (argv + 6)                                              // Label X.
            && IS_FLOAT (argv + 7)                                              // Label Y.
            && IS_FLOAT (argv + 8)                                              // Label font.
            && IS_FLOAT (argv + 9)                                              // Label font size.
            && IS_SYMBOLORFLOAT (argv + 10)                                     // Background color.
            && IS_SYMBOLORFLOAT (argv + 11))                                    // Label color.
    {
        gripSize                    = (int)atom_getFloatAtIndex (0,  argc, argv);
        panelWidth                  = (int)atom_getFloatAtIndex (1,  argc, argv);
        panelHeight                 = (int)atom_getFloatAtIndex (2,  argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (6,  argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (7,  argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (9,  argc, argv);
        
        iemgui_deserializeNamesByIndex (&x->x_gui, 3, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (8, argc, argv));
        iemgui_deserializeColors (&x->x_gui, argv + 10, NULL, argv + 11);
                
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 3, NULL);
        iemgui_deserializeColors (&x->x_gui, NULL, NULL, NULL);
    }

    x->x_gui.iem_glist      = (t_glist *)canvas_getCurrent();
    x->x_gui.iem_draw       = (t_iemfn)panel_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;

    x->x_gui.iem_width      = PD_MAX (gripSize, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_height     = PD_MAX (gripSize, IEM_PANEL_MINIMUM_SIZE);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
    
    x->x_panelWidth  = PD_MAX (panelWidth,  IEM_PANEL_MINIMUM_SIZE);
    x->x_panelHeight = PD_MAX (panelHeight, IEM_PANEL_MINIMUM_SIZE);

    SET_FLOAT (&x->x_t[0], 0.0);
    SET_FLOAT (&x->x_t[1], 0.0);
    
    return x;
}

static void panel_free (t_panel *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
    
    guistub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void panel_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("cnv"), 
        (t_newmethod)panel_new,
        (t_method)panel_free,
        sizeof (t_panel), 
        CLASS_DEFAULT | CLASS_NOINLET,
        A_GIMME,
        A_NULL);
        
    class_addMethod (c, (t_method)panel_dialog,             gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_gripSize,           gensym ("gripsize"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_backgroundColor,  gensym ("backgroundcolor"), A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelColor,       gensym ("labelcolor"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_panelSize,          gensym ("panelsize"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_getPosition,        gensym ("getposition"),     A_NULL);
    class_addMethod (c, (t_method)iemjump_send,             gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_receive,          gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)iemjump_label,            gensym ("label"),           A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)panel_gripSize,           gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_move,             gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_position,         gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_dummy,            gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelPosition,    gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)iemjump_labelFont,        gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_panelSize,          gensym ("vis_size"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)panel_getPosition,        gensym ("get_pos"),         A_NULL);
        
    class_addCreator ((t_newmethod)panel_new, gensym ("my_canvas"), A_GIMME, A_NULL);
        
    #endif
    
    panel_widgetBehavior.w_fnGetRectangle   = panel_behaviorGetRectangle;
    panel_widgetBehavior.w_fnDisplace       = iemgui_behaviorDisplace;
    panel_widgetBehavior.w_fnSelect         = iemgui_behaviorSelected;
    panel_widgetBehavior.w_fnActivate       = NULL;
    panel_widgetBehavior.w_fnDelete         = iemgui_behaviorDeleted;
    panel_widgetBehavior.w_fnVisible        = iemgui_behaviorVisible;
    panel_widgetBehavior.w_fnClick          = NULL;
    
    class_setWidgetBehavior (c, &panel_widgetBehavior);
    class_setHelpName (c, gensym ("cnv"));
    class_setSaveFunction (c, panel_behaviorSave);
    class_setPropertiesFunction (c, panel_behaviorProperties);
    
    panel_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
