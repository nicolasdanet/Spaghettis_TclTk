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

#define IEM_TOGGLE_DEFAULT_SIZE     15
#define IEM_TOGGLE_MINIMUM_SIZE     8

/* --------------- tgl     gui-toggle ------------------------- */

t_widgetbehavior toggle_widgetbehavior;
static t_class *toggle_class;

extern int sys_noloadbang;

/* widget helper functions */

void toggle_draw_update(t_toggle *x, t_glist *glist)
{
    if(glist_isvisible(glist))
    {
        t_canvas *canvas=glist_getcanvas(glist);

        sys_vgui(".x%lx.c itemconfigure %lxX1 -fill #%6.6x\n", canvas, x,
                 (x->x_on!=0.0)?x->x_gui.x_fcol:x->x_gui.x_bcol);
        sys_vgui(".x%lx.c itemconfigure %lxX2 -fill #%6.6x\n", canvas, x,
                 (x->x_on!=0.0)?x->x_gui.x_fcol:x->x_gui.x_bcol);
    }
}

void toggle_draw_new(t_toggle *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int w=1, xx=text_xpix(&x->x_gui.x_obj, glist), yy=text_ypix(&x->x_gui.x_obj, glist);

    if(x->x_gui.x_w >= 30)
        w = 2;
    if(x->x_gui.x_w >= 60)
        w = 3;
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xx, yy, xx + x->x_gui.x_w, yy + x->x_gui.x_h,
             x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxX1\n",
             canvas, xx+w+1, yy+w+1, xx + x->x_gui.x_w-w, yy + x->x_gui.x_h-w, w,
             (x->x_on!=0.0)?x->x_gui.x_fcol:x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxX2\n",
             canvas, xx+w+1, yy + x->x_gui.x_h-w-1, xx + x->x_gui.x_w-w, yy+w, w,
             (x->x_on!=0.0)?x->x_gui.x_fcol:x->x_gui.x_bcol, x);
    sys_vgui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xx+x->x_gui.x_ldx,
             yy+x->x_gui.x_ldy,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"",
             x->x_gui.x_fontsize,
             x->x_gui.x_lcol, x);

        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xx, yy + x->x_gui.x_h-1, xx + INLETS_WIDTH, yy + x->x_gui.x_h, x, 0);

        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xx, yy, xx + INLETS_WIDTH, yy+1, x, 0);
}

void toggle_draw_move(t_toggle *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int w=1, xx=text_xpix(&x->x_gui.x_obj, glist), yy=text_ypix(&x->x_gui.x_obj, glist);

    if(x->x_gui.x_w >= 30)
        w = 2;

    if(x->x_gui.x_w >= 60)
        w = 3;
    sys_vgui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x, xx, yy, xx + x->x_gui.x_w, yy + x->x_gui.x_h);
    sys_vgui(".x%lx.c itemconfigure %lxX1 -width %d\n", canvas, x, w);
    sys_vgui(".x%lx.c coords %lxX1 %d %d %d %d\n",
             canvas, x, xx+w+1, yy+w+1, xx + x->x_gui.x_w-w, yy + x->x_gui.x_h-w);
    sys_vgui(".x%lx.c itemconfigure %lxX2 -width %d\n", canvas, x, w);
    sys_vgui(".x%lx.c coords %lxX2 %d %d %d %d\n",
             canvas, x, xx+w+1, yy + x->x_gui.x_h-w-1, xx + x->x_gui.x_w-w, yy+w);
    sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xx+x->x_gui.x_ldx, yy+x->x_gui.x_ldy);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0, xx, yy + x->x_gui.x_h-1, xx + INLETS_WIDTH, yy + x->x_gui.x_h);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0, xx, yy, xx + INLETS_WIDTH, yy+1);
}

