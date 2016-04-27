/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdlib.h>
#include <stdio.h>
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"
#include "s_utf8.h" /*-- moo --*/
#include <string.h>
#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
#define snprintf sprintf_s
#endif

extern t_class *garray_class;
extern t_class *scalar_class;
extern t_pd pd_canvasMaker;
extern t_class *canvas_class;
extern t_class *vinlet_class;
extern t_class *voutlet_class;
extern t_widgetbehavior text_widgetBehavior;
extern t_pdinstance *pd_this;
extern t_symbol *main_directoryHelp;

void glist_readfrombinbuf(t_glist *x, t_buffer *b, char *filename,
    int selectem);

/* ------------------ forward declarations --------------- */
static void canvas_doclear(t_glist *x);
static void glist_setlastxy(t_glist *gl, int xval, int yval);
static void glist_donewloadbangs(t_glist *x);
static t_buffer *canvas_docopy(t_glist *x);
static void canvas_dopaste(t_glist *x, t_buffer *b);
static void canvas_paste(t_glist *x);
static void canvas_clearline(t_glist *x);
static t_buffer *copy_binbuf;
static char *canvas_textcopybuf;
static int canvas_textcopybufsize;
static t_glist *glist_finddirty(t_glist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void file_openHelp (const char *directory, const char *name)
{
    int f = -1;
    char *nameResult = NULL;
    char directoryResult[PD_STRING] = { 0 };
    
    if (*directory != 0) { 
        f = file_openWithDirectoryAndName (directory, name, PD_HELP, directoryResult, &nameResult, PD_STRING);
    }
    
    if (f < 0) {
        char *help = main_directoryHelp->s_name;
        f = file_openConsideringSearchPath (help, name, PD_HELP, directoryResult, &nameResult, PD_STRING);
    }
    
    if (f < 0) { post_error (PD_TRANSLATE ("help: couldn't find file for '%s'"), name); }
    else {
        close (f);
        buffer_openFile (NULL, gensym (nameResult), gensym (directoryResult));
    }
}

/* ---------------- generic widget behavior ------------------------- */

void gobj_getrect(t_gobj *x, t_glist *glist, int *x1, int *y1,
    int *x2, int *y2)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnGetRectangle)
        (*x->g_pd->c_behavior->w_fnGetRectangle)(x, glist, x1, y1, x2, y2);
}

void gobj_displace(t_gobj *x, t_glist *glist, int dx, int dy)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnDisplace)
        (*x->g_pd->c_behavior->w_fnDisplace)(x, glist, dx, dy);
}

void gobj_select(t_gobj *x, t_glist *glist, int state)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnSelect)
        (*x->g_pd->c_behavior->w_fnSelect)(x, glist, state);
}

void gobj_activate(t_gobj *x, t_glist *glist, int state)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnActivate)
        (*x->g_pd->c_behavior->w_fnActivate)(x, glist, state);
}

void gobj_delete(t_gobj *x, t_glist *glist)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnDelete)
        (*x->g_pd->c_behavior->w_fnDelete)(x, glist);
}

void gobj_save (t_gobj *x, t_buffer *b)
{
    t_class *c = x->g_pd;
    if (c->c_fnSave)
        (c->c_fnSave)(x, b);
}

int gobj_shouldvis(t_gobj *x, struct _glist *glist)
{
    t_object *ob;
            /* if our parent is a graph, and if that graph itself isn't
            visible, then we aren't either. */
    if (!glist->gl_haveWindow && glist->gl_isGraphOnParent && glist->gl_owner
        && !gobj_shouldvis(&glist->gl_obj.te_g, glist->gl_owner))
            return (0);
            /* if we're graphing-on-parent and the object falls outside the
            graph rectangle, don't draw it. */
    if (!glist->gl_haveWindow && glist->gl_isGraphOnParent && glist->gl_hasRectangle &&
        glist->gl_owner)
    {
        int x1, y1, x2, y2, gx1, gy1, gx2, gy2, m;
            /* for some reason the bounds check on arrays and scalars
            don't seem to apply here.  Perhaps this was in order to allow
            arrays to reach outside their containers?  I no longer understand
            this. */
        if (pd_class(&x->g_pd) == scalar_class
            || pd_class(&x->g_pd) == garray_class)
                return (1);
        gobj_getrect(&glist->gl_obj.te_g, glist->gl_owner, &x1, &y1, &x2, &y2);
        if (x1 > x2)
            m = x1, x1 = x2, x2 = m;
        if (y1 > y2)
            m = y1, y1 = y2, y2 = m;
        gobj_getrect(x, glist, &gx1, &gy1, &gx2, &gy2);
        /* post("graph %d %d %d %d, %s %d %d %d %d",
            x1, x2, y1, y2, class_getHelpName(x->g_pd), gx1, gx2, gy1, gy2); */
        if (gx1 < x1 || gx1 > x2 || gx2 < x1 || gx2 > x2 ||
            gy1 < y1 || gy1 > y2 || gy2 < y1 || gy2 > y2)
                return (0);
    }
    if (ob = canvas_castToObjectIfBox(&x->g_pd))
    {
        /* return true if the text box should be drawn.  We don't show text
        boxes inside graphs---except comments, if we're doing the new
        (goprect) style. */
        return (glist->gl_haveWindow ||
            (ob->te_g.g_pd != canvas_class &&
                ob->te_g.g_pd->c_behavior != &text_widgetBehavior) ||
            (ob->te_g.g_pd == canvas_class && (((t_glist *)ob)->gl_isGraphOnParent)) ||
            (glist->gl_hasRectangle && (ob->te_type == TYPE_TEXT)));
    }
    else return (1);
}

void gobj_vis(t_gobj *x, struct _glist *glist, int flag)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnVisible && gobj_shouldvis(x, glist))
        (*x->g_pd->c_behavior->w_fnVisible)(x, glist, flag);
}

int gobj_click(t_gobj *x, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnClick)
        return ((*x->g_pd->c_behavior->w_fnClick)(x,
            glist, xpix, ypix, shift, alt, dbl, doit));
    else return (0);
}

/* ------------------------ managing the selection ----------------- */

void glist_selectline(t_glist *x, t_outconnect *oc, int index1,
    int outno, int index2, int inno)
{
    if (x->gl_editor)
    {
        glist_noselect(x);
        x->gl_editor->e_selectedline = 1;
        x->gl_editor->e_selectline_index1 = index1;
        x->gl_editor->e_selectline_outno = outno;
        x->gl_editor->e_selectline_index2 = index2;
        x->gl_editor->e_selectline_inno = inno;
        x->gl_editor->e_selectline_tag = oc;
        sys_vGui(".x%lx.c itemconfigure %lxLINE -fill blue\n",
            x, x->gl_editor->e_selectline_tag);
    }    
}

void glist_deselectline(t_glist *x)
{
    if (x->gl_editor)
    {
        x->gl_editor->e_selectedline = 0;
        sys_vGui(".x%lx.c itemconfigure %lxLINE -fill black\n",
            x, x->gl_editor->e_selectline_tag);
    }    
}

int glist_isselected(t_glist *x, t_gobj *y)
{
    if (x->gl_editor)
    {
        t_selection *sel;
        for (sel = x->gl_editor->e_selection; sel; sel = sel->sel_next)
            if (sel->sel_what == y) return (1);
    }
    return (0);
}

    /* call this for unselected objects only */
void glist_select(t_glist *x, t_gobj *y)
{
    if (x->gl_editor)
    {
        t_selection *sel = (t_selection *)PD_MEMORY_GET(sizeof(*sel));
        if (x->gl_editor->e_selectedline)
            glist_deselectline(x);
            /* LATER #ifdef out the following check */
        if (glist_isselected(x, y)) { PD_BUG; }
        sel->sel_next = x->gl_editor->e_selection;
        sel->sel_what = y;
        x->gl_editor->e_selection = sel;
        gobj_select(y, x, 1);
    }
}

    /* recursively deselect everything in a gobj "g", if it happens to be
    a glist, in preparation for deselecting g itself in glist_dselect() */
static void glist_checkanddeselectall(t_glist *gl, t_gobj *g)
{
    t_glist *gl2;
    t_gobj *g2;
    if (pd_class(&g->g_pd) != canvas_class)
        return;
    gl2 = (t_glist *)g;
    for (g2 = gl2->gl_graphics; g2; g2 = g2->g_next)
        glist_checkanddeselectall(gl2, g2);
    glist_noselect(gl2);
}

    /* call this for selected objects only */
void glist_deselect(t_glist *x, t_gobj *y)
{
    int fixdsp = 0;
    if (x->gl_editor)
    {
        t_selection *sel, *sel2;
        t_boxtext *z = 0;
        if (!glist_isselected(x, y)) { PD_BUG; }
        if (x->gl_editor->e_textedfor)
        {
            t_boxtext *fuddy = glist_findrtext(x, (t_object *)y);
            if (x->gl_editor->e_textedfor == fuddy)
            {
                if (x->gl_editor->e_textdirty)
                {
                    z = fuddy;
                    canvas_stowconnections(glist_getcanvas(x));
                    glist_checkanddeselectall(x, y);
                }
                gobj_activate(y, x, 0);
            }
            if (class_hasMethod (pd_class (&y->g_pd), gensym ("dsp")))
                fixdsp = dsp_suspend();
        }
        if ((sel = x->gl_editor->e_selection)->sel_what == y)
        {
            x->gl_editor->e_selection = x->gl_editor->e_selection->sel_next;
            gobj_select(sel->sel_what, x, 0);
            PD_MEMORY_FREE(sel);
        }
        else
        {
            for (sel = x->gl_editor->e_selection; sel2 = sel->sel_next;
                sel = sel2)
            {
                if (sel2->sel_what == y)
                {
                    sel->sel_next = sel2->sel_next;
                    gobj_select(sel2->sel_what, x, 0);
                    PD_MEMORY_FREE(sel2);
                    break;
                }
            }
        }
        if (z)
        {
            char *buf;
            int bufsize;

            rtext_gettext(z, &buf, &bufsize);
            text_setto((t_object *)y, x, buf, bufsize);
            canvas_updateLinesByObject(x, (t_object *)y);
            x->gl_editor->e_textedfor = 0;
        }
        if (fixdsp)
            dsp_resume(1);
    }
}

void glist_noselect(t_glist *x)
{
    if (x->gl_editor)
    {
        while (x->gl_editor->e_selection)
            glist_deselect(x, x->gl_editor->e_selection->sel_what);
        if (x->gl_editor->e_selectedline)
            glist_deselectline(x);
    }
}

void glist_selectall(t_glist *x)
{
    if (x->gl_editor)
    {
        glist_noselect(x);
        if (x->gl_graphics)
        {
            t_selection *sel = (t_selection *)PD_MEMORY_GET(sizeof(*sel));
            t_gobj *y = x->gl_graphics;
            x->gl_editor->e_selection = sel;
            sel->sel_what = y;
            gobj_select(y, x, 1);
            while (y = y->g_next)
            {
                t_selection *sel2 = (t_selection *)PD_MEMORY_GET(sizeof(*sel2));
                sel->sel_next = sel2;
                sel = sel2;
                sel->sel_what = y;
                gobj_select(y, x, 1);
            }
            sel->sel_next = 0;
        }
    }
}

    /* get the index of a gobj in a glist.  If y is zero, return the
    total number of objects. */
int glist_getindex(t_glist *x, t_gobj *y)
{
    t_gobj *y2;
    int indx;

    for (y2 = x->gl_graphics, indx = 0; y2 && y2 != y; y2 = y2->g_next)
        indx++;
    return (indx);
}

    /* get the index of the object, among selected items, if "selected"
    is set; otherwise, among unselected ones.  If y is zero, just
    counts the selected or unselected objects. */
int glist_selectionindex(t_glist *x, t_gobj *y, int selected)
{
    t_gobj *y2;
    int indx;

    for (y2 = x->gl_graphics, indx = 0; y2 && y2 != y; y2 = y2->g_next)
        if (selected == glist_isselected(x, y2))
            indx++;
    return (indx);
}

static t_gobj *glist_nth(t_glist *x, int n)
{
    t_gobj *y;
    int indx;
    for (y = x->gl_graphics, indx = 0; y; y = y->g_next, indx++)
        if (indx == n)
            return (y);
    return (0);
}

/* ------------------- support for undo/redo  -------------------------- */

static t_undofn canvas_undo_fn;         /* current undo function if any */
static int canvas_undo_whatnext;        /* whether we can now UNDO or REDO */
static void *canvas_undo_buf;           /* data private to the undo function */
static t_glist *canvas_undo_canvas;    /* which canvas we can undo on */
static const char *canvas_undo_name;

