/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_7_guis.c written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 */
/* thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja */

/* name change to hradio by MSP and changed to
put out a "float" as in sliders, toggles, etc. */

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

#define IEM_HRADIO_DEFAULT_SIZE         15
#define IEM_HRADIO_MINIMUM_SIZE         8
#define IEM_HRADIO_MAXIMUM_BUTTONS      128

/* ------------- hdl     gui-horicontal dial ---------------------- */

t_widgetbehavior hradio_widgetbehavior;
static t_class *hradio_class;

/* widget helper functions */

void hradio_draw_update(t_gobj *client, t_glist *glist)
{
    t_hradio *x = (t_hradio *)client;
    if(glist_isvisible(glist))
    {
        t_canvas *canvas=glist_getcanvas(glist);

        sys_vgui(".x%lx.c itemconfigure %lxBUT%d -fill #%6.6x -outline #%6.6x\n",
                 canvas, x, x->x_drawn,
                 x->x_gui.x_bcol, x->x_gui.x_bcol);
        sys_vgui(".x%lx.c itemconfigure %lxBUT%d -fill #%6.6x -outline #%6.6x\n",
                 canvas, x, x->x_on,
                 x->x_gui.x_fcol, x->x_gui.x_fcol);
        x->x_drawn = x->x_on;
    }
}

void hradio_draw_new(t_hradio *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int n=x->x_number, i, dx=x->x_gui.x_w, s4=dx/4;
    int yy11=text_ypix(&x->x_gui.x_obj, glist), yy12=yy11+dx;
    int yy21=yy11+s4, yy22=yy12-s4;
    int xx11b=text_xpix(&x->x_gui.x_obj, glist), xx11=xx11b, xx21=xx11b+s4;
    int xx22=xx11b+dx-s4;


    for(i=0; i<n; i++)
    {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE%d\n",
                 canvas, xx11, yy11, xx11+dx, yy12,
                 x->x_gui.x_bcol, x, i);
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxBUT%d\n",
                 canvas, xx21, yy21, xx22, yy22,
                 (x->x_on==i)?x->x_gui.x_fcol:x->x_gui.x_bcol,
                 (x->x_on==i)?x->x_gui.x_fcol:x->x_gui.x_bcol, x, i);
        xx11 += dx;
        xx21 += dx;
        xx22 += dx;
        x->x_drawn = x->x_on;
    }
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xx11b+x->x_gui.x_ldx, yy11+x->x_gui.x_ldy,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"",
             x->x_gui.x_fontsize,
             x->x_gui.x_lcol, x);

    /*sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xx11b, yy12-1, xx11b + INLETS_WIDTH, yy12, x, 0);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xx11b, yy11, xx11b + INLETS_WIDTH, yy11+1, x, 0);*/

}

void hradio_draw_move(t_hradio *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int n=x->x_number, i, dx=x->x_gui.x_w, s4=dx/4;
    int yy11=text_ypix(&x->x_gui.x_obj, glist), yy12=yy11+dx;
    int yy21=yy11+s4, yy22=yy12-s4;
    int xx11b=text_xpix(&x->x_gui.x_obj, glist), xx11=xx11b, xx21=xx11b+s4;
    int xx22=xx11b+dx-s4;

    xx11 = xx11b;
    xx21=xx11b+s4;
    xx22=xx11b+dx-s4;
    for(i=0; i<n; i++)
    {
        sys_vgui(".x%lx.c coords %lxBASE%d %d %d %d %d\n",
                 canvas, x, i, xx11, yy11, xx11+dx, yy12);
        sys_vgui(".x%lx.c coords %lxBUT%d %d %d %d %d\n",
                 canvas, x, i, xx21, yy21, xx22, yy22);
        xx11 += dx;
        xx21 += dx;
        xx22 += dx;
    }
    sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xx11b+x->x_gui.x_ldx, yy11+x->x_gui.x_ldy);
    /*sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0, xx11b, yy12-1, xx11b + INLETS_WIDTH, yy12);
    sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0, xx11b, yy11, xx11b + INLETS_WIDTH, yy11+1);*/
}

void hradio_draw_erase(t_hradio* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int n=x->x_number, i;

    for(i=0; i<n; i++)
    {
        sys_vgui(".x%lx.c delete %lxBASE%d\n", canvas, x, i);
        sys_vgui(".x%lx.c delete %lxBUT%d\n", canvas, x, i);
    }
    sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void hradio_draw_config(t_hradio* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int n=x->x_number, i;

    sys_vgui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.x_fontsize,
             x->x_gui.x_fsf.x_selected?IEM_COLOR_SELECTED:x->x_gui.x_lcol,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"");
    for(i=0; i<n; i++)
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE%d -fill #%6.6x\n", canvas, x, i,
                 x->x_gui.x_bcol);
        sys_vgui(".x%lx.c itemconfigure %lxBUT%d -fill #%6.6x -outline #%6.6x\n", canvas, x, i,
                 (x->x_on==i)?x->x_gui.x_fcol:x->x_gui.x_bcol,
                 (x->x_on==i)?x->x_gui.x_fcol:x->x_gui.x_bcol);
    }
}

