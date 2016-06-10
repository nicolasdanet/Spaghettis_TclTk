
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

static void canvas_drawBoxObject (t_glist *glist,
    t_object *o,
    char *tag,
    int create,
    int a,
    int b,
    int c,
    int d)
{
    char *pattern = (pd_class (o) == text_class) ? "{6 4}" : "{}";      /* Dashes for badly created boxes? */
    
    if (create) {
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -dash %s"
                        " -tags %sBORDER\n",
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
                        b,  
                        pattern,
                        tag);
    } else {
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d\n",
                        canvas_getView (glist),
                        tag,
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
        sys_vGui (".x%lx.c itemconfigure %sBORDER -dash %s\n",
                        canvas_getView (glist),
                        tag,
                        pattern);
    }
}

static void canvas_drawBoxMessage (t_glist *glist,
    t_object *o,
    char *tag,
    int create,
    int a,
    int b,
    int c,
    int d)
{
    if (create) {
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
                        " -tags %sBORDER\n",
                        canvas_getView (glist),
                        a,
                        b,
                        c + 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d - 4,
                        c + 4,
                        d,
                        a,
                        d,
                        a,
                        b,
                        tag);
                    
    } else {
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        canvas_getView (glist),
                        tag,
                        a,
                        b,
                        c + 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d - 4,
                        c + 4,
                        d,
                        a,
                        d,
                        a,
                        b);
    }
}

static void canvas_drawBoxAtom (t_glist *glist,
    t_object *o,
    char *tag,
    int create,
    int a,
    int b,
    int c,
    int d)
{
    if (create) {
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d %d %d"
                        " -tags %sBORDER\n",
                        canvas_getView (glist),
                        a,
                        b,
                        c - 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b,
                        tag);
    } else {
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        canvas_getView (glist),
                        tag,
                        a,
                        b,
                        c - 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b);
    }
}

static void canvas_drawBoxComment (t_glist *glist,
    t_object *o,
    char *tag,
    int create,
    int a,
    int b,
    int c,
    int d)
{
    if (glist->gl_isEditMode) {
    //
    if (create) {
        sys_vGui (".x%lx.c create line %d %d %d %d"
                        " -tags [list %sBORDER COMMENTBAR]\n",
                        canvas_getView (glist),
                        c,
                        b,
                        c,
                        d,
                        tag);
    } else {
        sys_vGui (".x%lx.c coords %sBORDER %d %d %d %d\n",
                        canvas_getView (glist),
                        tag,
                        c,
                        b,
                        c,
                        d);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_drawBox (t_glist *glist, t_object *o, char *tag, int create)
{
    int a, b, c, d;
    
    text_behaviorGetRectangle (cast_gobj (o), glist, &a, &b, &c, &d);

    if (o->te_type == TYPE_OBJECT)          { canvas_drawBoxObject (glist, o, tag, create, a, b, c, d);  }
    else if (o->te_type == TYPE_MESSAGE)    { canvas_drawBoxMessage (glist, o, tag, create, a, b, c, d); }
    else if (o->te_type == TYPE_ATOM)       { canvas_drawBoxAtom (glist, o, tag, create, a, b, c, d);    }
    else if (o->te_type == TYPE_COMMENT)    { canvas_drawBoxComment (glist, o, tag, create, a, b, c, d); }

    if (canvas_castToObjectIfPatchable (o)) {
    // 
    canvas_drawInletsAndOutlets (glist, o, tag, create, a, b, c, d); 
    //
    }
}

void canvas_drawInletsAndOutlets (t_glist *glist,
    t_object *o,
    char *tag,
    int create,
    int a,
    int b,
    int c,
    int d)
{
    int i;
    int m = object_numberOfInlets (o);
    int n = object_numberOfOutlets (o);
    
    for (i = 0; i < m; i++) {
    //
    int offset = a + INLET_OFFSET ((c - a), i, m);
    
    if (create) {
        sys_vGui (".x%lx.c create rectangle %d %d %d %d -tags %sINLET%d\n",
                        canvas_getView (glist),
                        offset,
                        b,
                        offset + INLET_WIDTH,
                        b + INLET_HEIGHT,
                        tag,
                        i);
    } else {
        sys_vGui (".x%lx.c coords %sINLET%d %d %d %d %d\n",
                        canvas_getView (glist),
                        tag,
                        i,
                        offset,
                        b,
                        offset + INLET_WIDTH,
                        b + INLET_HEIGHT);
    }
    //
    }
    
    for (i = 0; i < n; i++) {
    //
    int offset = a + INLET_OFFSET ((c - a), i, n);
    
    if (create) {
        sys_vGui (".x%lx.c create rectangle %d %d %d %d -tags %sOUTLET%d\n",
                        canvas_getView (glist),
                        offset,
                        d - INLET_HEIGHT,
                        offset + INLET_WIDTH,
                        d,
                        tag,
                        i);
    } else {
        sys_vGui (".x%lx.c coords %sOUTLET%d %d %d %d %d\n",
                        canvas_getView (glist),
                        tag,
                        i,
                        offset,
                        d - INLET_HEIGHT,
                        offset + INLET_WIDTH,
                        d);
    }
    //
    }
}

void canvas_eraseBox(t_glist *glist, t_object *x, char *tag)
{
    if (x->te_type == TYPE_COMMENT && !glist->gl_isEditMode) return;
    sys_vGui(".x%lx.c delete %sBORDER\n",
        canvas_getView(glist), tag);
    canvas_eraseInletsAndOutlets(glist, x, tag);
}

void canvas_eraseInletsAndOutlets(t_glist *glist, t_object *ob, char *tag)
{
    int i, n;
    n = object_numberOfOutlets(ob);
    for (i = 0; i < n; i++)
        sys_vGui(".x%lx.c delete %sOUTLET%d\n",
            canvas_getView(glist), tag, i);
    n = object_numberOfInlets(ob);
    for (i = 0; i < n; i++)
        sys_vGui(".x%lx.c delete %sINLET%d\n",
            canvas_getView(glist), tag, i);
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
