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
#include "g_canvas.h"

#include "g_iem.h"
#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif


#define IEM_HSLIDER_DEFAULT_WIDTH    128
#define IEM_HSLIDER_DEFAULT_HEIGHT   15
#define IEM_HSLIDER_MINIMUM_WIDTH    8
#define IEM_HSLIDER_MINIMUM_HEIGHT   8

/* ------------ hsl    gui-horicontal  slider ----------------------- */

t_widgetbehavior hslider_widgetbehavior;
static t_class *hslider_class;

extern int sys_noloadbang;

/* widget helper functions */

static void hslider_draw_update(t_gobj *client, t_glist *glist)
{
    t_hslider *x = (t_hslider *)client;
    if (glist_isvisible(glist))
    {
        int r = text_xpix(&x->x_gui.x_obj, glist) + (x->x_val + 50)/100;
        int ypos=text_ypix(&x->x_gui.x_obj, glist);
        t_canvas *canvas=glist_getcanvas(glist);
        sys_vgui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
                 canvas, x, r, ypos+1,
                 r, ypos + x->x_gui.x_h);
    }
}

static void hslider_draw_new(t_hslider *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    int r = xpos + (x->x_val + 50)/100;
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xpos-3, ypos,
             xpos + x->x_gui.x_w+2, ypos + x->x_gui.x_h,
             x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create line %d %d %d %d -width 3 -fill #%6.6x -tags %lxKNOB\n",
             canvas, r, ypos+1, r,
             ypos + x->x_gui.x_h, x->x_gui.x_fcol, x);
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xpos+x->x_gui.x_ldx,
             ypos+x->x_gui.x_ldy,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"",
             x->x_gui.x_fontsize,
             x->x_gui.x_lcol, x);

        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xpos-3, ypos + x->x_gui.x_h-1,
             xpos+4, ypos + x->x_gui.x_h, x, 0);

        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xpos-3, ypos,
             xpos+4, ypos+1, x, 0);
}

static void hslider_draw_move(t_hslider *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    int r = xpos + (x->x_val + 50)/100;
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x,
             xpos-3, ypos,
             xpos + x->x_gui.x_w+2, ypos + x->x_gui.x_h);
    sys_vgui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
             canvas, x, r, ypos+1,
             r, ypos + x->x_gui.x_h);
    sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.x_ldx, ypos+x->x_gui.x_ldy);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos-3, ypos + x->x_gui.x_h-1,
             xpos+4, ypos + x->x_gui.x_h);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos-3, ypos,
             xpos+4, ypos+1);
}

static void hslider_draw_erase(t_hslider* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxKNOB\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void hslider_draw_config(t_hslider* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.x_fontsize,
             x->x_gui.x_fsf.x_selected?IEM_COLOR_SELECTED:x->x_gui.x_lcol,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"");
    sys_vgui(".x%lx.c itemconfigure %lxKNOB -fill #%6.6x\n", canvas, x, x->x_gui.x_fcol);
    sys_vgui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x, x->x_gui.x_bcol);
}

static void hslider_draw_io(t_hslider* x, t_glist* glist)
{
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos-3, ypos + x->x_gui.x_h-1,
        xpos+4, ypos + x->x_gui.x_h, x, 0);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos-3, ypos,
        xpos+4, ypos+1, x, 0);
}

static void hslider_draw_select(t_hslider* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_gui.x_fsf.x_selected)
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vgui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_NORMAL);
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.x_lcol);
    }
}

