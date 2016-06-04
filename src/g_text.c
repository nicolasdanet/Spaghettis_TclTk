
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_utf8.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;
extern t_class *gatom_class;
extern t_pd *pd_newest;

t_class *text_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void text_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_object *x = (t_object *)z;
    int width, height, iscomment = (x->te_type == TYPE_TEXT);
    t_float x1, y1, x2, y2;

        /* for number boxes, we know width and height a priori, and should
        report them here so that graphs can get swelled to fit. */
    
    if (x->te_type == TYPE_ATOM && x->te_width > 0)
    {
        int font = canvas_getFontSize(glist);
        int fontwidth = font_getHostFontWidth(font), fontheight = font_getHostFontHeight(font);
        width = (x->te_width > 0 ? x->te_width : 6) * fontwidth + 2;
        height = fontheight + 1; /* borrowed from TMARGIN, etc, in g_rtext.c */
    }
        /* if we're invisible we don't know our size so we just lie about
        it.  This is called on invisible boxes to establish order of inlets
        and possibly other reasons.
           To find out if the box is visible we can't just check the "vis"
        flag because we might be within the vis() routine and not have set
        that yet.  So we check directly whether the "rtext" list has been
        built.  LATER reconsider when "vis" flag should be on and off? */

    else if (glist->gl_editor && glist->gl_editor->e_text)
    {
        t_boxtext *y = boxtext_fetch(glist, x);
        width = boxtext_getWidth(y);
        height = boxtext_getHeight(y) - (iscomment << 1);
    }
    else width = height = 10;
    x1 = text_xpix(x, glist);
    y1 = text_ypix(x, glist);
    x2 = x1 + width;
    y2 = y1 + height;
    y1 += iscomment;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

void text_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_object *x = (t_object *)z;
    x->te_xCoordinate += dx;
    x->te_yCoordinate += dy;
    if (canvas_isMapped(glist))
    {
        t_boxtext *y = boxtext_fetch(glist, x);
        boxtext_displace(y, dx, dy);
        text_drawborder(x, glist, boxtext_getTag(y), 0);
        canvas_updateLinesByObject(glist, x);
    }
}

void text_select(t_gobj *z, t_glist *glist, int state)
{
    t_object *x = (t_object *)z;
    t_boxtext *y = boxtext_fetch(glist, x);
    boxtext_select(y, state);
    if (canvas_isMapped(glist) && gobj_isVisible(&x->te_g, glist))
        sys_vGui(".x%lx.c itemconfigure %sR -fill %s\n", glist, 
            boxtext_getTag(y), (state? "blue" : "black"));
}

void text_activate(t_gobj *z, t_glist *glist, int state)
{
    t_object *x = (t_object *)z;
    t_boxtext *y = boxtext_fetch(glist, x);
    if (z->g_pd != gatom_class) boxtext_activate (y, state);
}

void text_delete(t_gobj *z, t_glist *glist)
{
    t_object *x = (t_object *)z;
        canvas_deleteLinesByObject(glist, x);
}

void text_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_object *x = (t_object *)z;
    if (vis)
    {
        if (gobj_isVisible(&x->te_g, glist))
        {
            t_boxtext *y = boxtext_fetch(glist, x);
            if (x->te_type == TYPE_ATOM)
                glist_retext(glist, x);
            boxtext_draw(y);
            text_drawborder(x, glist, boxtext_getTag(y), 1);
        }
    }
    else
    {
        t_boxtext *y = boxtext_fetch(glist, x);
        if (gobj_isVisible(&x->te_g, glist))
        {
            boxtext_dirty (y);
            text_eraseborder(x, glist, boxtext_getTag(y));
            boxtext_erase(y);
        }
    }
}

