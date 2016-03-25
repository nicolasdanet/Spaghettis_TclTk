
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

t_symbol *iemgui_empty (void)
{
    return gensym ("empty");
}

static t_symbol *iemgui_parseEmpty (t_symbol *s)
{
    if (s == &s_) { return iemgui_empty(); }
    else { 
        return s;
    }
}

static t_symbol *iemgui_expandDollar (t_iem *iem, t_symbol *s)
{
    t_symbol *t = canvas_realizedollar (iem, s); return  (t == NULL ? iemgui_empty() : t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Ensure compatibility with the original format. */
/* By the way legacy predefined colors are not supported. */
/* Only the 6 MSB are kept for each component. */

// RRRRRRRRGGGGGGGGBBBBBBBB 
// RRRRRR..GGGGGG..BBBBBB..
// RRRRRRGGGGGGBBBBBB

static int iemgui_colorEncode (int color)
{
    int n = 0;
    
    n |= ((0xfc0000 & color) >> 6);
    n |= ((0xfc00 & color) >> 4);
    n |= ((0xfc & color) >> 2);
                      
    return (-1 -n);
}

static int iemgui_colorDecode (int color)
{
    PD_ASSERT (color < 0);
    
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

/* Floats are loaded as integers (mainly in order to enumerate things). */

static t_symbol *iemgui_fetchName (int i, t_atom *argv)
{
    if (IS_SYMBOL_AT (argv, i))     { return (atom_getSymbol (argv + i)); }
    else if (IS_FLOAT_AT (argv, i)) {
        char t[PD_STRING];
        string_sprintf (t, PD_STRING, "%d", (int)atom_getFloat (argv + i));
        return gensym (t);
    } else {
        return iemgui_empty();
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Unexpended names cannot be fetch at instantiation due to already substituted arguments. */
/* Consequently it must be done by looking up again the object's text buffer. */

static void iemgui_fetchUnexpanded (t_iem *iem, t_symbol **s, int i, t_symbol *fallback)
{
    if (!*s) {
        t_error err = PD_ERROR;
        t_buffer *b = iem->iem_obj.te_buffer;
        if (i < buffer_size (b)) {
            char t[PD_STRING];
            if (!(err = atom_toString (buffer_atoms (b) + i, t, PD_STRING))) { *s = gensym (t); }
        }
        if (err) {
            *s = (fallback ? fallback : iemgui_empty());
        }
    }
}

static void iemgui_fetchUnexpandedNames (t_iem *iem, t_iemnames *s)
{
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedSend, iem->iem_cacheIndex + 1, iem->iem_send);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedReceive, iem->iem_cacheIndex + 2, iem->iem_receive);
    iemgui_fetchUnexpanded (iem, &iem->iem_unexpandedLabel, iem->iem_cacheIndex + 3, iem->iem_label);
        
    s->n_unexpendedSend    = iem->iem_unexpandedSend;
    s->n_unexpendedReceive = iem->iem_unexpandedReceive;
    s->n_unexpendedLabel   = iem->iem_unexpandedLabel;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_loadColors (t_iem *iem, t_iemcolors *c)
{
    c->c_colorBackground = iemgui_colorEncode (iem->iem_colorBackground);
    c->c_colorForeground = iemgui_colorEncode (iem->iem_colorForeground);
    c->c_colorLabel      = iemgui_colorEncode (iem->iem_colorLabel);
}

void iemgui_saveColors (t_iem *iem, t_iemcolors *c)
{
    iem->iem_colorBackground = iemgui_colorDecode (c->c_colorBackground);
    iem->iem_colorForeground = iemgui_colorDecode (c->c_colorForeground);
    iem->iem_colorLabel      = iemgui_colorDecode (c->c_colorLabel);
}

void iemgui_loadFontStyle (t_iem *iem, int n)
{
    iem->iem_fontStyle = (char)n;
}

int iemgui_saveFontStyle (t_iem *iem)
{
    return (int)iem->iem_fontStyle;
}

void iemgui_loadLoadOnStart (t_iem *iem, int n)
{
    iem->iem_loadOnStart = ((n & 1) != 0);
    iem->iem_scale = (n & 2);
}

int iemgui_saveLoadOnStart (t_iem *iem)
{
    return ((iem->iem_loadOnStart ? 1 : 0) | (iem->iem_scale ? 2 : 0));
}

void iemgui_loadNamesByIndex (t_iem *iem, int i, t_atom *argv)
{
    iem->iem_send    = (argv ? iemgui_fetchName (i + 0, argv) : iemgui_empty());
    iem->iem_receive = (argv ? iemgui_fetchName (i + 1, argv) : iemgui_empty());
    iem->iem_label   = (argv ? iemgui_fetchName (i + 2, argv) : iemgui_empty());
    
    iem->iem_unexpandedSend    = NULL;
    iem->iem_unexpandedReceive = NULL;
    iem->iem_unexpandedLabel   = NULL;
    
    iem->iem_cacheIndex = i;    /* Cache this index for later lookup. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_checkSendReceiveLoop (t_iem *iem)
{
    iem->iem_goThrough = 1;
    
    if (iem->iem_canSend && iem->iem_canReceive) {
        if (!strcmp (iem->iem_send->s_name, iem->iem_receive->s_name)) {
            iem->iem_goThrough = 0;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void iemgui_setSend (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromRaute (iemgui_parseEmpty (s));
    iem->iem_unexpandedSend = t;
    iem->iem_send = iemgui_expandDollar (iem->iem_glist, t);
    iem->iem_canSend = (s == iemgui_empty()) ? 0 : 1;
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setReceive (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromRaute (iemgui_parseEmpty (s));
    if (iem->iem_canReceive) { pd_unbind (pd_cast (iem), iem->iem_receive); }
    iem->iem_unexpandedReceive = t;
    iem->iem_receive = iemgui_expandDollar (iem->iem_glist, t);
    iem->iem_canReceive = (s == iemgui_empty()) ? 0 : 1;
    if (iem->iem_canReceive) { pd_bind (pd_cast (iem), iem->iem_receive); }
    iemgui_checkSendReceiveLoop (iem);
}

void iemgui_setLabel (void *x, t_iem *iem, t_symbol *s)
{
    t_symbol *t = dollar_fromRaute (iemgui_parseEmpty (s));
    iem->iem_unexpandedLabel = t;
    iem->iem_label = iemgui_expandDollar (iem->iem_glist, t);

    if (glist_isvisible (iem->iem_glist)) {
        sys_vGui (".x%lx.c itemconfigure %lxLABEL -text {%s}\n",
            glist_getcanvas (iem->iem_glist),
            x,
            iem->iem_label != iemgui_empty() ? iem->iem_label->s_name : s_.s_name);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

void iem_color (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv)
{
    if (glist_isvisible (iem->iem_glist)) { (*iem->iem_draw)(x, iem->iem_glist, IEM_DRAW_CONFIG); }
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

    x->iem_isSelected = selected;
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

void iem_save (t_iem *iem, t_symbol **srl, t_iemcolors *c)
{
    //srl[0] = iem->iem_send;
    //srl[1] = iem->iem_receive;
    //srl[2] = iem->iem_label;
    iemgui_fetchUnexpandedNames (iem, srl);
    iemgui_loadColors (iem, c);
}

void iem_properties (t_iem *iem, t_symbol **srl)
{
    //srl[0] = iem->iem_send;
    //srl[1] = iem->iem_receive;
    //srl[2] = iem->iem_label;
    iemgui_fetchUnexpandedNames (iem, srl);
    srl[0] = dollar_toRaute (srl[0]);
    srl[1] = dollar_toRaute (srl[1]);
    srl[2] = dollar_toRaute (srl[2]);
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

    if(IS_SYMBOL_AT(argv,7))
        srl[0] = atom_getSymbolAtIndex(7, argc, argv);
    else if(IS_FLOAT_AT(argv,7))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(7, argc, argv));
        srl[0] = gensym(str);
    }
    if(IS_SYMBOL_AT(argv,8))
        srl[1] = atom_getSymbolAtIndex(8, argc, argv);
    else if(IS_FLOAT_AT(argv,8))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(8, argc, argv));
        srl[1] = gensym(str);
    }
    if(IS_SYMBOL_AT(argv,9))
        srl[2] = atom_getSymbolAtIndex(9, argc, argv);
    else if(IS_FLOAT_AT(argv,9))
    {
        sprintf(str, "%d", (int)(t_int)atom_getFloatAtIndex(9, argc, argv));
        srl[2] = gensym(str);
    }
    if(init != 0) init = 1;
    iem->iem_loadOnStart = init;
    if(!strcmp(srl[0]->s_name, "empty")) sndable = 0;
    if(!strcmp(srl[1]->s_name, "empty")) rcvable = 0;
    srl[0] = dollar_fromRaute (srl[0]);
    srl[1] = dollar_fromRaute (srl[1]);
    srl[2] = dollar_fromRaute (srl[2]);
    
    iem->iem_unexpandedSend    = srl[0];
    iem->iem_unexpandedReceive = srl[1];
    iem->iem_unexpandedLabel   = srl[2];
    
    srl[0] = iemgui_expandDollar (iem->iem_glist, srl[0]);
    srl[1] = iemgui_expandDollar (iem->iem_glist, srl[1]);
    srl[2] = iemgui_expandDollar (iem->iem_glist, srl[2]);
    
    if(rcvable)
    {
        if(strcmp(srl[1]->s_name, iem->iem_receive->s_name))
        {
            if(iem->iem_canReceive)
                pd_unbind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
            iem->iem_receive = srl[1];
            pd_bind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
        }
    }
    else if(!rcvable && iem->iem_canReceive)
    {
        pd_unbind(&iem->iem_obj.te_g.g_pd, iem->iem_receive);
        iem->iem_receive = srl[1];
    }
    iem->iem_send = srl[0];
    iem->iem_canSend = sndable;
    iem->iem_canReceive = rcvable;
    iem->iem_colorLabel = lcol & 0xffffff;
    iem->iem_colorForeground = fcol & 0xffffff;
    iem->iem_colorBackground = bcol & 0xffffff;
    iem->iem_label = srl[2];
    iem->iem_labelX = ldx;
    iem->iem_labelY = ldy;
    if(fs < 4)
        fs = 4;
    iem->iem_fontSize = fs;
    iemgui_checkSendReceiveLoop(iem);
    canvas_dirty(iem->iem_glist, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
