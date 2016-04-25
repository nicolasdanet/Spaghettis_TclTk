
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

#define DRAW_GRAPH_ON_PARENT_COLOR      "#ff8080"   /* Red. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class          *canvas_class;
extern t_class          *scalar_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_redraw (t_glist *glist)
{
    if (canvas_isVisible (glist)) {
        canvas_map (glist, 0);
        canvas_map (glist, 1);
    }
}

void canvas_drawLines (t_glist *glist)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                glist_getcanvas (glist),
                t.tr_lineStartX,
                t.tr_lineStartY,
                t.tr_lineEndX,
                t.tr_lineEndY, 
                (outlet_isSignal (t.tr_srcOutlet) ? 2 : 1),
                connection);
    //
    }
}

void canvas_updateLinesByObject (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    if (t.tr_srcObject == o || t.tr_destObject == o) {
    //
    if (canvas_isVisible (glist)) {
    //
    sys_vGui (".x%lx.c coords %lxLINE %d %d %d %d\n",
                glist_getcanvas (glist),
                connection,
                t.tr_lineStartX,
                t.tr_lineStartY,
                t.tr_lineEndX,
                t.tr_lineEndY);
    //
    }
    //
    }
    //
    }
}

void canvas_deleteLinesByObject (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    if (t.tr_srcObject == o || t.tr_destObject == o) {
    //
    if (canvas_isVisible (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                glist_getcanvas (glist),
                connection);
    //
    }

    object_disconnect (t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
    //
    }
    //
    }
}

void canvas_deleteLinesByInlets (t_glist *glist, t_object *o, t_inlet *inlet, t_outlet *outlet)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    int m = (t.tr_srcObject == o && t.tr_srcOutlet == outlet);
    int n = (t.tr_destObject == o && t.tr_destInlet == inlet);
    
    if (m || n) {
    //
    if (canvas_isVisible (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                glist_getcanvas (glist),
                connection);
    //
    }
                
    object_disconnect (t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
    //
    }
    //
    }
}

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_redrawAllScalars (t_glist *glist, int action)
{
    t_gobj *g = NULL;
    
    int visible = canvas_isVisible (glist);
    
    for (g = glist->gl_graphics; g; g = g->g_next) {
    //
    if (visible && pd_class (g) == scalar_class) {
    //
    switch (action) {
        case SCALAR_REDRAW  : scalar_redraw (cast_scalar (g), glist);   break;
        case SCALAR_DRAW    : gobj_vis (g, glist, 1);                   break;
        case SCALAR_ERASE   : gobj_vis (g, glist, 0);                   break;
    }
    //
    } 

    if (pd_class (g) == canvas_class) { canvas_redrawAllScalars (cast_glist (g), action); }
    //
    }
}

/* Note that the functions below are experimentals. */
/* The template argument are not used and everything is redrawn instead. */

void canvas_redrawAllByTemplate (t_template *dummy, int action)
{
    t_glist *glist = NULL;

    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) {
    //
    canvas_redrawAllScalars (glist, action);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_redrawAllByTemplateByCanvas (t_glist *glist, int action)
{
    canvas_redrawAllByTemplate (NULL, action);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*

void canvas_redrawAllByTemplateByCanvas (t_glist *glist, int action)
{
    t_gobj   *g = NULL;
    t_symbol *s = gensym ("struct");
    
    for (g = glist->gl_graphics; g; g = g->g_next) {
    //
    t_object *o = canvas_castToObjectIfBox (g);
    
    if (o && o->te_type == TYPE_OBJECT && buffer_size (o->te_buffer) < 2) {
    //
    t_atom *argv = buffer_atoms (o->te_buffer);
    
    if (IS_SYMBOL (argv) && IS_SYMBOL (argv + 1)) {
        if (GET_SYMBOL (argv) == s) {
            t_template *t = template_findbyname (GET_SYMBOL (argv + 1));
            canvas_redrawAllByTemplate (t, action);
        }
    }
    //
    }
    //
    }
    
    canvas_redrawAllByTemplate (NULL, action);
}

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