void canvas_setundo(t_glist *x, t_undofn undofn, void *buf,
    const char *name)
{
    int hadone = 0;
        /* blow away the old undo information.  In one special case the
        old undo info is re-used; if so we shouldn't free it here. */
    if (canvas_undo_fn && canvas_undo_buf && (buf != canvas_undo_buf))
    {
        (*canvas_undo_fn)(canvas_undo_canvas, canvas_undo_buf, UNDO_FREE);
        hadone = 1;
    }
    canvas_undo_canvas = x;
    canvas_undo_fn = undofn;
    canvas_undo_buf = buf;
    canvas_undo_whatnext = UNDO_UNDO;
    canvas_undo_name = name;
    /* if (x && canvas_isVisible(x) && canvas_isTopLevel(x))
            //
        sys_vGui("pdtk_undomenu .x%lx %s no\n", x, name);
    else if (hadone)
        sys_vGui("pdtk_undomenu nobody no no\n"); */
}

    /* clear undo if it happens to be for the canvas x.
     (but if x is 0, clear it regardless of who owns it.) */
void canvas_noundo(t_glist *x)
{
    if (!x || (x == canvas_undo_canvas))
        canvas_setundo(0, 0, 0, "foo");
}

static void canvas_undo(t_glist *x)
{
    int dspwas = dsp_suspend();
    if (x != canvas_undo_canvas) { PD_BUG; }
    else if (canvas_undo_whatnext != UNDO_UNDO) { PD_BUG; }
    else
    {
        /* post("undo"); */
        (*canvas_undo_fn)(canvas_undo_canvas, canvas_undo_buf, UNDO_UNDO);
            /* enable redo in menu */
        /*if (canvas_isVisible(x) && canvas_isTopLevel(x))
            sys_vGui("pdtk_undomenu .x%lx no %s\n", x, canvas_undo_name); */
        canvas_undo_whatnext = UNDO_REDO;
    }
    dsp_resume(dspwas);
}

static void canvas_redo(t_glist *x)
{
    int dspwas = dsp_suspend();
    if (x != canvas_undo_canvas) { PD_BUG; }
    else if (canvas_undo_whatnext != UNDO_REDO) { PD_BUG; }
    else
    {
        /* post("redo"); */
        (*canvas_undo_fn)(canvas_undo_canvas, canvas_undo_buf, UNDO_REDO);
            /* enable undo in menu */
        /*if (canvas_isVisible(x) && canvas_isTopLevel(x))
            sys_vGui("pdtk_undomenu .x%lx %s no\n", x, canvas_undo_name);*/
        canvas_undo_whatnext = UNDO_UNDO;
    }
    dsp_resume(dspwas);
}

/* ------- specific undo methods: 1. connect and disconnect -------- */

typedef struct _undo_connect    
{
    int u_index1;
    int u_outletno;
    int u_index2;
    int u_inletno;
} t_undo_connect;

static void *canvas_undo_set_disconnect(t_glist *x,
    int index1, int outno, int index2, int inno)
{
    t_undo_connect *buf = (t_undo_connect *)PD_MEMORY_GET(sizeof(*buf));
    buf->u_index1 = index1;
    buf->u_outletno = outno;
    buf->u_index2 = index2;
    buf->u_inletno = inno;
    return (buf);
}

void canvas_disconnect(t_glist *x,
    t_float index1, t_float outno, t_float index2, t_float inno)
{
    t_linetraverser t;
    t_outconnect *oc;
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        int srcno = canvas_getIndexOfObject(x, &t.tr_srcObject->te_g);
        int sinkno = canvas_getIndexOfObject(x, &t.tr_destObject->te_g);
        if (srcno == index1 && t.tr_srcIndexOfOutlet == outno &&
            sinkno == index2 && t.tr_destIndexOfInlet == inno)
        {
            sys_vGui(".x%lx.c delete %lxLINE\n", x, oc);
            object_disconnect(t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
            break;
        }
    }
}

static void canvas_undo_disconnect(t_glist *x, void *z, int action)
{
    t_undo_connect *buf = z;
    if (action == UNDO_UNDO)
    {
        canvas_connect(x, buf->u_index1, buf->u_outletno,
            buf->u_index2, buf->u_inletno);
    }
    else if (action == UNDO_REDO)
    {
        canvas_disconnect(x, buf->u_index1, buf->u_outletno,
            buf->u_index2, buf->u_inletno);
    }
    else if (action == UNDO_FREE)
        PD_MEMORY_FREE(buf);
}

    /* connect just calls disconnect actions backward... */
static void *canvas_undo_set_connect(t_glist *x,
    int index1, int outno, int index2, int inno)
{
    return (canvas_undo_set_disconnect(x, index1, outno, index2, inno));
}

static void canvas_undo_connect(t_glist *x, void *z, int action)
{
    int myaction;
    if (action == UNDO_UNDO)
        myaction = UNDO_REDO;
    else if (action == UNDO_REDO)
        myaction = UNDO_UNDO;
    else myaction = action;
    canvas_undo_disconnect(x, z, myaction);
}

/* ---------- ... 2. cut, clear, and typing into objects: -------- */

#define UCUT_CUT 1          /* operation was a cut */
#define UCUT_CLEAR 2        /* .. a clear */
#define UCUT_TEXT 3         /* text typed into a box */

typedef struct _undo_cut        
{
    t_buffer *u_objectbuf;      /* the object cleared or typed into */
    t_buffer *u_reconnectbuf;   /* connections into and out of object */
    t_buffer *u_redotextbuf;    /* buffer to paste back for redo if TEXT */
    int u_mode;                 /* from flags above */
} t_undo_cut;

static void *canvas_undo_set_cut(t_glist *x, int mode)
{
    t_undo_cut *buf;
    t_gobj *y;
    t_linetraverser t;
    t_outconnect *oc;
    int nnotsel= glist_selectionindex(x, 0, 0);
    buf = (t_undo_cut *)PD_MEMORY_GET(sizeof(*buf));
    buf->u_mode = mode;
    buf->u_redotextbuf = 0;

        /* store connections into/out of the selection */
    buf->u_reconnectbuf = buffer_new();
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        int issel1 = glist_isselected(x, &t.tr_srcObject->te_g);
        int issel2 = glist_isselected(x, &t.tr_destObject->te_g);
        if (issel1 != issel2)
        {
            buffer_vAppend(buf->u_reconnectbuf, "ssiiii;",
                gensym ("#X"), gensym ("connect"),
                (issel1 ? nnotsel : 0)
                    + glist_selectionindex(x, &t.tr_srcObject->te_g, issel1),
                t.tr_srcIndexOfOutlet,
                (issel2 ? nnotsel : 0) +
                    glist_selectionindex(x, &t.tr_destObject->te_g, issel2),
                t.tr_destIndexOfInlet);
        }
    }
    if (mode == UCUT_TEXT)
    {
        buf->u_objectbuf = canvas_docopy(x);
    }
    else if (mode == UCUT_CUT)
    {
        buf->u_objectbuf = 0;
    }
    else if (mode == UCUT_CLEAR)
    {
        buf->u_objectbuf = canvas_docopy(x);
    }
    return (buf);
}

static void canvas_undo_cut(t_glist *x, void *z, int action)
{
    t_undo_cut *buf = z;
    int mode = buf->u_mode;
    t_pd *boundx = s__X.s_thing;
    if (action == UNDO_UNDO)
    {
        if (mode == UCUT_CUT)
            canvas_dopaste(x, copy_binbuf);
        else if (mode == UCUT_CLEAR)
            canvas_dopaste(x, buf->u_objectbuf);
        else if (mode == UCUT_TEXT)
        {
            t_gobj *y1, *y2;
            glist_noselect(x);
            for (y1 = x->gl_graphics; y2 = y1->g_next; y1 = y2)
                ;
            if (y1)
            {
                if (!buf->u_redotextbuf)
                {
                    glist_noselect(x);
                    glist_select(x, y1);
                    buf->u_redotextbuf = canvas_docopy(x);
                    glist_noselect(x);
                }
                glist_delete(x, y1);
            }
            canvas_dopaste(x, buf->u_objectbuf);
        }
        s__X.s_thing = &x->gl_obj.te_g.g_pd;
        buffer_eval(buf->u_reconnectbuf, 0, 0, 0);
        s__X.s_thing = boundx;
    }
    else if (action == UNDO_REDO)
    {
        if (mode == UCUT_CUT || mode == UCUT_CLEAR)
            canvas_doclear(x);
        else if (mode == UCUT_TEXT)
        {
            t_gobj *y1, *y2;
            for (y1 = x->gl_graphics; y2 = y1->g_next; y1 = y2)
                ;
            if (y1)
                glist_delete(x, y1);
            canvas_dopaste(x, buf->u_redotextbuf);
            s__X.s_thing = &x->gl_obj.te_g.g_pd;
            buffer_eval(buf->u_reconnectbuf, 0, 0, 0);
            s__X.s_thing = boundx;
        }
    }
    else if (action == UNDO_FREE)
    {
        if (buf->u_objectbuf)
            buffer_free(buf->u_objectbuf);
        if (buf->u_reconnectbuf)
            buffer_free(buf->u_reconnectbuf);
        if (buf->u_redotextbuf)
            buffer_free(buf->u_redotextbuf);
        PD_MEMORY_FREE(buf);
    }
}

/* --------- 3. motion, including "tidy up" and stretching ----------- */

typedef struct _undo_move_elem  
{
    int e_index;
    int e_xpix;
    int e_ypix;
} t_undo_move_elem;

typedef struct _undo_move       
{
    t_undo_move_elem *u_vec;
    int u_n;
} t_undo_move;

static int canvas_undo_already_set_move;

static void *canvas_undo_set_move(t_glist *x, int selected)
{
    int x1, y1, x2, y2, i, indx;
    t_gobj *y;
    t_undo_move *buf =  (t_undo_move *)PD_MEMORY_GET(sizeof(*buf));
    buf->u_n = selected ? glist_selectionindex(x, 0, 1) : glist_getindex(x, 0);
    buf->u_vec = (t_undo_move_elem *)PD_MEMORY_GET(sizeof(*buf->u_vec) *
        (selected ? glist_selectionindex(x, 0, 1) : glist_getindex(x, 0)));
    if (selected)
    {
        for (y = x->gl_graphics, i = indx = 0; y; y = y->g_next, indx++)
            if (glist_isselected(x, y))
        {
            gobj_getrect(y, x, &x1, &y1, &x2, &y2);
            buf->u_vec[i].e_index = indx;
            buf->u_vec[i].e_xpix = x1;
            buf->u_vec[i].e_ypix = y1;
            i++;
        }
    }
    else
    {
        for (y = x->gl_graphics, indx = 0; y; y = y->g_next, indx++)
        {
            gobj_getrect(y, x, &x1, &y1, &x2, &y2);
            buf->u_vec[indx].e_index = indx;
            buf->u_vec[indx].e_xpix = x1;
            buf->u_vec[indx].e_ypix = y1;
        }
    }
    canvas_undo_already_set_move = 1;
    return (buf);
}

static void canvas_undo_move(t_glist *x, void *z, int action)
{
    t_undo_move *buf = z;
    if (action == UNDO_UNDO || action == UNDO_REDO)
    {
        int i;
        for (i = 0; i < buf->u_n; i++)
        {
            int x1, y1, x2, y2, newx, newy;
            t_gobj *y;
            newx = buf->u_vec[i].e_xpix;
            newy = buf->u_vec[i].e_ypix;
            y = glist_nth(x, buf->u_vec[i].e_index);
            if (y)
            {
                gobj_getrect(y, x, &x1, &y1, &x2, &y2);
                gobj_displace(y, x, newx-x1, newy - y1);
                buf->u_vec[i].e_xpix = x1;
                buf->u_vec[i].e_ypix = y1;
            }
        }
    }
    else if (action == UNDO_FREE)
    {
        PD_MEMORY_FREE(buf->u_vec);
        PD_MEMORY_FREE(buf);
    }
}

/* --------- 4. paste (also duplicate) ----------- */

typedef struct _undo_paste      
{
    int u_index;    /* index of first object pasted */  
} t_undo_paste;

static void *canvas_undo_set_paste(t_glist *x)
{
    t_undo_paste *buf =  (t_undo_paste *)PD_MEMORY_GET(sizeof(*buf));
    buf->u_index = glist_getindex(x, 0);
    return (buf);
}

static void canvas_undo_paste(t_glist *x, void *z, int action)
{
    t_undo_paste *buf = z;
    if (action == UNDO_UNDO)
    {
        t_gobj *y;
        glist_noselect(x);
        for (y = glist_nth(x, buf->u_index); y; y = y->g_next)
            glist_select(x, y);
        canvas_doclear(x);
    }
    else if (action == UNDO_REDO)
    {
        t_selection *sel;
        canvas_dopaste(x, copy_binbuf);
            /* if it was "duplicate" have to re-enact the displacement. */
        if (canvas_undo_name && canvas_undo_name[0] == 'd')
            for (sel = x->gl_editor->e_selection; sel; sel = sel->sel_next)
                gobj_displace(sel->sel_what, x, 10, 10);
    }
else if (action == UNDO_FREE)
        PD_MEMORY_FREE(buf);
}

    /* recursively check for abstractions to reload as result of a save. 
    Don't reload the one we just saved ("except") though. */
    /*  LATER try to do the same trick for externs. */
