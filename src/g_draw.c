
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DRAW_GRAPH_ON_PARENT_COLOR      "#ff8080"   /* Red. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *text_class;

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
    char *pattern = (pd_class (o) == text_class) ? "{6 4}" : "{}";
    
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

static void canvas_drawInletsAndOutlets (t_glist *glist,
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

static void canvas_eraseInletsAndOutlets (t_glist *glist, t_object *o, char *tag)
{
    int i;
    int m = object_numberOfInlets (o);
    int n = object_numberOfOutlets (o);
    
    for (i = 0; i < m; i++) { sys_vGui (".x%lx.c delete %sINLET%d\n",  canvas_getView (glist), tag, i); }
    for (i = 0; i < n; i++) { sys_vGui (".x%lx.c delete %sOUTLET%d\n", canvas_getView (glist), tag, i); }
}

void canvas_eraseBox (t_glist *glist, t_object *o, char *tag)
{
    if (o->te_type != TYPE_COMMENT || glist->gl_isEditMode) {   /* Comments have borders only in edit mode. */
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
    int a = glist->gl_graphMarginLeft;
    int b = glist->gl_graphMarginTop;
    int c = glist->gl_graphMarginLeft + glist->gl_graphWidth;
    int d = glist->gl_graphMarginTop + glist->gl_graphHeight;
    
    sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"
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
