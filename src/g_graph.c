
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* This file deals with the behavior of glists as either "text objects" or
"graphs" inside another glist.  LATER move the inlet/outlet code of g_canvas.c 
to this file... */

#include <stdlib.h>
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

#include "g_canvas.h"
#include "s_system.h"    /* for font_getHostFontSize */
#include <stdio.h>
#include <string.h>

extern t_class *garray_class;
extern t_class *scalar_class;
extern int canvas_magic;
extern t_class *canvas_class;
extern t_class *vinlet_class;
extern t_class *voutlet_class;
extern t_widgetbehavior text_widgetBehavior;

/* ---------------------- forward definitions ----------------- */

static void graph_vis(t_gobj *gr, t_glist *unused_glist, int vis);
static void graph_graphrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2);
static void graph_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2);

/* -------------------- maintaining the list -------------------- */

void glist_add(t_glist *x, t_gobj *y)
{
    t_object *ob;
    y->g_next = 0;
    if (!x->gl_graphics) x->gl_graphics = y;
    else
    {
        t_gobj *y2;
        for (y2 = x->gl_graphics; y2->g_next; y2 = y2->g_next);
        y2->g_next = y;
    }
    if (x->gl_editor && (ob = canvas_castToObjectIfPatchable(&y->g_pd)))
        boxtext_new(x, ob);
    if (x->gl_editor && x->gl_isGraphOnParent && !x->gl_hasRectangle
        && canvas_castToObjectIfPatchable(&y->g_pd))
    {
        x->gl_hasRectangle = 1;
        canvas_drawGraphOnParentRectangle(x);
    }
    if (canvas_isMapped(x))
        gobj_visibilityChanged(y, x, 1);
    if (class_hasDrawCommand(y->g_pd)) 
        canvas_redrawAllByTemplate(template_findbyname(canvas_makeBindSymbol(
            canvas_getView(x)->gl_name)), SCALAR_REDRAW);
}

    /* this is to protect against a hairy problem in which deleting
    a sub-canvas might delete an inlet on a box, after the box had
    been invisible-ized, so that we have to protect against redrawing it! */
int canvas_setdeleting(t_glist *x, int flag)
{
    int ret = x->gl_isDeleting;
    x->gl_isDeleting = flag;
    return (ret);
}

    /* delete an object from a glist and free it */
void glist_delete(t_glist *x, t_gobj *y)
{
    t_gobj *g;
    t_object *ob;
    int chkdsp = class_hasMethod (pd_class (&y->g_pd), sym_dsp);
    t_glist *canvas = canvas_getView(x);
    t_boxtext *rtext = 0;
    int drawcommand = class_hasDrawCommand(y->g_pd);
    int wasdeleting;
    
    wasdeleting = canvas_setdeleting(canvas, 1);
    if (x->gl_editor)
    {
        if (x->gl_editor->e_grabbed == y) x->gl_editor->e_grabbed = 0;
        if (canvas_isObjectSelected(x, y)) canvas_deselectObject(x, y);

            /* HACK -- we had phantom outlets not getting erased on the
            screen because the canvas_setdeleting() mechanism is too
            crude.  LATER carefully set up rules for when the rtexts
            should exist, so that they stay around until all the
            steps of becoming invisible are done.  In the meantime, just
            zap the inlets and outlets here... */
        if (pd_class(&y->g_pd) == canvas_class)
        {
            t_glist *gl = (t_glist *)y;
            if (gl->gl_isGraphOnParent && canvas_isMapped(x))
            {
                char tag[80];
                sprintf(tag, "graph%lx", (t_int)gl);
                glist_eraseio(x, &gl->gl_obj, tag);
            }
            else
            {
                if (canvas_isMapped(x))
                    text_eraseborder(&gl->gl_obj, x,
                        boxtext_getTag(glist_findrtext(x, &gl->gl_obj)));
            }
        }
    }
        /* if we're a drawing command, erase all scalars now, before deleting
        it; we'll redraw them once it's deleted below. */
    if (drawcommand)
        canvas_redrawAllByTemplate(template_findbyname(canvas_makeBindSymbol(
            canvas_getView(x)->gl_name)), SCALAR_ERASE);
    gobj_delete(y, x);
    if (canvas_isMapped(canvas))
    {
        gobj_visibilityChanged(y, x, 0);
    }
    if (x->gl_editor && (ob = canvas_castToObjectIfPatchable(&y->g_pd)))
        rtext = boxtext_new(x, ob);
    if (x->gl_graphics == y) x->gl_graphics = y->g_next;
    else for (g = x->gl_graphics; g; g = g->g_next)
        if (g->g_next == y)
    {
        g->g_next = y->g_next;
        break;
    }
    pd_free(&y->g_pd);
    if (rtext)
        boxtext_free (rtext);
    if (chkdsp) dsp_update();
    if (drawcommand)
        canvas_redrawAllByTemplate(template_findbyname(canvas_makeBindSymbol(
            canvas_getView(x)->gl_name)), SCALAR_DRAW);
    canvas_setdeleting(canvas, wasdeleting);
    x->gl_magic = ++canvas_magic;
}

    /* remove every object from a glist.  Experimental. */
