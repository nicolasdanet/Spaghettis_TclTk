
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_behaviorVisibilityChanged   (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_updateTitle (t_glist *glist)
{
    if (glist_hasWindow (glist)) {

        sys_vGui ("::ui_patch::setTitle %s {%s} {%s} %d\n",  // --
                        glist_getTagAsString (glist),
                        environment_getDirectoryAsString (glist_getEnvironment (glist)),
                        glist_getName (glist)->s_name,
                        glist_getDirty (glist));
    }
}

void glist_updateCursor (t_glist *glist, int type)
{
    static t_glist *lastGlist = NULL;           /* Static. */
    static int lastType = CURSOR_NOTHING;       /* Static. */
    static char *cursors[] =                    /* Static. */
        {
            "left_ptr",             // CURSOR_NOTHING
            "hand2",                // CURSOR_CLICK
            "sb_v_double_arrow",    // CURSOR_THICKEN
            "plus",                 // CURSOR_ADD
            "circle",               // CURSOR_CONNECT
            "sb_h_double_arrow"     // CURSOR_RESIZE
        };
    
    type = PD_CLAMP (type, CURSOR_NOTHING, CURSOR_RESIZE);
    
    PD_ASSERT (glist_hasWindow (glist));
    
    if (lastGlist != glist || lastType != type) {
    //
    sys_vGui ("%s configure -cursor %s\n", glist_getTagAsString (glist), cursors[type]);
    //
    }
    
    lastType = type; lastGlist = glist;
}

void glist_updateWindow (t_glist *glist)
{
    if (glist_isWindowable (glist) && glist_isOnScreen (glist)) { 
        canvas_map (glist, 0);
        canvas_map (glist, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_updateLines (t_glist *glist, t_object *o)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if (traverser_getSource (&t) == o || traverser_getDestination (&t) == o) {

        sys_vGui ("%s.c coords %lxLINE %d %d %d %d\n",
                        glist_getTagAsString (glist),
                        connection,
                        traverser_getStartX (&t),
                        traverser_getStartY (&t),
                        traverser_getEndX (&t),
                        traverser_getEndY (&t));

    }
    //
    }
    //
    }
    //
    }
}

void glist_updateGraph (t_glist *glist)
{  
    if (glist_isOnScreen (glist)) {
    //
    if (glist_hasWindow (glist)) { glist_updateWindow (glist); }
    else {

        PD_ASSERT (glist_hasParentOnScreen (glist));
        
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 0); 
        canvas_behaviorVisibilityChanged (cast_gobj (glist), glist_getParent (glist), 1);
    }
    //
    }
}

void glist_updateRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
        sys_vGui (".x%lx.c delete RECTANGLE\n", glist_getView (glist));
        glist_drawRectangle (glist);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_drawAllLines (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_outconnect *connection = NULL;
    t_traverser t;

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    sys_vGui ("%s.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                    glist_getTagAsString (glist),
                    traverser_getStartX (&t),
                    traverser_getStartY (&t),
                    traverser_getEndX (&t),
                    traverser_getEndY (&t), 
                    (outlet_isSignal (traverser_getOutlet (&t)) ? 2 : 1),
                    connection);
    //
    }
    //
    }
    //
    }
}

void glist_drawAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        t_object *o = NULL;
        if ((o = cast_objectIfConnectable (y)) && object_isComment (o)) {
            box_draw (box_fetch (glist, o));
        }
    }
    //
    }
    //
    }
}

void glist_drawRectangle (t_glist *glist)
{
    if (glist_isGraphOnParent (glist) && glist_hasWindow (glist)) {
    //
    if (!glist_isArray (glist)) {
    //
    int a = rectangle_getTopLeftX (glist_getGraphGeometry (glist));
    int b = rectangle_getTopLeftY (glist_getGraphGeometry (glist));
    int c = rectangle_getBottomRightX (glist_getGraphGeometry (glist));
    int d = rectangle_getBottomRightY (glist_getGraphGeometry (glist));
    
    sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                    " -dash {2 4}"  // --
                    " -fill #%06x"
                    " -tags RECTANGLE\n",
                    glist_getView (glist),
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
                    COLOR_GOP);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_eraseLine (t_glist *glist, t_outconnect *connection)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete %lxLINE\n", glist_getTagAsString (glist), connection);
    //
    }
    //
    }
}

void glist_eraseAllCommentBars (t_glist *glist)
{
    if (glist_hasWindow (glist))  {             /* Not shown in GOP. */
    //
    if (glist_isOnScreen (glist)) {
    //
    sys_vGui ("%s.c delete COMMENTBAR\n", glist_getTagAsString (glist));
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