int text_click(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int ctrl, int alt, int dbl, int doit)
{
    t_object *x = (t_object *)z;
    if (x->te_type == TYPE_OBJECT)
    {
        t_symbol *clicksym = sym_click;
        if (class_hasMethod (pd_class ((t_pd *)x), clicksym))
        {
            if (doit)
                pd_vMessage((t_pd *)x, clicksym, "fffff",
                    (double)xpix, (double)ypix,
                        (double)shift, (double)0, (double)alt);
            return (1);
        }
        else return (0);
    }
    else if (x->te_type == TYPE_ATOM)
    {
        if (doit)
            gatom_click((t_gatom *)x, (t_float)xpix, (t_float)ypix,
                (t_float)shift, (t_float)0, (t_float)alt);
        return (1);
    }
    else if (x->te_type == TYPE_MESSAGE)
    {
        if (doit)
            message_click((t_message *)x, (t_float)xpix, (t_float)ypix,
                (t_float)shift, (t_float)0, (t_float)alt);
        return (1);
    }
    else return (0);
}

void text_save(t_gobj *z, t_buffer *b)
{
    t_object *x = (t_object *)z;
    if (x->te_type == TYPE_OBJECT)
    {
            /* if we have a "saveto" method, and if we don't happen to be
            a canvas that's an abstraction, the saveto method does the work */
        if (class_hasMethod (pd_class ((t_pd *)x), sym__serialize) &&
            !((pd_class((t_pd *)x) == canvas_class) && 
                (canvas_isAbstraction ((t_glist *)x)
                    || canvas_istable((t_glist *)x))))
        {  
            mess1((t_pd *)x, sym__serialize, b);
            buffer_vAppend(b, "ssii", sym___hash__X, sym_restore,
                (int)x->te_xCoordinate, (int)x->te_yCoordinate);
        }
        else    /* otherwise just save the text */
        {
            buffer_vAppend(b, "ssii", sym___hash__X, sym_obj,
                (int)x->te_xCoordinate, (int)x->te_yCoordinate);
        }
        buffer_serialize(b, x->te_buffer);
    }
    else if (x->te_type == TYPE_MESSAGE)
    {
        buffer_vAppend(b, "ssii", sym___hash__X, sym_msg,
            (int)x->te_xCoordinate, (int)x->te_yCoordinate);
        buffer_serialize(b, x->te_buffer);
    }
    else if (x->te_type == TYPE_ATOM)
    {
        t_atomtype t = ((t_gatom *)x)->a_atom.a_type;
        t_symbol *sel = (t == A_SYMBOL ? sym_symbolatom :
            (t == A_FLOAT ? sym_floatatom : sym_intatom));
        
        t_symbol *label = dollar_toHash (utils_substituteIfEmpty (cast_gatom (x)->a_label, 1));
        t_symbol *symfrom = dollar_toHash (utils_substituteIfEmpty (cast_gatom (x)->a_unexpandedReceive, 1));
        t_symbol *symto = dollar_toHash (utils_substituteIfEmpty (cast_gatom (x)->a_unexpandedSend, 1));
        buffer_vAppend(b, "ssiiifffsss", sym___hash__X, sel,
            (int)x->te_xCoordinate, (int)x->te_yCoordinate, (int)x->te_width,
            (double)((t_gatom *)x)->a_lowRange,
            (double)((t_gatom *)x)->a_highRange,
            (double)((t_gatom *)x)->a_position,
            label, symfrom, symto);
    }           
    else        
    {
        buffer_vAppend(b, "ssii", sym___hash__X, sym_text,
            (int)x->te_xCoordinate, (int)x->te_yCoordinate);
        buffer_serialize(b, x->te_buffer);
    }
    if (x->te_width)
        buffer_vAppend(b, ",si", sym_f, (int)x->te_width);
    buffer_vAppend(b, ";");
}

    /* this one is for everyone but "gatoms"; it's imposed in m_class.c */
t_widgetbehavior text_widgetBehavior =      /* Shared. */
{
    text_getrect,
    text_displace,
    text_select,
    text_activate,
    text_delete,
    text_vis,
    text_click
};

/* -------------------- the "text" class  ------------ */

#ifdef __APPLE__
#define EXTRAPIX 2
#else
#define EXTRAPIX 1
#endif

    /* draw inlets and outlets for a text object or for a graph. */