void glist_clear(t_glist *x)
{
    t_gobj *y, *y2;
    int dspstate = 0, suspended = 0;
    t_symbol *dspsym = sym_dsp;
    while (y = x->gl_graphics)
    {
            /* to avoid unnecessary DSP resorting, we suspend DSP
            only if we hit a patchable object. */
        if (!suspended && canvas_castToObjectIfPatchable(&y->g_pd) && class_hasMethod (pd_class (&y->g_pd), dspsym))
        {
            dspstate = dsp_suspend();
            suspended = 1;
        }
            /* here's the real deletion. */
        glist_delete(x, y);
    }
    if (suspended)
        dsp_resume(dspstate);
}

void glist_retext(t_glist *glist, t_object *y)
{
    t_glist *c = canvas_getView(glist);
        /* check that we have built rtexts yet.  LATER need a better test. */
    if (glist->gl_editor && glist->gl_editor->e_text)
    {
        t_boxtext *rt = glist_findrtext(glist, y);
        if (rt)
            rtext_retext(rt);
    }
}

void glist_grab(t_glist *x, t_gobj *y, t_motionfn motionfn,
    t_keyfn keyfn, int xpos, int ypos)
{
    t_glist *x2 = canvas_getView(x);
    if (motionfn)
        x2->gl_editor->e_action = ACTION_PASS;
    else x2->gl_editor->e_action = 0;
    x2->gl_editor->e_grabbed = y;
    x2->gl_editor->e_fnMotion = motionfn;
    x2->gl_editor->e_fnKey = keyfn;
    x2->gl_editor->e_previousX = xpos;
    x2->gl_editor->e_previousY = ypos;
}

static t_float gobj_getxforsort(t_gobj *g)
{
    if (pd_class(&g->g_pd) == scalar_class)
    {
        t_float x1, y1;
        scalar_getbasexy((t_scalar *)g, &x1, &y1);
        return(x1);
    }
    else return (0);
}

static t_gobj *glist_merge(t_glist *x, t_gobj *g1, t_gobj *g2)
{
    t_gobj *g = 0, *g9 = 0;
    t_float f1 = 0, f2 = 0;
    if (g1)
        f1 = gobj_getxforsort(g1);
    if (g2)
        f2 = gobj_getxforsort(g2);
    while (1)
    {
        if (g1)
        {
            if (g2)
            {
                if (f1 <= f2)
                    goto put1;
                else goto put2;
            }
            else goto put1;     
        }
        else if (g2)
            goto put2;
        else break;
    put1:
        if (g9)
            g9->g_next = g1, g9 = g1;
        else g9 = g = g1;
        if (g1 = g1->g_next)
            f1 = gobj_getxforsort(g1);
        g9->g_next = 0;
        continue;
    put2:
        if (g9)
            g9->g_next = g2, g9 = g2;
        else g9 = g = g2;
        if (g2 = g2->g_next)
            f2 = gobj_getxforsort(g2);
        g9->g_next = 0;
        continue;
    }
    return (g);
}

static t_gobj *glist_dosort(t_glist *x,
    t_gobj *g, int nitems)
{
    if (nitems < 2)
        return (g);
    else
    {
        int n1 = nitems/2, n2 = nitems - n1, i;
        t_gobj *g2, *g3;
        for (g2 = g, i = n1-1; i--; g2 = g2->g_next)
            ;
        g3 = g2->g_next;
        g2->g_next = 0;
        g = glist_dosort(x, g, n1);
        g3 = glist_dosort(x, g3, n2);
        return (glist_merge(x, g, g3));
    }
}

void glist_sort(t_glist *x)
{
    int nitems = 0, foo = 0;
    t_float lastx = -1e37;
    t_gobj *g;
    for (g = x->gl_graphics; g; g = g->g_next)
    {
        t_float x1 = gobj_getxforsort(g);
        if (x1 < lastx)
            foo = 1;
        lastx = x1;
        nitems++;
    }
    if (foo)
        x->gl_graphics = glist_dosort(x, x->gl_graphics, nitems);
}

/* --------------- inlets and outlets  ----------- */


t_inlet *canvas_addinlet(t_glist *x, t_pd *who, t_symbol *s)
{
    t_inlet *ip = inlet_new(&x->gl_obj, who, s, 0);
    if (!x->gl_isLoading && x->gl_parent && canvas_isMapped(x->gl_parent))
    {
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 0);
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 1);
        canvas_updateLinesByObject(x->gl_parent, &x->gl_obj);
    }
    if (!x->gl_isLoading) canvas_resortinlets(x);
    return (ip);
}

void canvas_rminlet(t_glist *x, t_inlet *ip)
{
    t_glist *owner = x->gl_parent;
    int redraw = (owner && canvas_isMapped(owner) && (!owner->gl_isDeleting)
        && canvas_canHaveWindow(owner));
    
    if (owner) canvas_deleteLinesByInlets(owner, &x->gl_obj, ip, 0);
    if (redraw)
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 0);
    inlet_free(ip);
    if (redraw)
    {
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 1);
        canvas_updateLinesByObject(x->gl_parent, &x->gl_obj);
    }
}

