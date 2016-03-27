
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

#define IEM_BANG_DEFAULT_SIZE       15
#define IEM_BANG_DEFAULT_HOLD       250
#define IEM_BANG_DEFAULT_BREAK      50
#define IEM_BANG_DEFAULT_LABELX     17
#define IEM_BANG_DEFAULT_LABELY     7
#define IEM_BANG_DEFAULT_FONTSIZE   10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_BANG_MINIMUM_SIZE       8
#define IEM_BANG_MINIMUM_HOLD       10
#define IEM_BANG_MINIMUM_BREAK      10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior bng_widgetBehavior;

static t_class *bng_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void bng_draw_update(t_bng *x, t_glist *glist)
{
    if(glist_isvisible(glist))
    {
        sys_vGui(".x%lx.c itemconfigure %lxBUT -fill #%6.6x\n", glist_getcanvas(glist), x,
                 x->x_flashed?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
    }
}

void bng_draw_new(t_bng *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xpos, ypos,
             xpos + x->x_gui.iem_width, ypos + x->x_gui.iem_height,
             x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create oval %d %d %d %d -fill #%6.6x -tags %lxBUT\n",
             canvas, xpos+1, ypos+1,
             xpos + x->x_gui.iem_width-1, ypos + x->x_gui.iem_height-1,
             x->x_flashed?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xpos+x->x_gui.iem_labelX,
             ypos+x->x_gui.iem_labelY,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
             x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);
    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xpos,
             ypos + x->x_gui.iem_height-1, xpos + INLETS_WIDTH,
             ypos + x->x_gui.iem_height, x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xpos, ypos,
             xpos + INLETS_WIDTH, ypos+1, x, 0);*/
}

void bng_draw_move(t_bng *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + x->x_gui.iem_width, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxBUT %d %d %d %d\n",
             canvas, x, xpos+1,ypos+1,
             xpos + x->x_gui.iem_width-1, ypos + x->x_gui.iem_height-1);
    sys_vGui(".x%lx.c itemconfigure %lxBUT -fill #%6.6x\n", canvas, x,
             x->x_flashed?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY);
    /*
    sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0, xpos,
             ypos + x->x_gui.iem_height-1, xpos + INLETS_WIDTH,
             ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0, xpos, ypos,
             xpos + INLETS_WIDTH, ypos+1);*/
}

void bng_draw_erase(t_bng* x, t_glist* glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxBUT\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void bng_draw_config(t_bng* x, t_glist* glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    sys_vGui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxBUT -fill #%6.6x\n", canvas, x,
             x->x_flashed?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
}

void bng_draw_io(t_bng* x, t_glist* glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);
    /*
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos,
        ypos + x->x_gui.iem_height-1, xpos + INLETS_WIDTH,
        ypos + x->x_gui.iem_height, x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos, ypos,
        xpos + INLETS_WIDTH, ypos+1, x, 0); */
}

void bng_draw_select(t_bng* x, t_glist* glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    if(x->x_gui.iem_isSelected)
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxBUT -outline #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_NORMAL);
        sys_vGui(".x%lx.c itemconfigure %lxBUT -outline #%6.6x\n", canvas, x, IEM_COLOR_NORMAL);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorLabel);
    }
}

void bng_draw(t_bng *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        bng_draw_update(x, glist);
    else if(mode == IEM_DRAW_MOVE)
        bng_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        bng_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        bng_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        bng_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        bng_draw_config(x, glist);
    else if(mode >= IEM_DRAW_IO)
        bng_draw_io(x, glist);
}

/* ------------------------ bng widgetbehaviour----------------------------- */

static void bng_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_bng *x = (t_bng *)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist);
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist);
    *xp2 = *xp1 + x->x_gui.iem_width;
    *yp2 = *yp1 + x->x_gui.iem_height;
}

static void bng_save(t_gobj *z, t_buffer *b)
{
    t_bng *x = (t_bng *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iemgui_serialize(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiiiisssiiiiiii", gensym("#X"),gensym("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate, (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym("bng"), x->x_gui.iem_width,
                x->x_flashHold, x->x_flashBreak,
                iemgui_serializeLoadbang(&x->x_gui),
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_serializeFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2]);
    buffer_vAppend(b, ";");
}