void glist_drawio(t_glist *glist, t_object *ob, int firsttime,
    char *tag, int x1, int y1, int x2, int y2)
{
    int n = object_numberOfOutlets(ob), nplus = (n == 1 ? 1 : n-1), i;
    int width = x2 - x1;
    for (i = 0; i < n; i++)
    {
        int onset = x1 + (width - INLET_WIDTH) * i / nplus;
        if (firsttime)
            sys_vGui(".x%lx.c create rectangle %d %d %d %d \
-tags [list %so%d outlet]\n",
                canvas_getView(glist),
                onset, y2 - 1,
                onset + INLET_WIDTH, y2,
                tag, i);
        else
            sys_vGui(".x%lx.c coords %so%d %d %d %d %d\n",
                canvas_getView(glist), tag, i,
                onset, y2 - 1,
                onset + INLET_WIDTH, y2);
    }
    n = object_numberOfInlets(ob);
    nplus = (n == 1 ? 1 : n-1);
    for (i = 0; i < n; i++)
    {
        int onset = x1 + (width - INLET_WIDTH) * i / nplus;
        if (firsttime)
            sys_vGui(".x%lx.c create rectangle %d %d %d %d \
-tags [list %si%d inlet]\n",
                canvas_getView(glist),
                onset, y1,
                onset + INLET_WIDTH, y1 + EXTRAPIX,
                tag, i);
        else
            sys_vGui(".x%lx.c coords %si%d %d %d %d %d\n",
                canvas_getView(glist), tag, i,
                onset, y1,
                onset + INLET_WIDTH, y1 + EXTRAPIX);
    }
}

void text_drawborder(t_object *x, t_glist *glist,
    char *tag, int firsttime)
{
    t_object *ob;
    int x1, y1, x2, y2, width, height;
    text_getrect(&x->te_g, glist, &x1, &y1, &x2, &y2);
    width = x2 - x1;
    height = y2 - y1;
    
    if (x->te_type == TYPE_OBJECT)
    {
        char *pattern = ((pd_class((t_pd *)x) == text_class) ? "-" : "\"\"");
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d -dash %s -tags [list %sR obj]\n",
                canvas_getView(glist),
                    x1, y1,  x2, y1,  x2, y2,  x1, y2,  x1, y1,  pattern, tag);
        else
        {
            sys_vGui(".x%lx.c coords %sR\
 %d %d %d %d %d %d %d %d %d %d\n",
                canvas_getView(glist), tag,
                    x1, y1,  x2, y1,  x2, y2,  x1, y2,  x1, y1);
            sys_vGui(".x%lx.c itemconfigure %sR -dash %s\n",
                canvas_getView(glist), tag, pattern);
        }
    }
    else if (x->te_type == TYPE_MESSAGE)
    {
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d %d %d %d %d -tags [list %sR msg]\n",
                canvas_getView(glist),
                x1, y1,  x2+4, y1,  x2, y1+4,  x2, y2-4,  x2+4, y2,
                x1, y2,  x1, y1,
                    tag);
        else
            sys_vGui(".x%lx.c coords %sR\
 %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                canvas_getView(glist), tag,
                x1, y1,  x2+4, y1,  x2, y1+4,  x2, y2-4,  x2+4, y2,
                x1, y2,  x1, y1);
    }
    else if (x->te_type == TYPE_ATOM)
    {
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d %d %d -tags [list %sR atom]\n",
                canvas_getView(glist),
                x1, y1,  x2-4, y1,  x2, y1+4,  x2, y2,  x1, y2,  x1, y1,
                    tag);
        else
            sys_vGui(".x%lx.c coords %sR\
 %d %d %d %d %d %d %d %d %d %d %d %d\n",
                canvas_getView(glist), tag,
                x1, y1,  x2-4, y1,  x2, y1+4,  x2, y2,  x1, y2,  x1, y1);
    }
        /* for comments, just draw a bar on RHS if unlocked; when a visible
        canvas is unlocked we have to call this anew on all comments, and when
        locked we erase them all via the annoying "COMMENTBAR" tag. */
    else if (x->te_type == TYPE_TEXT && glist->gl_isEditMode)
    {
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d -tags [list %sR COMMENTBAR]\n",
                canvas_getView(glist),
                x2, y1,  x2, y2, tag);
        else
            sys_vGui(".x%lx.c coords %sR %d %d %d %d\n",
                canvas_getView(glist), tag, x2, y1,  x2, y2);
    }
        /* draw inlets/outlets */
    
    if (ob = canvas_castToObjectIfPatchable((t_pd *)x))
        glist_drawio(glist, ob, firsttime, tag, x1, y1, x2, y2);
}