void hradio_draw_io(t_hradio* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);

    /*sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas,
        xpos, ypos + x->x_gui.x_w-1,
        xpos + INLETS_WIDTH, ypos + x->x_gui.x_w,
        x, 0);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas,
        xpos, ypos,
        xpos + INLETS_WIDTH, ypos+1, x, 0);*/
}

void hradio_draw_select(t_hradio* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int n=x->x_number, i;

    if(x->x_gui.x_fsf.x_selected)
    {
        for(i=0; i<n; i++)
        {
            sys_vgui(".x%lx.c itemconfigure %lxBASE%d -outline #%6.6x\n", canvas, x, i,
                     IEM_COLOR_SELECTED);
        }
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        for(i=0; i<n; i++)
        {
            sys_vgui(".x%lx.c itemconfigure %lxBASE%d -outline #%6.6x\n", canvas, x, i,
                     IEM_COLOR_NORMAL);
        }
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x,
                 x->x_gui.x_lcol);
    }
}

void hradio_draw(t_hradio *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        sys_queuegui(x, glist, hradio_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        hradio_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        hradio_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        hradio_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        hradio_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        hradio_draw_config(x, glist);
    else if(mode >= IEM_DRAW_IO)
        hradio_draw_io(x, glist);
}

/* ------------------------ hdl widgetbehaviour----------------------------- */

static void hradio_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_hradio *x = (t_hradio *)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w*x->x_number;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void hradio_save(t_gobj *z, t_buffer *b)
{
    t_hradio *x = (t_hradio *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iem_save(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiiiisssiiiiiiif", gensym("#X"),gensym("obj"),
                (int)x->x_gui.x_obj.te_xCoordinate, (int)x->x_gui.x_obj.te_yCoordinate,
                gensym("hradio"),
                x->x_gui.x_w,
                x->x_change, iem_symargstoint(&x->x_gui.x_isa), x->x_number,
                srl[0], srl[1], srl[2],
                x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[1], bflcol[2], x->x_fval);
    buffer_vAppend(b, ";");
}

static void hradio_properties(t_gobj *z, t_glist *owner)
{
    t_hradio *x = (t_hradio *)z;
    char buf[800];
    t_symbol *srl[3];

    iem_properties(&x->x_gui, srl);

    sprintf(buf, "::ui_iem::create %%s {Radio Button} \
            %d %d Size 0 0 empty \
            0 empty 0 empty \
            -1 empty empty \
            %d \
            %d 256 {Number Of Buttons} \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.x_w, IEM_HRADIO_MINIMUM_SIZE,
            x->x_gui.x_isa.x_loadinit,
            x->x_number,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol, 0xffffff & x->x_gui.x_lcol);
    gfxstub_new(&x->x_gui.x_obj.te_g.g_pd, x, buf);
}

static void hradio_dialog(t_hradio *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *srl[3];
    int a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int chg = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
    int num = (int)(t_int)atom_getFloatAtIndex(6, argc, argv);

    if(chg != 0) chg = 1;
    x->x_change = chg;
    iem_dialog(&x->x_gui, srl, argc, argv);
    x->x_gui.x_w = iem_clip_size(a);
    x->x_gui.x_h = x->x_gui.x_w;
    if(x->x_number != num)
    {
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_ERASE);
        x->x_number = num;
        if(x->x_on >= x->x_number)
        {
            x->x_on = x->x_number - 1;
        }
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_NEW);
    }
    else
    {
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_CONFIG);
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_IO);
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_MOVE);
        canvas_fixlines(x->x_gui.x_glist, (t_text*)x);
    }

}

static void hradio_set(t_hradio *x, t_float f)
{
    int i=(int)f;

    x->x_fval = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_number)
        i = x->x_number-1;

    x->x_on = i;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);

}

static void hradio_bang(t_hradio *x)
{
        float outval = (0 ? x->x_on : x->x_fval);
        outlet_float(x->x_gui.x_obj.te_outlet, outval);
        if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
            pd_float(x->x_gui.x_snd->s_thing, outval);
}

static void hradio_fout(t_hradio *x, t_float f)
{
    int i=(int)f;

    x->x_fval = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_number)
        i = x->x_number-1;

        float outval = (0 ? i : x->x_fval);
        x->x_on = i;
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
        outlet_float(x->x_gui.x_obj.te_outlet, outval);
        if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
            pd_float(x->x_gui.x_snd->s_thing, outval);
}

