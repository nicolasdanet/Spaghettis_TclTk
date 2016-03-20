
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

#define IEM_MINIMUM_SIZE            8
#define IEM_MINIMUM_FONTSIZE        4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_COLOR(n)                ((-1 -(n)) & 0xffffff)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Ensure compatibility with the legacy format. */ 
/* Only the 6 MSB are kept for each component. */

// RRRRRRRRGGGGGGGGBBBBBBBB 
// RRRRRR..GGGGGG..BBBBBB..
// RRRRRRGGGGGGBBBBBB

static int iem_colorEncode (int color)
{
    int n = 0;
    
    n |= ((0xfc0000 & color) >> 6);
    n |= ((0xfc00 & color) >> 4);
    n |= ((0xfc & color) >> 2);
                      
    return (-1 -n);
}

static int iem_colorDecode (int color)
{
    PD_ASSERT (color < 0);      /* Predefined colors not supported. */
    
    int n = 0;
    
    color = (-1 -color);
    
    n |= ((color & 0x3f000) << 6);
    n |= ((color & 0xfc0) << 4);
    n |= ((color & 0x3f) << 2);

    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iem_saveColors (t_iem *iem, int *bflcol)
{
    bflcol[0] = iem_colorEncode (iem->iem_colorBackground);
    bflcol[1] = iem_colorEncode (iem->iem_colorForeground);
    bflcol[2] = iem_colorEncode (iem->iem_colorLabel);
}

void iem_loadColors (t_iem *iem, int *bflcol)
{
    iem->iem_colorBackground = iem_colorDecode (bflcol[0]);
    iem->iem_colorForeground = iem_colorDecode (bflcol[1]);
    iem->iem_colorLabel      = iem_colorDecode (bflcol[2]);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/*------------------ global functions -------------------------*/


int iem_clip_size(int size)
{
    if(size < IEM_MINIMUM_SIZE)
        size = IEM_MINIMUM_SIZE;
    return(size);
}

int iem_clip_font(int size)
{
    if(size < IEM_MINIMUM_FONTSIZE)
        size = IEM_MINIMUM_FONTSIZE;
    return(size);
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
    iem->iem_flags.iem_goThrough = 1;
    if(iem->iem_flags.iem_canSend && iem->iem_flags.iem_canReceive)
    {
        if(!strcmp(iem->iem_send->s_name, iem->iem_receive->s_name))
            iem->iem_flags.iem_goThrough = 0;
    }
}

t_symbol *iem_new_dogetname(t_iem *iem, int indx, t_atom *argv)
{
    if (IS_SYMBOL_INDEX(argv, indx))
        return (atom_getSymbolAtIndex(indx, 100000, argv));
    else if (IS_FLOAT_INDEX(argv, indx))
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
        iem->iem_send = iem_new_dogetname(iem, indx, argv);
        iem->iem_receive = iem_new_dogetname(iem, indx+1, argv);
        iem->iem_label = iem_new_dogetname(iem, indx+2, argv);
    }
    else iem->iem_send = iem->iem_receive = iem->iem_label = gensym("empty");
    iem->iem_unexpandedSend = iem->iem_unexpandedReceive =
        iem->iem_unexpandedLabel = 0;
    iem->iem_indexBuffer = indx;
    iem->iem_indexLabel = indx + 3;
}

    /* convert symbols in "$" form to the expanded symbols */
void iem_all_dollararg2sym(t_iem *iem, t_symbol **srlsym)
{
        /* save unexpanded ones for later */
    iem->iem_unexpandedSend = srlsym[0];
    iem->iem_unexpandedReceive = srlsym[1];
    iem->iem_unexpandedLabel = srlsym[2];
    srlsym[0] = canvas_realizedollar(iem->iem_glist, srlsym[0]);
    srlsym[1] = canvas_realizedollar(iem->iem_glist, srlsym[1]);
    srlsym[2] = canvas_realizedollar(iem->iem_glist, srlsym[2]);
}

    /* initialize a single symbol in unexpanded form.  We reach into the
    binbuf to grab them; if there's nothing there, set it to the
    fallback; if still nothing, set to "empty". */
static void iem_init_sym2dollararg(t_iem *iem, t_symbol **symp,
    int indx, t_symbol *fallback)
{
    if (!*symp)
    {
        t_buffer *b = iem->iem_obj.te_buffer;
        if (buffer_size(b) > indx)
        {
            char buf[80];
            atom_toString(buffer_atoms(b) + indx, buf, 80);
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
    iem_init_sym2dollararg(iem, &iem->iem_unexpandedSend,
        iem->iem_indexBuffer+1, iem->iem_send);
    iem_init_sym2dollararg(iem, &iem->iem_unexpandedReceive,
        iem->iem_indexBuffer+2, iem->iem_receive);
    iem_init_sym2dollararg(iem, &iem->iem_unexpandedLabel,
        iem->iem_indexLabel, iem->iem_label);
    srlsym[0] = iem->iem_unexpandedSend;
    srlsym[1] = iem->iem_unexpandedReceive;
    srlsym[2] = iem->iem_unexpandedLabel;
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
    iem->iem_unexpandedSend = snd;
    iem->iem_send = snd = canvas_realizedollar(iem->iem_glist, snd);
    iem->iem_flags.iem_canSend = sndable;
    iem_verify_snd_ne_rcv(iem);
    (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_IO);
}

void iem_receive(void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *rcv;
    int pargc, tail_len, nth_arg, rcvable=1;
    t_atom *pargv;

    if(!strcmp(s->s_name, "empty")) rcvable = 0;
    rcv = iem_raute2dollar(s);
    iem->iem_unexpandedReceive = rcv;
    rcv = canvas_realizedollar(iem->iem_glist, rcv);
    if(rcvable)
    {
        if(strcmp(rcv->s_name, iem->iem_receive->s_name))
        {
            if(iem->iem_flags.iem_canReceive)
                pd_unbind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
            iem->iem_receive = rcv;
            pd_bind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
        }
    }
    else if(!rcvable && iem->iem_flags.iem_canReceive)
    {
        pd_unbind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
        iem->iem_receive = rcv;
    }
    iem->iem_flags.iem_canReceive = rcvable;
    iem_verify_snd_ne_rcv(iem);
    (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_IO);
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

    old = iem->iem_label;
    iem->iem_unexpandedLabel = iem_raute2dollar(s);
    iem->iem_label = canvas_realizedollar(iem->iem_glist, iem->iem_unexpandedLabel);

    if(glist_isvisible(iem->iem_glist) && iem->iem_label != old)
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -text {%s} \n",
                 glist_getcanvas(iem->iem_glist), x,
                 strcmp(s->s_name, "empty")?iem->iem_label->s_name:"");
}

void iem_label_pos(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->iem_labelX = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    iem->iem_labelY = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(glist_isvisible(iem->iem_glist))
        sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
                 glist_getcanvas(iem->iem_glist), x,
                 text_xpix((t_object *)x,iem->iem_glist)+iem->iem_labelX,
                 text_ypix((t_object *)x,iem->iem_glist)+iem->iem_labelY);
}

void iem_label_font(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    int f = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(f < 4)
        f = 4;
    iem->iem_fontSize = f;
    if(glist_isvisible(iem->iem_glist))
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d]\n",
                 glist_getcanvas(iem->iem_glist), x,
                 iem->iem_fontSize);
}