extern t_inlet *vinlet_getit(t_pd *x);


void canvas_resortinlets(t_glist *x)
{
    int ninlets = 0, i, j, xmax;
    t_gobj *y, **vec, **vp, **maxp;
    
    for (ninlets = 0, y = x->gl_graphics; y; y = y->g_next)
        if (pd_class(&y->g_pd) == vinlet_class) ninlets++;

    if (ninlets < 2) return;
    
    vec = (t_gobj **)PD_MEMORY_GET(ninlets * sizeof(*vec));
    
    for (y = x->gl_graphics, vp = vec; y; y = y->g_next)
        if (pd_class(&y->g_pd) == vinlet_class) *vp++ = y;
    
    for (i = ninlets; i--;)
    {
        t_inlet *ip;
        for (vp = vec, xmax = -PD_INT_MAX, maxp = 0, j = ninlets;
            j--; vp++)
        {
            int x1, y1, x2, y2;
            t_gobj *g = *vp;
            if (!g) continue;
            gobj_getRectangle(g, x, &x1, &y1, &x2, &y2);
            if (x1 > xmax) xmax = x1, maxp = vp;
        }
        if (!maxp) break;
        y = *maxp;
        *maxp = 0;
        ip = vinlet_getit(&y->g_pd);
        
        object_moveInletFirst(&x->gl_obj, ip);
    }
    PD_MEMORY_FREE(vec);
    if (x->gl_parent && canvas_isMapped(x->gl_parent))
        canvas_updateLinesByObject(x->gl_parent, &x->gl_obj);
}

t_outlet *canvas_addoutlet(t_glist *x, t_pd *who, t_symbol *s)
{
    t_outlet *op = outlet_new(&x->gl_obj, s);
    if (!x->gl_isLoading && x->gl_parent && canvas_isMapped(x->gl_parent))
    {
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 0);
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 1);
        canvas_updateLinesByObject(x->gl_parent, &x->gl_obj);
    }
    if (!x->gl_isLoading) canvas_resortoutlets(x);
    return (op);
}

void canvas_rmoutlet(t_glist *x, t_outlet *op)
{
    t_glist *owner = x->gl_parent;
    int redraw = (owner && canvas_isMapped(owner) && (!owner->gl_isDeleting)
        && canvas_canHaveWindow(owner));
    
    if (owner) canvas_deleteLinesByInlets(owner, &x->gl_obj, 0, op);
    if (redraw)
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 0);

    outlet_free(op);
    if (redraw)
    {
        gobj_visibilityChanged(&x->gl_obj.te_g, x->gl_parent, 1);
        canvas_updateLinesByObject(x->gl_parent, &x->gl_obj);
    }
}

extern t_outlet *voutlet_getit(t_pd *x);

void canvas_resortoutlets(t_glist *x)
{
    int noutlets = 0, i, j, xmax;
    t_gobj *y, **vec, **vp, **maxp;
    
    for (noutlets = 0, y = x->gl_graphics; y; y = y->g_next)
        if (pd_class(&y->g_pd) == voutlet_class) noutlets++;

    if (noutlets < 2) return;
    
    vec = (t_gobj **)PD_MEMORY_GET(noutlets * sizeof(*vec));
    
    for (y = x->gl_graphics, vp = vec; y; y = y->g_next)
        if (pd_class(&y->g_pd) == voutlet_class) *vp++ = y;
    
    for (i = noutlets; i--;)
    {
        t_outlet *ip;
        for (vp = vec, xmax = -PD_INT_MAX, maxp = 0, j = noutlets;
            j--; vp++)
        {
            int x1, y1, x2, y2;
            t_gobj *g = *vp;
            if (!g) continue;
            gobj_getRectangle(g, x, &x1, &y1, &x2, &y2);
            if (x1 > xmax) xmax = x1, maxp = vp;
        }
        if (!maxp) break;
        y = *maxp;
        *maxp = 0;
        ip = voutlet_getit(&y->g_pd);
        
        object_moveOutletFirst(&x->gl_obj, ip);
    }
    PD_MEMORY_FREE(vec);
    if (x->gl_parent && canvas_isMapped(x->gl_parent))
        canvas_updateLinesByObject(x->gl_parent, &x->gl_obj);
}

/* ----------calculating coordinates and controlling appearance --------- */


static void graph_bounds(t_glist *x, t_float x1, t_float y1,
    t_float x2, t_float y2)
{
    x->gl_valueStart = x1;
    x->gl_valueEnd = x2;
    x->gl_valueUp = y1;
    x->gl_valueDown = y2;
    if (x->gl_valueEnd == x->gl_valueStart ||
        x->gl_valueDown == x->gl_valueUp)
    {
        post_error ("graph: empty bounds rectangle");
        x1 = y1 = 0;
        x2 = y2 = 1;
    }
    glist_redraw(x);
}

static void graph_xticks(t_glist *x,
    t_float point, t_float inc, t_float f)
{
    x->gl_tickX.k_point = point;
    x->gl_tickX.k_inc = inc;
    x->gl_tickX.k_lperb = f;
    glist_redraw(x);
}