static void hradio_float(t_hradio *x, t_float f)
{
    int i=(int)f;
    x->x_fval = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_number)
        i = x->x_number-1;

        float outval = (0 ? i : x->x_fval);
        x->x_on = i;
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
        if (x->x_gui.x_fsf.x_put_in2out)
        {
            outlet_float(x->x_gui.x_obj.te_outlet, outval);
            if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
                pd_float(x->x_gui.x_snd->s_thing, outval);
        }
}

static void hradio_click(t_hradio *x, t_float xpos, t_float ypos, t_float shift, t_float ctrl, t_float alt)
{
    int xx = (int)xpos - (int)text_xpix(&x->x_gui.x_obj, x->x_gui.x_glist);

    hradio_fout(x, (t_float)(xx / x->x_gui.x_w));
}

static int hradio_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        hradio_click((t_hradio *)z, (t_float)xpix, (t_float)ypix, (t_float)shift, 0, (t_float)alt);
    return (1);
}

static void hradio_loadbang(t_hradio *x)
{
    if(x->x_gui.x_isa.x_loadinit)
        hradio_bang(x);
}

static void hradio_number(t_hradio *x, t_float num)
{
    int n=(int)num;

    if(n < 1)
        n = 1;
    if(n > IEM_HRADIO_MAXIMUM_BUTTONS)
        n = IEM_HRADIO_MAXIMUM_BUTTONS;
    if(n != x->x_number)
    {
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_ERASE);
        x->x_number = n;
        if(x->x_on >= x->x_number)
            x->x_on = x->x_number - 1;
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_NEW);
    }
}

static void hradio_size(t_hradio *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_gui.x_w = iem_clip_size((int)(t_int)atom_getFloatAtIndex(0, ac, av));
    x->x_gui.x_h = x->x_gui.x_w;
    iem_size((void *)x, &x->x_gui);
}

static void hradio_delta(t_hradio *x, t_symbol *s, int ac, t_atom *av)
{iem_delta((void *)x, &x->x_gui, s, ac, av);}

static void hradio_pos(t_hradio *x, t_symbol *s, int ac, t_atom *av)
{iem_pos((void *)x, &x->x_gui, s, ac, av);}

static void hradio_color(t_hradio *x, t_symbol *s, int ac, t_atom *av)
{iem_color((void *)x, &x->x_gui, s, ac, av);}

static void hradio_send(t_hradio *x, t_symbol *s)
{iem_send(x, &x->x_gui, s);}

static void hradio_receive(t_hradio *x, t_symbol *s)
{iem_receive(x, &x->x_gui, s);}

static void hradio_label(t_hradio *x, t_symbol *s)
{iem_label((void *)x, &x->x_gui, s);}

