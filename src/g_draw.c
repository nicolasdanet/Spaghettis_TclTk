
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DRAW_GRAPH_ON_PARENT_COLOR      "#ff8080"       /* Red. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *text_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_redraw (t_glist *glist)
{
    if (canvas_isMapped (glist)) { canvas_map (glist, 0); canvas_map (glist, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_drawLines (t_glist *glist)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
    //
    sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    canvas_getView (glist),
                    linetraverser_getStartX (&t),
                    linetraverser_getStartY (&t),
                    linetraverser_getEndX (&t),
                    linetraverser_getEndY (&t), 
                    (outlet_isSignal (linetraverser_getOutlet (&t)) ? 2 : 1),
                    connection);
    //
    }
}

void canvas_updateLinesByObject (t_glist *glist, t_object *o)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
    //
    if (linetraverser_getSource (&t) == o || linetraverser_getDestination (&t) == o) {
    //
    if (canvas_isMapped (glist)) {
    //
    sys_vGui (".x%lx.c coords %lxLINE %d %d %d %d\n",
                    canvas_getView (glist),
                    connection,
                    linetraverser_getStartX (&t),
                    linetraverser_getStartY (&t),
                    linetraverser_getEndX (&t),
                    linetraverser_getEndY (&t));
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

    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
    //
    if (linetraverser_getSource (&t) == o || linetraverser_getDestination (&t) == o) {
    //
    if (canvas_isMapped (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                    canvas_getView (glist),
                    connection);
    //
    }

    linetraverser_disconnect (&t);
    //
    }
    //
    }
}

void canvas_deleteLinesByInlets (t_glist *glist, t_object *o, t_inlet *inlet, t_outlet *outlet)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;

    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
    //
    int m = (linetraverser_getSource (&t) == o && linetraverser_getOutlet (&t) == outlet);
    int n = (linetraverser_getDestination (&t) == o && linetraverser_getInlet (&t) == inlet);
    
    if (m || n) {
    //
    if (canvas_isMapped (glist)) {
    //
    sys_vGui (".x%lx.c delete %lxLINE\n",
                    canvas_getView (glist),
                    connection);
    //
    }
                
    linetraverser_disconnect (&t);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_drawBoxObject (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    char *pattern = (pd_class (o) == text_class) ? "{6 4}" : "{}";  // --
    
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
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
                        pattern,        /* Dashes for badly created boxes? */
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

static void canvas_drawBoxMessage (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
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

static void canvas_drawBoxAtom (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
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

static void canvas_drawBoxComment (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    if (glist->gl_isEditMode) {
    //
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui (".x%lx.c create line %d %d %d %d"
                        " -tags [list %sBORDER COMMENTBAR]\n",      // --
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

static void canvas_drawInletsAndOutlets (t_glist *glist, t_object *o, char *tag, int create, t_rectangle *r)
{
    int i;
    int m = object_getNumberOfInlets (o);
    int n = object_getNumberOfOutlets (o);
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    for (i = 0; i < m; i++) {
    //
    int offset = a + inlet_offset ((c - a), i, m);
    
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
    int offset = a + inlet_offset ((c - a), i, n);
    
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

void canvas_drawBox (t_glist *glist, t_object *o, char *tag, int create)
{
    t_rectangle r;
    
    text_behaviorGetRectangle (cast_gobj (o), glist, &r);

    if (object_isObject (o))            { canvas_drawBoxObject (glist, o, tag, create, &r);        }
    else if (object_isMessage (o))      { canvas_drawBoxMessage (glist, o, tag, create, &r);       }
    else if (object_isAtom (o))         { canvas_drawBoxAtom (glist, o, tag, create, &r);          }
    else if (object_isComment (o))      { canvas_drawBoxComment (glist, o, tag, create, &r);       }
    if (cast_objectIfConnectable (o))   { canvas_drawInletsAndOutlets (glist, o, tag, create, &r); }
}

static void canvas_eraseInletsAndOutlets (t_glist *glist, t_object *o, char *tag)
{
    int i;
    int m = object_getNumberOfInlets (o);
    int n = object_getNumberOfOutlets (o);
    
    for (i = 0; i < m; i++) { sys_vGui (".x%lx.c delete %sINLET%d\n",  canvas_getView (glist), tag, i); }
    for (i = 0; i < n; i++) { sys_vGui (".x%lx.c delete %sOUTLET%d\n", canvas_getView (glist), tag, i); }
}

void canvas_eraseBox (t_glist *glist, t_object *o, char *tag)
{
    if (!object_isComment (o) || glist->gl_isEditMode) {   /* Comments have borders only in edit mode. */
    //
    sys_vGui (".x%lx.c delete %sBORDER\n", canvas_getView (glist), tag); 
    canvas_eraseInletsAndOutlets (glist, o, tag);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_drawGraphOnParentRectangle (t_glist *glist)
{
    if (glist->gl_isGraphOnParent && glist->gl_hasWindow) {
    //
    if (!canvas_isGraph (glist)) {
    //
    int a = rectangle_getTopLeftX (&glist->gl_geometryGraph);
    int b = rectangle_getTopLeftY (&glist->gl_geometryGraph);
    int c = rectangle_getBottomRightX (&glist->gl_geometryGraph);
    int d = rectangle_getBottomRightY (&glist->gl_geometryGraph);
    
    sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"  // --
                    " -fill " DRAW_GRAPH_ON_PARENT_COLOR
                    " -tags RECTANGLE\n",
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
    //
    }
}

void canvas_updateGraphOnParentRectangle (t_glist *glist)
{
    if (glist->gl_isGraphOnParent && glist->gl_hasWindow) {
    //
    if (!canvas_isGraph (glist)) {
        sys_vGui (".x%lx.c delete RECTANGLE\n", canvas_getView (glist));
        canvas_drawGraphOnParentRectangle (glist);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