void bng_setFlashtime(t_bng *x, int ftbreak, int fthold)
{
    if(ftbreak > fthold)
    {
        int h;

        h = ftbreak;
        ftbreak = fthold;
        fthold = h;
    }
    if(ftbreak < IEM_BANG_MINIMUM_BREAK)
        ftbreak = IEM_BANG_MINIMUM_BREAK;
    if(fthold < IEM_BANG_MINIMUM_HOLD)
        fthold = IEM_BANG_MINIMUM_HOLD;
    x->x_flashBreak = ftbreak;
    x->x_flashHold = fthold;
}

static void bng_properties(t_gobj *z, t_glist *owner)
{
    t_bng *x = (t_bng *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_serializeNames(&x->x_gui, srl);
    sprintf(buf, "::ui_iem::create %%s Bang \
            %d %d Size 0 0 empty \
            %d {Flash Break} %d {Flash Hold} \
            -1 empty empty \
            %d \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.iem_width, IEM_BANG_MINIMUM_SIZE,
            x->x_flashBreak, x->x_flashHold,
            x->x_gui.iem_loadbang,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, 0xffffff & x->x_gui.iem_colorForeground, 0xffffff & x->x_gui.iem_colorLabel);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

static void bng_set(t_bng *x)
{
    if(x->x_flashed)
    {
        x->x_flashed = 0;
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        clock_delay(x->x_clockBreak, x->x_flashBreak);
        x->x_flashed = 1;
    }
    else
    {
        x->x_flashed = 1;
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    }
    clock_delay(x->x_clockHold, x->x_flashHold);
}

static void bng_bout1(t_bng *x)/*wird nur mehr gesendet, wenn snd != rcv*/
{
    if(!x->x_gui.iem_goThrough)
    {
        x->x_gui.iem_isLocked = 1;
        clock_delay(x->x_clockLock, 2);
    }
    outlet_bang(x->x_gui.iem_obj.te_outlet);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing && x->x_gui.iem_goThrough)
        pd_bang(x->x_gui.iem_send->s_thing);
}

static void bng_bout2(t_bng *x)/*wird immer gesendet, wenn moeglich*/
{
    if(!x->x_gui.iem_goThrough)
    {
        x->x_gui.iem_isLocked = 1;
        clock_delay(x->x_clockLock, 2);
    }
    outlet_bang(x->x_gui.iem_obj.te_outlet);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_bang(x->x_gui.iem_send->s_thing);
}

static void bng_bang(t_bng *x)/*wird nur mehr gesendet, wenn snd != rcv*/
{
    if(!x->x_gui.iem_isLocked)
    {
        bng_set(x);
        bng_bout1(x);
    }
}

static void bng_bang2(t_bng *x)/*wird immer gesendet, wenn moeglich*/
{
    if(!x->x_gui.iem_isLocked)
    {
        bng_set(x);
        bng_bout2(x);
    }
}

static void bng_dialog(t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int fthold = (int)(t_int)atom_getFloatAtIndex(2, argc, argv);
    int ftbreak = (int)(t_int)atom_getFloatAtIndex(3, argc, argv);
    iemgui_fromDialog(&x->x_gui, argc, argv);

    x->x_gui.iem_width = PD_MAX (a, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    bng_setFlashtime(x, ftbreak, fthold);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_IO);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, (t_object*)x);
}

static void bng_click(t_bng *x, t_float xpos, t_float ypos, t_float shift, t_float ctrl, t_float alt)
{
    bng_set(x);
    bng_bout2(x);
}

static int bng_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        bng_click((t_bng *)z, (t_float)xpix, (t_float)ypix, (t_float)shift, 0, (t_float)alt);
    return (1);
}

static void bng_float(t_bng *x, t_float f)
{bng_bang2(x);}

static void bng_symbol(t_bng *x, t_symbol *s)
{bng_bang2(x);}

static void bng_pointer(t_bng *x, t_gpointer *gp)
{bng_bang2(x);}

static void bng_list(t_bng *x, t_symbol *s, int ac, t_atom *av)
{
    bng_bang2(x);
}

static void bng_anything(t_bng *x, t_symbol *s, int argc, t_atom *argv)
{bng_bang2(x);}

