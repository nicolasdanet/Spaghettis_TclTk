
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

#define IEM_VRADIO_DEFAULT_LABELX    0
#define IEM_VRADIO_DEFAULT_LABELY   -8
#define IEM_VRADIO_DEFAULT_BUTTONS   8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_buttonsNumber (t_hradio *x, t_float numberOfButtons);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior vradio_widgetBehavior;

static t_class *vradio_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void vradio_draw_update(t_gobj *client, t_glist *glist)
{
    t_hradio *x = (t_hradio *)client;
    if(glist_isvisible(glist))
    {
        t_glist *canvas=glist_getcanvas(glist);

        sys_vGui(".x%lx.c itemconfigure %lxBUTTON%d -fill #%6.6x -outline #%6.6x\n",
                 canvas, x, x->x_stateDrawn,
                 x->x_gui.iem_colorBackground, x->x_gui.iem_colorBackground);
        sys_vGui(".x%lx.c itemconfigure %lxBUTTON%d -fill #%6.6x -outline #%6.6x\n",
                 canvas, x, x->x_state,
                 x->x_gui.iem_colorForeground, x->x_gui.iem_colorForeground);
        x->x_stateDrawn = x->x_state;
    }
}

void vradio_draw_new(t_vradio *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i, dy=x->x_gui.iem_height, s4=dy/4;
    int yy11b=text_ypix(&x->x_gui.iem_obj, glist); 
    int yy11=yy11b, yy12=yy11+dy;
    int yy21=yy11+s4, yy22=yy12-s4;
    int xx11=text_xpix(&x->x_gui.iem_obj, glist), xx12=xx11+dy;
    int xx21=xx11+s4, xx22=xx12-s4;

    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE%d\n",
                 canvas, xx11, yy11, xx12, yy12,
                 x->x_gui.iem_colorBackground, x, i);
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxBUTTON%d\n",
                 canvas, xx21, yy21, xx22, yy22,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground, x, i);
        yy11 += dy;
        yy12 += dy;
        yy21 += dy;
        yy22 += dy;
        x->x_stateDrawn = x->x_state;
    }
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xx11+x->x_gui.iem_labelX, yy11b+x->x_gui.iem_labelY,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
             x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);

       /* sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xx11, yy11-1, xx11 + INLETS_WIDTH, yy11, x, 0);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xx11, yy11b, xx11 + INLETS_WIDTH, yy11b+1, x, 0);*/
}

void vradio_draw_move(t_vradio *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i, dy=x->x_gui.iem_height, s4=dy/4;
    int yy11b=text_ypix(&x->x_gui.iem_obj, glist);
    int yy11=yy11b, yy12=yy11+dy;
    int yy21=yy11+s4, yy22=yy12-s4;
    int xx11=text_xpix(&x->x_gui.iem_obj, glist), xx12=xx11+dy;
    int xx21=xx11+s4, xx22=xx12-s4;

    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c coords %lxBASE%d %d %d %d %d\n",
                 canvas, x, i, xx11, yy11, xx12, yy12);
        sys_vGui(".x%lx.c coords %lxBUTTON%d %d %d %d %d\n",
                 canvas, x, i, xx21, yy21, xx22, yy22);
        yy11 += dy;
        yy12 += dy;
        yy21 += dy;
        yy22 += dy;
    }
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xx11+x->x_gui.iem_labelX, yy11b+x->x_gui.iem_labelY);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0, xx11, yy11-1, xx11 + INLETS_WIDTH, yy11);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0, xx11, yy11b, xx11 + INLETS_WIDTH, yy11b+1);*/
}

void vradio_draw_erase(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i;

    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c delete %lxBASE%d\n", canvas, x, i);
        sys_vGui(".x%lx.c delete %lxBUTTON%d\n", canvas, x, i);
    }
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void vradio_draw_config(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i;

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE%d -fill #%6.6x\n", canvas, x, i,
                 x->x_gui.iem_colorBackground);
        sys_vGui(".x%lx.c itemconfigure %lxBUTTON%d -fill #%6.6x -outline #%6.6x\n", canvas, x, i,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
    }
}

void vradio_draw_io(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos,
        ypos+(x->x_numberOfButtons*x->x_gui.iem_height)-1,
        xpos+ INLETS_WIDTH,
        ypos+(x->x_numberOfButtons*x->x_gui.iem_height), x, 0);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos, ypos,
        xpos+ INLETS_WIDTH, ypos+1,
        x, 0);*/
}