static void glist_doreload(t_glist *gl, t_symbol *name, t_symbol *dir,
    t_gobj *except)
{
    t_gobj *g;
    int i, nobj = glist_getindex(gl, 0);  /* number of objects */
    int hadwindow = (gl->gl_editor != 0);
    for (g = gl->gl_graphics, i = 0; g && i < nobj; i++)
    {
        if (g != except && pd_class(&g->g_pd) == canvas_class &&
            canvas_isAbstraction((t_glist *)g) &&
                ((t_glist *)g)->gl_name == name &&
                    canvas_getEnvironment ((t_glist *)g)->ce_directory == dir)
        {
                /* we're going to remake the object, so "g" will go stale.
                Get its index here, and afterward restore g.  Also, the
                replacement will be at the end of the list, so we don't
                do g = g->g_next in this case. */
            int j = glist_getindex(gl, g);
            if (!gl->gl_editor)
                canvas_vis(gl, 1);
            if (!gl->gl_editor) { PD_BUG; }
            glist_noselect(gl);
            glist_select(gl, g);
            canvas_setundo(gl, canvas_undo_cut,
                canvas_undo_set_cut(gl, UCUT_CLEAR), "clear");
            canvas_doclear(gl);
            canvas_undo(gl);
            glist_noselect(gl);
            g = glist_nth(gl, j);
        }
        else
        {
            if (g != except && pd_class(&g->g_pd) == canvas_class)
                glist_doreload((t_glist *)g, name, dir, except);
             g = g->g_next;
        }
    }
    if (!hadwindow && gl->gl_editor)
        canvas_vis(glist_getcanvas(gl), 0);
}

    /* this flag stops canvases from being marked "dirty" if we have to touch
    them to reload an abstraction; also suppress window list update */
int editor_reloading = 0;    /* Shared. */

    /* call canvas_doreload on everyone */
void canvas_reload(t_symbol *name, t_symbol *dir, t_gobj *except)
{
    t_glist *x;
    int dspwas = dsp_suspend();
    editor_reloading = 1;
        /* find all root canvases */
    for (x = pd_this->pd_roots; x; x = x->gl_next)
        glist_doreload(x, name, dir, except);
    editor_reloading = 0;
    dsp_resume(dspwas);
}

/* ------------------------ event handling ------------------------ */

static char *cursorlist[] = {
    "$::var(cursorRunNothing)",
    "$::var(cursorRunClickMe)",
    "$::var(cursorRunThicken)",
    "$::var(cursorRunAddPoint)",
    "$::var(cursorEditNothing)",
    "$::var(cursorEditConnect)",
    "$::var(cursorEditDisconnect)",
    "$::var(cursorEditResize)"
};

void canvas_setcursor(t_glist *x, unsigned int cursornum)
{
    static t_glist *xwas;
    static unsigned int cursorwas;
    if (cursornum >= sizeof(cursorlist)/sizeof *cursorlist)
    {
        PD_BUG;
        return;
    }
    if (xwas != x || cursorwas != cursornum)
    {
        sys_vGui(".x%lx configure -cursor %s\n", x, cursorlist[cursornum]);
        xwas = x;
        cursorwas = cursornum;
    }
}

    /* check if a point lies in a gobj.  */
int canvas_hitbox(t_glist *x, t_gobj *y, int xpos, int ypos,
    int *x1p, int *y1p, int *x2p, int *y2p)
{
    int x1, y1, x2, y2;
    t_object *ob;
    if (!gobj_shouldvis(y, x))
        return (0);
    gobj_getrect(y, x, &x1, &y1, &x2, &y2);
    if (xpos >= x1 && xpos <= x2 && ypos >= y1 && ypos <= y2)
    {
        *x1p = x1;
        *y1p = y1;
        *x2p = x2;
        *y2p = y2;
        return (1);
    }
    else return (0);
}

    /* find the last gobj, if any, containing the point. */
static t_gobj *canvas_findhitbox(t_glist *x, int xpos, int ypos,
    int *x1p, int *y1p, int *x2p, int *y2p)
{
    t_gobj *y, *rval = 0;
    int x1, y1, x2, y2;
    *x1p = -0x7fffffff;
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        if (canvas_hitbox(x, y, xpos, ypos, &x1, &y1, &x2, &y2)
            && (x1 > *x1p))
                *x1p = x1, *y1p = y1, *x2p = x2, *y2p = y2, rval = y; 
    }
        /* if there are at least two selected objects, we'd prefer
        to find a selected one (never mind which) to the one we got. */
    if (x->gl_editor && x->gl_editor->e_selection &&
        x->gl_editor->e_selection->sel_next && !glist_isselected(x, y))
    {
        t_selection *sel;
        for (sel = x->gl_editor->e_selection; sel; sel = sel->sel_next)
            if (canvas_hitbox(x, sel->sel_what, xpos, ypos, &x1, &y1, &x2, &y2))
                *x1p = x1, *y1p = y1, *x2p = x2, *y2p = y2,
                    rval = sel->sel_what; 
    }
    return (rval);
}

    /* right-clicking on a canvas object pops up a menu. */
static void canvas_rightclick(t_glist *x, int xpos, int ypos, t_gobj *y)
{
    int canprop, canopen;
    canprop = (!y || (y && class_hasPropertiesFunction (pd_class(&y->g_pd))));
    canopen = (y && class_hasMethod (pd_class (&y->g_pd), gensym ("open")));
    sys_vGui("::ui_menu::showPopup .x%lx %d %d %d %d\n",
        x, xpos, ypos, canprop, canopen);
}

/* ----  editors -- perhaps this and "vis" should go to g_editor.c ------- */

static t_editor *editor_new(t_glist *owner)
{
    char buf[40];
    t_editor *x = (t_editor *)PD_MEMORY_GET(sizeof(*x));
    x->e_connectbuf = buffer_new();
    x->e_deleted = buffer_new();
    x->e_glist = owner;
    sprintf(buf, ".x%lx", (t_int)owner);
    x->e_guiconnect = guiconnect_new(&owner->gl_obj.te_g.g_pd, gensym (buf));
    x->e_clock = 0;
    return (x);
}

static void editor_free(t_editor *x, t_glist *y)
{
    glist_noselect(y);
    guiconnect_release(x->e_guiconnect, 1000);
    buffer_free(x->e_connectbuf);
    buffer_free(x->e_deleted);
    if (x->e_clock)
        clock_free(x->e_clock);
    PD_MEMORY_FREE((void *)x);
}

    /* recursively create or destroy all editors of a glist and its 
    sub-glists, as long as they aren't toplevels. */
void canvas_create_editor(t_glist *x)
{
    t_gobj *y;
    t_object *ob;
    if (!x->gl_editor)
    {
        x->gl_editor = editor_new(x);
        for (y = x->gl_graphics; y; y = y->g_next)
            if (ob = canvas_castToObjectIfBox(&y->g_pd))
                rtext_new(x, ob);
    }
}

void canvas_destroy_editor(t_glist *x)
{
    t_gobj *y;
    t_object *ob;
    glist_noselect(x);
    if (x->gl_editor)
    {
        t_boxtext *rtext;
        while (rtext = x->gl_editor->e_rtext)
            rtext_free(rtext);
        editor_free(x->gl_editor, x);
        x->gl_editor = 0;
    }
}

void canvas_map(t_glist *x, t_float f);

    /* we call this when we want the window to become visible, mapped, and
    in front of all windows; or with "f" zero, when we want to get rid of
    the window. */
void canvas_vis(t_glist *x, t_float f)
{
    char buf[30];
    int flag = (f != 0);
    if (flag)
    {
        /* If a subpatch/abstraction has GOP/gl_isGraphOnParent set, then it will have
         * a gl_editor already, if its not, it will not have a gl_editor.
         * canvas_create_editor(x) checks if a gl_editor is already created,
         * so its ok to run it on a canvas that already has a gl_editor. */
        if (x->gl_editor && x->gl_haveWindow)
        {           /* just put us in front */
            sys_vGui("::bringToFront .x%lx\n", x);  
        }
        else
        {
            char cbuf[PD_STRING];
            int cbuflen;
            t_glist *c = x;
            canvas_create_editor(x);
            sys_vGui("::ui_patch::create .x%lx %d %d +%d+%d %d\n", x,
                (int)(x->gl_windowBottomRightX - x->gl_windowTopLeftX),
                (int)(x->gl_windowBottomRightY - x->gl_windowTopLeftY),
                (int)(x->gl_windowTopLeftX), (int)(x->gl_windowTopLeftY),
                x->gl_isEditMode);
           snprintf(cbuf, PD_STRING - 2, "::ui_patch::pdtk_canvas_setparents .x%lx",
                (unsigned long)c);
            while (c->gl_owner) {
                c = c->gl_owner;
                cbuflen = strlen(cbuf);
                snprintf(cbuf + cbuflen,
                         PD_STRING - cbuflen - 2,/* leave 2 for "\n\0" */
                         " .x%lx", (unsigned long)c);
            }
            strcat(cbuf, "\n");
            // sys_gui(cbuf);
            canvas_updateTitle(x);
            x->gl_haveWindow = 1;
        }
    }
    else    /* make invisible */
    {
        int i;
        t_glist *x2;
        if (!x->gl_haveWindow)
        {
                /* bug workaround -- a graph in a visible patch gets "invised"
                when the patch is closed, and must lose the editor here.  It's
                probably not the natural place to do this.  Other cases like
                subpatches fall here too but don'd need the editor freed, so
                we check if it exists. */
            if (x->gl_editor)
                canvas_destroy_editor(x);
            return;
        }
        glist_noselect(x);
        if (canvas_isVisible(x))
            canvas_map(x, 0);
        canvas_destroy_editor(x);
        sys_vGui("destroy .x%lx\n", x);
        for (i = 1, x2 = x; x2; x2 = x2->gl_next, i++)
            ;
            /* if we're a graph on our parent, and if the parent exists
               and is visible, show ourselves on parent. */
        if (canvas_isGraphOnParent(x) && x->gl_owner)
        {
            t_glist *gl2 = x->gl_owner;
            if (canvas_isVisible(gl2))
                gobj_vis(&x->gl_obj.te_g, gl2, 0);
            x->gl_haveWindow = 0;
            if (canvas_isVisible(gl2) && !gl2->gl_isDeleting)
                gobj_vis(&x->gl_obj.te_g, gl2, 1);
        }
        else x->gl_haveWindow = 0;
    }
}

    /* set a canvas up as a graph-on-parent.  Set reasonable defaults for
    any missing paramters and redraw things if necessary. */
void canvas_setgraph(t_glist *x, int flag, int nogoprect)
{
    x->gl_hideText = !(!(flag&2));
    
    flag = (flag&1);
    
    if (!flag && canvas_isGraphOnParent(x))
    {
        if (x->gl_owner && !x->gl_isLoading && canvas_isVisible(x->gl_owner))
            gobj_vis(&x->gl_obj.te_g, x->gl_owner, 0);
        x->gl_isGraphOnParent = 0;
        if (x->gl_owner && !x->gl_isLoading && canvas_isVisible(x->gl_owner))
        {
            gobj_vis(&x->gl_obj.te_g, x->gl_owner, 1);
            canvas_updateLinesByObject(x->gl_owner, &x->gl_obj);
        }
    }
    else if (flag)
    {
        if (x->gl_width <= 0)
            x->gl_width = GLIST_DEFAULT_WIDTH;

        if (x->gl_height <= 0)
            x->gl_height = GLIST_DEFAULT_HEIGHT;

        if (x->gl_owner && !x->gl_isLoading && canvas_isVisible(x->gl_owner))
            gobj_vis(&x->gl_obj.te_g, x->gl_owner, 0);
        x->gl_isGraphOnParent = 1;
        x->gl_hasRectangle = !nogoprect;
        if (canvas_isVisible(x) && x->gl_hasRectangle)
            glist_redraw(x);
        if (x->gl_owner && !x->gl_isLoading && canvas_isVisible(x->gl_owner))
        {
            gobj_vis(&x->gl_obj.te_g, x->gl_owner, 1);
            canvas_updateLinesByObject(x->gl_owner, &x->gl_obj);
        }
    }
}

void garray_properties(t_garray *x);

    /* tell GUI to create a properties dialog on the canvas.  We tell
    the user the negative of the "pixel" y scale to make it appear to grow
    naturally upward, whereas pixels grow downward. */
void canvas_properties(t_gobj*z, t_glist*unused)
{
    t_glist *x = (t_glist*)z;
    t_gobj *y;
    char graphbuf[200];
    if (canvas_isGraphOnParent(x) != 0)
        sprintf(graphbuf,
            "::ui_canvas::show %%s %g %g %d %g %g %g %g %d %d %d %d\n",
                0., 0.,
                canvas_isGraphOnParent (x) | (x->gl_hideText << 1),//1,
                x->gl_indexStart, x->gl_valueUp, x->gl_indexEnd, x->gl_valueDown, 
                (int)x->gl_width, (int)x->gl_height,
                (int)x->gl_marginX, (int)x->gl_marginY);
    else sprintf(graphbuf,
            "::ui_canvas::show %%s %g %g %d %g %g %g %g %d %d %d %d\n",
                glist_dpixtodx(x, 1), glist_dpixtody(x, 1),
                (x->gl_hideText << 1),
                0., 1., 1., -1., 
                (int)x->gl_width, (int)x->gl_height,
                (int)x->gl_marginX, (int)x->gl_marginY);
    guistub_new(&x->gl_obj.te_g.g_pd, x, graphbuf);
        /* if any arrays are in the graph, put out their dialogs too */
    for (y = x->gl_graphics; y; y = y->g_next)
        if (pd_class(&y->g_pd) == garray_class) 
            garray_properties((t_garray *)y);
}

    /* called from the gui when "OK" is selected on the canvas properties
        dialog. */