static void bng_loadbang(t_bng *x)
{
    if(x->x_gui.iem_loadbang)
    {
        bng_set(x);
        bng_bout2(x);
    }
}

static void bng_size(t_bng *x, t_symbol *s, int ac, t_atom *av)
{
    int w = atom_getFloatAtIndex(0, ac, av);
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    iemgui_boxChanged((void *)x, &x->x_gui);
}

static void bng_delta(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void bng_pos(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void bng_flashtime(t_bng *x, t_symbol *s, int ac, t_atom *av)
{
    bng_setFlashtime(x, (int)(t_int)atom_getFloatAtIndex(0, ac, av),
                     (int)(t_int)atom_getFloatAtIndex(1, ac, av));
}

static void bng_color(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setColor((void *)x, &x->x_gui, s, ac, av);}

static void bng_send(t_bng *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void bng_receive(t_bng *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void bng_label(t_bng *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

static void bng_label_pos(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void bng_label_font(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void bng_init(t_bng *x, t_float f)
{
    x->x_gui.iem_loadbang = (f==0.0)?0:1;
}

static void bng_tick_hld(t_bng *x)
{
    x->x_flashed = 0;
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

static void bng_tick_brk(t_bng *x)
{
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

static void bng_tick_lck(t_bng *x)
{
    x->x_gui.iem_isLocked = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *bng_new (t_symbol *s, int argc, t_atom *argv)
{
    t_bng *x = (t_bng *)pd_new (bng_class);
    
    int size            = IEM_BANG_DEFAULT_SIZE;
    int flashHold       = IEM_BANG_DEFAULT_HOLD;
    int flashBreak      = IEM_BANG_DEFAULT_BREAK;
    int labelX          = IEM_BANG_DEFAULT_LABELX;
    int labelY          = IEM_BANG_DEFAULT_LABELY;
    int labelFontSize   = IEM_BANG_DEFAULT_FONTSIZE;
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
        
    bng_setFlashtime (x, flashBreak, flashHold);
    
    x->x_clockHold = clock_new (x, (t_method)bng_tick_hld);
    x->x_clockBreak = clock_new (x, (t_method)bng_tick_brk);
    x->x_clockLock = clock_new (x, (t_method)bng_tick_lck);
    
    outlet_new (cast_object (x), &s_bang);
    
    return x;
}

static void bng_ff(t_bng *x)
{
    if(x->x_gui.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    clock_free(x->x_clockLock);
    clock_free(x->x_clockBreak);
    clock_free(x->x_clockHold);
    gfxstub_deleteforkey(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bng_setup (void) 
{
    t_class *c = NULL;
    
    c = class_new (gensym ("bng"), 
            (t_newmethod)bng_new, 
            (t_method)bng_ff, 
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
    
    class_addMethod (c, (t_method)bng_dialog,       gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_loadbang,     gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)bng_size,         gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_delta,        gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_pos,          gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_flashtime,    gensym ("flashtime"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_color,        gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_send,         gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)bng_receive,      gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)bng_label,        gensym ("label"),           A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)bng_label_font,   gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_label_pos,    gensym ("label_position"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_init,         gensym ("initialize"),      A_FLOAT, A_NULL);
    
    /* Legacy names for compatibility. */
    
    class_addMethod (c, (t_method)bng_pos,          gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_label_pos,    gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)bng_init,         gensym ("init"),            A_FLOAT, A_NULL);

    bng_widgetBehavior.w_getrectfn  = bng_getrect;
    bng_widgetBehavior.w_displacefn = iemgui_behaviorDisplace;
    bng_widgetBehavior.w_selectfn   = iemgui_behaviorSelected;
    bng_widgetBehavior.w_activatefn = NULL;
    bng_widgetBehavior.w_deletefn   = iemgui_behaviorDeleted;
    bng_widgetBehavior.w_visfn      = iemgui_behaviorVisible;
    bng_widgetBehavior.w_clickfn    = bng_newclick;
    
    class_setWidgetBehavior (c, &bng_widgetBehavior);
    class_setHelpName (c, gensym ("bng"));
    class_setSaveFunction (c, bng_save);
    class_setPropertiesFunction (c, bng_properties);
    
    bng_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