void vradio_draw_select(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i;

    if(x->x_gui.iem_isSelected)
    {
        for(i=0; i<n; i++)
        {
            sys_vGui(".x%lx.c itemconfigure %lxBASE%d -outline #%6.6x\n", canvas, x, i,
                     IEM_COLOR_SELECTED);
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        for(i=0; i<n; i++)
        {
            sys_vGui(".x%lx.c itemconfigure %lxBASE%d -outline #%6.6x\n", canvas, x, i,
                     IEM_COLOR_NORMAL);
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x,
                 x->x_gui.iem_colorLabel);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vradio_draw(t_vradio *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        interface_guiQueueAddIfNotAlreadyThere(x, glist, vradio_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        vradio_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        vradio_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        vradio_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        vradio_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        vradio_draw_config(x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_out (t_vradio *x)
{
    outlet_float (cast_object (x)->te_outlet, x->x_floatValue);
    
    if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing) {
        pd_float (x->x_gui.iem_send->s_thing, x->x_floatValue);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_bang (t_vradio *x)
{
    vradio_out (x);
}

static void vradio_float(t_vradio *x, t_float f)
{
    x->x_state = PD_CLAMP ((int)f, 0, x->x_numberOfButtons - 1);
    x->x_floatValue = f;
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    
    if (x->x_gui.iem_goThrough) {
        vradio_out (x); 
    }
}

static void vradio_click (t_hradio *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    t_float f = ((b - text_ypix (cast_object (x), x->x_gui.iem_glist)) / x->x_gui.iem_height);
    int i = PD_CLAMP ((int)f, 0, x->x_numberOfButtons - 1);

    x->x_state = i;
    x->x_floatValue = i;
    
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        
    vradio_out (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_loadbang (t_vradio *x)
{
    if (x->x_gui.iem_loadbang) { vradio_bang (x); }
}

static void vradio_initialize (t_vradio *x, t_float f)
{
    x->x_gui.iem_loadbang = (f != 0.0);
}

static void vradio_dialog (t_vradio *x, t_symbol *s, int argc, t_atom *argv)
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

    if (x->x_numberOfButtons != numberOfButtons) { vradio_buttonsNumber (x, (t_float)numberOfButtons); } 
    else {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
        canvas_fixlines (x->x_gui.iem_glist, cast_object (x));
    }
    //
    }
}

static void vradio_size(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{
    int w = atom_getFloatAtIndex(0, ac, av);
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    iemgui_boxChanged((void *)x, &x->x_gui);
}

static void vradio_delta(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void vradio_pos(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void vradio_label_font(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void vradio_label_pos(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void vradio_set(t_vradio *x, t_float f)
{
    int i=(int)f;
    int old;

    x->x_floatValue = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_numberOfButtons)
        i = x->x_numberOfButtons-1;

    x->x_state = i;
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

static void vradio_buttonsNumber (t_hradio *x, t_float numberOfButtons)
{
    int n = PD_CLAMP ((int)numberOfButtons, 1, IEM_MAXIMUM_BUTTONS);

    if (n != x->x_numberOfButtons) {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_ERASE);
        x->x_numberOfButtons = numberOfButtons;
        x->x_state = PD_MIN (x->x_state, x->x_numberOfButtons - 1);
        x->x_floatValue = x->x_state;
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_NEW);
        canvas_fixlines (x->x_gui.iem_glist, cast_object (x));
    }
}

static void vradio_send(t_vradio *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void vradio_receive(t_vradio *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void vradio_label(t_vradio *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_hradio *x = (t_hradio *)z;
    
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height * x->x_numberOfButtons;
}

static int vradio_behaviorClick (t_gobj *z, t_glist *glist, int a, int b, int shift, int alt, int dbl, int k)
{
    if (k) {
        vradio_click ((t_hradio *)z, (t_float)a, (t_float)b, (t_float)shift, (t_float)0, (t_float)alt);
    }
    
    return 1;
}

static void vradio_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_vradio *x = (t_vradio *)z;

    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize(&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiiiisssiiiiiiif",
        gensym ("#X"),
        gensym ("obj"),
        (int)cast_object (z)->te_xCoordinate,
        (int)cast_object (z)->te_yCoordinate,
        gensym ("vradio"),
        x->x_gui.iem_width,                                         // Size.
        x->x_changed,                                               // Dummy.
        iemgui_serializeLoadbang (&x->x_gui),                       // Loadbang.
        x->x_numberOfButtons,                                       // Number of buttons.
        names.n_unexpandedSend,                                     // Send.
        names.n_unexpandedReceive,                                  // Receive.
        names.n_unexpandedLabel,                                    // Label.
        x->x_gui.iem_labelX,                                        // label X.
        x->x_gui.iem_labelY,                                        // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                      // Label font.
        x->x_gui.iem_fontSize,                                      // Label font size.
        colors.c_colorBackground,                                   // Background color.
        colors.c_colorForeground,                                   // Foreground color.
        colors.c_colorLabel,                                        // Label color.
        x->x_floatValue);                                           // Float value.
        
    buffer_vAppend (b, ";");
}

static void vradio_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_vradio *x = (t_vradio *)z;
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
    
    gfxstub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_dummy (t_vradio *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vradio_new (t_symbol *s, int argc, t_atom *argv)
{
    t_vradio *x = (t_vradio *)pd_new (vradio_class);
    
    int size            = IEM_DEFAULT_SIZE;
    int state           = 0;
    int labelX          = IEM_VRADIO_DEFAULT_LABELX;
    int labelY          = IEM_VRADIO_DEFAULT_LABELY;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    int changed         = 1;
    int numberOfButtons = IEM_VRADIO_DEFAULT_BUTTONS;
    t_float floatValue  = 0.0;
    t_iemcolors colors  = IEM_COLORS_DEFAULT;
    
    if (argc == 15                                                  // --
            && IS_FLOAT (argv + 0)                                  // Size.
            && IS_FLOAT (argv + 1)                                  // Dummy.
            && IS_FLOAT (argv + 2)                                  // Loadbang.
            && IS_FLOAT (argv + 3)                                  // Number of buttons.
            && (IS_SYMBOL (argv + 4) || IS_FLOAT (argv + 4))        // Send.
            && (IS_SYMBOL (argv + 5) || IS_FLOAT (argv + 5))        // Receive.
            && (IS_SYMBOL (argv + 6) || IS_FLOAT (argv + 6))        // Label.
            && IS_FLOAT (argv + 7)                                  // Label X.
            && IS_FLOAT (argv + 8)                                  // Label Y.
            && IS_FLOAT (argv + 9)                                  // Label font.
            && IS_FLOAT (argv + 10)                                 // Label font size.
            && IS_FLOAT (argv + 11)                                 // Background color.
            && IS_FLOAT (argv + 12)                                 // Foreground color.
            && IS_FLOAT (argv + 13)                                 // Label color.
            && IS_FLOAT (argv + 14))                                // Float value.
    {
        size                        = (int)atom_getFloatAtIndex (0, argc, argv);
        changed                     = (int)atom_getFloatAtIndex (1, argc, argv);
        numberOfButtons             = (int)atom_getFloatAtIndex (3, argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (7, argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (8, argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (10, argc, argv);
        colors.c_colorBackground    = (int)atom_getFloatAtIndex (11, argc, argv);
        colors.c_colorForeground    = (int)atom_getFloatAtIndex (12, argc, argv);
        colors.c_colorLabel         = (int)atom_getFloatAtIndex (13, argc, argv);
        floatValue                  = atom_getFloatAtIndex (14, argc, argv);
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (2, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 4, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (9, argc, argv));
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 4, NULL);
    }
    
    x->x_gui.iem_glist      = (t_glist *)canvas_getcurrent();
    x->x_gui.iem_draw       = (t_iemfn)vradio_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_checkSendReceiveLoop (&x->x_gui);
    iemgui_deserializeColors (&x->x_gui, &colors);
    
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
    
    return x;
}

static void vradio_free (t_vradio *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
    
    gfxstub_deleteforkey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vradio_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("vradio"),
            (t_newmethod)vradio_new,
            (t_method)vradio_free,
            sizeof (t_vradio),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, vradio_bang);
    class_addFloat (c, vradio_float);
    class_addClick (c, vradio_click);
    
    class_addMethod (c, (t_method)vradio_loadbang,      gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)vradio_initialize,    gensym ("initialize"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_dialog,        gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_size,          gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_delta,         gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_pos,           gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_label_font,    gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_label_pos,     gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_set,           gensym ("set"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_buttonsNumber, gensym ("buttonsnumber"),   A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_send,          gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)vradio_receive,       gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)vradio_label,         gensym ("label"),           A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)vradio_initialize,    gensym ("init"),            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_delta,         gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_pos,           gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_dummy,         gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_label_pos,     gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_label_font,    gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_buttonsNumber, gensym ("number"),          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_dummy,         gensym ("single_change"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_dummy,         gensym ("double_change"),   A_GIMME, A_NULL);
    
    #endif
    
    vradio_widgetBehavior.w_getrectfn   = vradio_behaviorGetRectangle;
    vradio_widgetBehavior.w_displacefn  = iemgui_behaviorDisplace;
    vradio_widgetBehavior.w_selectfn    = iemgui_behaviorSelected;
    vradio_widgetBehavior.w_activatefn  = NULL;
    vradio_widgetBehavior.w_deletefn    = iemgui_behaviorDeleted;
    vradio_widgetBehavior.w_visfn       = iemgui_behaviorVisible;
    vradio_widgetBehavior.w_clickfn     = vradio_behaviorClick;
    
    class_setWidgetBehavior (c, &vradio_widgetBehavior);
    class_setHelpName (c, gensym ("vradio"));
    class_setSaveFunction (c, vradio_behaviorSave);
    class_setPropertiesFunction (c, vradio_behaviorProperties);
    
    vradio_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