static void graph_yticks(t_glist *x,
    t_float point, t_float inc, t_float f)
{
    x->gl_tickY.k_point = point;
    x->gl_tickY.k_inc = inc;
    x->gl_tickY.k_lperb = f;
    glist_redraw(x);
}

/*
static void graph_xlabel(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    if (argc < 1) post_error ("graph_xlabel: no y value given");
    else
    {
        x->gl_xlabely = atom_getFloat(argv);
        argv++; argc--;
        x->gl_xlabel = (t_symbol **)PD_MEMORY_RESIZE(x->gl_xlabel, 
            x->gl_nxlabels * sizeof (t_symbol *), argc * sizeof (t_symbol *));
        x->gl_nxlabels = argc;
        for (i = 0; i < argc; i++) x->gl_xlabel[i] = atom_gensym(&argv[i]);
    }
    glist_redraw(x);
}
    
static void graph_ylabel(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    if (argc < 1) post_error ("graph_ylabel: no x value given");
    else
    {
        x->gl_ylabelx = atom_getFloat(argv);
        argv++; argc--;
        x->gl_ylabel = (t_symbol **)PD_MEMORY_RESIZE(x->gl_ylabel, 
            x->gl_nylabels * sizeof (t_symbol *), argc * sizeof (t_symbol *));
        x->gl_nylabels = argc;
        for (i = 0; i < argc; i++) x->gl_ylabel[i] = atom_gensym(&argv[i]);
    }
    glist_redraw(x);
}
*/

/****** routines to convert pixels to X or Y value and vice versa ******/

    /* convert an x pixel value to an x coordinate value */
t_float glist_pixelstox(t_glist *x, t_float xpix)
{
        /* if we appear as a text box on parent, our range in our
        coordinates (x1, etc.) specifies the coordinate range
        of a one-pixel square at top left of the window. */
    if (!x->gl_isGraphOnParent)
        return (x->gl_valueStart + (x->gl_valueEnd - x->gl_valueStart) * xpix);

        /* if we're a graph when shown on parent, but own our own
        window right now, our range in our coordinates (x1, etc.) is spread
        over the visible window size, given by screenx1, etc. */  
    else if (x->gl_isGraphOnParent && x->gl_haveWindow)
        return (x->gl_valueStart + (x->gl_valueEnd - x->gl_valueStart) * 
            (xpix) / (x->gl_windowBottomRightX - x->gl_windowTopLeftX));

        /* otherwise, we appear in a graph within a parent glist,
         so get our screen rectangle on parent and transform. */
    else 
    {
        int x1, y1, x2, y2;
        if (!x->gl_parent) { PD_BUG; }       
        graph_graphrect(&x->gl_obj.te_g, x->gl_parent, &x1, &y1, &x2, &y2);
        return (x->gl_valueStart + (x->gl_valueEnd - x->gl_valueStart) * 
            (xpix - x1) / (x2 - x1));
    }
}

t_float glist_pixelstoy(t_glist *x, t_float ypix)
{
    if (!x->gl_isGraphOnParent)
        return (x->gl_valueUp + (x->gl_valueDown - x->gl_valueUp) * ypix);
    else if (x->gl_isGraphOnParent && x->gl_haveWindow)
        return (x->gl_valueUp + (x->gl_valueDown - x->gl_valueUp) * 
                (ypix) / (x->gl_windowBottomRightY - x->gl_windowTopLeftY));
    else 
    {
        int x1, y1, x2, y2;
        if (!x->gl_parent) { PD_BUG; }
        graph_graphrect(&x->gl_obj.te_g, x->gl_parent, &x1, &y1, &x2, &y2);
        return (x->gl_valueUp + (x->gl_valueDown - x->gl_valueUp) * 
            (ypix - y1) / (y2 - y1));
    }
}

    /* convert an x coordinate value to an x pixel location in window */
t_float glist_xtopixels(t_glist *x, t_float xval)
{
    if (!x->gl_isGraphOnParent)
        return ((xval - x->gl_valueStart) / (x->gl_valueEnd - x->gl_valueStart));
    else if (x->gl_isGraphOnParent && x->gl_haveWindow)
        return (x->gl_windowBottomRightX - x->gl_windowTopLeftX) * 
            (xval - x->gl_valueStart) / (x->gl_valueEnd - x->gl_valueStart);
    else
    {
        int x1, y1, x2, y2;
        if (!x->gl_parent) { PD_BUG; }
        graph_graphrect(&x->gl_obj.te_g, x->gl_parent, &x1, &y1, &x2, &y2);
        return (x1 + (x2 - x1) * (xval - x->gl_valueStart) / (x->gl_valueEnd - x->gl_valueStart));
    }
}

t_float glist_ytopixels(t_glist *x, t_float yval)
{
    if (!x->gl_isGraphOnParent)
        return ((yval - x->gl_valueUp) / (x->gl_valueDown - x->gl_valueUp));
    else if (x->gl_isGraphOnParent && x->gl_haveWindow)
        return (x->gl_windowBottomRightY - x->gl_windowTopLeftY) * 
                (yval - x->gl_valueUp) / (x->gl_valueDown - x->gl_valueUp);
    else 
    {
        int x1, y1, x2, y2;
        if (!x->gl_parent) { PD_BUG; }
        graph_graphrect(&x->gl_obj.te_g, x->gl_parent, &x1, &y1, &x2, &y2);
        return (y1 + (y2 - y1) * (yval - x->gl_valueUp) / (x->gl_valueDown - x->gl_valueUp));
    }
}

    /* convert an X screen distance to an X coordinate increment.
      This is terribly inefficient;
      but probably not a big enough CPU hog to warrant optimizing. */