static void canvas_donecanvasdialog(t_glist *x,
    t_symbol *s, int argc, t_atom *argv)
{


    t_float xperpix, yperpix, x1, y1, x2, y2, xpix, ypix, xmargin, ymargin; 
    int graphme, redraw = 0;

    xperpix = atom_getFloatAtIndex(0, argc, argv);
    yperpix = atom_getFloatAtIndex(1, argc, argv);
    graphme = (int)(atom_getFloatAtIndex(2, argc, argv));
    x1 = atom_getFloatAtIndex(3, argc, argv);
    y1 = atom_getFloatAtIndex(4, argc, argv);
    x2 = atom_getFloatAtIndex(5, argc, argv);
    y2 = atom_getFloatAtIndex(6, argc, argv);
    xpix = atom_getFloatAtIndex(7, argc, argv);
    ypix = atom_getFloatAtIndex(8, argc, argv);
    xmargin = atom_getFloatAtIndex(9, argc, argv);
    ymargin = atom_getFloatAtIndex(10, argc, argv);
    
    x->gl_width = xpix;
    x->gl_height = ypix;
    x->gl_marginX = xmargin;
    x->gl_marginY = ymargin;

    // yperpix = -yperpix;
    if (xperpix == 0)
        xperpix = 1;
    if (yperpix == 0)
        yperpix = 1;

    if (graphme)
    {
        if (x1 != x2)
            x->gl_indexStart = x1, x->gl_indexEnd = x2;
        else x->gl_indexStart = 0, x->gl_indexEnd = 1;
        if (y1 != y2)
            x->gl_valueUp = y1, x->gl_valueDown = y2;
        else x->gl_valueUp = 0, x->gl_valueDown = 1;
    }
    else
    {
        if (xperpix != glist_dpixtodx(x, 1) || yperpix != glist_dpixtody(x, 1))
            redraw = 1;
        if (xperpix > 0)
        {
            x->gl_indexStart = 0;
            x->gl_indexEnd = xperpix;
        }
        else
        {
            x->gl_indexStart = -xperpix * (x->gl_windowBottomRightX - x->gl_windowTopLeftX);
            x->gl_indexEnd = x->gl_indexStart + xperpix;
        }
        if (yperpix > 0)
        {
            x->gl_valueUp = 0;
            x->gl_valueDown = yperpix;
        }
        else
        {
            x->gl_valueUp = -yperpix * (x->gl_windowBottomRightY - x->gl_windowTopLeftY);
            x->gl_valueDown = x->gl_valueUp + yperpix;
        }
    }
        /* LATER avoid doing 2 redraws here (possibly one inside setgraph) */
    canvas_setgraph(x, graphme, 0);
    canvas_dirty(x, 1);
    if (x->gl_haveWindow)
        canvas_redraw(x);
    else if (canvas_isVisible(x->gl_owner))
    {
        gobj_vis(&x->gl_obj.te_g, x->gl_owner, 0);
        gobj_vis(&x->gl_obj.te_g, x->gl_owner, 1);
    }
}

    /* called from the gui when a popup menu comes back with "properties,"
        "open," or "help." */
static void canvas_done_popup(t_glist *x, t_float which, t_float xpos, t_float ypos)
{
    char pathbuf[PD_STRING], namebuf[PD_STRING];
    t_gobj *y;
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        int x1, y1, x2, y2;
        if (canvas_hitbox(x, y, xpos, ypos, &x1, &y1, &x2, &y2))
        {
            if (which == 0)     /* properties */
            {
                if (!class_hasPropertiesFunction (pd_class(&y->g_pd)))
                    continue;
                (*class_getPropertiesFunction (pd_class(&y->g_pd)))(y, x);
                return;
            }
            else if (which == 1)    /* open */
            {
                if (!class_hasMethod (pd_class (&y->g_pd), gensym ("open")))
                    continue;
                pd_vMessage(&y->g_pd, gensym ("open"), "");
                return;
            }
            else    /* help */
            {
                char *dir;
                if (pd_class(&y->g_pd) == canvas_class &&
                    canvas_isAbstraction((t_glist *)y))
                {
                    t_object *ob = (t_object *)y;
                    int ac = buffer_size(ob->te_buffer);
                    t_atom *av = buffer_atoms(ob->te_buffer);
                    if (ac < 1)
                        return;
                    atom_toString(av, namebuf, PD_STRING);
                    dir = canvas_getEnvironment ((t_glist *)y)->ce_directory->s_name;
                }
                else
                {
                    strcpy(namebuf, class_getHelpName(pd_class(&y->g_pd)));
                    dir = class_getExternalDirectory(pd_class(&y->g_pd));
                }

                file_openHelp (dir, namebuf);
                return;
            }
        }
    }
    if (which == 0)
        canvas_properties(&x->gl_obj.te_g, 0);
    /*else if (which == 2)
        file_openHelp("intro.pd", canvas_getDirectory((t_glist *)x)->s_name);*/
}

#define NOMOD 0
#define SHIFTMOD 1
#define CTRLMOD 2
#define ALTMOD 4
#define RIGHTCLICK 8

static double canvas_upclicktime;
static int canvas_upx, canvas_upy;
#define DCLICKINTERVAL 0.25

    /* mouse click */
void canvas_doclick(t_glist *x, int xpos, int ypos, int which,
    int mod, int doit)
{
    t_gobj *y;
    int shiftmod, runmode, altmod, doublemod = 0, rightclick;
    int x1=0, y1=0, x2=0, y2=0, clickreturned = 0;
    
    if (!x->gl_editor)
    {
        PD_BUG;
        return;
    }
    
    shiftmod = (mod & SHIFTMOD);
    runmode = ((mod & CTRLMOD) || (!x->gl_isEditMode));
    altmod = (mod & ALTMOD);
    rightclick = (mod & RIGHTCLICK);

    canvas_undo_already_set_move = 0;

            /* if keyboard was grabbed, notify grabber and cancel the grab */
    if (doit && x->gl_editor->e_grab && x->gl_editor->e_keyfn)
    {
        (* x->gl_editor->e_keyfn) (x->gl_editor->e_grab, 0);
        glist_grab(x, 0, 0, 0, 0, 0);
    }

    if (doit && !runmode && xpos == canvas_upx && ypos == canvas_upy &&
        sys_getRealTimeInSeconds() - canvas_upclicktime < DCLICKINTERVAL)
            doublemod = 1;
    x->gl_editor->e_lastmoved = 0;
    if (doit)
    {
        x->gl_editor->e_grab = 0;
        x->gl_editor->e_onmotion = ACTION_NONE;
    }
    /* post("click %d %d %d %d", xpos, ypos, which, mod); */
    
    if (x->gl_editor->e_onmotion != ACTION_NONE)
        return;

    x->gl_editor->e_xwas = xpos;
    x->gl_editor->e_ywas = ypos;

    if (runmode && !rightclick)
    {
        for (y = x->gl_graphics; y; y = y->g_next)
        {
                /* check if the object wants to be clicked */
            if (canvas_hitbox(x, y, xpos, ypos, &x1, &y1, &x2, &y2)
                && (clickreturned = gobj_click(y, x, xpos, ypos,
                    shiftmod, ((mod & CTRLMOD) && (!x->gl_isEditMode)) || altmod,
                        0, doit)))
                            break;
        }
        if (!doit)
        {
            if (y)
                canvas_setcursor(x, clickreturned);
            else canvas_setcursor(x, CURSOR_NOTHING);
        }
        return;
    }
        /* if not a runmode left click, fall here. */
    if (y = canvas_findhitbox(x, xpos, ypos, &x1, &y1, &x2, &y2))
    {
        t_object *ob = canvas_castToObjectIfBox(&y->g_pd);
            /* check you're in the rectangle */
        ob = canvas_castToObjectIfBox(&y->g_pd);
        if (rightclick)
            canvas_rightclick(x, xpos, ypos, y);
        else if (shiftmod)
        {
            if (doit)
            {
                t_boxtext *rt;
                if (ob && (rt = x->gl_editor->e_textedfor) &&
                    rt == glist_findrtext(x, ob))
                {
                    rtext_mouse(rt, xpos - x1, ypos - y1, BOX_TEXT_SHIFT);
                    x->gl_editor->e_onmotion = ACTION_DRAG;
                    x->gl_editor->e_xwas = x1;
                    x->gl_editor->e_ywas = y1;
                }
                else
                {
                    if (glist_isselected(x, y))
                        glist_deselect(x, y);
                    else glist_select(x, y);
                }
            }
        }
        else
        {
            int noutlet;
                /* resize?  only for "true" text boxes or canvases*/
            if (ob && !x->gl_editor->e_selection &&
                (ob->te_g.g_pd->c_behavior == &text_widgetBehavior ||
                    canvas_castToGlist(&ob->te_g.g_pd)) &&
                        xpos >= x2-4 && ypos < y2-4)
            {
                if (doit)
                {
                    if (!glist_isselected(x, y))
                    {
                        glist_noselect(x);
                        glist_select(x, y);
                    }
                    x->gl_editor->e_onmotion = ACTION_RESIZE;
                    x->gl_editor->e_xwas = x1;
                    x->gl_editor->e_ywas = y1;
                    x->gl_editor->e_xnew = xpos;
                    x->gl_editor->e_ynew = ypos;
                }                                   
                else canvas_setcursor(x, CURSOR_EDIT_RESIZE);
            }
                /* look for an outlet */
            else if (ob && (noutlet = object_numberOfOutlets(ob)) && ypos >= y2-4)
            {
                int width = x2 - x1;
                int nout1 = (noutlet > 1 ? noutlet - 1 : 1);
                int closest = ((xpos-x1) * (nout1) + width/2)/width;
                int hotspot = x1 +
                    (width - INLETS_WIDTH) * closest / (nout1);
                if (closest < noutlet &&
                    xpos >= (hotspot-1) && xpos <= hotspot + (INLETS_WIDTH+1))
                {
                    if (doit)
                    {
                        int issignal = object_isSignalOutlet(ob, closest);
                        x->gl_editor->e_onmotion = ACTION_CONNECT;
                        x->gl_editor->e_xwas = xpos;
                        x->gl_editor->e_ywas = ypos;
                        sys_vGui(
                          ".x%lx.c create line %d %d %d %d -width %d -tags x\n",
                                x, xpos, ypos, xpos, ypos,
                                    (issignal ? 2 : 1));
                    }                                   
                    else canvas_setcursor(x, CURSOR_EDIT_CONNECT);
                }
                else if (doit)
                    goto nooutletafterall;
                else canvas_setcursor(x, CURSOR_EDIT_NOTHING);
            }
                /* not in an outlet; select and move */
            else if (doit)
            {
                t_boxtext *rt;
                    /* check if the box is being text edited */
            nooutletafterall:
                if (ob && (rt = x->gl_editor->e_textedfor) &&
                    rt == glist_findrtext(x, ob))
                {
                    rtext_mouse(rt, xpos - x1, ypos - y1,
                        (doublemod ? BOX_TEXT_DOUBLE : BOX_TEXT_DOWN));
                    x->gl_editor->e_onmotion = ACTION_DRAG;
                    x->gl_editor->e_xwas = x1;
                    x->gl_editor->e_ywas = y1;
                }
                else
                {
                        /* otherwise select and drag to displace */
                    if (!glist_isselected(x, y))
                    {
                        glist_noselect(x);
                        glist_select(x, y);
                    }
                    x->gl_editor->e_onmotion = ACTION_MOVE;
                }
            }
            else canvas_setcursor(x, CURSOR_EDIT_NOTHING);
        }
        return;
    }
        /* if right click doesn't hit any boxes, call rightclick
            routine anyway */
    if (rightclick)
        canvas_rightclick(x, xpos, ypos, 0);

        /* if not an editing action, and if we didn't hit a
        box, set cursor and return */
    if (runmode || rightclick)
    {
        canvas_setcursor(x, CURSOR_NOTHING);
        return;
    }
        /* having failed to find a box, we try lines now. */
    if (!runmode && !altmod && !shiftmod)
    {
        t_linetraverser t;
        t_outconnect *oc;
        t_float fx = xpos, fy = ypos;
        t_glist *glist2 = glist_getcanvas(x);
        canvas_traverseLinesStart(&t, glist2);
        while (oc = canvas_traverseLinesNext(&t))
        {
            t_float lx1 = t.tr_lineStartX, ly1 = t.tr_lineStartY,
                lx2 = t.tr_lineEndX, ly2 = t.tr_lineEndY;
            t_float area = (lx2 - lx1) * (fy - ly1) -
                (ly2 - ly1) * (fx - lx1);
            t_float dsquare = (lx2-lx1) * (lx2-lx1) + (ly2-ly1) * (ly2-ly1);
            if (area * area >= 50 * dsquare) continue;
            if ((lx2-lx1) * (fx-lx1) + (ly2-ly1) * (fy-ly1) < 0) continue;
            if ((lx2-lx1) * (lx2-fx) + (ly2-ly1) * (ly2-fy) < 0) continue;
            if (doit)
            {
                glist_selectline(glist2, oc, 
                    canvas_getIndexOfObject(glist2, &t.tr_srcObject->te_g), t.tr_srcIndexOfOutlet,
                    canvas_getIndexOfObject(glist2, &t.tr_destObject->te_g), t.tr_destIndexOfInlet);
            }
            canvas_setcursor(x, CURSOR_EDIT_DISCONNECT);
            return;
        }
    }
    canvas_setcursor(x, CURSOR_EDIT_NOTHING);
    if (doit)
    {
        if (!shiftmod) glist_noselect(x);
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags x\n",
              x, xpos, ypos, xpos, ypos);
        x->gl_editor->e_xwas = xpos;
        x->gl_editor->e_ywas = ypos;
        x->gl_editor->e_onmotion = ACTION_REGION;
    }
}