void hslider_draw(t_hslider *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        sys_queuegui(x, glist, hslider_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        hslider_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        hslider_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        hslider_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        hslider_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        hslider_draw_config(x, glist);
    else if(mode >= IEM_DRAW_IO)
        hslider_draw_io(x, glist);
}

/* ------------------------ hsl widgetbehaviour----------------------------- */


static void hslider_getrect(t_gobj *z, t_glist *glist,
                            int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_hslider* x = (t_hslider*)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist) - 3;
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w + 5;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void hslider_save(t_gobj *z, t_binbuf *b)
{
    t_hslider *x = (t_hslider *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iem_save(&x->x_gui, srl, bflcol);
    binbuf_addv(b, "ssiisiiffiisssiiiiiiiii", gensym("#X"),gensym("obj"),
                (int)x->x_gui.x_obj.te_xpix, (int)x->x_gui.x_obj.te_ypix,
                gensym("hsl"), x->x_gui.x_w, x->x_gui.x_h,
                (t_float)x->x_min, (t_float)x->x_max,
                x->x_lin0_log1, iem_symargstoint(&x->x_gui.x_isa),
                srl[0], srl[1], srl[2],
                x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[1], bflcol[2],
                x->x_val, x->x_steady);
    binbuf_addv(b, ";");
}

void hslider_check_width(t_hslider *x, int w)
{
    if(w < IEM_HSLIDER_MINIMUM_WIDTH)
        w = IEM_HSLIDER_MINIMUM_WIDTH;
    x->x_gui.x_w = w;
    if(x->x_val > (x->x_gui.x_w*100 - 100))
    {
        x->x_pos = x->x_gui.x_w*100 - 100;
        x->x_val = x->x_pos;
    }
    if(x->x_lin0_log1)
        x->x_k = log(x->x_max/x->x_min)/(double)(x->x_gui.x_w - 1);
    else
        x->x_k = (x->x_max - x->x_min)/(double)(x->x_gui.x_w - 1);
}

void hslider_check_minmax(t_hslider *x, double min, double max)
{
    if(x->x_lin0_log1)
    {
        if((min == 0.0)&&(max == 0.0))
            max = 1.0;
        if(max > 0.0)
        {
            if(min <= 0.0)
                min = 0.01*max;
        }
        else
        {
            if(min > 0.0)
                max = 0.01*min;
        }
    }
    x->x_min = min;
    x->x_max = max;
    if(x->x_lin0_log1)
        x->x_k = log(x->x_max/x->x_min)/(double)(x->x_gui.x_w - 1);
    else
        x->x_k = (x->x_max - x->x_min)/(double)(x->x_gui.x_w - 1);
}

static void hslider_properties(t_gobj *z, t_glist *owner)
{
    t_hslider *x = (t_hslider *)z;
    char buf[800];
    t_symbol *srl[3];

    iem_properties(&x->x_gui, srl);
    sprintf(buf, "::pd_iem::create %%s Slider \
            %d %d {Slider Width} %d %d {Slider Height} \
            %g {Value Left} %g {Value Right} \
            %d Linear Logarithmic \
            %d \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            %d\n",
            x->x_gui.x_w, IEM_HSLIDER_MINIMUM_WIDTH, x->x_gui.x_h, IEM_HSLIDER_MINIMUM_HEIGHT,
            x->x_min, x->x_max,
            x->x_lin0_log1, 
            x->x_gui.x_isa.x_loadinit,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol, 0xffffff & x->x_gui.x_lcol,
            x->x_steady);
    gfxstub_new(&x->x_gui.x_obj.te_g.g_pd, x, buf);
}

static void hslider_set(t_hslider *x, t_floatarg f)    /* bugfix */
{
    int old = x->x_val;
    double g;

    x->x_fval = f;
    if (x->x_min > x->x_max)
    {
        if(f > x->x_min)
            f = x->x_min;
        if(f < x->x_max)
            f = x->x_max;
    }
    else
    {
        if(f > x->x_max)
            f = x->x_max;
        if(f < x->x_min)
            f = x->x_min;
    }
    if(x->x_lin0_log1)
        g = log(f/x->x_min)/x->x_k;
    else
        g = (f - x->x_min) / x->x_k;
    x->x_val = (int)(100.0*g + 0.49999);
    x->x_pos = x->x_val;
    if(x->x_val != old)
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
}

    /* compute numeric value (fval) from pixel location (val) and range */
static t_float hslider_getfval(t_hslider *x)
{
    t_float fval;
    if (x->x_lin0_log1)
        fval = x->x_min*exp(x->x_k*(double)(x->x_val)*0.01);
    else fval = (double)(x->x_val)*0.01*x->x_k + x->x_min;
    if ((fval < 1.0e-10) && (fval > -1.0e-10))
        fval = 0.0;
    return (fval);
}

static void hslider_bang(t_hslider *x)
{
    double out;

    if (PD_COMPATIBILITY < 46)
        out = hslider_getfval(x);
    else out = x->x_fval;
    outlet_float(x->x_gui.x_obj.te_outlet, out);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, out);
}

static void hslider_dialog(t_hslider *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *srl[3];
    int w = (int)atom_getintarg(0, argc, argv);
    int h = (int)atom_getintarg(1, argc, argv);
    double min = (double)atom_getfloatarg(2, argc, argv);
    double max = (double)atom_getfloatarg(3, argc, argv);
    int lilo = (int)atom_getintarg(4, argc, argv);
    int steady = (int)atom_getintarg(16, argc, argv);

    if(lilo != 0) lilo = 1;
    x->x_lin0_log1 = lilo;
    if(steady)
        x->x_steady = 1;
    else
        x->x_steady = 0;
    iem_dialog(&x->x_gui, srl, argc, argv);
    x->x_gui.x_h = iem_clip_size(h);
    hslider_check_width(x, w);
    hslider_check_minmax(x, min, max);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_IO);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.x_glist, (t_text*)x);
}