void toggle_draw_erase(t_toggle* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxX1\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxX2\n", canvas, x);
    sys_vgui(".x%lx.c delete %lxLABEL\n", canvas, x);
    if(!x->x_gui.x_fsf.x_snd_able)
        sys_vgui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    if(!x->x_gui.x_fsf.x_rcv_able)
        sys_vgui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void toggle_draw_config(t_toggle* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.x_fontsize,
             x->x_gui.x_fsf.x_selected?IEM_COLOR_SELECTED:x->x_gui.x_lcol,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"");
    sys_vgui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x,
             x->x_gui.x_bcol);
    sys_vgui(".x%lx.c itemconfigure %lxX1 -fill #%6.6x\n", canvas, x,
             x->x_on?x->x_gui.x_fcol:x->x_gui.x_bcol);
    sys_vgui(".x%lx.c itemconfigure %lxX2 -fill #%6.6x\n", canvas, x,
             x->x_on?x->x_gui.x_fcol:x->x_gui.x_bcol);
}

void toggle_draw_io(t_toggle* x, t_glist* glist)
{
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos,
        ypos + x->x_gui.x_h-1, xpos + INLETS_WIDTH,
        ypos + x->x_gui.x_h, x, 0);
    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos, ypos,
        xpos + INLETS_WIDTH, ypos+1, x, 0);
}

void toggle_draw_select(t_toggle* x, t_glist* glist)
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

void toggle_draw(t_toggle *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        toggle_draw_update(x, glist);
    else if(mode == IEM_DRAW_MOVE)
        toggle_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        toggle_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        toggle_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        toggle_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        toggle_draw_config(x, glist);
    else if(mode >= IEM_DRAW_IO)
        toggle_draw_io(x, glist);
}

/* ------------------------ tgl widgetbehaviour----------------------------- */

static void toggle_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_toggle *x = (t_toggle *)z;

    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void toggle_save(t_gobj *z, t_binbuf *b)
{
    t_toggle *x = (t_toggle *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iem_save(&x->x_gui, srl, bflcol);
    binbuf_addv(b, "ssiisiisssiiiiiiiff", gensym("#X"),gensym("obj"),
                (int)x->x_gui.x_obj.te_xpix,
                (int)x->x_gui.x_obj.te_ypix,
                gensym("tgl"), x->x_gui.x_w,
                iem_symargstoint(&x->x_gui.x_isa),
                srl[0], srl[1], srl[2],
                x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[1], bflcol[2], x->x_on, x->x_nonzero);
    binbuf_addv(b, ";");
}

static void toggle_properties(t_gobj *z, t_glist *owner)
{
    t_toggle *x = (t_toggle *)z;
    char buf[800];
    t_symbol *srl[3];

    iem_properties(&x->x_gui, srl);
    sprintf(buf, "::ui_iem::create %%s Toggle \
            %d %d Size 0 0 empty \
            %g {Non-Zero Value} 0 empty \
            -1 empty empty \
            %d \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.x_w, IEM_TOGGLE_MINIMUM_SIZE,
            x->x_nonzero,
            x->x_gui.x_isa.x_loadinit,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, 0xffffff & x->x_gui.x_fcol, 0xffffff & x->x_gui.x_lcol);
    gfxstub_new(&x->x_gui.x_obj.te_g.g_pd, x, buf);
}

static void toggle_bang(t_toggle *x)
{
    x->x_on = (x->x_on==0.0)?x->x_nonzero:0.0;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
    outlet_float(x->x_gui.x_obj.te_outlet, x->x_on);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, x->x_on);
}

static void toggle_dialog(t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *srl[3];
    int a = (int)atom_getintarg(0, argc, argv);
    t_float nonzero = (t_float)atom_getfloatarg(2, argc, argv);

    if(nonzero == 0.0)
        nonzero = 1.0;
    x->x_nonzero = nonzero;
    if(x->x_on != 0.0)
        x->x_on = x->x_nonzero;
    iem_dialog(&x->x_gui, srl, argc, argv);
    x->x_gui.x_w = iem_clip_size(a);
    x->x_gui.x_h = x->x_gui.x_w;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_IO);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.x_glist, (t_text*)x);
}

static void toggle_click(t_toggle *x, t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{toggle_bang(x);}

static int toggle_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        toggle_click((t_toggle *)z, (t_floatarg)xpix, (t_floatarg)ypix, (t_floatarg)shift, 0, (t_floatarg)alt);
    return (1);
}