void canvas_mouse(t_glist *x, t_float xpos, t_float ypos,
    t_float which, t_float mod)
{
    canvas_doclick(x, xpos, ypos, which, mod, 1);
}

int canvas_isconnected (t_glist *x, t_object *ob1, int n1,
    t_object *ob2, int n2)
{
    t_linetraverser t;
    t_outconnect *oc;
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
        if (t.tr_srcObject == ob1 && t.tr_srcIndexOfOutlet == n1 &&
            t.tr_destObject == ob2 && t.tr_destIndexOfInlet == n2) 
                return (1);
    return (0);
}

void canvas_doconnect(t_glist *x, int xpos, int ypos, int which, int doit)
{
    int x11=0, y11=0, x12=0, y12=0;
    t_gobj *y1;
    int x21=0, y21=0, x22=0, y22=0;
    t_gobj *y2;
    int xwas = x->gl_editor->e_xwas,
        ywas = x->gl_editor->e_ywas;
    if (doit) sys_vGui(".x%lx.c delete x\n", x);
    else sys_vGui(".x%lx.c coords x %d %d %d %d\n",
            x, x->gl_editor->e_xwas,
                x->gl_editor->e_ywas, xpos, ypos);

    if ((y1 = canvas_findhitbox(x, xwas, ywas, &x11, &y11, &x12, &y12))
        && (y2 = canvas_findhitbox(x, xpos, ypos, &x21, &y21, &x22, &y22)))
    {
        t_object *ob1 = canvas_castToObjectIfBox(&y1->g_pd);
        t_object *ob2 = canvas_castToObjectIfBox(&y2->g_pd);
        int noutlet1, ninlet2;
        if (ob1 && ob2 && ob1 != ob2 &&
            (noutlet1 = object_numberOfOutlets(ob1))
            && (ninlet2 = object_numberOfInlets(ob2)))
        {
            int width1 = x12 - x11, closest1, hotspot1;
            int width2 = x22 - x21, closest2, hotspot2;
            int lx1, lx2, ly1, ly2;
            t_outconnect *oc;

            if (noutlet1 > 1)
            {
                closest1 = ((xwas-x11) * (noutlet1-1) + width1/2)/width1;
                hotspot1 = x11 +
                    (width1 - INLETS_WIDTH) * closest1 / (noutlet1-1);
            }
            else closest1 = 0, hotspot1 = x11;

            if (ninlet2 > 1)
            {
                closest2 = ((xpos-x21) * (ninlet2-1) + width2/2)/width2;
                hotspot2 = x21 +
                    (width2 - INLETS_WIDTH) * closest2 / (ninlet2-1);
            }
            else closest2 = 0, hotspot2 = x21;

            if (closest1 >= noutlet1)
                closest1 = noutlet1 - 1;
            if (closest2 >= ninlet2)
                closest2 = ninlet2 - 1;

            if (canvas_isconnected (x, ob1, closest1, ob2, closest2))
            {
                canvas_setcursor(x, CURSOR_EDIT_NOTHING);
                return;
            }
            if (object_isSignalOutlet(ob1, closest1) &&
                !object_isSignalInlet(ob2, closest2))
            {
                if (doit)
                    post_error ("can't connect signal outlet to control inlet");
                canvas_setcursor(x, CURSOR_EDIT_NOTHING);
                return;
            }
            if (doit)
            {
                oc = object_connect(ob1, closest1, ob2, closest2);
                lx1 = x11 + (noutlet1 > 1 ?
                        ((x12-x11-INLETS_WIDTH) * closest1)/(noutlet1-1) : 0)
                             + ((INLETS_WIDTH - 1) / 2);
                ly1 = y12;
                lx2 = x21 + (ninlet2 > 1 ?
                        ((x22-x21-INLETS_WIDTH) * closest2)/(ninlet2-1) : 0)
                            + ((INLETS_WIDTH - 1) / 2);
                ly2 = y21;
                sys_vGui(".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    glist_getcanvas(x),
                        lx1, ly1, lx2, ly2,
                            (object_isSignalOutlet(ob1, closest1) ? 2 : 1), oc);
                canvas_dirty(x, 1);
                canvas_setundo(x, canvas_undo_connect,
                    canvas_undo_set_connect(x, 
                        canvas_getIndexOfObject(x, &ob1->te_g), closest1,
                        canvas_getIndexOfObject(x, &ob2->te_g), closest2),
                        "connect");
            }
            else canvas_setcursor(x, CURSOR_EDIT_CONNECT);
            return;
        }
    }
    canvas_setcursor(x, CURSOR_EDIT_NOTHING);
}

void canvas_selectinrect(t_glist *x, int lox, int loy, int hix, int hiy)
{
    t_gobj *y;
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        int x1, y1, x2, y2;
        gobj_getrect(y, x, &x1, &y1, &x2, &y2);
        if (hix >= x1 && lox <= x2 && hiy >= y1 && loy <= y2
            && !glist_isselected(x, y))
                glist_select(x, y);
    }
}

static void canvas_doregion(t_glist *x, int xpos, int ypos, int doit)
{
    if (doit)
    {
        int lox, loy, hix, hiy;
        if (x->gl_editor->e_xwas < xpos)
            lox = x->gl_editor->e_xwas, hix = xpos;
        else hix = x->gl_editor->e_xwas, lox = xpos;
        if (x->gl_editor->e_ywas < ypos)
            loy = x->gl_editor->e_ywas, hiy = ypos;
        else hiy = x->gl_editor->e_ywas, loy = ypos;
        canvas_selectinrect(x, lox, loy, hix, hiy);
        sys_vGui(".x%lx.c delete x\n", x);
        x->gl_editor->e_onmotion = ACTION_NONE;
    }
    else sys_vGui(".x%lx.c coords x %d %d %d %d\n",
            x, x->gl_editor->e_xwas,
                x->gl_editor->e_ywas, xpos, ypos);
}

void canvas_mouseup(t_glist *x,
    t_float fxpos, t_float fypos, t_float fwhich)
{
    int xpos = fxpos, ypos = fypos, which = fwhich;
    /* post("mouseup %d %d %d", xpos, ypos, which); */
    if (!x->gl_editor)
    {
        PD_BUG;
        return;
    }

    canvas_upclicktime = sys_getRealTimeInSeconds();
    canvas_upx = xpos;
    canvas_upy = ypos;

    if (x->gl_editor->e_onmotion == ACTION_CONNECT)
        canvas_doconnect(x, xpos, ypos, which, 1);
    else if (x->gl_editor->e_onmotion == ACTION_REGION)
        canvas_doregion(x, xpos, ypos, 1);
    else if (x->gl_editor->e_onmotion == ACTION_MOVE ||
        x->gl_editor->e_onmotion == ACTION_RESIZE)
    {
            /* after motion or resizing, if there's only one text item
                selected, activate the text */
        if (x->gl_editor->e_selection &&
            !(x->gl_editor->e_selection->sel_next))
        {
            t_gobj *g = x->gl_editor->e_selection->sel_what;
            t_glist *gl2;
                /* first though, check we aren't an abstraction with a
                dirty sub-patch that would be discarded if we edit this. */
            if (pd_class(&g->g_pd) == canvas_class &&
                canvas_isAbstraction((t_glist *)g) &&
                    (gl2 = glist_finddirty((t_glist *)g)))
            {
                pd_vMessage(&gl2->gl_obj.te_g.g_pd, gensym ("open"), "");
                x->gl_editor->e_onmotion = ACTION_NONE;
                sys_vGui(
"::ui_confirm::checkAction .x%lx { Discard changes to %s? } { ::ui_interface::pdsend .x%lx dirty 0 } { no }\n",
                    canvas_getRoot(gl2),
                        canvas_getRoot(gl2)->gl_name->s_name, gl2);
                return;
            }
                /* OK, activate it */
            gobj_activate(x->gl_editor->e_selection->sel_what, x, 1);
        }
    }

    x->gl_editor->e_onmotion = ACTION_NONE;
}

    /* displace the selection by (dx, dy) pixels */
static void canvas_displaceselection(t_glist *x, int dx, int dy)
{
    t_selection *y;
    int resortin = 0, resortout = 0;
    if (!canvas_undo_already_set_move)
    {
        canvas_setundo(x, canvas_undo_move, canvas_undo_set_move(x, 1),
            "motion");
        canvas_undo_already_set_move = 1;
    }
    for (y = x->gl_editor->e_selection; y; y = y->sel_next)
    {
        t_class *cl = pd_class(&y->sel_what->g_pd);
        gobj_displace(y->sel_what, x, dx, dy);
        if (cl == vinlet_class) resortin = 1;
        else if (cl == voutlet_class) resortout = 1;
    }
    if (resortin) canvas_resortinlets(x);
    if (resortout) canvas_resortoutlets(x);
    sys_vGui("::ui_patch::updateScrollRegion .x%lx.c\n", x);
    if (x->gl_editor->e_selection)
        canvas_dirty(x, 1);
}

    /* this routine is called whenever a key is pressed or released.  "x"
    may be zero if there's no current canvas.  The first argument is true or
    false for down/up; the second one is either a symbolic key name (e.g.,
    "Right" or an Ascii key number.  The third is the shift key. */
