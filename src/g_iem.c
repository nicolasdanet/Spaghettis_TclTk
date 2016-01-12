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
#include "m_private.h"
#include "m_macros.h"
#include "g_canvas.h"

#include "g_iem.h"
#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/*  #define GGEE_HSLIDER_COMPATIBLE  */

/*------------------ global varaibles -------------------------*/

#define IEM_COLOR_MAXIMUM       30
#define IEM_MINIMUM_SIZE    8
#define IEM_FONT_MINIMUM_SIZE   4

int iem_color_hex[]=
{
    16579836, 10526880, 4210752, 16572640, 16572608,
    16579784, 14220504, 14220540, 14476540, 16308476,
    14737632, 8158332, 2105376, 16525352, 16559172,
    15263784, 1370132, 2684148, 3952892, 16003312,
    12369084, 6316128, 0, 9177096, 5779456,
    7874580, 2641940, 17488, 5256, 5767248
};

/*------------------ global functions -------------------------*/


int iem_clip_size(int size)
{
    if(size < IEM_MINIMUM_SIZE)
        size = IEM_MINIMUM_SIZE;
    return(size);
}

int iem_clip_font(int size)
{
    if(size < IEM_FONT_MINIMUM_SIZE)
        size = IEM_FONT_MINIMUM_SIZE;
    return(size);
}

int iem_modulo_color(int col)
{
    while(col >= IEM_COLOR_MAXIMUM)
        col -= IEM_COLOR_MAXIMUM;
    while(col < 0)
        col += IEM_COLOR_MAXIMUM;
    return(col);
}

t_symbol *iem_dollar2raute(t_symbol *s)
{
    char buf[PD_STRING+1], *s1, *s2;
    if (strlen(s->s_name) >= PD_STRING)
        return (s);
    for (s1 = s->s_name, s2 = buf; ; s1++, s2++)
    {
        if (*s1 == '$')
            *s2 = '#';
        else if (!(*s2 = *s1))
            break;
    }
    return(gensym(buf));
}

t_symbol *iem_raute2dollar(t_symbol *s)
{
    char buf[PD_STRING+1], *s1, *s2;
    if (strlen(s->s_name) >= PD_STRING)
        return (s);
    for (s1 = s->s_name, s2 = buf; ; s1++, s2++)
    {
        if (*s1 == '#')
            *s2 = '$';
        else if (!(*s2 = *s1))
            break;
    }
    return(gensym(buf));
}

void iem_verify_snd_ne_rcv(t_iem *iem)
{
    iem->x_fsf.x_put_in2out = 1;
    if(iem->x_fsf.x_snd_able && iem->x_fsf.x_rcv_able)
    {
        if(!strcmp(iem->x_snd->s_name, iem->x_rcv->s_name))
            iem->x_fsf.x_put_in2out = 0;
    }
}

t_symbol *iem_new_dogetname(t_iem *iem, int indx, t_atom *argv)
{
    if (IS_SYMBOL(argv, indx))
        return (atom_getSymbolAtIndex(indx, 100000, argv));
    else if (IS_FLOAT(argv, indx))
    {
        char str[80];
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(indx, 100000, argv));
        return (gensym(str));
    }
    else return (gensym("empty"));
}

void iem_new_getnames(t_iem *iem, int indx, t_atom *argv)
{
    if (argv)
    {
        iem->x_snd = iem_new_dogetname(iem, indx, argv);
        iem->x_rcv = iem_new_dogetname(iem, indx+1, argv);
        iem->x_lab = iem_new_dogetname(iem, indx+2, argv);
    }
    else iem->x_snd = iem->x_rcv = iem->x_lab = gensym("empty");
    iem->x_snd_unexpanded = iem->x_rcv_unexpanded =
        iem->x_lab_unexpanded = 0;
    iem->x_binbufindex = indx;
    iem->x_labelbindex = indx + 3;
}

    /* convert symbols in "$" form to the expanded symbols */