static void toggle_set(t_toggle *x, t_floatarg f)
{
    int old = (x->x_on != 0);
    x->x_on = f;
    if (f != 0.0 && PD_COMPATIBILITY < 46)
        x->x_nonzero = f;
    if ((x->x_on != 0) != old)
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_UPDATE);
}

static void toggle_float(t_toggle *x, t_floatarg f)
{
    toggle_set(x, f);
    if(x->x_gui.x_fsf.x_put_in2out)
    {
        outlet_float(x->x_gui.x_obj.te_outlet, x->x_on);
        if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
            pd_float(x->x_gui.x_snd->s_thing, x->x_on);
    }
}

static void toggle_fout(t_toggle *x, t_floatarg f)
{
    toggle_set(x, f);
    outlet_float(x->x_gui.x_obj.te_outlet, x->x_on);
    if(x->x_gui.x_fsf.x_snd_able && x->x_gui.x_snd->s_thing)
        pd_float(x->x_gui.x_snd->s_thing, x->x_on);
}

static void toggle_loadbang(t_toggle *x)
{
    if(!sys_noloadbang && x->x_gui.x_isa.x_loadinit)
        toggle_fout(x, (t_float)x->x_on);
}

static void toggle_size(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_gui.x_w = iem_clip_size((int)atom_getintarg(0, ac, av));
    x->x_gui.x_h = x->x_gui.x_w;
    iem_size((void *)x, &x->x_gui);
}

static void toggle_delta(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iem_delta((void *)x, &x->x_gui, s, ac, av);}

static void toggle_pos(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iem_pos((void *)x, &x->x_gui, s, ac, av);}

static void toggle_color(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iem_color((void *)x, &x->x_gui, s, ac, av);}

static void toggle_send(t_toggle *x, t_symbol *s)
{iem_send(x, &x->x_gui, s);}

static void toggle_receive(t_toggle *x, t_symbol *s)
{iem_receive(x, &x->x_gui, s);}

static void toggle_label(t_toggle *x, t_symbol *s)
{iem_label((void *)x, &x->x_gui, s);}

static void toggle_label_font(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iem_label_font((void *)x, &x->x_gui, s, ac, av);}

static void toggle_label_pos(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iem_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void toggle_init(t_toggle *x, t_floatarg f)
{
    x->x_gui.x_isa.x_loadinit = (f==0.0)?0:1;
}

static void toggle_nonzero(t_toggle *x, t_floatarg f)
{
    if(f != 0.0)
        x->x_nonzero = f;
}

static void *toggle_new(t_symbol *s, int argc, t_atom *argv)
{
    t_toggle *x = (t_toggle *)pd_new(toggle_class);
    int bflcol[]={-262144, -1, -1};
    int a=IEM_TOGGLE_DEFAULT_SIZE, f=0;
    int ldx=17, ldy=7;
    int fs=10;
    t_float on=0.0, nonzero=1.0;
    char str[144];

    iem_inttosymargs(&x->x_gui.x_isa, 0);
    iem_inttofstyle(&x->x_gui.x_fsf, 0);

    if(((argc == 13)||(argc == 14))&&IS_FLOAT(argv,0)
       &&IS_FLOAT(argv,1)
       &&(IS_SYMBOL(argv,2)||IS_FLOAT(argv,2))
       &&(IS_SYMBOL(argv,3)||IS_FLOAT(argv,3))
       &&(IS_SYMBOL(argv,4)||IS_FLOAT(argv,4))
       &&IS_FLOAT(argv,5)&&IS_FLOAT(argv,6)
       &&IS_FLOAT(argv,7)&&IS_FLOAT(argv,8)&&IS_FLOAT(argv,9)
       &&IS_FLOAT(argv,10)&&IS_FLOAT(argv,11)&&IS_FLOAT(argv,12))
    {
        a = (int)atom_getintarg(0, argc, argv);
        iem_inttosymargs(&x->x_gui.x_isa, atom_getintarg(1, argc, argv));
        iem_new_getnames(&x->x_gui, 2, argv);
        ldx = (int)atom_getintarg(5, argc, argv);
        ldy = (int)atom_getintarg(6, argc, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, atom_getintarg(7, argc, argv));
        fs = (int)atom_getintarg(8, argc, argv);
        bflcol[0] = (int)atom_getintarg(9, argc, argv);
        bflcol[1] = (int)atom_getintarg(10, argc, argv);
        bflcol[2] = (int)atom_getintarg(11, argc, argv);
        on = (t_float)atom_getfloatarg(12, argc, argv);
    }
    else iem_new_getnames(&x->x_gui, 2, 0);
    if((argc == 14)&&IS_FLOAT(argv,13))
        nonzero = (t_float)atom_getfloatarg(13, argc, argv);
    x->x_gui.x_draw = (t_iemfunptr)toggle_draw;

    x->x_gui.x_fsf.x_snd_able = 1;
    x->x_gui.x_fsf.x_rcv_able = 1;
    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.x_snd->s_name, "empty"))
        x->x_gui.x_fsf.x_snd_able = 0;
    if (!strcmp(x->x_gui.x_rcv->s_name, "empty"))
        x->x_gui.x_fsf.x_rcv_able = 0;

    x->x_nonzero = (nonzero!=0.0)?nonzero:1.0;
    if(x->x_gui.x_isa.x_loadinit)
        x->x_on = (on!=0.0)?nonzero:0.0;
    else
        x->x_on = 0.0;
    if (x->x_gui.x_fsf.x_rcv_able)
        pd_bind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;

    if(fs < 4)
        fs = 4;
    x->x_gui.x_fontsize = fs;
    x->x_gui.x_w = iem_clip_size(a);
    x->x_gui.x_h = x->x_gui.x_w;
    iem_all_colfromload(&x->x_gui, bflcol);
    iem_verify_snd_ne_rcv(&x->x_gui);
    outlet_new(&x->x_gui.x_obj, &s_float);
    return (x);
}