void canvas_key(t_glist *x, t_symbol *s, int ac, t_atom *av)
{
    static t_symbol *keynumsym, *keyupsym, *keynamesym;
    int keynum, fflag;
    t_symbol *gotkeysym;
        
    int down, shift;

    if (ac < 3)
        return;

    canvas_undo_already_set_move = 0;
    down = (atom_getFloat(av) != 0);  /* nonzero if it's a key down */
    shift = (atom_getFloat(av+2) != 0);  /* nonzero if shift-ed */
    if (av[1].a_type == A_SYMBOL)
        gotkeysym = av[1].a_w.w_symbol;
    else if (av[1].a_type == A_FLOAT)
    {
        int sz;
        char buf[UTF8_MAXIMUM_BYTES + 1];
        switch((int)(av[1].a_w.w_float))
        {
        case 8:  gotkeysym = gensym ("BackSpace"); break;
        case 9:  gotkeysym = gensym ("Tab"); break;
        case 10: gotkeysym = gensym ("Return"); break;
        case 27: gotkeysym = gensym ("Escape"); break;
        case 32: gotkeysym = gensym ("Space"); break;
        case 127:gotkeysym = gensym ("Delete"); break;
        default:
        /*-- moo: assume keynum is a Unicode codepoint; encode as UTF-8 --*/
            sz = u8_wc_toutf8 (buf, (UCS4_CODE_POINT)(av[1].a_w.w_float));
            buf[sz] = 0;
            gotkeysym = gensym (buf);
        }
    }
    else gotkeysym = gensym ("?");
    fflag = (av[0].a_type == A_FLOAT ? av[0].a_w.w_float : 0);
    keynum = (av[1].a_type == A_FLOAT ? av[1].a_w.w_float : 0);
    if (keynum == '\\' || keynum == '{' || keynum == '}')
    {
        post("keycode %d: dropped", (int)keynum);
        return;
    }
#if 0
    post("keynum %d, down %d", (int)keynum, down);
#endif
    if (keynum == '\r') keynum = '\n';
    if (av[1].a_type == A_SYMBOL &&
        !strcmp(av[1].a_w.w_symbol->s_name, "Return"))
            keynum = '\n';
    if (!keynumsym)
    {
        keynumsym = gensym ("#key");
        keyupsym = gensym ("#keyup");
        keynamesym = gensym ("#keyname");
    }
#ifdef __APPLE__
        if (keynum == 30 || keynum == 63232)
            keynum = 0, gotkeysym = gensym ("Up");
        else if (keynum == 31 || keynum == 63233)
            keynum = 0, gotkeysym = gensym ("Down");
        else if (keynum == 28 || keynum == 63234)
            keynum = 0, gotkeysym = gensym ("Left");
        else if (keynum == 29 || keynum == 63235)
            keynum = 0, gotkeysym = gensym ("Right");
        else if (keynum == 63273)
            keynum = 0, gotkeysym = gensym ("Home");
        else if (keynum == 63275)
            keynum = 0, gotkeysym = gensym ("End");
        else if (keynum == 63276)
            keynum = 0, gotkeysym = gensym ("Prior");
        else if (keynum == 63277)
            keynum = 0, gotkeysym = gensym ("Next");
#endif
    if (keynumsym->s_thing && down)
        pd_float(keynumsym->s_thing, (t_float)keynum);
    if (keyupsym->s_thing && !down)
        pd_float(keyupsym->s_thing, (t_float)keynum);
    if (keynamesym->s_thing)
    {
        t_atom at[2];
        at[0] = av[0];
        SET_FLOAT(at, down);
        SET_SYMBOL(at+1, gotkeysym);
        pd_list(keynamesym->s_thing, 2, at);
    }
    if (!x || !x->gl_editor)  /* if that 'invis'ed the window, we'd better stop. */
        return;
    if (x && down)
    {
        t_object *ob;
            /* cancel any dragging action */
        if (x->gl_editor->e_onmotion == ACTION_MOVE)
            x->gl_editor->e_onmotion = ACTION_NONE;
            /* if an object has "grabbed" keys just send them on */
        if (x->gl_editor->e_grab
            && x->gl_editor->e_keyfn && keynum)
                (* x->gl_editor->e_keyfn)
                    (x->gl_editor->e_grab, (t_float)keynum);
            /* if a text editor is open send the key on, as long as
            it is either "real" (has a key number) or else is an arrow key. */
        else if (x->gl_editor->e_textedfor && (keynum
            || !strcmp(gotkeysym->s_name, "Up")
            || !strcmp(gotkeysym->s_name, "Down")
            || !strcmp(gotkeysym->s_name, "Left")
            || !strcmp(gotkeysym->s_name, "Right")))
        {
                /* send the key to the box's editor */
            if (!x->gl_editor->e_textdirty)
            {
                canvas_setundo(x, canvas_undo_cut,
                    canvas_undo_set_cut(x, UCUT_TEXT), "typing");
            }
            rtext_key(x->gl_editor->e_textedfor,
                (int)keynum, gotkeysym);
            if (x->gl_editor->e_textdirty)
                canvas_dirty(x, 1);
        }
            /* check for backspace or clear */
        else if (keynum == 8 || keynum == 127)
        {
            if (x->gl_editor->e_selectedline)
                canvas_clearline(x);
            else if (x->gl_editor->e_selection)
            {
                canvas_setundo(x, canvas_undo_cut,
                    canvas_undo_set_cut(x, UCUT_CLEAR), "clear");
                canvas_doclear(x);
            }
        }
                /* check for arrow keys */
        else if (!strcmp(gotkeysym->s_name, "Up"))
            canvas_displaceselection(x, 0, shift ? -10 : -1);
        else if (!strcmp(gotkeysym->s_name, "Down"))
            canvas_displaceselection(x, 0, shift ? 10 : 1);
        else if (!strcmp(gotkeysym->s_name, "Left"))
            canvas_displaceselection(x, shift ? -10 : -1, 0);
        else if (!strcmp(gotkeysym->s_name, "Right"))
            canvas_displaceselection(x, shift ? 10 : 1, 0);
    }
        /* if control key goes up or down, and if we're in edit mode, change
        cursor to indicate how the click action changes */
    if (x && keynum == 0 && x->gl_isEditMode &&
        !strncmp(gotkeysym->s_name, "Control", 7))
            canvas_setcursor(x, down ?
                CURSOR_NOTHING :CURSOR_EDIT_NOTHING);
}

static void delay_move(t_glist *x)
{
    canvas_displaceselection(x, 
       x->gl_editor->e_xnew - x->gl_editor->e_xwas,
       x->gl_editor->e_ynew - x->gl_editor->e_ywas);
    x->gl_editor->e_xwas = x->gl_editor->e_xnew;
    x->gl_editor->e_ywas = x->gl_editor->e_ynew;
}

void canvas_motion(t_glist *x, t_float xpos, t_float ypos,
    t_float fmod)
{ 
    /* post("motion %d %d", xpos, ypos); */
    int mod = fmod;
    if (!x->gl_editor)
    {
        PD_BUG;
        return;
    }
    glist_setlastxy(x, xpos, ypos);
    if (x->gl_editor->e_onmotion == ACTION_MOVE)
    {
        if (!x->gl_editor->e_clock)
            x->gl_editor->e_clock = clock_new(x, (t_method)delay_move);
        clock_unset(x->gl_editor->e_clock);
        clock_delay(x->gl_editor->e_clock, 5);
        x->gl_editor->e_xnew = xpos;
        x->gl_editor->e_ynew = ypos;
    }
    else if (x->gl_editor->e_onmotion == ACTION_REGION)
        canvas_doregion(x, xpos, ypos, 0);
    else if (x->gl_editor->e_onmotion == ACTION_CONNECT)
        canvas_doconnect(x, xpos, ypos, 0, 0);
    else if (x->gl_editor->e_onmotion == ACTION_PASS)
    {
        if (!x->gl_editor->e_motionfn) { PD_BUG; }
        (*x->gl_editor->e_motionfn)(&x->gl_editor->e_grab->g_pd,
            xpos - x->gl_editor->e_xwas,
            ypos - x->gl_editor->e_ywas);
        x->gl_editor->e_xwas = xpos;
        x->gl_editor->e_ywas = ypos;
    }
    else if (x->gl_editor->e_onmotion == ACTION_DRAG)
    {
        t_boxtext *rt = x->gl_editor->e_textedfor;
        if (rt)
            rtext_mouse(rt, xpos - x->gl_editor->e_xwas,
                ypos - x->gl_editor->e_ywas, BOX_TEXT_DRAG);
    }
    else if (x->gl_editor->e_onmotion == ACTION_RESIZE)
    {
        int x11=0, y11=0, x12=0, y12=0; 
        t_gobj *y1;
        if (y1 = canvas_findhitbox(x,
            x->gl_editor->e_xwas, x->gl_editor->e_ywas,
                &x11, &y11, &x12, &y12))
        {
            int wantwidth = xpos - x11;
            t_object *ob = canvas_castToObjectIfBox(&y1->g_pd);
            if (ob && ob->te_g.g_pd->c_behavior == &text_widgetBehavior ||
                    (canvas_castToGlist(&ob->te_g.g_pd) &&
                        !((t_glist *)ob)->gl_isGraphOnParent))
            {
                wantwidth = wantwidth / font_getHostFontWidth(canvas_getFontSize(x));
                if (wantwidth < 1)
                    wantwidth = 1;
                ob->te_width = wantwidth;
                gobj_vis(y1, x, 0);
                canvas_updateLinesByObject(x, ob);
                gobj_vis(y1, x, 1);
            }
            else if (ob && ob->te_g.g_pd == canvas_class)
            {
                gobj_vis(y1, x, 0);
                ((t_glist *)ob)->gl_width += xpos - x->gl_editor->e_xnew;
                ((t_glist *)ob)->gl_height += ypos - x->gl_editor->e_ynew;
                x->gl_editor->e_xnew = xpos;
                x->gl_editor->e_ynew = ypos;
                canvas_updateLinesByObject(x, ob);
                gobj_vis(y1, x, 1);
            }
            else post("not resizable");
        }
    }
    else canvas_doclick(x, xpos, ypos, 0, mod, 0);
    
    x->gl_editor->e_lastmoved = 1;
}

void canvas_startmotion(t_glist *x)
{
    int xval, yval;
    if (!x->gl_editor) return;
    glist_getnextxy(x, &xval, &yval);
    if (xval == 0 && yval == 0) return;
    x->gl_editor->e_onmotion = ACTION_MOVE;
    x->gl_editor->e_xwas = xval;
    x->gl_editor->e_ywas = yval; 
}

void canvas_print(t_glist *x, t_symbol *s)
{
    if (*s->s_name) sys_vGui(".x%lx.c postscript -file %s\n", x, s->s_name);
    else sys_vGui(".x%lx.c postscript -file x.ps\n", x);
}

    /* find a dirty sub-glist, if any, of this one (including itself) */
static t_glist *glist_finddirty(t_glist *x)
{
    t_gobj *g;
    t_glist *g2;
    if (x->gl_environment && x->gl_isDirty)
        return (x);
    for (g = x->gl_graphics; g; g = g->g_next)
        if (pd_class(&g->g_pd) == canvas_class &&
            (g2 = glist_finddirty((t_glist *)g)))
                return (g2);
    return (0);
}

    /* quit, after calling glist_finddirty() on all toplevels and verifying
    the user really wants to discard changes  */
void global_shouldQuit(void *dummy)
{
    t_glist *g, *g2;
        /* find all root canvases */
    for (g = pd_this->pd_roots; g; g = g->gl_next)
        if (g2 = glist_finddirty(g))
    {
        canvas_vis(g2, 1);
            sys_vGui("::ui_confirm::checkClose .x%lx { ::ui_interface::pdsend $top menusave 1 } { ::ui_interface::pdsend .x%lx menuclose 3 } {}\n",
                     canvas_getRoot(g2), g2);
        return;
    }
    if (0)
        sys_vGui("::ui_confirm::checkAction .console { Really quit? } { ::ui_interface::pdsend pd quit } { yes }\n");
    else { interface_quit(0); }
}

    /* close a window (or possibly quit Pd), checking for dirty flags.
    The "force" parameter is interpreted as follows:
        0 - request from GUI to close, verifying whether clean or dirty
        1 - request from GUI to close, no verification
        2 - verified - mark this one clean, then continue as in 1
        3 - verified - mark this one clean, then verify-and-quit
    */
void canvas_menuclose(t_glist *x, t_float fforce)
{
    int force = fforce;
    t_glist *g;
    if (x->gl_owner && (force == 0 || force == 1))
        canvas_vis(x, 0);   /* if subpatch, just invis it */
    else if (force == 0)    
    {
        g = glist_finddirty(x);
        if (g)
        {
            pd_vMessage(&g->gl_obj.te_g.g_pd, gensym ("open"), "");
            sys_vGui("::ui_confirm::checkClose .x%lx { ::ui_interface::pdsend $top menusave 1 } { ::ui_interface::pdsend .x%lx menuclose 2 } {}\n",
                     canvas_getRoot(g), g);
            return;
        }
        else if (0)
        {
            sys_vGui("::ui_confirm::checkAction .x%lx { Close this window? } { ::ui_interface::pdsend .x%lx menuclose 1 } { yes }\n",
                     canvas_getRoot(x), x);
        }
        else pd_free(&x->gl_obj.te_g.g_pd);
    }
    else if (force == 1)
        pd_free(&x->gl_obj.te_g.g_pd);
    else if (force == 2)
    {
        canvas_dirty(x, 0);
        while (x->gl_owner)
            x = x->gl_owner;
        g = glist_finddirty(x);
        if (g)
        {
            pd_vMessage(&g->gl_obj.te_g.g_pd, gensym ("open"), "");
            sys_vGui("::ui_confirm::checkClose .x%lx { ::ui_interface::pdsend $top menusave 1 } { ::ui_interface::pdsend .x%lx menuclose 2 } {}\n",
                     canvas_getRoot(x), g);
            return;
        }
        else pd_free(&x->gl_obj.te_g.g_pd);
    }
    else if (force == 3)
    {
        canvas_dirty(x, 0);
        global_shouldQuit(0);
    }
}

    /* put up a dialog which may call canvas_font back to do the work */
static void canvas_menufont(t_glist *x)
{
/*  char buf[80];
    t_glist *x2 = canvas_getRoot(x);
    guistub_destroyWithKey(x2);
    sprintf(buf, "pdtk_canvas_dofont %%s %d\n", x2->gl_fontSize);
    guistub_new(&x2->gl_obj.te_g.g_pd, &x2->gl_obj.te_g.g_pd, buf); */
}

static int canvas_find_index, canvas_find_wholeword;
static t_buffer *canvas_findbuf;

    /* function to support searching */
static int atoms_match(int inargc, t_atom *inargv, int searchargc,
    t_atom *searchargv, int wholeword)
{
    int indexin, nmatched;
    for (indexin = 0; indexin <= inargc - searchargc; indexin++)
    {
        for (nmatched = 0; nmatched < searchargc; nmatched++)
        {
            t_atom *a1 = &inargv[indexin + nmatched], 
                *a2 = &searchargv[nmatched];
            if (a1->a_type == A_SEMICOLON || a1->a_type == A_COMMA)
            {
                if (a2->a_type != a1->a_type)
                    goto nomatch;
            }
            else if (a1->a_type == A_FLOAT || a1->a_type == A_DOLLAR)
            {
                if (a2->a_type != a1->a_type || 
                    a1->a_w.w_float != a2->a_w.w_float)
                        goto nomatch;
            }
            else if (a1->a_type == A_SYMBOL || a1->a_type == A_DOLLARSYMBOL)
            {
                if ((a2->a_type != A_SYMBOL && a2->a_type != A_DOLLARSYMBOL)
                    || (wholeword && a1->a_w.w_symbol != a2->a_w.w_symbol)
                    || (!wholeword &&  !strstr(a1->a_w.w_symbol->s_name,
                                        a2->a_w.w_symbol->s_name)))
                        goto nomatch;
            }           
        }
        return (1);
    nomatch: ;
    }
    return (0);
}

    /* find an atom or string of atoms */