t_float glist_dpixtodx(t_glist *x, t_float dxpix)
{ 
    return (dxpix * (glist_pixelstox(x, 1) - glist_pixelstox(x, 0)));
}

t_float glist_dpixtody(t_glist *x, t_float dypix)
{
    return (dypix * (glist_pixelstoy(x, 1) - glist_pixelstoy(x, 0)));
}

    /* get the window location in pixels of a "text" object.  The
    object's x and y positions are in pixels when the glist they're
    in is toplevel.  Otherwise, if it's a new-style graph-on-parent
    (so gl_hasRectangle is set) we use the offset into the framing subrectangle
    as an offset into the parent rectangle.  Finally, it might be an old,
    proportional-style GOP.  In this case we do a coordinate transformation. */
int text_xpix(t_object *x, t_glist *glist)
{
    if (canvas_canHaveWindow (glist))
        return (x->te_xCoordinate);
    else if (glist->gl_hasRectangle)
        return (glist_xtopixels(glist, glist->gl_valueStart) +
            x->te_xCoordinate - glist->gl_marginX);
    else return (glist_xtopixels(glist, 
            glist->gl_valueStart + (glist->gl_valueEnd - glist->gl_valueStart) * 
                x->te_xCoordinate / (glist->gl_windowBottomRightX - glist->gl_windowTopLeftX)));
}

int text_ypix(t_object *x, t_glist *glist)
{
    if (canvas_canHaveWindow (glist))
        return (x->te_yCoordinate);
    else if (glist->gl_hasRectangle)
        return (glist_ytopixels(glist, glist->gl_valueUp) +
            x->te_yCoordinate - glist->gl_marginY);
    else return (glist_ytopixels(glist, 
            glist->gl_valueUp + (glist->gl_valueDown - glist->gl_valueUp) * 
                x->te_yCoordinate / (glist->gl_windowBottomRightY - glist->gl_windowTopLeftY)));
}

    /* redraw all the items in a glist.  We construe this to mean
    redrawing in its own window and on parent, as needed in each case.
    This is too conservative -- for instance, when you draw an "open"
    rectangle on the parent, you shouldn't have to redraw the window!  */
void glist_redraw(t_glist *x)
{  
    if (canvas_isMapped(x))
    {
            /* LATER fix the graph_vis() code to handle both cases */
        if (canvas_canHaveWindow(x))
        {
            t_gobj *g;
            t_linetraverser t;
            t_outconnect *oc;
            for (g = x->gl_graphics; g; g = g->g_next)
            {
                gobj_visibilityChanged(g, x, 0);
                gobj_visibilityChanged(g, x, 1);
            }
                /* redraw all the lines */
            canvas_traverseLinesStart(&t, x);
            while (oc = canvas_traverseLinesNext(&t))
                sys_vGui(".x%lx.c coords %lxLINE %d %d %d %d\n",
                    canvas_getView(x), oc,
                        t.tr_lineStartX, t.tr_lineStartY, t.tr_lineEndX, t.tr_lineEndY);
            canvas_deleteGraphOnParentRectangle(x);
            if (x->gl_hasRectangle)
            {
                canvas_drawGraphOnParentRectangle(x);
            }
        }
        if (x->gl_parent && canvas_isMapped(x->gl_parent))
        {
            graph_vis(&x->gl_obj.te_g, x->gl_parent, 0); 
            graph_vis(&x->gl_obj.te_g, x->gl_parent, 1);
        }
    }
}

/* --------------------------- widget behavior  ------------------- */

int garray_getname(t_garray *x, t_symbol **namep);


    /* Note that some code in here would also be useful for drawing
    graph decorations in toplevels... */