static void hslider_motion(t_hslider *x, t_floatarg dx, t_floatarg dy)
{
    int old = x->x_val;

    if(x->x_gui.x_fsf.x_finemoved)
        x->x_pos += (int)dx;
    else
        x->x_pos += 100*(int)dx;
    x->x_val = x->x_pos;
    if(x->x_val > (100*x->x_gui.x_w - 100))
    {
        x->x_val = 100*x->x_gui.x_w - 100;
        x->x_pos += 50;
        x->x_pos -= x->x_pos%100;
    }
    if(x->x_val < 0)
    {
        x->x_val = 0;
        x->x_pos -= 50;
        x->x_pos -= x->x_pos%100;
    }
    x->x_fval = hslider_getfval(x);
    if (old != x->x_val)
    {
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
        hslider_bang(x);
    }
}

static void hslider_click(t_hslider *x, t_floatarg xpos, t_floatarg ypos,
                          t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    if(!x->x_steady)
        x->x_val = (int)(100.0 * (xpos - text_xpix(&x->x_gui.x_obj, x->x_gui.x_glist)));
    if(x->x_val > (100*x->x_gui.x_w - 100))
        x->x_val = 100*x->x_gui.x_w - 100;
    if(x->x_val < 0)
        x->x_val = 0;
    x->x_fval = hslider_getfval(x);
    x->x_pos = x->x_val;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
    hslider_bang(x);
    glist_grab(x->x_gui.x_glist, &x->x_gui.x_obj.te_g, (t_glistmotionfn)hslider_motion,
               0, xpos, ypos);
}

static int hslider_newclick(t_gobj *z, struct _glist *glist,
                            int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_hslider* x = (t_hslider *)z;

    if(doit)
    {
        hslider_click( x, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift,
                       0, (t_floatarg)alt);
        if(shift)
            x->x_gui.x_fsf.x_finemoved = 1;
        else
            x->x_gui.x_fsf.x_finemoved = 0;
    }
    return (1);
}

