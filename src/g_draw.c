
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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
extern t_class          *text_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_redraw (t_glist *glist)
{
    if (canvas_isMapped (glist)) { canvas_map (glist, 0); canvas_map (glist, 1); }
}

void canvas_drawLines (t_glist *glist)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    canvas_getView (glist),
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
    if (canvas_isMapped (glist)) {
    //
    sys_vGui (".x%lx.c coords %lxLINE %d %d %d %d\n",
                    canvas_getView (glist),
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
    if (canvas_isMapped (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                    canvas_getView (glist),
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
    if (canvas_isMapped (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                    canvas_getView (glist),
                    connection);
    //
    }
                
    object_disconnect (t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* draw inlets and outlets for a text object or for a graph. */
void canvas_drawInletsAndOutlets(t_glist *glist, t_object *ob,
    char *tag, int firsttime, int x1, int y1, int x2, int y2)
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
                onset + INLET_WIDTH, y1 + INLET_HEIGHT,
                tag, i);
        else
            sys_vGui(".x%lx.c coords %si%d %d %d %d %d\n",
                canvas_getView(glist), tag, i,
                onset, y1,
                onset + INLET_WIDTH, y1 + INLET_HEIGHT);
    }
}

void canvas_drawBordersOfBox(t_glist *glist, t_object *x,
    char *tag, int firsttime)
{
    t_object *ob;
    int x1, y1, x2, y2, width, height;
    text_behaviorGetRectangle(&x->te_g, glist, &x1, &y1, &x2, &y2);
    width = x2 - x1;
    height = y2 - y1;
    
    if (x->te_type == TYPE_OBJECT)
    {
        char *pattern = ((pd_class((t_pd *)x) == text_class) ? "-" : "\"\"");
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d -dash %s -tags [list %sBORDER obj]\n",
                canvas_getView(glist),
                    x1, y1,  x2, y1,  x2, y2,  x1, y2,  x1, y1,  pattern, tag);
        else
        {
            sys_vGui(".x%lx.c coords %sBORDER\
 %d %d %d %d %d %d %d %d %d %d\n",
                canvas_getView(glist), tag,
                    x1, y1,  x2, y1,  x2, y2,  x1, y2,  x1, y1);
            sys_vGui(".x%lx.c itemconfigure %sBORDER -dash %s\n",
                canvas_getView(glist), tag, pattern);
        }
    }
    else if (x->te_type == TYPE_MESSAGE)
    {
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d %d %d %d %d -tags [list %sBORDER msg]\n",
                canvas_getView(glist),
                x1, y1,  x2+4, y1,  x2, y1+4,  x2, y2-4,  x2+4, y2,
                x1, y2,  x1, y1,
                    tag);
        else
            sys_vGui(".x%lx.c coords %sBORDER\
 %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                canvas_getView(glist), tag,
                x1, y1,  x2+4, y1,  x2, y1+4,  x2, y2-4,  x2+4, y2,
                x1, y2,  x1, y1);
    }
    else if (x->te_type == TYPE_ATOM)
    {
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d %d %d -tags [list %sBORDER atom]\n",
                canvas_getView(glist),
                x1, y1,  x2-4, y1,  x2, y1+4,  x2, y2,  x1, y2,  x1, y1,
                    tag);
        else
            sys_vGui(".x%lx.c coords %sBORDER\
 %d %d %d %d %d %d %d %d %d %d %d %d\n",
                canvas_getView(glist), tag,
                x1, y1,  x2-4, y1,  x2, y1+4,  x2, y2,  x1, y2,  x1, y1);
    }
        /* for comments, just draw a bar on RHS if unlocked; when a visible
        canvas is unlocked we have to call this anew on all comments, and when
        locked we erase them all via the annoying "COMMENTBAR" tag. */
    else if (x->te_type == TYPE_COMMENT && glist->gl_isEditMode)
    {
        if (firsttime)
            sys_vGui(".x%lx.c create line\
 %d %d %d %d -tags [list %sBORDER COMMENTBAR]\n",
                canvas_getView(glist),
                x2, y1,  x2, y2, tag);
        else
            sys_vGui(".x%lx.c coords %sBORDER %d %d %d %d\n",
                canvas_getView(glist), tag, x2, y1,  x2, y2);
    }
        /* draw inlets/outlets */
    
    if (ob = canvas_castToObjectIfPatchable((t_pd *)x))
        canvas_drawInletsAndOutlets(glist, ob, tag, firsttime, x1, y1, x2, y2);
}

void canvas_eraseInletsAndOutlets(t_glist *glist, t_object *ob, char *tag)
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

void canvas_eraseBordersOfBox(t_glist *glist, t_object *x, char *tag)
{
    if (x->te_type == TYPE_COMMENT && !glist->gl_isEditMode) return;
    sys_vGui(".x%lx.c delete %sBORDER\n",
        canvas_getView(glist), tag);
    canvas_eraseInletsAndOutlets(glist, x, tag);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_drawGraphOnParentRectangle (t_glist *glist)
{
    int a = glist->gl_marginX;
    int b = glist->gl_marginY;
    int c = glist->gl_marginX + glist->gl_width;
    int d = glist->gl_marginY + glist->gl_height;
    
    sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"
                    " -fill " DRAW_GRAPH_ON_PARENT_COLOR
                    " -tags GOP\n",
                    canvas_getView (glist),
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
    sys_vGui (".x%lx.c delete GOP\n", canvas_getView (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_redrawAllScalars (t_glist *glist, int action)
{
    t_gobj *g = NULL;
    
    int visible = canvas_isMapped (glist);
    
    for (g = glist->gl_graphics; g; g = g->g_next) {
    //
    if (visible && pd_class (g) == scalar_class) {
    //
    switch (action) {
        case SCALAR_REDRAW  : scalar_redraw (cast_scalar (g), glist);   break;
        case SCALAR_DRAW    : gobj_visibilityChanged (g, glist, 1);     break;
        case SCALAR_ERASE   : gobj_visibilityChanged (g, glist, 0);     break;
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
    t_symbol *s = sym_struct;
    
    for (g = glist->gl_graphics; g; g = g->g_next) {
    //
    t_object *o = canvas_castToObjectIfPatchable (g);
    
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