static void graph_vis(t_gobj *gr, t_glist *parent_glist, int vis)
{
    t_glist *x = (t_glist *)gr;
    char tag[50];
    t_gobj *g;
    int x1, y1, x2, y2;
        /* ordinary subpatches: just act like a text object */
    if (!x->gl_isGraphOnParent)
    {
        text_widgetBehavior.w_fnVisible(gr, parent_glist, vis);
        return;
    }

    if (vis && canvas_hasGraphOnParentTitle (x))
        rtext_draw(glist_findrtext(parent_glist, &x->gl_obj));
    graph_getrect(gr, parent_glist, &x1, &y1, &x2, &y2);
    if (!vis)
        rtext_erase(glist_findrtext(parent_glist, &x->gl_obj));

    sprintf(tag, "graph%lx", (t_int)x);
    if (vis)
        glist_drawio(parent_glist, &x->gl_obj, 1,
            tag, x1, y1, x2, y2);
    else glist_eraseio(parent_glist, &x->gl_obj, tag);
        /* if we look like a graph but have been moved to a toplevel,
        just show the bounding rectangle */
    if (x->gl_haveWindow)
    {
        if (vis)
        {
            sys_vGui(".x%lx.c create polygon\
 %d %d %d %d %d %d %d %d %d %d -tags [list %s graph] -fill #c0c0c0\n",
                canvas_getView(x->gl_parent),
                x1, y1, x1, y2, x2, y2, x2, y1, x1, y1, tag);
        }
        else
        {
            sys_vGui(".x%lx.c delete %s\n",
                canvas_getView(x->gl_parent), tag);
        }
        return;
    }
        /* otherwise draw (or erase) us as a graph inside another glist. */
    if (vis)
    {
        int i;
        t_float f;
        t_gobj *g;
        t_symbol *arrayname;
        t_garray *ga;
        /* char *ylabelanchor =
            (x->gl_ylabelx > 0.5*(x->gl_valueStart + x->gl_valueEnd) ? "w" : "e");
        char *xlabelanchor =
            (x->gl_xlabely > 0.5*(x->gl_valueUp + x->gl_valueDown) ? "s" : "n"); */
            
            /* draw a rectangle around the graph */
        sys_vGui(".x%lx.c create line\
            %d %d %d %d %d %d %d %d %d %d -tags [list %s graph]\n",
            canvas_getView(x->gl_parent),
            x1, y1, x1, y2, x2, y2, x2, y1, x1, y1, tag);
        
            /* if there's just one "garray" in the graph, write its name
                along the top */
        for (i = (y1 < y2 ? y1 : y2)-1, g = x->gl_graphics; g; g = g->g_next)
            if (g->g_pd == garray_class &&
                !garray_getname((t_garray *)g, &arrayname))
        {
            i -= font_getHostFontHeight(canvas_getFontSize(x));
            sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor nw\
             -font [::getFont %d] -tags [list %s label graph]\n",
             (long)canvas_getView(x), x1, i, arrayname->s_name,
                font_getHostFontSize(canvas_getFontSize(x)), tag);
        }
        
            /* draw ticks on horizontal borders.  If lperb field is
            zero, this is disabled. */
        if (x->gl_tickX.k_lperb)
        {
            t_float upix, lpix;
            if (y2 < y1)
                upix = y1, lpix = y2;
            else upix = y2, lpix = y1;
            for (i = 0, f = x->gl_tickX.k_point;
                f < 0.99 * x->gl_valueEnd + 0.01*x->gl_valueStart; i++,
                    f += x->gl_tickX.k_inc)
            {
                int tickpix = (i % x->gl_tickX.k_lperb ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)glist_xtopixels(x, f), (int)upix,
                    (int)glist_xtopixels(x, f), (int)upix - tickpix, tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)glist_xtopixels(x, f), (int)lpix,
                    (int)glist_xtopixels(x, f), (int)lpix + tickpix, tag);
            }
            for (i = 1, f = x->gl_tickX.k_point - x->gl_tickX.k_inc;
                f > 0.99 * x->gl_valueStart + 0.01*x->gl_valueEnd;
                    i++, f -= x->gl_tickX.k_inc)
            {
                int tickpix = (i % x->gl_tickX.k_lperb ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)glist_xtopixels(x, f), (int)upix,
                    (int)glist_xtopixels(x, f), (int)upix - tickpix, tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    (int)glist_xtopixels(x, f), (int)lpix,
                    (int)glist_xtopixels(x, f), (int)lpix + tickpix, tag);
            }
        }

            /* draw ticks in vertical borders*/
        if (x->gl_tickY.k_lperb)
        {
            t_float ubound, lbound;
            if (x->gl_valueDown < x->gl_valueUp)
                ubound = x->gl_valueUp, lbound = x->gl_valueDown;
            else ubound = x->gl_valueDown, lbound = x->gl_valueUp;
            for (i = 0, f = x->gl_tickY.k_point;
                f < 0.99 * ubound + 0.01 * lbound;
                    i++, f += x->gl_tickY.k_inc)
            {
                int tickpix = (i % x->gl_tickY.k_lperb ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x1, (int)glist_ytopixels(x, f), 
                    x1 + tickpix, (int)glist_ytopixels(x, f), tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x2, (int)glist_ytopixels(x, f), 
                    x2 - tickpix, (int)glist_ytopixels(x, f), tag);
            }
            for (i = 1, f = x->gl_tickY.k_point - x->gl_tickY.k_inc;
                f > 0.99 * lbound + 0.01 * ubound;
                    i++, f -= x->gl_tickY.k_inc)
            {
                int tickpix = (i % x->gl_tickY.k_lperb ? 2 : 4);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x1, (int)glist_ytopixels(x, f), 
                    x1 + tickpix, (int)glist_ytopixels(x, f), tag);
                sys_vGui(".x%lx.c create line %d %d %d %d -tags [list %s graph]\n",
                    canvas_getView(x->gl_parent),
                    x2, (int)glist_ytopixels(x, f), 
                    x2 - tickpix, (int)glist_ytopixels(x, f), tag);
            }
        }
        /*
        for (i = 0; i < x->gl_nxlabels; i++)
            sys_vGui(".x%lx.c create text\
 %d %d -text {%s} -font [::getFont %d] -anchor %s -tags [list %s label graph]\n",
                canvas_getView(x),
                (int)glist_xtopixels(x, atof(x->gl_xlabel[i]->s_name)),
                (int)glist_ytopixels(x, x->gl_xlabely),
                x->gl_xlabel[i]->s_name,
                     canvas_getFontSize(x), xlabelanchor, tag);

    
        for (i = 0; i < x->gl_nylabels; i++)
            sys_vGui(".x%lx.c create text\
 %d %d -text {%s} -font [::getFont %d] -anchor %s -tags [list %s label graph]\n",
                canvas_getView(x),
                (int)glist_xtopixels(x, x->gl_ylabelx),
                (int)glist_ytopixels(x, atof(x->gl_ylabel[i]->s_name)),
                x->gl_ylabel[i]->s_name,
                canvas_getFontSize(x), ylabelanchor, tag);
        */
            /* draw contents of graph as glist */
        for (g = x->gl_graphics; g; g = g->g_next)
            gobj_visibilityChanged(g, x, 1);
    }
    else
    {
        sys_vGui(".x%lx.c delete %s\n",
            canvas_getView(x->gl_parent), tag);
        for (g = x->gl_graphics; g; g = g->g_next)
            gobj_visibilityChanged(g, x, 0);
    }
}

    /* get the graph's rectangle, not counting extra swelling for controls
    to keep them inside the graph.  This is the "logical" pixel size. */