static void hradio_label_pos(t_hradio *x, t_symbol *s, int ac, t_atom *av)
{iem_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void hradio_label_font(t_hradio *x, t_symbol *s, int ac, t_atom *av)
{iem_label_font((void *)x, &x->x_gui, s, ac, av);}

static void hradio_init(t_hradio *x, t_float f)
{
    x->x_gui.x_isa.x_loadinit = (f==0.0)?0:1;
}

static void hradio_double_change(t_hradio *x)
{x->x_change = 1;}

static void hradio_single_change(t_hradio *x)
{x->x_change = 0;}

static void *hradio_donew(t_symbol *s, int argc, t_atom *argv)
{
    t_hradio *x = (t_hradio *)pd_new(hradio_class);
    int bflcol[]={-262144, -1, -1};
    int a=IEM_HRADIO_DEFAULT_SIZE, on = 0, f=0;
    int ldx=0, ldy=-8, chg=1, num=8;
    int fs=10;
    //int ftbreak=IEM_BANG_DEFAULT_BREAK, fthold=IEM_BANG_DEFAULT_HOLD;
    char str[144];
    float fval = 0;

    iem_inttosymargs(&x->x_gui.x_isa, 0);
    iem_inttofstyle(&x->x_gui.x_fsf, 0);

    if((argc == 15)&&IS_FLOAT(argv,0)&&IS_FLOAT(argv,1)&&IS_FLOAT(argv,2)
       &&IS_FLOAT(argv,3)
       &&(IS_SYMBOL(argv,4)||IS_FLOAT(argv,4))
       &&(IS_SYMBOL(argv,5)||IS_FLOAT(argv,5))
       &&(IS_SYMBOL(argv,6)||IS_FLOAT(argv,6))
       &&IS_FLOAT(argv,7)&&IS_FLOAT(argv,8)
       &&IS_FLOAT(argv,9)&&IS_FLOAT(argv,10)&&IS_FLOAT(argv,11)
       &&IS_FLOAT(argv,12)&&IS_FLOAT(argv,13)&&IS_FLOAT(argv,14))
    {
        a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        chg = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, (t_int)atom_getFloatAtIndex(2, argc, argv));
        num = (int)(t_int)atom_getFloatAtIndex(3, argc, argv);
        iem_new_getnames(&x->x_gui, 4, argv);
        ldx = (int)(t_int)atom_getFloatAtIndex(7, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(8, argc, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, (t_int)atom_getFloatAtIndex(9, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(11, argc, argv);
        bflcol[1] = (int)(t_int)atom_getFloatAtIndex(12, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(13, argc, argv);
        fval = atom_getFloatAtIndex(14, argc, argv);
    }
    else iem_new_getnames(&x->x_gui, 4, 0);
    x->x_gui.x_draw = (t_iemfunptr)hradio_draw;
    x->x_gui.x_fsf.x_snd_able = 1;
    x->x_gui.x_fsf.x_rcv_able = 1;
    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.x_snd->s_name, "empty"))
        x->x_gui.x_fsf.x_snd_able = 0;
    if (!strcmp(x->x_gui.x_rcv->s_name, "empty"))
        x->x_gui.x_fsf.x_rcv_able = 0;

    if(num < 1)
        num = 1;
    if(num > IEM_HRADIO_MAXIMUM_BUTTONS)
        num = IEM_HRADIO_MAXIMUM_BUTTONS;
    x->x_number = num;
    x->x_fval = fval;
    on = fval;
    if(on < 0)
        on = 0;
    if(on >= x->x_number)
        on = x->x_number - 1;
    if(x->x_gui.x_isa.x_loadinit)
        x->x_on = on;
    else
        x->x_on = 0;
    x->x_change = (chg==0)?0:1;
    if (x->x_gui.x_fsf.x_rcv_able)
        pd_bind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.x_fontsize = fs;
    x->x_gui.x_w = iem_clip_size(a);
    x->x_gui.x_h = x->x_gui.x_w;
    iem_verify_snd_ne_rcv(&x->x_gui);
    iem_all_colfromload(&x->x_gui, bflcol);
    outlet_new(&x->x_gui.x_obj, &s_list);
    return (x);
}

static void *hradio_new(t_symbol *s, int argc, t_atom *argv)
{
    return (hradio_donew(s, argc, argv));
}

static void hradio_ff(t_hradio *x)
{
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_unbind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    gfxstub_deleteforkey(x);
}

void g_hradio_setup(void)
{
    hradio_class = class_new(gensym("hradio"), (t_newmethod)hradio_new,
        (t_method)hradio_ff, sizeof(t_hradio), 0, A_GIMME, 0);
    class_addBang(hradio_class, hradio_bang);
    class_addFloat(hradio_class, hradio_float);
    class_addMethod(hradio_class, (t_method)hradio_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(hradio_class, (t_method)hradio_dialog, gensym("dialog"),
                    A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_loadbang,
        gensym("loadbang"), 0);
    class_addMethod(hradio_class, (t_method)hradio_set,
        gensym("set"), A_FLOAT, 0);
    class_addMethod(hradio_class, (t_method)hradio_size,
        gensym("size"), A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_delta,
        gensym("delta"), A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_pos,
        gensym("pos"), A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_color,
        gensym("color"), A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_send,
        gensym("send"), A_DEFSYMBOL, 0);
    class_addMethod(hradio_class, (t_method)hradio_receive,
        gensym("receive"), A_DEFSYMBOL, 0);
    class_addMethod(hradio_class, (t_method)hradio_label,
        gensym("label"), A_DEFSYMBOL, 0);
    class_addMethod(hradio_class, (t_method)hradio_label_pos,
        gensym("label_pos"), A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_label_font,
        gensym("label_font"), A_GIMME, 0);
    class_addMethod(hradio_class, (t_method)hradio_init,
        gensym("init"), A_FLOAT, 0);
    class_addMethod(hradio_class, (t_method)hradio_number,
        gensym("number"), A_FLOAT, 0);
    class_addMethod(hradio_class, (t_method)hradio_single_change,
        gensym("single_change"), 0);
    class_addMethod(hradio_class, (t_method)hradio_double_change,
        gensym("double_change"), 0);
    hradio_widgetbehavior.w_getrectfn = hradio_getrect;
    hradio_widgetbehavior.w_displacefn = iem_displace;
    hradio_widgetbehavior.w_selectfn = iem_select;
    hradio_widgetbehavior.w_activatefn = NULL;
    hradio_widgetbehavior.w_deletefn = iem_delete;
    hradio_widgetbehavior.w_visfn = iem_vis;
    hradio_widgetbehavior.w_clickfn = hradio_newclick;
    class_setWidget(hradio_class, &hradio_widgetbehavior);
    class_setHelpName(hradio_class, gensym("hradio"));
    class_setSaveFunction(hradio_class, hradio_save);
    class_setPropertiesFunction(hradio_class, hradio_properties);
}
