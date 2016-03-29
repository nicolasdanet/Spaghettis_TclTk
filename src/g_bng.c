
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

#define IEM_BANG_DEFAULT_HOLD       250
#define IEM_BANG_DEFAULT_BREAK      50

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_BANG_MINIMUM_HOLD       10
#define IEM_BANG_MINIMUM_BREAK      10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior bng_widgetBehavior;

static t_class *bng_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void bng_drawUpdate (t_bng *x, t_glist *glist)
{
    if (glist_isvisible (glist)) {
        sys_vGui (".x%lx.c itemconfigure %lxBUTTON -fill #%6.6x\n", 
            glist_getcanvas (glist),
            x,
            x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    }
}

void bng_drawMove (t_bng *x, t_glist *glist)
{
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c coords %lxBASE %d %d %d %d\n",
                canvas,
                x,
                a,
                b,
                a + x->x_gui.iem_width,
                b + x->x_gui.iem_height);
    sys_vGui (".x%lx.c coords %lxBUTTON %d %d %d %d\n",
                canvas,
                x,
                a + 1,
                b + 1,
                a + x->x_gui.iem_width - 1,
                b + x->x_gui.iem_height - 1);
    sys_vGui (".x%lx.c itemconfigure %lxBUTTON -fill #%6.6x\n", 
                canvas,
                x,
                x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c coords %lxLABEL %d %d\n",
                canvas,
                x,
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY);
}

void bng_drawNew (t_bng *x, t_glist *glist)
{
    int a = text_xpix (cast_object (x), glist);
    int b = text_ypix (cast_object (x), glist);
    
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
                canvas,
                a,
                b,
                a + x->x_gui.iem_width,
                b + x->x_gui.iem_height,
                x->x_gui.iem_colorBackground,
                x);
    sys_vGui (".x%lx.c create oval %d %d %d %d -fill #%6.6x -tags %lxBUTTON\n",
                canvas,
                a + 1,
                b + 1,
                a + x->x_gui.iem_width - 1,
                b + x->x_gui.iem_height - 1,
                x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground,
                x);
    sys_vGui (".x%lx.c create text %d %d -text {%s}"                    // --
                " -anchor w"
                " -font [::getFont %d]"                                 // --
                " -fill #%6.6x -tags [list %lxLABEL label text]\n",     // --
                canvas, 
                a + x->x_gui.iem_labelX,
                b + x->x_gui.iem_labelY,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "",
                x->x_gui.iem_fontSize,
                x->x_gui.iem_colorLabel,
                x);
}

void bng_drawSelect (t_bng *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : IEM_COLOR_NORMAL);
    sys_vGui (".x%lx.c itemconfigure %lxBUTTON -outline #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : IEM_COLOR_NORMAL);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n",
                canvas,
                x, 
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel);
}

void bng_drawErase (t_bng *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c delete %lxBASE\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxBUTTON\n",
                canvas,
                x);
    sys_vGui (".x%lx.c delete %lxLABEL\n",
                canvas,
                x);
}

void bng_drawConfig (t_bng *x, t_glist *glist)
{
    t_glist *canvas = glist_getcanvas (glist);

    sys_vGui (".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n",
                canvas,
                x,
                x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxBUTTON -fill #%6.6x\n",
                canvas,
                x,
                x->x_flashed ? x->x_gui.iem_colorForeground : x->x_gui.iem_colorBackground);
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",     // --
                canvas,
                x,
                x->x_gui.iem_fontSize,
                x->x_gui.iem_isSelected ? IEM_COLOR_SELECTED : x->x_gui.iem_colorLabel,
                (x->x_gui.iem_label != iemgui_empty()) ? x->x_gui.iem_label->s_name : "");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

void bng_setFlashTimes (t_bng *x, int flashBreak, int flashHold)
{
    if (flashBreak > flashHold) { int h = flashBreak; flashBreak = flashHold; flashHold = h; }
    
    x->x_flashTimeBreak = PD_MAX (flashBreak, IEM_BANG_MINIMUM_BREAK);
    x->x_flashTimeHold  = PD_MAX (flashHold, IEM_BANG_MINIMUM_HOLD);
}