static int canvas_dofind(t_glist *x, int *myindexp)
{
    t_gobj *y;
    int findargc = buffer_size(canvas_findbuf), didit = 0;
    t_atom *findargv = buffer_atoms(canvas_findbuf);
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        t_object *ob = 0;
        if (ob = canvas_castToObjectIfBox(&y->g_pd))
        {
            if (atoms_match(buffer_size(ob->te_buffer), 
                buffer_atoms(ob->te_buffer), findargc, findargv,
                    canvas_find_wholeword))
            {
                if (*myindexp == canvas_find_index)
                {
                    glist_noselect(x);
                    pd_vMessage(&x->gl_obj.te_g.g_pd, gensym ("open"), "");
                    canvas_editmode(x, 1.);
                    glist_select(x, y);
                    didit = 1;
                }
                (*myindexp)++;
            }
        }
    }
    for (y = x->gl_graphics; y; y = y->g_next)
        if (pd_class(&y->g_pd) == canvas_class)
            didit |= canvas_dofind((t_glist *)y, myindexp);
    return (didit);
}

/*
static void canvas_find(t_glist *x, t_symbol *s, t_float wholeword)
{
    int myindex = 0, found;
    t_symbol *decodedsym = sys_decodedialog(s);
    if (!canvas_findbuf)
        canvas_findbuf = buffer_new();
    buffer_withStringUnzeroed(canvas_findbuf, decodedsym->s_name, strlen(decodedsym->s_name));
    canvas_find_index = 0;
    canvas_find_wholeword = wholeword;
    canvas_whichfind = x;
    found = canvas_dofind(x, &myindex);
    if (found)
        canvas_find_index = 1;
    sys_vGui("::dialog_find::pdtk_showfindresult .x%lx %d %d %d\n", x, found, canvas_find_index,
        myindex);
}

static void canvas_find_again(t_glist *x)
{
    int myindex = 0, found;
    if (!canvas_findbuf || !canvas_whichfind)
        return;
    found = canvas_dofind(canvas_whichfind, &myindex);
    sys_vGui("::dialog_find::pdtk_showfindresult .x%lx %d %d %d\n", x, found, ++canvas_find_index,
        myindex);
}
*/
void canvas_find_parent(t_glist *x)
{
    if (x->gl_owner)
        canvas_vis(glist_getcanvas(x->gl_owner), 1);
}

void canvas_stowconnections(t_glist *x)
{
    t_gobj *selhead = 0, *seltail = 0, *nonhead = 0, *nontail = 0, *y, *y2;
    t_linetraverser t;
    t_outconnect *oc;
    if (!x->gl_editor) return;
        /* split list to "selected" and "unselected" parts */ 
    for (y = x->gl_graphics; y; y = y2)
    {
        y2 = y->g_next;
        if (glist_isselected(x, y))
        {
            if (seltail)
            {
                seltail->g_next = y;
                seltail = y;
                y->g_next = 0;
            }
            else
            {
                selhead = seltail = y;
                seltail->g_next = 0;
            }
        }
        else
        {
            if (nontail)
            {
                nontail->g_next = y;
                nontail = y;
                y->g_next = 0;
            }
            else
            {
                nonhead = nontail = y;
                nontail->g_next = 0;
            }
        }
    }
        /* move the selected part to the end */
    if (!nonhead) x->gl_graphics = selhead;
    else x->gl_graphics = nonhead, nontail->g_next = selhead;

        /* add connections to binbuf */
    buffer_reset(x->gl_editor->e_connectbuf);
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        int s1 = glist_isselected(x, &t.tr_srcObject->te_g);
        int s2 = glist_isselected(x, &t.tr_destObject->te_g);
        if (s1 != s2)
            buffer_vAppend(x->gl_editor->e_connectbuf, "ssiiii;",
                gensym ("#X"), gensym ("connect"),
                    glist_getindex(x, &t.tr_srcObject->te_g), t.tr_srcIndexOfOutlet,
                        glist_getindex(x, &t.tr_destObject->te_g), t.tr_destIndexOfInlet);
    }
}

void canvas_restoreconnections(t_glist *x)
{
    t_pd *boundx = s__X.s_thing;
    s__X.s_thing = &x->gl_obj.te_g.g_pd;
    buffer_eval(x->gl_editor->e_connectbuf, 0, 0, 0);
    s__X.s_thing = boundx;
}

static t_buffer *canvas_docopy(t_glist *x)
{
    t_gobj *y;
    t_linetraverser t;
    t_outconnect *oc;
    t_buffer *b = buffer_new();
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        if (glist_isselected(x, y))
            gobj_save(y, b);
    }
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if (glist_isselected(x, &t.tr_srcObject->te_g)
            && glist_isselected(x, &t.tr_destObject->te_g))
        {
            buffer_vAppend(b, "ssiiii;", gensym ("#X"), gensym ("connect"),
                glist_selectionindex(x, &t.tr_srcObject->te_g, 1), t.tr_srcIndexOfOutlet,
                glist_selectionindex(x, &t.tr_destObject->te_g, 1), t.tr_destIndexOfInlet);
        }
    }
    return (b);
}

static void canvas_copy(t_glist *x)
{
    if (!x->gl_editor || !x->gl_editor->e_selection)
        return;
    buffer_free(copy_binbuf);
    copy_binbuf = canvas_docopy(x);
    if (x->gl_editor->e_textedfor)
    {
        char *buf;
        int bufsize;
        rtext_getseltext(x->gl_editor->e_textedfor, &buf, &bufsize);
        sys_gui("clipboard clear\n");
        sys_vGui("clipboard append {%.*s}\n", bufsize, buf);
    }
}

static void canvas_clearline(t_glist *x)
{
    if (x->gl_editor->e_selectedline)
    {
        canvas_disconnect(x, x->gl_editor->e_selectline_index1,
             x->gl_editor->e_selectline_outno,
             x->gl_editor->e_selectline_index2,
             x->gl_editor->e_selectline_inno);
        canvas_dirty(x, 1);
        canvas_setundo(x, canvas_undo_disconnect,
            canvas_undo_set_disconnect(x,
                x->gl_editor->e_selectline_index1,
                x->gl_editor->e_selectline_outno,
                x->gl_editor->e_selectline_index2,
                x->gl_editor->e_selectline_inno),
            "disconnect");
    }
}

extern t_pd *pd_newest;
static void canvas_doclear(t_glist *x)
{
    t_gobj *y, *y2;
    int dspstate;

    dspstate = dsp_suspend();
    if (x->gl_editor->e_selectedline)
    {
        canvas_disconnect(x, x->gl_editor->e_selectline_index1,
             x->gl_editor->e_selectline_outno,
             x->gl_editor->e_selectline_index2,
             x->gl_editor->e_selectline_inno);
        canvas_setundo(x, canvas_undo_disconnect,
            canvas_undo_set_disconnect(x,
                x->gl_editor->e_selectline_index1,
                x->gl_editor->e_selectline_outno,
                x->gl_editor->e_selectline_index2,
                x->gl_editor->e_selectline_inno),
            "disconnect");
    }
        /* if text is selected, deselecting it might remake the
        object. So we deselect it and hunt for a "new" object on
        the glist to reselect. */
    if (x->gl_editor->e_textedfor)
    {
        t_gobj *selwas = x->gl_editor->e_selection->sel_what;
        pd_newest = 0;
        glist_noselect(x);
        if (pd_newest)
        {
            for (y = x->gl_graphics; y; y = y->g_next)
                if (&y->g_pd == pd_newest) glist_select(x, y);
        }
    }
    while (1)   /* this is pretty wierd...  should rewrite it */
    {
        for (y = x->gl_graphics; y; y = y2)
        {
            y2 = y->g_next;
            if (glist_isselected(x, y))
            {
                glist_delete(x, y);
                goto next;
            }
        }
        goto restore;
    next: ;
    }
restore:
    dsp_resume(dspstate);
    canvas_dirty(x, 1);
}

static void canvas_cut(t_glist *x)
{
    if (!x->gl_editor)  /* ignore if invisible */ 
        return;
    if (x->gl_editor && x->gl_editor->e_selectedline)   /* delete line */
        canvas_clearline(x);
    else if (x->gl_editor->e_textedfor) /* delete selected text in a box */
    {
        char *buf;
        int bufsize;
        rtext_getseltext(x->gl_editor->e_textedfor, &buf, &bufsize);
        if (!bufsize && x->gl_editor->e_selection &&
            !x->gl_editor->e_selection->sel_next)
        {
                /* if the text is already empty, delete the box.  We
                first clear 'textedfor' so that canvas_doclear later will
                think the whole box was selected, not the text */
            x->gl_editor->e_textedfor = 0;
            goto deleteobj;
        }
        canvas_copy(x);
        rtext_key(x->gl_editor->e_textedfor, 127, &s_);
        canvas_dirty(x, 1);
    }
    else if (x->gl_editor && x->gl_editor->e_selection)
    {
    deleteobj:      /* delete one or more objects */
        canvas_setundo(x, canvas_undo_cut,
            canvas_undo_set_cut(x, UCUT_CUT), "cut");
        canvas_copy(x);
        canvas_doclear(x);
        sys_vGui("::ui_patch::updateScrollRegion .x%lx.c\n", x);
    }
}

static int paste_onset;
static t_glist *paste_canvas;

static void glist_donewloadbangs(t_glist *x)
{
    if (x->gl_editor)
    {
        t_selection *sel;
        for (sel = x->gl_editor->e_selection; sel; sel = sel->sel_next)
            if (pd_class(&sel->sel_what->g_pd) == canvas_class)
                canvas_loadbang((t_glist *)(&sel->sel_what->g_pd));
    }
}

static void canvas_dopaste(t_glist *x, t_buffer *b)
{
    t_gobj *newgobj, *last, *g2;
    int dspstate = dsp_suspend(), nbox, count;
    t_symbol *asym = gensym ("#A");
        /* save and clear bindings to symbols #a, $N, $X; restore when done */
    t_pd *boundx = s__X.s_thing, *bounda = asym->s_thing, 
        *boundn = s__N.s_thing;
    asym->s_thing = 0;
    s__X.s_thing = &x->gl_obj.te_g.g_pd;
    s__N.s_thing = &pd_canvasMaker;

    canvas_editmode(x, 1.);
    glist_noselect(x);
    for (g2 = x->gl_graphics, nbox = 0; g2; g2 = g2->g_next) nbox++;

    paste_onset = nbox;
    paste_canvas = x;

    buffer_eval(b, 0, 0, 0);
    for (g2 = x->gl_graphics, count = 0; g2; g2 = g2->g_next, count++)
        if (count >= nbox)
            glist_select(x, g2);
    paste_canvas = 0;
    dsp_resume(dspstate);
    canvas_dirty(x, 1);
    sys_vGui("::ui_patch::updateScrollRegion .x%lx.c\n", x);
    glist_donewloadbangs(x);
    asym->s_thing = bounda;
    s__X.s_thing = boundx;
    s__N.s_thing = boundn;
}

static void canvas_paste(t_glist *x)
{
    if (!x->gl_editor)
        return;
    if (x->gl_editor->e_textedfor)
    {
        /* simulate keystrokes as if the copy buffer were typed in. */
        // sys_vGui("::ui_object::pasteText .x%lx\n", x);
    }
    else
    {
        canvas_setundo(x, canvas_undo_paste, canvas_undo_set_paste(x),
            "paste");
        canvas_dopaste(x, copy_binbuf);
    }
}

static void canvas_duplicate(t_glist *x)
{
    if (!x->gl_editor)
        return;
    if (x->gl_editor->e_onmotion == ACTION_NONE && x->gl_editor->e_selection)
    {
        t_selection *y;
        canvas_copy(x);
        canvas_setundo(x, canvas_undo_paste, canvas_undo_set_paste(x),
            "duplicate");
        canvas_dopaste(x, copy_binbuf);
        for (y = x->gl_editor->e_selection; y; y = y->sel_next)
            gobj_displace(y->sel_what, x,
                10, 10);
        canvas_dirty(x, 1);
    }
}

static void canvas_selectall(t_glist *x)
{
    t_gobj *y;
    if (!x->gl_editor)
        return;
    if (!x->gl_isEditMode)
        canvas_editmode(x, 1);
            /* if everyone is already selected deselect everyone */
    if (!glist_selectionindex(x, 0, 0))
        glist_noselect(x);
    else for (y = x->gl_graphics; y; y = y->g_next)
    {
        if (!glist_isselected(x, y))
            glist_select(x, y);
    }
}