void iem_all_dollararg2sym(t_iem *iem, t_symbol **srlsym)
{
        /* save unexpanded ones for later */
    iem->x_snd_unexpanded = srlsym[0];
    iem->x_rcv_unexpanded = srlsym[1];
    iem->x_lab_unexpanded = srlsym[2];
    srlsym[0] = canvas_realizedollar(iem->x_glist, srlsym[0]);
    srlsym[1] = canvas_realizedollar(iem->x_glist, srlsym[1]);
    srlsym[2] = canvas_realizedollar(iem->x_glist, srlsym[2]);
}

    /* initialize a single symbol in unexpanded form.  We reach into the
    binbuf to grab them; if there's nothing there, set it to the
    fallback; if still nothing, set to "empty". */
static void iem_init_sym2dollararg(t_iem *iem, t_symbol **symp,
    int indx, t_symbol *fallback)
{
    if (!*symp)
    {
        t_buffer *b = iem->x_obj.te_buffer;
        if (buffer_getSize(b) > indx)
        {
            char buf[80];
            atom_toString(buffer_getAtoms(b) + indx, buf, 80);
            *symp = gensym(buf);
        }
        else if (fallback)
            *symp = fallback;
        else *symp = gensym("empty");
    }
}

    /* get the unexpanded versions of the symbols; initialize them if
    necessary. */
void iem_all_sym2dollararg(t_iem *iem, t_symbol **srlsym)
{
    iem_init_sym2dollararg(iem, &iem->x_snd_unexpanded,
        iem->x_binbufindex+1, iem->x_snd);
    iem_init_sym2dollararg(iem, &iem->x_rcv_unexpanded,
        iem->x_binbufindex+2, iem->x_rcv);
    iem_init_sym2dollararg(iem, &iem->x_lab_unexpanded,
        iem->x_labelbindex, iem->x_lab);
    srlsym[0] = iem->x_snd_unexpanded;
    srlsym[1] = iem->x_rcv_unexpanded;
    srlsym[2] = iem->x_lab_unexpanded;
}

void iem_all_col2save(t_iem *iem, int *bflcol)
{
    bflcol[0] = -1 - (((0xfc0000 & iem->x_bcol) >> 6)|
                      ((0xfc00 & iem->x_bcol) >> 4)|((0xfc & iem->x_bcol) >> 2));
    bflcol[1] = -1 - (((0xfc0000 & iem->x_fcol) >> 6)|
                      ((0xfc00 & iem->x_fcol) >> 4)|((0xfc & iem->x_fcol) >> 2));
    bflcol[2] = -1 - (((0xfc0000 & iem->x_lcol) >> 6)|
                      ((0xfc00 & iem->x_lcol) >> 4)|((0xfc & iem->x_lcol) >> 2));
}

void iem_all_colfromload(t_iem *iem, int *bflcol)
{
    if(bflcol[0] < 0)
    {
        bflcol[0] = -1 - bflcol[0];
        iem->x_bcol = ((bflcol[0] & 0x3f000) << 6)|((bflcol[0] & 0xfc0) << 4)|
            ((bflcol[0] & 0x3f) << 2);
    }
    else
    {
        bflcol[0] = iem_modulo_color(bflcol[0]);
        iem->x_bcol = iem_color_hex[bflcol[0]];
    }
    if(bflcol[1] < 0)
    {
        bflcol[1] = -1 - bflcol[1];
        iem->x_fcol = ((bflcol[1] & 0x3f000) << 6)|((bflcol[1] & 0xfc0) << 4)|
            ((bflcol[1] & 0x3f) << 2);
    }
    else
    {
        bflcol[1] = iem_modulo_color(bflcol[1]);
        iem->x_fcol = iem_color_hex[bflcol[1]];
    }
    if(bflcol[2] < 0)
    {
        bflcol[2] = -1 - bflcol[2];
        iem->x_lcol = ((bflcol[2] & 0x3f000) << 6)|((bflcol[2] & 0xfc0) << 4)|
            ((bflcol[2] & 0x3f) << 2);
    }
    else
    {
        bflcol[2] = iem_modulo_color(bflcol[2]);
        iem->x_lcol = iem_color_hex[bflcol[2]];
    }
}