static void graph_graphrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_glist *x = (t_glist *)z;
    int x1 = text_xpix(&x->gl_obj, glist);
    int y1 = text_ypix(&x->gl_obj, glist);
    int x2, y2;
    x2 = x1 + x->gl_width;
    y2 = y1 + x->gl_height;

    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

    /* get the rectangle, enlarged to contain all the "contents" --
    meaning their formal bounds rectangles. */
static void graph_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int x1 = PD_INT_MAX, y1 = PD_INT_MAX, x2 = -PD_INT_MAX, y2 = -PD_INT_MAX;
    t_glist *x = (t_glist *)z;
    if (x->gl_isGraphOnParent)
    {
        int hadwindow;
        t_gobj *g;
        t_object *ob;
        int x21, y21, x22, y22;

        graph_graphrect(z, glist, &x1, &y1, &x2, &y2);
        if (canvas_hasGraphOnParentTitle(x))
        {
            text_widgetBehavior.w_fnGetRectangle(z, glist, &x21, &y21, &x22, &y22);
            if (x22 > x2) 
                x2 = x22;
            if (y22 > y2) 
                y2 = y22;
        }
        if (!x->gl_hasRectangle)
        {
            /* expand the rectangle to fit in text objects; this applies only
            to the old (0.37) graph-on-parent behavior. */
            /* lie about whether we have our own window to affect gobj_getRectangle
            calls below.  */
            hadwindow = x->gl_haveWindow;
            x->gl_haveWindow = 0;
            for (g = x->gl_graphics; g; g = g->g_next)
            {
                    /* don't do this for arrays, just let them hang outside the
                    box.  And ignore "text" objects which aren't shown on 
                    parent */
                if (pd_class(&g->g_pd) == garray_class ||
                    canvas_castToObjectIfPatchable(&g->g_pd))
                        continue;
                gobj_getRectangle(g, x, &x21, &y21, &x22, &y22);
                if (x22 > x2) 
                    x2 = x22;
                if (y22 > y2) 
                    y2 = y22;
            }
            x->gl_haveWindow = hadwindow;
        }
    }
    else text_widgetBehavior.w_fnGetRectangle(z, glist, &x1, &y1, &x2, &y2);
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void graph_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_glist *x = (t_glist *)z;
    if (!x->gl_isGraphOnParent)
        text_widgetBehavior.w_fnDisplace(z, glist, dx, dy);
    else
    {
        x->gl_obj.te_xCoordinate += dx;
        x->gl_obj.te_yCoordinate += dy;
        glist_redraw(x);
        canvas_updateLinesByObject(glist, &x->gl_obj);
    }
}

static void graph_select(t_gobj *z, t_glist *glist, int state)
{
    t_glist *x = (t_glist *)z;
    if (!x->gl_isGraphOnParent)
        text_widgetBehavior.w_fnSelect(z, glist, state);
    else
    {
        t_boxtext *y = glist_findrtext(glist, &x->gl_obj);
        if (canvas_hasGraphOnParentTitle (x))
            rtext_select(y, state);
        sys_vGui(".x%lx.c itemconfigure %sR -fill %s\n", glist, 
        boxtext_getTag(y), (state? "blue" : "black"));
        sys_vGui(".x%lx.c itemconfigure graph%lx -fill %s\n",
            canvas_getView(glist), z, (state? "blue" : "black"));
    }
}

static void graph_activate(t_gobj *z, t_glist *glist, int state)
{
    t_glist *x = (t_glist *)z;
    if (canvas_hasGraphOnParentTitle(x))
        text_widgetBehavior.w_fnActivate(z, glist, state);
}