static void canvas_reselect(t_glist *x)
{
    t_gobj *g, *gwas;
    t_selection *sel;
    t_object *ob;
        /* if someone is text editing, and if only one object is 
        selected,  deselect everyone and reselect.  */
    if (x->gl_editor->e_textedfor)
    {
            /* only do this if exactly one item is selected. */
        if ((gwas = x->gl_editor->e_selection->sel_what) &&
            !x->gl_editor->e_selection->sel_next)
        {
            int nobjwas = glist_getindex(x, 0),
                indx = canvas_getIndexOfObject(x, x->gl_editor->e_selection->sel_what);
            glist_noselect(x);
            for (g = x->gl_graphics; g; g = g->g_next)
                if (g == gwas)
            {
                glist_select(x, g);
                return;
            }
                /* "gwas" must have disappeared; just search to the last
                object and select it */
            for (g = x->gl_graphics; g; g = g->g_next)
                if (!g->g_next)
                    glist_select(x, g);
        }
    }
    else if (x->gl_editor->e_selection &&
        !x->gl_editor->e_selection->sel_next)
            /* otherwise activate first item in selection */
            gobj_activate(x->gl_editor->e_selection->sel_what, x, 1);
}

extern t_class *text_class;

void canvas_connect(t_glist *x, t_float fwhoout, t_float foutno,
    t_float fwhoin, t_float finno)
{
    int whoout = fwhoout, outno = foutno, whoin = fwhoin, inno = finno;
    t_gobj *src = 0, *sink = 0;
    t_object *objsrc, *objsink;
    t_outconnect *oc;
    int nin = whoin, nout = whoout;
    if (paste_canvas == x) whoout += paste_onset, whoin += paste_onset;
    for (src = x->gl_graphics; whoout; src = src->g_next, whoout--)
        if (!src->g_next) goto bad; /* bug fix thanks to Hannes */
    for (sink = x->gl_graphics; whoin; sink = sink->g_next, whoin--)
        if (!sink->g_next) goto bad;
    
        /* check they're both patchable objects */
    if (!(objsrc = canvas_castToObjectIfBox(&src->g_pd)) ||
        !(objsink = canvas_castToObjectIfBox(&sink->g_pd)))
            goto bad;
    
        /* if object creation failed, make dummy inlets or outlets
        as needed */ 
    if (pd_class(&src->g_pd) == text_class && objsrc->te_type == TYPE_OBJECT)
        while (outno >= object_numberOfOutlets(objsrc))
            outlet_new(objsrc, 0);
    if (pd_class(&sink->g_pd) == text_class && objsink->te_type == TYPE_OBJECT)
        while (inno >= object_numberOfInlets(objsink))
            inlet_new(objsink, &objsink->te_g.g_pd, 0, 0);

    if (!(oc = object_connect(objsrc, outno, objsink, inno))) goto bad;
    if (canvas_isVisible(x))
    {
        sys_vGui(".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
            glist_getcanvas(x), 0, 0, 0, 0,
            (object_isSignalOutlet(objsrc, outno) ? 2 : 1),oc);
        canvas_updateLinesByObject(x, objsrc);
    }
    return;

bad:
    post("%s %d %d %d %d (%s->%s) connection failed", 
        x->gl_name->s_name, nout, outno, nin, inno,
            (src? class_getName(pd_class(&src->g_pd)) : "???"),
            (sink? class_getName(pd_class(&sink->g_pd)) : "???"));
}

#define XTOLERANCE 18
#define YTOLERANCE 17
#define NHIST 35

    /* LATER might have to speed this up */
static void canvas_tidy(t_glist *x)
{
    t_gobj *y, *y2, *y3;
    int ax1, ay1, ax2, ay2, bx1, by1, bx2, by2;
    int histogram[NHIST], *ip, i, besthist, bestdist;
        /* if nobody is selected, this means do it to all boxes;
        othewise just the selection */
    int all = (x->gl_editor ? (x->gl_editor->e_selection == 0) : 1);

    canvas_setundo(x, canvas_undo_move, canvas_undo_set_move(x, !all),
        "motion");

        /* tidy horizontally */
    for (y = x->gl_graphics; y; y = y->g_next)
        if (all || glist_isselected(x, y))
    {
        gobj_getrect(y, x, &ax1, &ay1, &ax2, &ay2);

        for (y2 = x->gl_graphics; y2; y2 = y2->g_next)
            if (all || glist_isselected(x, y2))
        {
            gobj_getrect(y2, x, &bx1, &by1, &bx2, &by2);
            if (by1 <= ay1 + YTOLERANCE && by1 >= ay1 - YTOLERANCE &&
                bx1 < ax1)
                    goto nothorizhead;
        }

        for (y2 = x->gl_graphics; y2; y2 = y2->g_next)
            if (all || glist_isselected(x, y2))
        {
            gobj_getrect(y2, x, &bx1, &by1, &bx2, &by2);
            if (by1 <= ay1 + YTOLERANCE && by1 >= ay1 - YTOLERANCE
                && by1 != ay1)
                    gobj_displace(y2, x, 0, ay1-by1);
        }
    nothorizhead: ;
    }
        /* tidy vertically.  First guess the user's favorite vertical spacing */
    for (i = NHIST, ip = histogram; i--; ip++) *ip = 0;
    for (y = x->gl_graphics; y; y = y->g_next)
        if (all || glist_isselected(x, y))
    {
        gobj_getrect(y, x, &ax1, &ay1, &ax2, &ay2);
        for (y2 = x->gl_graphics; y2; y2 = y2->g_next)
            if (all || glist_isselected(x, y2))
        {
            gobj_getrect(y2, x, &bx1, &by1, &bx2, &by2);
            if (bx1 <= ax1 + XTOLERANCE && bx1 >= ax1 - XTOLERANCE)
            {
                int distance = by1-ay2;
                if (distance >= 0 && distance < NHIST)
                    histogram[distance]++;
            }
        }
    }
    for (i = 2, besthist = 0, bestdist = 4, ip = histogram + 2;
        i < (NHIST-2); i++, ip++)
    {
        int hit = ip[-2] + 2 * ip[-1] + 3 * ip[0] + 2* ip[1] + ip[2];
        if (hit > besthist)
        {
            besthist = hit;
            bestdist = i;
        }
    }
    post("best vertical distance %d", bestdist);
    for (y = x->gl_graphics; y; y = y->g_next)
        if (all || glist_isselected(x, y))
    {
        int keep = 1;
        gobj_getrect(y, x, &ax1, &ay1, &ax2, &ay2);
        for (y2 = x->gl_graphics; y2; y2 = y2->g_next)
            if (all || glist_isselected(x, y2))
        {
            gobj_getrect(y2, x, &bx1, &by1, &bx2, &by2);
            if (bx1 <= ax1 + XTOLERANCE && bx1 >= ax1 - XTOLERANCE &&
                ay1 >= by2 - 10 && ay1 < by2 + NHIST)
                    goto nothead;
        }
        while (keep)
        {
            keep = 0;
            for (y2 = x->gl_graphics; y2; y2 = y2->g_next)
                if (all || glist_isselected(x, y2))
            {
                gobj_getrect(y2, x, &bx1, &by1, &bx2, &by2);
                if (bx1 <= ax1 + XTOLERANCE && bx1 >= ax1 - XTOLERANCE &&
                    by1 > ay1 && by1 < ay2 + NHIST)
                {
                    int vmove = ay2 + bestdist - by1;
                    gobj_displace(y2, x, ax1-bx1, vmove);
                    ay1 = by1 + vmove;
                    ay2 = by2 + vmove;
                    keep = 1;
                    break;
                }
            }
        }
    nothead: ;
    }
    canvas_dirty(x, 1);
}

static void canvas_texteditor(t_glist *x)
{
    t_boxtext *foo;
    char *buf;
    int bufsize;
    if (foo = x->gl_editor->e_textedfor)
        rtext_gettext(foo, &buf, &bufsize);
    else buf = "", bufsize = 0;
    sys_vGui("pdtk_pd_texteditor {%.*s}\n", bufsize, buf);
    
}

void global_key (void *dummy, t_symbol *s, int ac, t_atom *av)
{
        /* canvas_key checks for zero */
    canvas_key(0, s, ac, av);
}

void canvas_editmode(t_glist *x, t_float state)
{
    if (x->gl_isEditMode == (unsigned int) state)
        return;
    x->gl_isEditMode = (unsigned int) state;
    if (x->gl_isEditMode && canvas_isVisible(x) && canvas_isTopLevel(x))
    {
        t_gobj *g;
        t_object *ob;
        canvas_setcursor(x, CURSOR_EDIT_NOTHING);
        for (g = x->gl_graphics; g; g = g->g_next)
            if ((ob = canvas_castToObjectIfBox(&g->g_pd)) && ob->te_type == TYPE_TEXT)
        {
            t_boxtext *y = glist_findrtext(x, ob);
            text_drawborder(ob, x,
                rtext_gettag(y), rtext_width(y), rtext_height(y), 1);
        }
    }
    else
    {
        glist_noselect(x);
        if (canvas_isVisible(x) && canvas_isTopLevel(x))
        {
            canvas_setcursor(x, CURSOR_NOTHING);
            sys_vGui(".x%lx.c delete commentbar\n", glist_getcanvas(x));
        }
    }
    if (canvas_isVisible(x))
      sys_vGui("::ui_patch::setEditMode .x%lx %d\n",
          glist_getcanvas(x), x->gl_isEditMode);
}

    /* called by canvas_font below */
static void canvas_dofont(t_glist *x, t_float font, t_float xresize,
    t_float yresize)
{
    t_gobj *y;
    x->gl_fontSize = font;
    if (xresize != 1 || yresize != 1)
    {
        canvas_setundo(x, canvas_undo_move, canvas_undo_set_move(x, 0),
            "motion");
        for (y = x->gl_graphics; y; y = y->g_next)
        {
            int x1, x2, y1, y2, nx1, ny1;
            gobj_getrect(y, x, &x1, &y1, &x2, &y2);
            nx1 = x1 * xresize + 0.5;
            ny1 = y1 * yresize + 0.5;
            gobj_displace(y, x, nx1-x1, ny1-y1);
        }
    }
    if (canvas_isVisible(x))
        glist_redraw(x);
    for (y = x->gl_graphics; y; y = y->g_next)
        if (canvas_castToGlist(&y->g_pd)  && !canvas_isAbstraction((t_glist *)y))
            canvas_dofont((t_glist *)y, font, xresize, yresize);
}

    /* canvas_menufont calls up a TK dialog which calls this back */
static void canvas_font(t_glist *x, t_float font, t_float resize,
    t_float whichresize)
{
    t_float realresize, realresx = 1, realresy = 1;
    t_glist *x2 = canvas_getRoot(x);
    if (!resize) realresize = 1;
    else
    {
        if (resize < 20) resize = 20;
        if (resize > 500) resize = 500;
        realresize = resize * 0.01;
    }
    if (whichresize != 3) realresx = realresize;
    if (whichresize != 2) realresy = realresize;
    canvas_dofont(x2, font, realresx, realresy);
    font_setDefaultFontSize (font);
}

static t_glist *canvas_last_glist;
static int canvas_last_glist_x, canvas_last_glist_y;

void glist_getnextxy(t_glist *gl, int *xpix, int *ypix)
{
    if (canvas_last_glist == gl)
        *xpix = canvas_last_glist_x, *ypix = canvas_last_glist_y;
    else *xpix = *ypix = 40;
}

static void glist_setlastxy(t_glist *gl, int xval, int yval)
{
    canvas_last_glist = gl;
    canvas_last_glist_x = xval;
    canvas_last_glist_y = yval;
}


void g_editor_setup(void)
{
/* ------------------------ events ---------------------------------- */
    class_addMethod(canvas_class, (t_method)canvas_mouse, gensym ("mouse"),
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_mouseup, gensym ("mouseup"),
        A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_key, gensym ("key"),
        A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_motion, gensym ("motion"),
        A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);

/* ------------------------ menu actions ---------------------------- */
    class_addMethod(canvas_class, (t_method)canvas_menuclose,
        gensym ("menuclose"), A_DEFFLOAT, 0);
    class_addMethod(canvas_class, (t_method)canvas_cut,
        gensym ("cut"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_copy,
        gensym ("copy"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_paste,
        gensym ("paste"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_duplicate,
        gensym ("duplicate"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_selectall,
        gensym ("selectall"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_reselect,
        gensym ("reselect"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_undo,
        gensym ("undo"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_redo,
        gensym ("redo"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_tidy,
        gensym ("tidy"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_texteditor,
        gensym ("texteditor"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_editmode,
        gensym ("editmode"), A_DEFFLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_print,
        gensym ("print"), A_SYMBOL, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_menufont,
        gensym ("menufont"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_font,
        gensym ("font"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    /*class_addMethod(canvas_class, (t_method)canvas_find,
        gensym ("find"), A_SYMBOL, A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_find_again,
        gensym ("findagain"), A_NULL);*/
    class_addMethod(canvas_class, (t_method)canvas_find_parent,
        gensym ("findparent"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_done_popup,
        gensym ("done-popup"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_donecanvasdialog,
        gensym ("donecanvasdialog"), A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_arraydialog,
        gensym ("arraydialog"), A_SYMBOL, A_FLOAT, A_FLOAT, A_NULL);

/* -------------- connect method used in reading files ------------------ */
    class_addMethod(canvas_class, (t_method)canvas_connect,
        gensym ("connect"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);

    class_addMethod(canvas_class, (t_method)canvas_disconnect,
        gensym ("disconnect"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
/* -------------- copy buffer ------------------ */
    copy_binbuf = buffer_new();
}