int iem_compatible_col(int i)
{
    int j;

    if(i >= 0)
    {
        j = iem_modulo_color(i);
        return(iem_color_hex[(j)]);
    }
    else
        return((-1 -i)&0xffffff);
}

void iem_all_dollar2raute(t_symbol **srlsym)
{
    srlsym[0] = iem_dollar2raute(srlsym[0]);
    srlsym[1] = iem_dollar2raute(srlsym[1]);
    srlsym[2] = iem_dollar2raute(srlsym[2]);
}

void iem_all_raute2dollar(t_symbol **srlsym)
{
    srlsym[0] = iem_raute2dollar(srlsym[0]);
    srlsym[1] = iem_raute2dollar(srlsym[1]);
    srlsym[2] = iem_raute2dollar(srlsym[2]);
}

void iem_send(void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *snd;
    int pargc, tail_len, nth_arg, sndable=1;
    t_atom *pargv;

    if(!strcmp(s->s_name, "empty")) sndable = 0;
    snd = iem_raute2dollar(s);
    iem->x_snd_unexpanded = snd;
    iem->x_snd = snd = canvas_realizedollar(iem->x_glist, snd);
    iem->x_fsf.x_snd_able = sndable;
    iem_verify_snd_ne_rcv(iem);
    (*iem->x_draw)(x, iem->x_glist, IEM_DRAW_IO);
}

void iem_receive(void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *rcv;
    int pargc, tail_len, nth_arg, rcvable=1;
    t_atom *pargv;

    if(!strcmp(s->s_name, "empty")) rcvable = 0;
    rcv = iem_raute2dollar(s);
    iem->x_rcv_unexpanded = rcv;
    rcv = canvas_realizedollar(iem->x_glist, rcv);
    if(rcvable)
    {
        if(strcmp(rcv->s_name, iem->x_rcv->s_name))
        {
            if(iem->x_fsf.x_rcv_able)
                pd_unbind(&iem->x_obj.te_g.g_pd, iem->x_rcv);
            iem->x_rcv = rcv;
            pd_bind(&iem->x_obj.te_g.g_pd, iem->x_rcv);
        }
    }
    else if(!rcvable && iem->x_fsf.x_rcv_able)
    {
        pd_unbind(&iem->x_obj.te_g.g_pd, iem->x_rcv);
        iem->x_rcv = rcv;
    }
    iem->x_fsf.x_rcv_able = rcvable;
    iem_verify_snd_ne_rcv(iem);
    (*iem->x_draw)(x, iem->x_glist, IEM_DRAW_IO);
}

void iem_label(void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *old;
    int pargc, tail_len, nth_arg;
    t_atom *pargv;

        /* tb: fix for empty label { */
        if (s == gensym(""))
                s = gensym("empty");
        /* tb } */

    old = iem->x_lab;
    iem->x_lab_unexpanded = iem_raute2dollar(s);
    iem->x_lab = canvas_realizedollar(iem->x_glist, iem->x_lab_unexpanded);

    if(glist_isvisible(iem->x_glist) && iem->x_lab != old)
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -text {%s} \n",
                 glist_getcanvas(iem->x_glist), x,
                 strcmp(s->s_name, "empty")?iem->x_lab->s_name:"");
}

void iem_label_pos(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->x_ldx = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    iem->x_ldy = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(glist_isvisible(iem->x_glist))
        sys_vgui(".x%lx.c coords %lxLABEL %d %d\n",
                 glist_getcanvas(iem->x_glist), x,
                 text_xpix((t_object *)x,iem->x_glist)+iem->x_ldx,
                 text_ypix((t_object *)x,iem->x_glist)+iem->x_ldy);
}