static void bng_updateFlash (t_bng *x)
{
    if (x->x_flashed) {
        x->x_flashed = 0; (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        x->x_flashed = 1;
        clock_delay (x->x_clockBreak, x->x_flashTimeBreak);
        
    } else {
        x->x_flashed = 1; (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    }
    
    clock_delay (x->x_clockHold, x->x_flashTimeHold);
}

static void bng_out (t_bng *x)
{
    outlet_bang (cast_object (x)->te_outlet);
    
    if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing) { pd_bang (x->x_gui.iem_send->s_thing); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

static void bng_click (t_bng *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    bng_updateFlash (x);
    bng_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bng_loadbang (t_bng *x)
{
    if (x->x_gui.iem_loadbang) { bng_bang (x); }
}

static void bng_initialize (t_bng *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void bng_dialog (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == IEM_DIALOG_SIZE) {
    //
    int size        = (int)atom_getFloatAtIndex (0, argc, argv);
    int flashHold   = (int)atom_getFloatAtIndex (2, argc, argv);
    int flashBreak  = (int)atom_getFloatAtIndex (3, argc, argv);
    
    iemgui_fromDialog (&x->x_gui, argc, argv);

    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    
    bng_setFlashTimes (x, flashBreak, flashHold);
    
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    
    canvas_fixlines (x->x_gui.iem_glist, (t_object*)x);
    //
    }
}

static void bng_size (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int size = atom_getFloatAtIndex (0, argc, argv);
    x->x_gui.iem_width  = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = PD_MAX (size, IEM_MINIMUM_WIDTH);
    iemgui_boxChanged ((void *)x, &x->x_gui);
    //
    }
}

static void bng_move (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 2) { iemgui_movePosition ((void *)x, &x->x_gui, s, argc, argv); }
}

static void bng_position (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 2) { iemgui_setPosition ((void *)x, &x->x_gui, s, argc, argv); }
}

static void bng_flashtime (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 2) {
    //
    int flashBreak = (int)atom_getFloatAtIndex (0, argc, argv);
    int flashHold  = (int)atom_getFloatAtIndex (1, argc, argv);
    bng_setFlashTimes (x, flashBreak, flashHold);
    //
    }
}

static void bng_color (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

static void bng_labelFont (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 2) { iemgui_setLabelFont ((void *)x, &x->x_gui, s, argc, argv); }
}

static void bng_labelPosition (t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelPosition ((void *)x, &x->x_gui, s, argc, argv);
}

static void bng_send (t_bng *x, t_symbol *s)
{
    iemgui_setSend ((void *)x, &x->x_gui, s);
}

static void bng_receive (t_bng *x, t_symbol *s)
{
    iemgui_setReceive ((void *)x, &x->x_gui, s);
}