static void toggle_ff(t_toggle *x)
{
    if(x->x_gui.x_fsf.x_rcv_able)
        pd_unbind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    gfxstub_deleteforkey(x);
}

void g_toggle_setup(void)
{
    toggle_class = class_new(gensym("tgl"), (t_newmethod)toggle_new,
                             (t_method)toggle_ff, sizeof(t_toggle), 0, A_GIMME, 0);
    class_addCreator((t_newmethod)toggle_new, gensym("toggle"), A_GIMME, 0);
    class_addbang(toggle_class, toggle_bang);
    class_addfloat(toggle_class, toggle_float);
    class_addmethod(toggle_class, (t_method)toggle_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_dialog, gensym("dialog"),
                    A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_loadbang, gensym("loadbang"), 0);
    class_addmethod(toggle_class, (t_method)toggle_set, gensym("set"), A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_size, gensym("size"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_delta, gensym("delta"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_pos, gensym("pos"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_color, gensym("color"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_send, gensym("send"), A_DEFSYMBOL, 0);
    class_addmethod(toggle_class, (t_method)toggle_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addmethod(toggle_class, (t_method)toggle_label, gensym("label"), A_DEFSYMBOL, 0);
    class_addmethod(toggle_class, (t_method)toggle_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_label_font, gensym("label_font"), A_GIMME, 0);
    class_addmethod(toggle_class, (t_method)toggle_init, gensym("init"), A_FLOAT, 0);
    class_addmethod(toggle_class, (t_method)toggle_nonzero, gensym("nonzero"), A_FLOAT, 0);
    toggle_widgetbehavior.w_getrectfn = toggle_getrect;
    toggle_widgetbehavior.w_displacefn = iem_displace;
    toggle_widgetbehavior.w_selectfn = iem_select;
    toggle_widgetbehavior.w_activatefn = NULL;
    toggle_widgetbehavior.w_deletefn = iem_delete;
    toggle_widgetbehavior.w_visfn = iem_vis;
    toggle_widgetbehavior.w_clickfn = toggle_newclick;
    class_setwidget(toggle_class, &toggle_widgetbehavior);
    class_sethelpsymbol(toggle_class, gensym("tgl"));
    class_setsavefn(toggle_class, toggle_save);
    class_setpropertiesfn(toggle_class, toggle_properties);
}