void glist_eraseio(t_glist *glist, t_object *ob, char *tag)
{
    int i, n;
    n = object_numberOfOutlets(ob);
    for (i = 0; i < n; i++)
        sys_vGui(".x%lx.c delete %so%d\n",
            canvas_getView(glist), tag, i);
    n = object_numberOfInlets(ob);
    for (i = 0; i < n; i++)
        sys_vGui(".x%lx.c delete %si%d\n",
            canvas_getView(glist), tag, i);
}

void text_eraseborder(t_object *x, t_glist *glist, char *tag)
{
    if (x->te_type == TYPE_TEXT && !glist->gl_isEditMode) return;
    sys_vGui(".x%lx.c delete %sR\n",
        canvas_getView(glist), tag);
    glist_eraseio(glist, x, tag);
}

    /* change text; if TYPE_OBJECT, remake it.  */
void text_setto(t_object *x, t_glist *glist, char *buf, int bufsize)
{
    if (x->te_type == TYPE_OBJECT)
    {
        t_buffer *b = buffer_new();
        int natom1, natom2, widthwas = x->te_width;
        t_atom *vec1, *vec2;
        buffer_withStringUnzeroed(b, buf, bufsize);
        natom1 = buffer_size(x->te_buffer);
        vec1 = buffer_atoms(x->te_buffer);
        natom2 = buffer_size(b);
        vec2 = buffer_atoms(b);
            /* special case: if  pd args change just pass the message on. */
        if (natom1 >= 1 && natom2 >= 1 && vec1[0].a_type == A_SYMBOL
            && !strcmp(vec1[0].a_w.w_symbol->s_name, "pd") &&
             vec2[0].a_type == A_SYMBOL
            && !strcmp(vec2[0].a_w.w_symbol->s_name, "pd"))
        {
            pd_message((t_pd *)x, sym_rename, natom2-1, vec2+1);
            buffer_free(x->te_buffer);
            x->te_buffer = b;
        }
        else  /* normally, just destroy the old one and make a new one. */
        {
            int xwas = x->te_xCoordinate, ywas = x->te_yCoordinate;
            glist_delete(glist, &x->te_g);
            canvas_makeTextObject(glist, xwas, ywas, widthwas, 0, b);
            canvas_restoreCachedLines (canvas_getView(glist));
                /* if it's an abstraction loadbang it here */
            if (pd_newest && pd_class(pd_newest) == canvas_class)
                canvas_loadbang((t_glist *)pd_newest);
        }
            /* if we made a new "pd" or changed a window name,
                update window list */
        /*if (natom2 >= 1  && vec2[0].a_type == A_SYMBOL
            && !strcmp(vec2[0].a_w.w_symbol->s_name, "pd"))
                canvas_updatewindowlist();*/
    }
    else buffer_withStringUnzeroed(x->te_buffer, buf, bufsize);
}

    /* this gets called when a message gets sent to an object whose creation
    failed, presumably because of loading a patch with a missing extern or
    abstraction */
static void text_anything(t_object *x, t_symbol *s, int argc, t_atom *argv)
{
}

void text_setup(void)
{
    text_class = class_new (sym_text, 0, 0, sizeof(t_object),
        CLASS_NOINLET | CLASS_DEFAULT, 0);
    class_addAnything(text_class, text_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