static void bng_label (t_bng *x, t_symbol *s)
{
    iemgui_setLabel ((void *)x, &x->x_gui, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bng_taskHold (t_bng *x)
{
    x->x_flashed = 0;
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

static void bng_taskBreak (t_bng *x)
{
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bng_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static int bng_behaviorClick (t_gobj *z, t_glist *glist, int a, int b, int shift, int alt, int dbl, int k)
{
    if (k) { bng_click ((t_bng *)z, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt); }
    return 1;
}

static void bng_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_bng *x = (t_bng *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (cast_iem (z), &names, &colors);
    
    buffer_vAppend (b, "ssiisiiiisssiiiiiii", 
        gensym ("#X"),
        gensym ("obj"),
        (int)cast_object (z)->te_xCoordinate,
        (int)cast_object (z)->te_yCoordinate,
        gensym ("bng"), 
        x->x_gui.iem_width,                                     // Size.
        x->x_flashTimeHold,                                     // Flash hold.
        x->x_flashTimeBreak,                                    // Flash break.
        iemgui_serializeLoadbang (&x->x_gui),                   // Loadbang.
        names.n_unexpandedSend,                                 // Send.
        names.n_unexpandedReceive,                              // Receive.
        names.n_unexpandedLabel,                                // Label.
        x->x_gui.iem_labelX,                                    // Label X.
        x->x_gui.iem_labelY,                                    // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                  // Label font.
        x->x_gui.iem_fontSize,                                  // Label font size.
        colors.c_colorBackground,                               // Background color.
        colors.c_colorForeground,                               // Foreground color.
        colors.c_colorLabel);                                   // Label color.
                
    buffer_vAppend (b, ";");
}

static void bng_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_bng *x = (t_bng *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);
    
    err = string_sprintf (t, PD_STRING,
            "::ui_iem::create %%s Bang"
            " %d %d Size 0 0 empty"
            " %d {Flash Break} %d {Flash Hold}"     // --
            " -1 empty empty"
            " %d"
            " -1 -1 empty"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_flashTimeBreak, x->x_flashTimeHold,
            x->x_gui.iem_loadbang,
            names.n_unexpandedSend->s_name, names.n_unexpandedReceive->s_name,
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, x->x_gui.iem_colorForeground, x->x_gui.iem_colorLabel);

    PD_ASSERT (!err);
    
    gfxstub_new (cast_pd (z), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *bng_new (t_symbol *s, int argc, t_atom *argv)
{
    t_bng *x = (t_bng *)pd_new (bng_class);
    
    int size            = IEM_DEFAULT_SIZE;
    int flashHold       = IEM_BANG_DEFAULT_HOLD;
    int flashBreak      = IEM_BANG_DEFAULT_BREAK;
    int labelX          = IEM_DEFAULT_LABELX;
    int labelY          = IEM_DEFAULT_LABELY;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    t_iemcolors colors  = IEM_COLORS_DEFAULT;
    
    if (argc == 14                                              // --
            && IS_FLOAT (argv)                                  // Size.
            && IS_FLOAT (argv + 1)                              // Flash hold.
            && IS_FLOAT (argv + 2)                              // Flash break.
            && IS_FLOAT (argv + 3)                              // Loadbang.
            && (IS_SYMBOL (argv + 4) || IS_FLOAT (argv + 4))    // Send.
            && (IS_SYMBOL (argv + 5) || IS_FLOAT (argv + 5))    // Receive.
            && (IS_SYMBOL (argv + 6) || IS_FLOAT (argv + 6))    // Label.
            && IS_FLOAT (argv + 7)                              // Label X.
            && IS_FLOAT (argv + 8)                              // Label Y.
            && IS_FLOAT (argv + 9)                              // Label font.
            && IS_FLOAT (argv + 10)                             // Label font size.
            && IS_FLOAT (argv + 11)                             // Background color.
            && IS_FLOAT (argv + 12)                             // Foreground color.
            && IS_FLOAT (argv + 13))                            // Label color.
    {
        size                        = (int)atom_getFloatAtIndex (0,  argc, argv);
        flashHold                   = (int)atom_getFloatAtIndex (1,  argc, argv);
        flashBreak                  = (int)atom_getFloatAtIndex (2,  argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (7,  argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (8,  argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (10, argc, argv);
        colors.c_colorBackground    = (int)atom_getFloatAtIndex (11, argc, argv);
        colors.c_colorForeground    = (int)atom_getFloatAtIndex (12, argc, argv);
        colors.c_colorLabel         = (int)atom_getFloatAtIndex (13, argc, argv);
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (3, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 4, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (9, argc, argv));
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 4, NULL);
    }

    x->x_gui.iem_glist      = (t_glist *)canvas_getcurrent();
    x->x_gui.iem_draw       = (t_iemfn)bng_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_deserializeColors (&x->x_gui, &colors);
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    bng_setFlashTimes (x, flashBreak, flashHold);
    
    x->x_clockHold  = clock_new (x, (t_method)bng_taskHold);
    x->x_clockBreak = clock_new (x, (t_method)bng_taskBreak);
    
    outlet_new (cast_object (x), &s_bang);
    
    return x;
}

static void bng_free (t_bng *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    clock_free (x->x_clockBreak);
    clock_free (x->x_clockHold);
    
    gfxstub_deleteforkey (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bng_setup (void) 
{
    t_class *c = NULL;
    
    c = class_new (gensym ("bng"), 
            (t_newmethod)bng_new, 
            (t_method)bng_free, 
            sizeof (t_bng), 
            CLASS_DEFAULT,
            A_GIMME, 
            A_NULL);
    
    class_addBang (c, bng_bang);
    class_addFloat (c, bng_float);
    class_addSymbol (c, bng_symbol);
    class_addPointer (c, bng_pointer);
    class_addList (c, bng_list);
    class_addAnything (c, bng_anything);
    class_addClick (c, bng_click);
    
    class_addMethod (c, (t_method)bng_loadbang,         gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)bng_initialize,       gensym ("initialize"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bng_dialog,           gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_size,             gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_move,             gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_position,         gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_flashtime,        gensym ("flashtime"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_labelFont,        gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_labelPosition,    gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_send,             gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)bng_receive,          gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)bng_label,            gensym ("label"),           A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)bng_initialize,       gensym ("init"),            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)bng_color,            gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_move,             gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_position,         gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_labelPosition,    gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_labelFont,        gensym ("label_font"),      A_GIMME, A_NULL);

    #endif
    
    bng_widgetBehavior.w_getrectfn  = bng_behaviorGetRectangle;
    bng_widgetBehavior.w_displacefn = iemgui_behaviorDisplace;
    bng_widgetBehavior.w_selectfn   = iemgui_behaviorSelected;
    bng_widgetBehavior.w_activatefn = NULL;
    bng_widgetBehavior.w_deletefn   = iemgui_behaviorDeleted;
    bng_widgetBehavior.w_visfn      = iemgui_behaviorVisible;
    bng_widgetBehavior.w_clickfn    = bng_behaviorClick;
    
    class_setWidgetBehavior (c, &bng_widgetBehavior);
    class_setHelpName (c, gensym ("bng"));
    class_setSaveFunction (c, bng_behaviorSave);
    class_setPropertiesFunction (c, bng_behaviorProperties);
    
    bng_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
