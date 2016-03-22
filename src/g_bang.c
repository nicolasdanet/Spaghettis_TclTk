/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_7_guis.c written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 */
/* thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

#include "g_iem.h"
#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define IEM_BANG_DEFAULT_SIZE       15
#define IEM_BANG_MINIMUM_SIZE       8
#define IEM_BANG_DEFAULT_HOLD       250
#define IEM_BANG_DEFAULT_BREAK      50
#define IEM_BANG_MINIMUM_HOLD       10
#define IEM_BANG_MINIMUM_BREAK      10

/* --------------- bng     gui-bang ------------------------- */

t_widgetbehavior bng_widgetbehavior;
static t_class *bng_class;

/*  widget helper functions  */


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
    t_canvas *canvas=glist_getcanvas(glist);

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
    t_canvas *canvas=glist_getcanvas(glist);

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
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxBUT\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void bng_draw_config(t_bng* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_flags.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    sys_vGui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxBUT -fill #%6.6x\n", canvas, x,
             x->x_flashed?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
}

void bng_draw_io(t_bng* x, t_glist* glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);
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
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_gui.iem_flags.iem_isSelected)
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

    iem_save(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiiiisssiiiiiii", gensym("#X"),gensym("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate, (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym("bng"), x->x_gui.iem_width,
                x->x_flashtime_hold, x->x_flashtime_break,
                iem_symargstoint(&x->x_gui.x_isa),
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iem_fstyletoint(&x->x_gui.iem_flags), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2]);
    buffer_vAppend(b, ";");
}

void bng_check_minmax(t_bng *x, int ftbreak, int fthold)
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
    x->x_flashtime_break = ftbreak;
    x->x_flashtime_hold = fthold;
}

static void bng_properties(t_gobj *z, t_glist *owner)
{
    t_bng *x = (t_bng *)z;
    char buf[800];
    t_symbol *srl[3];

    iem_properties(&x->x_gui, srl);
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
            x->x_flashtime_break, x->x_flashtime_hold,
            x->x_gui.x_isa.iem_loadOnStart,
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
        clock_delay(x->x_clock_brk, x->x_flashtime_break);
        x->x_flashed = 1;
    }
    else
    {
        x->x_flashed = 1;
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    }
    clock_delay(x->x_clock_hld, x->x_flashtime_hold);
}

static void bng_bout1(t_bng *x)/*wird nur mehr gesendet, wenn snd != rcv*/
{
    if(!x->x_gui.iem_flags.iem_goThrough)
    {
        x->x_gui.x_isa.iem_isLocked = 1;
        clock_delay(x->x_clock_lck, 2);
    }
    outlet_bang(x->x_gui.iem_obj.te_outlet);
    if(x->x_gui.iem_flags.iem_canSend && x->x_gui.iem_send->s_thing && x->x_gui.iem_flags.iem_goThrough)
        pd_bang(x->x_gui.iem_send->s_thing);
}

static void bng_bout2(t_bng *x)/*wird immer gesendet, wenn moeglich*/
{
    if(!x->x_gui.iem_flags.iem_goThrough)
    {
        x->x_gui.x_isa.iem_isLocked = 1;
        clock_delay(x->x_clock_lck, 2);
    }
    outlet_bang(x->x_gui.iem_obj.te_outlet);
    if(x->x_gui.iem_flags.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_bang(x->x_gui.iem_send->s_thing);
}

static void bng_bang(t_bng *x)/*wird nur mehr gesendet, wenn snd != rcv*/
{
    if(!x->x_gui.x_isa.iem_isLocked)
    {
        bng_set(x);
        bng_bout1(x);
    }
}

static void bng_bang2(t_bng *x)/*wird immer gesendet, wenn moeglich*/
{
    if(!x->x_gui.x_isa.iem_isLocked)
    {
        bng_set(x);
        bng_bout2(x);
    }
}

static void bng_dialog(t_bng *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *srl[3];
    int a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int fthold = (int)(t_int)atom_getFloatAtIndex(2, argc, argv);
    int ftbreak = (int)(t_int)atom_getFloatAtIndex(3, argc, argv);
    iem_dialog(&x->x_gui, srl, argc, argv);

    x->x_gui.iem_width = PD_MAX (a, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    bng_check_minmax(x, ftbreak, fthold);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_IO);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, (t_text*)x);
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
    if(x->x_gui.x_isa.iem_loadOnStart)
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
    iem_size((void *)x, &x->x_gui);
}