void iem_label_font(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    int f = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(f < 4)
        f = 4;
    iem->x_fontsize = f;
    if(glist_isvisible(iem->x_glist))
        sys_vgui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d]\n",
                 glist_getcanvas(iem->x_glist), x,
                 iem->x_fontsize);
}

void iem_size(void *x, t_iem *iem)
{
    if(glist_isvisible(iem->x_glist))
    {
        (*iem->x_draw)(x, iem->x_glist, IEM_DRAW_MOVE);
        canvas_fixlines(iem->x_glist, (t_text*)x);
    }
}

void iem_delta(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->x_obj.te_xCoordinate += (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    iem->x_obj.te_yCoordinate += (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(glist_isvisible(iem->x_glist))
    {
        (*iem->x_draw)(x, iem->x_glist, IEM_DRAW_MOVE);
        canvas_fixlines(iem->x_glist, (t_text*)x);
    }
}

void iem_pos(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->x_obj.te_xCoordinate = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    iem->x_obj.te_yCoordinate = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(glist_isvisible(iem->x_glist))
    {
        (*iem->x_draw)(x, iem->x_glist, IEM_DRAW_MOVE);
        canvas_fixlines(iem->x_glist, (t_text*)x);
    }
}

void iem_color(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->x_bcol = iem_compatible_col((t_int)atom_getFloatAtIndex(0, ac, av));
    if(ac > 2)
    {
        iem->x_fcol = iem_compatible_col((t_int)atom_getFloatAtIndex(1, ac, av));
        iem->x_lcol = iem_compatible_col((t_int)atom_getFloatAtIndex(2, ac, av));
    }
    else
        iem->x_lcol = iem_compatible_col((t_int)atom_getFloatAtIndex(1, ac, av));
    if(glist_isvisible(iem->x_glist))
        (*iem->x_draw)(x, iem->x_glist, IEM_DRAW_CONFIG);
}

void iem_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_iem *x = (t_iem *)z;

    x->x_obj.te_xCoordinate += dx;
    x->x_obj.te_yCoordinate += dy;
    (*x->x_draw)((void *)z, glist, IEM_DRAW_MOVE);
    canvas_fixlines(glist, (t_text *)z);
}

void iem_select(t_gobj *z, t_glist *glist, int selected)
{
    t_iem *x = (t_iem *)z;

    x->x_fsf.x_selected = selected;
    (*x->x_draw)((void *)z, glist, IEM_DRAW_SELECT);
}

void iem_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelines(glist, (t_text *)z);
}

void iem_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_iem *x = (t_iem *)z;

    if (vis)
        (*x->x_draw)((void *)z, glist, IEM_DRAW_NEW);
    else
    {
        (*x->x_draw)((void *)z, glist, IEM_DRAW_ERASE);
        sys_unqueuegui(z);
    }
}

void iem_save(t_iem *iem, t_symbol **srl, int *bflcol)
{
    srl[0] = iem->x_snd;
    srl[1] = iem->x_rcv;
    srl[2] = iem->x_lab;
    iem_all_sym2dollararg(iem, srl);
    iem_all_col2save(iem, bflcol);
}

void iem_properties(t_iem *iem, t_symbol **srl)
{
    srl[0] = iem->x_snd;
    srl[1] = iem->x_rcv;
    srl[2] = iem->x_lab;
    iem_all_sym2dollararg(iem, srl);
    iem_all_dollar2raute(srl);
}