void iem_size(void *x, t_iem *iem)
{
    if(glist_isvisible(iem->iem_glist))
    {
        (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_MOVE);
        canvas_fixlines(iem->iem_glist, (t_text*)x);
    }
}

void iem_delta(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->iem_obj.te_xCoordinate += (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    iem->iem_obj.te_yCoordinate += (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(glist_isvisible(iem->iem_glist))
    {
        (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_MOVE);
        canvas_fixlines(iem->iem_glist, (t_text*)x);
    }
}

void iem_pos(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->iem_obj.te_xCoordinate = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    iem->iem_obj.te_yCoordinate = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
    if(glist_isvisible(iem->iem_glist))
    {
        (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_MOVE);
        canvas_fixlines(iem->iem_glist, (t_text*)x);
    }
}

void iem_color(void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av)
{
    iem->iem_colorBackground = IEM_COLOR((t_int)atom_getFloatAtIndex(0, ac, av));
    if(ac > 2)
    {
        iem->iem_colorForeground = IEM_COLOR((t_int)atom_getFloatAtIndex(1, ac, av));
        iem->iem_colorLabel = IEM_COLOR((t_int)atom_getFloatAtIndex(2, ac, av));
    }
    else
        iem->iem_colorLabel = IEM_COLOR((t_int)atom_getFloatAtIndex(1, ac, av));
    if(glist_isvisible(iem->iem_glist))
        (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_CONFIG);
}

void iem_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_iem *x = (t_iem *)z;

    x->iem_obj.te_xCoordinate += dx;
    x->iem_obj.te_yCoordinate += dy;
    (*x->iem_draw)((void *)z, glist, IEM_DRAW_MOVE);
    canvas_fixlines(glist, (t_text *)z);
}

void iem_select(t_gobj *z, t_glist *glist, int selected)
{
    t_iem *x = (t_iem *)z;

    x->iem_flags.iem_isSelected = selected;
    (*x->iem_draw)((void *)z, glist, IEM_DRAW_SELECT);
}

void iem_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelines(glist, (t_text *)z);
}