static void hslider_size(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{
    hslider_check_width(x, (int)atom_getintarg(0, ac, av));
    if(ac > 1)
        x->x_gui.x_h = iem_clip_size((int)atom_getintarg(1, ac, av));
    iem_size((void *)x, &x->x_gui);
}

static void hslider_delta(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{iem_delta((void *)x, &x->x_gui, s, ac, av);}

static void hslider_pos(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{iem_pos((void *)x, &x->x_gui, s, ac, av);}

static void hslider_range(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{
    hslider_check_minmax(x, (double)atom_getfloatarg(0, ac, av),
                         (double)atom_getfloatarg(1, ac, av));
}

static void hslider_color(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{iem_color((void *)x, &x->x_gui, s, ac, av);}

static void hslider_send(t_hslider *x, t_symbol *s)
{iem_send(x, &x->x_gui, s);}

static void hslider_receive(t_hslider *x, t_symbol *s)
{iem_receive(x, &x->x_gui, s);}

static void hslider_label(t_hslider *x, t_symbol *s)
{iem_label((void *)x, &x->x_gui, s);}

static void hslider_label_pos(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{iem_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void hslider_label_font(t_hslider *x, t_symbol *s, int ac, t_atom *av)
{iem_label_font((void *)x, &x->x_gui, s, ac, av);}

static void hslider_log(t_hslider *x)
{
    x->x_lin0_log1 = 1;
    hslider_check_minmax(x, x->x_min, x->x_max);
}

static void hslider_lin(t_hslider *x)
{
    x->x_lin0_log1 = 0;
    x->x_k = (x->x_max - x->x_min)/(double)(x->x_gui.x_w - 1);
}

static void hslider_init(t_hslider *x, t_floatarg f)
{
    x->x_gui.x_isa.x_loadinit = (f==0.0)?0:1;
}

static void hslider_steady(t_hslider *x, t_floatarg f)
{
    x->x_steady = (f==0.0)?0:1;
}

static void hslider_float(t_hslider *x, t_floatarg f)
{
    double out;

    hslider_set(x, f);
    if(x->x_gui.x_fsf.x_put_in2out)
        hslider_bang(x);
}

static void hslider_loadbang(t_hslider *x)
{
    if(!sys_noloadbang && x->x_gui.x_isa.x_loadinit)
    {
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
        hslider_bang(x);
    }
}

static void *hslider_new(t_symbol *s, int argc, t_atom *argv)
{
    t_hslider *x = (t_hslider *)pd_new(hslider_class);
    int bflcol[]={-262144, -1, -1};
    int w=IEM_HSLIDER_DEFAULT_WIDTH, h=IEM_HSLIDER_DEFAULT_HEIGHT;
    int lilo=0, ldx=-2, ldy=-8, f=0, steady=1;
    int fs=10;
    double min=0.0, max=(double)(IEM_HSLIDER_DEFAULT_WIDTH-1);
    char str[144];
    float v = 0;

    iem_inttosymargs(&x->x_gui.x_isa, 0);
    iem_inttofstyle(&x->x_gui.x_fsf, 0);

    if(((argc == 17)||(argc == 18))&&IS_FLOAT(argv,0)&&IS_FLOAT(argv,1)
       &&IS_FLOAT(argv,2)&&IS_FLOAT(argv,3)
       &&IS_FLOAT(argv,4)&&IS_FLOAT(argv,5)
       &&(IS_SYMBOL(argv,6)||IS_FLOAT(argv,6))
       &&(IS_SYMBOL(argv,7)||IS_FLOAT(argv,7))
       &&(IS_SYMBOL(argv,8)||IS_FLOAT(argv,8))
       &&IS_FLOAT(argv,9)&&IS_FLOAT(argv,10)
       &&IS_FLOAT(argv,11)&&IS_FLOAT(argv,12)&&IS_FLOAT(argv,13)
       &&IS_FLOAT(argv,14)&&IS_FLOAT(argv,15)&&IS_FLOAT(argv,16))
    {
        w = (int)atom_getintarg(0, argc, argv);
        h = (int)atom_getintarg(1, argc, argv);
        min = (double)atom_getfloatarg(2, argc, argv);
        max = (double)atom_getfloatarg(3, argc, argv);
        lilo = (int)atom_getintarg(4, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(5, argc, argv));
        iem_new_getnames(&x->x_gui, 6, argv);
        ldx = (int)atom_getintarg(9, argc, argv);
        ldy = (int)atom_getintarg(10, argc, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(11, argc, argv));
        fs = (int)atom_getintarg(12, argc, argv);
        bflcol[0] = (int)atom_getintarg(13, argc, argv);
        bflcol[1] = (int)atom_getintarg(14, argc, argv);
        bflcol[2] = (int)atom_getintarg(15, argc, argv);
        v = atom_getfloatarg(16, argc, argv);
    }
    else iem_new_getnames(&x->x_gui, 6, 0);
    if((argc == 18)&&IS_FLOAT(argv,17))
        steady = (int)atom_getintarg(17, argc, argv);

    x->x_gui.x_draw = (t_iemfunptr)hslider_draw;

    x->x_gui.x_fsf.x_snd_able = 1;
    x->x_gui.x_fsf.x_rcv_able = 1;

    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    if (x->x_gui.x_isa.x_loadinit)
        x->x_val = v;
    else x->x_val = 0;
    x->x_pos = x->x_val;
    if(lilo != 0) lilo = 1;
    x->x_lin0_log1 = lilo;
    if(steady != 0) steady = 1;
    x->x_steady = steady;
    if (!strcmp(x->x_gui.x_snd->s_name, "empty"))
        x->x_gui.x_fsf.x_snd_able = 0;
    if (!strcmp(x->x_gui.x_rcv->s_name, "empty"))
        x->x_gui.x_fsf.x_rcv_able = 0;

    if (x->x_gui.x_fsf.x_rcv_able)
        pd_bind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.x_fontsize = fs;
    x->x_gui.x_h = iem_clip_size(h);
    hslider_check_width(x, w);
    hslider_check_minmax(x, min, max);
    iem_all_colfromload(&x->x_gui, bflcol);
    iem_verify_snd_ne_rcv(&x->x_gui);
    outlet_new(&x->x_gui.x_obj, &s_float);
    x->x_fval = hslider_getfval(x);
    return (x);
}

static void hslider_free(t_hslider *x)
{
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_unbind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    gfxstub_deleteforkey(x);
}

void g_hslider_setup(void)
{
    hslider_class = class_new(gensym("hsl"), (t_newmethod)hslider_new,
                              (t_method)hslider_free, sizeof(t_hslider), 0, A_GIMME, 0);
#ifndef GGEE_HSLIDER_COMPATIBLE
    class_addcreator((t_newmethod)hslider_new, gensym("hslider"), A_GIMME, 0);
#endif
    class_addbang(hslider_class,hslider_bang);
    class_addfloat(hslider_class,hslider_float);
    class_addmethod(hslider_class, (t_method)hslider_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(hslider_class, (t_method)hslider_motion, gensym("motion"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(hslider_class, (t_method)hslider_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_loadbang, gensym("loadbang"), 0);
    class_addmethod(hslider_class, (t_method)hslider_set, gensym("set"), A_FLOAT, 0);
    class_addmethod(hslider_class, (t_method)hslider_size, gensym("size"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_delta, gensym("delta"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_pos, gensym("pos"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_range, gensym("range"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_color, gensym("color"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_send, gensym("send"), A_DEFSYMBOL, 0);
    class_addmethod(hslider_class, (t_method)hslider_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addmethod(hslider_class, (t_method)hslider_label, gensym("label"), A_DEFSYMBOL, 0);
    class_addmethod(hslider_class, (t_method)hslider_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_label_font, gensym("label_font"), A_GIMME, 0);
    class_addmethod(hslider_class, (t_method)hslider_log, gensym("log"), 0);
    class_addmethod(hslider_class, (t_method)hslider_lin, gensym("lin"), 0);
    class_addmethod(hslider_class, (t_method)hslider_init, gensym("init"), A_FLOAT, 0);
    class_addmethod(hslider_class, (t_method)hslider_steady, gensym("steady"), A_FLOAT, 0);
    hslider_widgetbehavior.w_getrectfn =    hslider_getrect;
    hslider_widgetbehavior.w_displacefn =   iem_displace;
    hslider_widgetbehavior.w_selectfn =     iem_select;
    hslider_widgetbehavior.w_activatefn =   NULL;
    hslider_widgetbehavior.w_deletefn =     iem_delete;
    hslider_widgetbehavior.w_visfn =        iem_vis;
    hslider_widgetbehavior.w_clickfn =      hslider_newclick;
    class_setwidget(hslider_class, &hslider_widgetbehavior);
    class_sethelpsymbol(hslider_class, gensym("hsl"));
    class_setsavefn(hslider_class, hslider_save);
    class_setpropertiesfn(hslider_class, hslider_properties);
}