static void graph_delete(t_gobj *z, t_glist *glist)
{
    t_glist *x = (t_glist *)z;
    t_gobj *y;
    while (y = x->gl_graphics)
        glist_delete(x, y);
    if (canvas_isMapped(x))
        text_widgetBehavior.w_fnDelete(z, glist);
            /* if we have connections to the actual 'canvas' object, zap
            them as well (e.g., array or scalar objects that are implemented
            as canvases with "real" inlets).  Connections to ordinary canvas
            in/outlets already got zapped when we cleared the contents above */
    canvas_deleteLinesByObject(glist, &x->gl_obj);
}

static t_float graph_lastxpix, graph_lastypix;

static void graph_motion(void *z, t_float dx, t_float dy)
{
    t_glist *x = (t_glist *)z;
    t_float newxpix = graph_lastxpix + dx, newypix = graph_lastypix + dy;
    t_garray *a = (t_garray *)(x->gl_graphics);
    int oldx = 0.5 + glist_pixelstox(x, graph_lastxpix);
    int newx = 0.5 + glist_pixelstox(x, newxpix);
    t_word *vec;
    int nelem, i;
    t_float oldy = glist_pixelstoy(x, graph_lastypix);
    t_float newy = glist_pixelstoy(x, newypix);
    graph_lastxpix = newxpix;
    graph_lastypix = newypix;
        /* verify that the array is OK */
    if (!a || pd_class((t_pd *)a) != garray_class)
        return;
    if (!garray_getfloatwords(a, &nelem, &vec))
        return;
    if (oldx < 0) oldx = 0;
    if (oldx >= nelem)
        oldx = nelem - 1;
    if (newx < 0) newx = 0;
    if (newx >= nelem)
        newx = nelem - 1;
    if (oldx < newx - 1)
    {
        for (i = oldx + 1; i <= newx; i++)
            vec[i].w_float = newy + (oldy - newy) *
                ((t_float)(newx - i))/(t_float)(newx - oldx);
    }
    else if (oldx > newx + 1)
    {
        for (i = oldx - 1; i >= newx; i--)
            vec[i].w_float = newy + (oldy - newy) *
                ((t_float)(newx - i))/(t_float)(newx - oldx);
    }
    else vec[newx].w_float = newy;
    garray_redraw(a);
}

static int graph_click(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int ctrl, int alt, int dbl, int doit)
{
    t_glist *x = (t_glist *)z;
    t_gobj *y;
    int clickreturned = 0;
    if (!x->gl_isGraphOnParent)
        return (text_widgetBehavior.w_fnClick(z, glist,
            xpix, ypix, shift, ctrl, alt, dbl, doit));
    else if (x->gl_haveWindow)
        return (0);
    else
    {
        for (y = x->gl_graphics; y; y = y->g_next)
        {
            int x1, y1, x2, y2;
                /* check if the object wants to be clicked */
            if (gobj_hit(y, x, xpix, ypix, &x1, &y1, &x2, &y2)
                &&  (clickreturned = gobj_click(y, x, xpix, ypix,
                    shift, ctrl, alt, 0, doit)))
                        break;
        }
        if (!doit)
        {
            if (y)
                canvas_setCursorType(canvas_getView(x), clickreturned);
            else canvas_setCursorType(canvas_getView(x), CURSOR_NOTHING);
        }
        return (clickreturned); 
    }
}

t_widgetbehavior graph_widgetbehavior =
{
    graph_getrect,
    graph_displace,
    graph_select,
    graph_activate,
    graph_delete,
    graph_vis,
    graph_click,
};

    /* find the graph most recently added to this glist;
        if none exists, return 0. */

t_glist *glist_findgraph(t_glist *x)
{
    t_gobj *y = 0, *z;
    for (z = x->gl_graphics; z; z = z->g_next)
        if (pd_class(&z->g_pd) == canvas_class && ((t_glist *)z)->gl_isGraphOnParent)
            y = z;
    return ((t_glist *)y);
}

extern void canvas_menuarray(t_glist *canvas);

void g_graph_setup_class(t_class *c)
{
    class_setWidgetBehavior(c, &graph_widgetbehavior);
    class_addMethod(c, (t_method)graph_bounds, sym_bounds,
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(c, (t_method)graph_xticks, sym_xticks,
        A_FLOAT, A_FLOAT, A_FLOAT, 0);
    /* class_addMethod(c, (t_method)graph_xlabel, gen_sym ("xlabel"),
        A_GIMME, 0); */
    class_addMethod(c, (t_method)graph_yticks, sym_yticks,
        A_FLOAT, A_FLOAT, A_FLOAT, 0);
    /* class_addMethod(c, (t_method)graph_ylabel, gen_sym ("ylabel"),
        A_GIMME, 0); */
    class_addMethod(c, (t_method)graph_array, sym_array,
        A_SYMBOL, A_FLOAT, A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addMethod(c, (t_method)canvas_menuarray,
        sym_menuarray, A_NULL); /* LEGACY !!! */
    /* class_addMethod(c, (t_method)glist_sort,
        sym_sort, A_NULL); */
}

void g_graph_setup( void)
{
    g_graph_setup_class(canvas_class);
}

