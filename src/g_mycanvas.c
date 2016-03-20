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

#define IEM_PANEL_DEFAULT_GRIP_SIZE     15

/* ---------- cnv  my gui-canvas for a window ---------------- */

t_widgetbehavior my_canvas_widgetbehavior;
static t_class *my_canvas_class;

/* widget helper functions */

void my_canvas_draw_new(t_my_canvas *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxRECT\n",
             canvas, xpos, ypos,
             xpos + x->x_vis_w, ypos + x->x_vis_h,
             x->x_gui.x_bcol, x->x_gui.x_bcol, x);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -outline #%6.6x -tags %lxBASE\n",
             canvas, xpos, ypos,
             xpos + x->x_gui.x_w, ypos + x->x_gui.x_h,
             x->x_gui.x_bcol, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xpos+x->x_gui.x_ldx, ypos+x->x_gui.x_ldy,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"",
             x->x_gui.x_fontsize,
             x->x_gui.x_lcol, x);
}

void my_canvas_draw_move(t_my_canvas *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.x_obj, glist);
    int ypos=text_ypix(&x->x_gui.x_obj, glist);
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c coords %lxRECT %d %d %d %d\n",
             canvas, x, xpos, ypos, xpos + x->x_vis_w,
             ypos + x->x_vis_h);
    sys_vGui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + x->x_gui.x_w, ypos + x->x_gui.x_h);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.x_ldx,
             ypos+x->x_gui.x_ldy);
}

void my_canvas_draw_erase(t_my_canvas* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxRECT\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
}

void my_canvas_draw_config(t_my_canvas* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxRECT -fill #%6.6x -outline #%6.6x\n", canvas, x,
             x->x_gui.x_bcol, x->x_gui.x_bcol);
    sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x,
             x->x_gui.x_fsf.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.x_bcol);
    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.x_fontsize,
             x->x_gui.x_lcol,
             strcmp(x->x_gui.x_lab->s_name, "empty")?x->x_gui.x_lab->s_name:"");
}

void my_canvas_draw_select(t_my_canvas* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_gui.x_fsf.iem_isSelected)
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, x->x_gui.x_bcol);
    }
}

void my_canvas_draw(t_my_canvas *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_MOVE)
        my_canvas_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        my_canvas_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        my_canvas_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        my_canvas_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        my_canvas_draw_config(x, glist);
}

/* ------------------------ cnv widgetbehaviour----------------------------- */

static void my_canvas_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_my_canvas *x = (t_my_canvas *)z;
    
    *xp1 = text_xpix(&x->x_gui.x_obj, glist);
    *yp1 = text_ypix(&x->x_gui.x_obj, glist);
    *xp2 = *xp1 + x->x_gui.x_w;
    *yp2 = *yp1 + x->x_gui.x_h;
}

static void my_canvas_save(t_gobj *z, t_buffer *b)
{
    t_my_canvas *x = (t_my_canvas *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iem_save(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiiisssiiiiiii", gensym("#X"),gensym("obj"),
                (int)x->x_gui.x_obj.te_xCoordinate, (int)x->x_gui.x_obj.te_yCoordinate,
                gensym("cnv"), x->x_gui.x_w, x->x_vis_w, x->x_vis_h,
                srl[0], srl[1], srl[2], x->x_gui.x_ldx, x->x_gui.x_ldy,
                iem_fstyletoint(&x->x_gui.x_fsf), x->x_gui.x_fontsize,
                bflcol[0], bflcol[2], iem_symargstoint(&x->x_gui.x_isa));
    buffer_vAppend(b, ";");
}

static void my_canvas_properties(t_gobj *z, t_glist *owner)
{
    t_my_canvas *x = (t_my_canvas *)z;
    char buf[800];
    t_symbol *srl[3];

    iem_properties(&x->x_gui, srl);
    sprintf(buf, "::ui_iem::create %%s Panel \
            %d %d {Grip Size} 0 0 empty \
            %d {Panel Width} %d {Panel Height} \
            -1 empty empty \
            -1 \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.x_w, 1,
            x->x_vis_w, x->x_vis_h,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.x_ldx, x->x_gui.x_ldy,
            x->x_gui.x_fontsize,
            0xffffff & x->x_gui.x_bcol, -1/*no frontcolor*/, 0xffffff & x->x_gui.x_lcol);
    gfxstub_new(&x->x_gui.x_obj.te_g.g_pd, x, buf);
}

static void my_canvas_get_pos(t_my_canvas *x)
{
    if(x->x_gui.x_fsf.iem_canSend && x->x_gui.x_snd->s_thing)
    {
        x->x_at[0].a_w.w_float = text_xpix(&x->x_gui.x_obj, x->x_gui.x_glist);
        x->x_at[1].a_w.w_float = text_ypix(&x->x_gui.x_obj, x->x_gui.x_glist);
        pd_list(x->x_gui.x_snd->s_thing, 2, x->x_at);
    }
}

static void my_canvas_dialog(t_my_canvas *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *srl[3];
    int a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int w = (int)(t_int)atom_getFloatAtIndex(2, argc, argv);
    int h = (int)(t_int)atom_getFloatAtIndex(3, argc, argv);
    iem_dialog(&x->x_gui, srl, argc, argv);

    x->x_gui.x_isa.iem_initializeAtLoad = 0;
    if(a < 1)
        a = 1;
    x->x_gui.x_w = a;
    x->x_gui.x_h = x->x_gui.x_w;
    if(w < 1)
        w = 1;
    x->x_vis_w = w;
    if(h < 1)
        h = 1;
    x->x_vis_h = h;
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_MOVE);
}

