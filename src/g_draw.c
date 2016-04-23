
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DRAW_GRAPH_ON_PARENT_COLOR  "#ff8080"       /* Red. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class          *canvas_class;
extern t_class          *scalar_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_drawGraphOnParentRectangle (t_glist *glist)
{
    int a = glist->gl_marginX;
    int b = glist->gl_marginY;
    int c = glist->gl_marginX + glist->gl_width;
    int d = glist->gl_marginY + glist->gl_height;
    
    sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d" 
                " -fill " DRAW_GRAPH_ON_PARENT_COLOR
                " -tags GOP\n",
                glist_getcanvas (glist),
                a,
                b,
                c,
                b,
                c,
                d,
                a,
                d,
                a,
                b);
}

void canvas_deleteGraphOnParentRectangle (t_glist *glist)
{
    sys_vGui (".x%lx.c delete GOP\n",  glist_getcanvas (glist));
}

void canvas_redraw(t_glist *x)
{
    if (canvas_isVisible(x))
    {
        canvas_map(x, 0);
        canvas_map(x, 1);
    }
}

void canvas_drawlines(t_glist *x)
{
    t_linetraverser t;
    t_outconnect *oc;
    {
        canvas_traverseLinesStart(&t, x);
        while (oc = canvas_traverseLinesNext(&t))
            sys_vGui(".x%lx.c create line %d %d %d %d -width %d -tags [list l%lx cord]\n",
                    glist_getcanvas(x),
                        t.tr_lineStartX, t.tr_lineStartY, t.tr_lineEndX, t.tr_lineEndY, 
                            (outlet_isSignal(t.tr_srcOutlet) ? 2:1),
                                oc);
    }
}


/******************* redrawing  data *********************/

    /* redraw all "scalars" (do this if a drawing command is changed.) 
    LATER we'll use the "template" information to select which ones we
    redraw.   Action = 0 for redraw, 1 for draw only, 2 for erase. */
static void glist_redrawall(t_glist *gl, int action)
{
    t_gobj *g;
    int vis = canvas_isVisible(gl);
    for (g = gl->gl_graphics; g; g = g->g_next)
    {
        t_class *cl;
        if (vis && g->g_pd == scalar_class)
        {
            if (action == 1)
            {
                if (canvas_isVisible(gl))
                    gobj_vis(g, gl, 1);
            }
            else if (action == 2)
            {
                if (canvas_isVisible(gl))
                    gobj_vis(g, gl, 0);
            }
            else scalar_redraw((t_scalar *)g, gl);
        }
        else if (g->g_pd == canvas_class)
            glist_redrawall((t_glist *)g, action);
    }
}

    /* public interface for above. */
void canvas_redrawallfortemplate(t_template *template, int action)
{
    t_glist *x;
        /* find all root canvases */
    for (x = pd_this->pd_roots; x; x = x->gl_next)
        glist_redrawall(x, action);
}

    /* find the template defined by a canvas, and redraw all elements
    for that */
void canvas_redrawallfortemplatecanvas(t_glist *x, int action)
{
    t_gobj *g;
    t_template *tmpl;
    t_symbol *s1 = gensym ("struct");
    for (g = x->gl_graphics; g; g = g->g_next)
    {
        t_object *ob = canvas_castToObjectIfBox(&g->g_pd);
        t_atom *argv;
        if (!ob || ob->te_type != TYPE_OBJECT ||
            buffer_size(ob->te_buffer) < 2)
            continue;
        argv = buffer_atoms(ob->te_buffer);
        if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_SYMBOL
            || argv[0].a_w.w_symbol != s1)
                continue;
        tmpl = template_findbyname(argv[1].a_w.w_symbol);
        canvas_redrawallfortemplate(tmpl, action);
    }
    canvas_redrawallfortemplate(0, action);
}

void canvas_fixlines (t_glist *x, t_object *text)
{
    t_linetraverser t;
    t_outconnect *oc;

    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if (t.tr_srcObject == text || t.tr_destObject == text)
        {
            sys_vGui(".x%lx.c coords l%lx %d %d %d %d\n",
                glist_getcanvas(x), oc,
                    t.tr_lineStartX, t.tr_lineStartY, t.tr_lineEndX, t.tr_lineEndY);
        }
    }
}

    /* kill all lines for the object */
void canvas_deletelines(t_glist *x, t_object *text)
{
    t_linetraverser t;
    t_outconnect *oc;
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if (t.tr_srcObject == text || t.tr_destObject == text)
        {
            if (canvas_isVisible(x))
            {
                sys_vGui(".x%lx.c delete l%lx\n",
                    glist_getcanvas(x), oc);
            }
            object_disconnect(t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
        }
    }
}

    /* kill all lines for one inlet or outlet */
void canvas_deletelinesforio(t_glist *x, t_object *text,
    t_inlet *inp, t_outlet *outp)
{
    t_linetraverser t;
    t_outconnect *oc;
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if ((t.tr_srcObject == text && t.tr_srcOutlet == outp) ||
            (t.tr_destObject == text && t.tr_destInlet == inp))
        {
            if (canvas_isVisible(x))
            {
                sys_vGui(".x%lx.c delete l%lx\n",
                    glist_getcanvas(x), oc);
            }
            object_disconnect(t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