static void bng_delta(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iem_delta((void *)x, &x->x_gui, s, ac, av);}

static void bng_pos(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iem_pos((void *)x, &x->x_gui, s, ac, av);}

static void bng_flashtime(t_bng *x, t_symbol *s, int ac, t_atom *av)
{
    bng_check_minmax(x, (int)(t_int)atom_getFloatAtIndex(0, ac, av),
                     (int)(t_int)atom_getFloatAtIndex(1, ac, av));
}

static void bng_color(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iem_color((void *)x, &x->x_gui, s, ac, av);}

static void bng_send(t_bng *x, t_symbol *s)
{iem_send(x, &x->x_gui, s);}

static void bng_receive(t_bng *x, t_symbol *s)
{iem_receive(x, &x->x_gui, s);}

static void bng_label(t_bng *x, t_symbol *s)
{iem_label((void *)x, &x->x_gui, s);}

static void bng_label_pos(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iem_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void bng_label_font(t_bng *x, t_symbol *s, int ac, t_atom *av)
{iem_label_font((void *)x, &x->x_gui, s, ac, av);}

static void bng_init(t_bng *x, t_float f)
{
    x->x_gui.x_isa.iem_loadOnStart = (f==0.0)?0:1;
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
    x->x_gui.x_isa.iem_isLocked = 0;
}

static void *bng_new(t_symbol *s, int argc, t_atom *argv)
{
    t_bng *x = (t_bng *)pd_new(bng_class);
    int bflcol[]={-262144, -1, -1};
    int a=IEM_BANG_DEFAULT_SIZE;
    int ldx=17, ldy=7;
    int fs=10;
    int ftbreak=IEM_BANG_DEFAULT_BREAK,
        fthold=IEM_BANG_DEFAULT_HOLD;
    char str[144];

    iem_inttosymargs(&x->x_gui.x_isa, 0);
    iem_inttofstyle(&x->x_gui.iem_flags, 0);

    if((argc == 14)&&IS_FLOAT_AT(argv,0)
       &&IS_FLOAT_AT(argv,1)&&IS_FLOAT_AT(argv,2)
       &&IS_FLOAT_AT(argv,3)
       &&(IS_SYMBOL_AT(argv,4)||IS_FLOAT_AT(argv,4))
       &&(IS_SYMBOL_AT(argv,5)||IS_FLOAT_AT(argv,5))
       &&(IS_SYMBOL_AT(argv,6)||IS_FLOAT_AT(argv,6))
       &&IS_FLOAT_AT(argv,7)&&IS_FLOAT_AT(argv,8)
       &&IS_FLOAT_AT(argv,9)&&IS_FLOAT_AT(argv,10)&&IS_FLOAT_AT(argv,11)
       &&IS_FLOAT_AT(argv,12)&&IS_FLOAT_AT(argv,13))
    {

        a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        fthold = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        ftbreak = (int)(t_int)atom_getFloatAtIndex(2, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, (t_int)atom_getFloatAtIndex(3, argc, argv));
        iem_setNamesByIndex(&x->x_gui, 4, argv);
        ldx = (int)(t_int)atom_getFloatAtIndex(7, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(8, argc, argv);
        iem_inttofstyle(&x->x_gui.iem_flags, (t_int)atom_getFloatAtIndex(9, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(11, argc, argv);
        bflcol[1] = (int)(t_int)atom_getFloatAtIndex(12, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(13, argc, argv);
    }
    else iem_setNamesByIndex(&x->x_gui, 4, 0);

    x->x_gui.iem_draw = (t_iemfn)bng_draw;

    x->x_gui.iem_flags.iem_canSend = 1;
    x->x_gui.iem_flags.iem_canReceive = 1;
    x->x_flashed = 0;
    x->x_gui.iem_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.iem_send->s_name, "empty"))
        x->x_gui.iem_flags.iem_canSend = 0;
    if (!strcmp(x->x_gui.iem_receive->s_name, "empty"))
        x->x_gui.iem_flags.iem_canReceive = 0;

    if (x->x_gui.iem_flags.iem_canReceive)
        pd_bind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    x->x_gui.iem_labelX = ldx;
    x->x_gui.iem_labelY = ldy;

    if(fs < 4)
        fs = 4;
    x->x_gui.iem_fontSize = fs;
    x->x_gui.iem_width = PD_MAX (a, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    bng_check_minmax(x, ftbreak, fthold);
    iem_setColors(&x->x_gui, bflcol);
    x->x_gui.x_isa.iem_isLocked = 0;
    iem_checkSendReceiveLoop(&x->x_gui);
    x->x_clock_hld = clock_new(x, (t_method)bng_tick_hld);
    x->x_clock_brk = clock_new(x, (t_method)bng_tick_brk);
    x->x_clock_lck = clock_new(x, (t_method)bng_tick_lck);
    outlet_new(&x->x_gui.iem_obj, &s_bang);
    return (x);
}

static void bng_ff(t_bng *x)
{
    if(x->x_gui.iem_flags.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    clock_free(x->x_clock_lck);
    clock_free(x->x_clock_brk);
    clock_free(x->x_clock_hld);
    gfxstub_deleteforkey(x);
}

void g_bang_setup(void)
{
    bng_class = class_new(gensym("bng"), (t_newmethod)bng_new,
                          (t_method)bng_ff, sizeof(t_bng), 0, A_GIMME, 0);
    class_addBang(bng_class, bng_bang);
    class_addFloat(bng_class, bng_float);
    class_addSymbol(bng_class, bng_symbol);
    class_addPointer(bng_class, bng_pointer);
    class_addList(bng_class, bng_list);
    class_addAnything(bng_class, bng_anything);
    class_addMethod(bng_class, (t_method)bng_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(bng_class, (t_method)bng_dialog, gensym("dialog"),
                    A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_loadbang, gensym("loadbang"), 0);
    class_addMethod(bng_class, (t_method)bng_size, gensym("size"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_delta, gensym("delta"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_pos, gensym("pos"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_flashtime, gensym("flashtime"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_color, gensym("color"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_send, gensym("send"), A_DEFSYMBOL, 0);
    class_addMethod(bng_class, (t_method)bng_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addMethod(bng_class, (t_method)bng_label, gensym("label"), A_DEFSYMBOL, 0);
    class_addMethod(bng_class, (t_method)bng_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_label_font, gensym("label_font"), A_GIMME, 0);
    class_addMethod(bng_class, (t_method)bng_init, gensym("init"), A_FLOAT, 0);
    bng_widgetbehavior.w_getrectfn = bng_getrect;
    bng_widgetbehavior.w_displacefn = iem_displace;
    bng_widgetbehavior.w_selectfn = iem_select;
    bng_widgetbehavior.w_activatefn = NULL;
    bng_widgetbehavior.w_deletefn = iem_delete;
    bng_widgetbehavior.w_visfn = iem_vis;
    bng_widgetbehavior.w_clickfn = bng_newclick;
    class_setWidget(bng_class, &bng_widgetbehavior);
    class_setHelpName(bng_class, gensym("bng"));
    class_setSaveFunction(bng_class, bng_save);
    class_setPropertiesFunction(bng_class, bng_properties);
}