static void my_canvas_size(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{
    int i = (int)(t_int)atom_getFloatAtIndex(0, ac, av);

    if(i < 1)
        i = 1;
    x->x_gui.x_w = i;
    x->x_gui.x_h = i;
    iem_size((void *)x, &x->x_gui);
}

static void my_canvas_delta(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{iem_delta((void *)x, &x->x_gui, s, ac, av);}

static void my_canvas_pos(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{iem_pos((void *)x, &x->x_gui, s, ac, av);}

static void my_canvas_vis_size(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{
    int i;

    i = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    if(i < 1)
        i = 1;
    x->x_vis_w = i;
    if(ac > 1)
    {
        i = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
        if(i < 1)
            i = 1;
    }
    x->x_vis_h = i;
    if(glist_isvisible(x->x_gui.x_glist))
        (*x->x_gui.x_draw)(x, x->x_gui.x_glist, IEM_DRAW_MOVE);
}

static void my_canvas_color(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{iem_color((void *)x, &x->x_gui, s, ac, av);}

static void my_canvas_send(t_my_canvas *x, t_symbol *s)
{iem_send(x, &x->x_gui, s);}

static void my_canvas_receive(t_my_canvas *x, t_symbol *s)
{iem_receive(x, &x->x_gui, s);}

static void my_canvas_label(t_my_canvas *x, t_symbol *s)
{iem_label((void *)x, &x->x_gui, s);}

static void my_canvas_label_pos(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{iem_label_pos((void *)x, &x->x_gui, s, ac, av);}

static void my_canvas_label_font(t_my_canvas *x, t_symbol *s, int ac, t_atom *av)
{iem_label_font((void *)x, &x->x_gui, s, ac, av);}

static void *my_canvas_new(t_symbol *s, int argc, t_atom *argv)
{
    t_my_canvas *x = (t_my_canvas *)pd_new(my_canvas_class);
    int bflcol[]={-233017, -1, -66577};
    int a=IEM_PANEL_DEFAULT_GRIP_SIZE, w=100, h=60;
    int ldx=20, ldy=12, f=2, i=0;
    int fs=14;
    char str[144];

    iem_inttosymargs(&x->x_gui.x_isa, 0);
    iem_inttofstyle(&x->x_gui.x_fsf, 0);

    if(((argc >= 10)&&(argc <= 13))
       &&IS_FLOAT_INDEX(argv,0)&&IS_FLOAT_INDEX(argv,1)&&IS_FLOAT_INDEX(argv,2))
    {
        a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        w = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        h = (int)(t_int)atom_getFloatAtIndex(2, argc, argv);
    }
    if((argc >= 12)&&(IS_SYMBOL_INDEX(argv,3)||IS_FLOAT_INDEX(argv,3))&&(IS_SYMBOL_INDEX(argv,4)||IS_FLOAT_INDEX(argv,4)))
    {
        i = 2;
        iem_new_getnames(&x->x_gui, 3, argv);
    }
    else if((argc == 11)&&(IS_SYMBOL_INDEX(argv,3)||IS_FLOAT_INDEX(argv,3)))
    {
        i = 1;
        iem_new_getnames(&x->x_gui, 3, argv);
    }
    else iem_new_getnames(&x->x_gui, 3, 0);

    if(((argc >= 10)&&(argc <= 13))
       &&(IS_SYMBOL_INDEX(argv,i+3)||IS_FLOAT_INDEX(argv,i+3))&&IS_FLOAT_INDEX(argv,i+4)
       &&IS_FLOAT_INDEX(argv,i+5)&&IS_FLOAT_INDEX(argv,i+6)
       &&IS_FLOAT_INDEX(argv,i+7)&&IS_FLOAT_INDEX(argv,i+8)
       &&IS_FLOAT_INDEX(argv,i+9))
    {
            /* disastrously, the "label" sits in a different part of the
            message.  So we have to track its location separately (in
            the slot x_labelbindex) and initialize it specially here. */
        iem_new_dogetname(&x->x_gui, i+3, argv);
        x->x_gui.x_labelbindex = i+4;
        ldx = (int)(t_int)atom_getFloatAtIndex(i+4, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(i+5, argc, argv);
        iem_inttofstyle(&x->x_gui.x_fsf, (t_int)atom_getFloatAtIndex(i+6, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(i+7, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(i+8, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(i+9, argc, argv);
    }
    if((argc == 13)&&IS_FLOAT_INDEX(argv,i+10))
    {
        iem_inttosymargs(&x->x_gui.x_isa, (t_int)atom_getFloatAtIndex(i+10, argc, argv));
    }
    x->x_gui.x_draw = (t_iemfn)my_canvas_draw;
    x->x_gui.x_fsf.iem_canSend = 1;
    x->x_gui.x_fsf.iem_canReceive = 1;
    x->x_gui.x_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.x_snd->s_name, "empty"))
        x->x_gui.x_fsf.iem_canSend = 0;
    if (!strcmp(x->x_gui.x_rcv->s_name, "empty"))
        x->x_gui.x_fsf.iem_canReceive = 0;
    if(a < 1)
        a = 1;
    x->x_gui.x_w = a;
    x->x_gui.x_h = x->x_gui.x_w;
    if(w < 1)
        w = 1;
    x->x_vis_w = w;
    if(h < 1)
        h = 1;
    x->x_vis_h = h;

    if (x->x_gui.x_fsf.iem_canReceive)
        pd_bind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    x->x_gui.x_ldx = ldx;
    x->x_gui.x_ldy = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.x_fontsize = fs;
    iem_all_colfromload(&x->x_gui, bflcol);
    x->x_at[0].a_type = A_FLOAT;
    x->x_at[1].a_type = A_FLOAT;
    iem_verify_snd_ne_rcv(&x->x_gui);
    return (x);
}

static void my_canvas_ff(t_my_canvas *x)
{
    if(x->x_gui.x_fsf.iem_canReceive)
        pd_unbind(&x->x_gui.x_obj.te_g.g_pd, x->x_gui.x_rcv);
    gfxstub_deleteforkey(x);
}

void g_mycanvas_setup(void)
{
    my_canvas_class = class_new(gensym("cnv"), (t_newmethod)my_canvas_new,
                                (t_method)my_canvas_ff, sizeof(t_my_canvas), CLASS_NOINLET, A_GIMME, 0);
    class_addCreator((t_newmethod)my_canvas_new, gensym("my_canvas"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_dialog, gensym("dialog"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_size, gensym("size"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_delta, gensym("delta"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_pos, gensym("pos"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_vis_size, gensym("vis_size"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_color, gensym("color"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_send, gensym("send"), A_DEFSYMBOL, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_label, gensym("label"), A_DEFSYMBOL, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_label_font, gensym("label_font"), A_GIMME, 0);
    class_addMethod(my_canvas_class, (t_method)my_canvas_get_pos, gensym("get_pos"), 0);

    my_canvas_widgetbehavior.w_getrectfn = my_canvas_getrect;
    my_canvas_widgetbehavior.w_displacefn = iem_displace;
    my_canvas_widgetbehavior.w_selectfn = iem_select;
    my_canvas_widgetbehavior.w_activatefn = NULL;
    my_canvas_widgetbehavior.w_deletefn = iem_delete;
    my_canvas_widgetbehavior.w_visfn = iem_vis;
    my_canvas_widgetbehavior.w_clickfn = NULL;
    class_setWidget(my_canvas_class, &my_canvas_widgetbehavior);
    class_setHelpName(my_canvas_class, gensym("cnv"));
    class_setSaveFunction(my_canvas_class, my_canvas_save);
    class_setPropertiesFunction(my_canvas_class, my_canvas_properties);
}