void iem_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_iem *x = (t_iem *)z;

    if (vis)
        (*x->iem_draw)((void *)z, glist, IEM_DRAW_NEW);
    else
    {
        (*x->iem_draw)((void *)z, glist, IEM_DRAW_ERASE);
        interface_guiQueueRemove(z);
    }
}

void iem_save(t_iem *iem, t_symbol **srl, int *bflcol)
{
    srl[0] = iem->iem_send;
    srl[1] = iem->iem_receive;
    srl[2] = iem->iem_label;
    iem_all_sym2dollararg(iem, srl);
    iem_saveColors(iem, bflcol);
}

void iem_properties(t_iem *iem, t_symbol **srl)
{
    srl[0] = iem->iem_send;
    srl[1] = iem->iem_receive;
    srl[2] = iem->iem_label;
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

    if(IS_SYMBOL_INDEX(argv,7))
        srl[0] = atom_getSymbolAtIndex(7, argc, argv);
    else if(IS_FLOAT_INDEX(argv,7))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(7, argc, argv));
        srl[0] = gensym(str);
    }
    if(IS_SYMBOL_INDEX(argv,8))
        srl[1] = atom_getSymbolAtIndex(8, argc, argv);
    else if(IS_FLOAT_INDEX(argv,8))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(8, argc, argv));
        srl[1] = gensym(str);
    }
    if(IS_SYMBOL_INDEX(argv,9))
        srl[2] = atom_getSymbolAtIndex(9, argc, argv);
    else if(IS_FLOAT_INDEX(argv,9))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(9, argc, argv));
        srl[2] = gensym(str);
    }
    if(init != 0) init = 1;
    iem->x_isa.iem_initializeAtLoad = init;
    if(!strcmp(srl[0]->s_name, "empty")) sndable = 0;
    if(!strcmp(srl[1]->s_name, "empty")) rcvable = 0;
    iem_all_raute2dollar(srl);
    iem_all_dollararg2sym(iem, srl);
    if(rcvable)
    {
        if(strcmp(srl[1]->s_name, iem->iem_receive->s_name))
        {
            if(iem->iem_flags.iem_canReceive)
                pd_unbind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
            iem->iem_receive = srl[1];
            pd_bind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
        }
    }
    else if(!rcvable && iem->iem_flags.iem_canReceive)
    {
        pd_unbind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
        iem->iem_receive = srl[1];
    }
    iem->iem_send = srl[0];
    iem->iem_flags.iem_canSend = sndable;
    iem->iem_flags.iem_canReceive = rcvable;
    iem->iem_colorLabel = lcol & 0xffffff;
    iem->iem_colorForeground = fcol & 0xffffff;
    iem->iem_colorBackground = bcol & 0xffffff;
    iem->iem_label = srl[2];
    iem->iem_labelX = ldx;
    iem->iem_labelY = ldy;
    if(fs < 4)
        fs = 4;
    iem->iem_fontSize = fs;
    iem_verify_snd_ne_rcv(iem);
    canvas_dirty(iem->iem_glist, 1);
}

/* pre-0.46 the flags were 1 for 'loadinit' and 1<<20 for 'scale'.
Starting in 0.46, take either 1<<20 or 1<<1 for 'scale' and save to both
bits (so that old versions can read files we write).  In the future (2015?)
we can stop writing the annoying  1<<20 bit. */
#define LOADINIT 1
#define SCALE 2
#define SCALEBIS (1<<20)

void iem_inttosymargs(t_iemarguments *symargp, int n)
{
    memset(symargp, 0, sizeof(*symargp));
    symargp->iem_initializeAtLoad = ((n & LOADINIT) != 0);
    symargp->iem_scale = ((n & SCALE) || (n & SCALEBIS)) ;
    symargp->iem_flash = 0;
    symargp->iem_isLocked = 0;
}

int iem_symargstoint(t_iemarguments *symargp)
{
    return ((symargp->iem_initializeAtLoad ? LOADINIT : 0) | (symargp->iem_scale ? (SCALE | SCALEBIS) : 0));
}

void iem_inttofstyle(t_iemflags *fstylep, int n)
{
    memset (fstylep, 0, sizeof (*fstylep));
    fstylep->iem_font = (char)n;
}

int iem_fstyletoint(t_iemflags *fstylep)
{
    return (int)fstylep->iem_font;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