void iem_dialog(t_iem *iem, t_symbol **srl, int argc, t_atom *argv)
{
    char str[144];
    int init = (int)(t_int)atom_getFloatAtIndex(5, argc, argv);
    int ldx = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
    int ldy = (int)(t_int)atom_getFloatAtIndex(11, argc, argv);
    int fs = (int)(t_int)atom_getFloatAtIndex(12, argc, argv);
    int bcol = (int)(t_int)atom_getFloatAtIndex(13, argc, argv);
    int fcol = (int)(t_int)atom_getFloatAtIndex(14, argc, argv);
    int lcol = (int)(t_int)atom_getFloatAtIndex(15, argc, argv);
    int sndable=1, rcvable=1;

    if(IS_SYMBOL(argv,7))
        srl[0] = atom_getSymbolAtIndex(7, argc, argv);
    else if(IS_FLOAT(argv,7))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(7, argc, argv));
        srl[0] = gensym(str);
    }
    if(IS_SYMBOL(argv,8))
        srl[1] = atom_getSymbolAtIndex(8, argc, argv);
    else if(IS_FLOAT(argv,8))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(8, argc, argv));
        srl[1] = gensym(str);
    }
    if(IS_SYMBOL(argv,9))
        srl[2] = atom_getSymbolAtIndex(9, argc, argv);
    else if(IS_FLOAT(argv,9))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(9, argc, argv));
        srl[2] = gensym(str);
    }
    if(init != 0) init = 1;
    iem->x_isa.x_loadinit = init;
    if(!strcmp(srl[0]->s_name, "empty")) sndable = 0;
    if(!strcmp(srl[1]->s_name, "empty")) rcvable = 0;
    iem_all_raute2dollar(srl);
    iem_all_dollararg2sym(iem, srl);
    if(rcvable)
    {
        if(strcmp(srl[1]->s_name, iem->x_rcv->s_name))
        {
            if(iem->x_fsf.x_rcv_able)
                pd_unbind(&iem->x_obj.te_g.g_pd, iem->x_rcv);
            iem->x_rcv = srl[1];
            pd_bind(&iem->x_obj.te_g.g_pd, iem->x_rcv);
        }
    }
    else if(!rcvable && iem->x_fsf.x_rcv_able)
    {
        pd_unbind(&iem->x_obj.te_g.g_pd, iem->x_rcv);
        iem->x_rcv = srl[1];
    }
    iem->x_snd = srl[0];
    iem->x_fsf.x_snd_able = sndable;
    iem->x_fsf.x_rcv_able = rcvable;
    iem->x_lcol = lcol & 0xffffff;
    iem->x_fcol = fcol & 0xffffff;
    iem->x_bcol = bcol & 0xffffff;
    iem->x_lab = srl[2];
    iem->x_ldx = ldx;
    iem->x_ldy = ldy;
    if(fs < 4)
        fs = 4;
    iem->x_fontsize = fs;
    iem_verify_snd_ne_rcv(iem);
    canvas_dirty(iem->x_glist, 1);
}

/* pre-0.46 the flags were 1 for 'loadinit' and 1<<20 for 'scale'.
Starting in 0.46, take either 1<<20 or 1<<1 for 'scale' and save to both
bits (so that old versions can read files we write).  In the future (2015?)
we can stop writing the annoying  1<<20 bit. */
#define LOADINIT 1
#define SCALE 2
#define SCALEBIS (1<<20)

void iem_inttosymargs(t_iem_init_symargs *symargp, int n)
{
    memset(symargp, 0, sizeof(*symargp));
    symargp->x_loadinit = ((n & LOADINIT) != 0);
    symargp->x_scale = ((n & SCALE) || (n & SCALEBIS)) ;
    symargp->x_flashed = 0;
    symargp->x_locked = 0;
}

int iem_symargstoint(t_iem_init_symargs *symargp)
{
    return ((symargp->x_loadinit ? LOADINIT : 0) |
        (symargp->x_scale ? (SCALE | SCALEBIS) : 0));
}

void iem_inttofstyle(t_iem_fstyle_flags *fstylep, int n)
{
    memset (fstylep, 0, sizeof (*fstylep));
    fstylep->x_font_style = (char)n;
}

int iem_fstyletoint(t_iem_fstyle_flags *fstylep)
{
    return (int)fstylep->x_font_style;
}
