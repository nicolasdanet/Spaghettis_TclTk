
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_behaviorGetRectangle             (t_gobj *, t_glist *, t_rectangle *);
void glist_behaviorDisplaced                (t_gobj *, t_glist *, int, int);
void glist_behaviorSelected                 (t_gobj *, t_glist *, int);
void glist_behaviorActivated                (t_gobj *, t_glist *, int);
void glist_behaviorDeleted                  (t_gobj *, t_glist *);
void glist_behaviorVisibilityChanged        (t_gobj *, t_glist *, int);
int  glist_behaviorMouse                    (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_widgetbehavior glist_widgetbehavior =     /* Shared. */
    {
        glist_behaviorGetRectangle,
        glist_behaviorDisplaced,
        glist_behaviorSelected,
        glist_behaviorActivated,
        glist_behaviorDeleted,
        glist_behaviorVisibilityChanged,
        glist_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_behaviorVisibilityChangedProceed (t_glist *, t_glist *, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void glist_behaviorGetRectangleProceed (t_glist *x, t_glist *owner, t_rectangle *r)
{
    glist_getRectangleOnParent (x, r);
}

static void glist_behaviorDisplacedProceed (t_glist *x, t_glist *owner, int deltaX, int deltaY)
{
    if (glist_isParentOnScreen (x)) { glist_behaviorVisibilityChangedProceed (x, owner, 0, 1); }
    
    object_setSnappedX (cast_object (x), object_getX (cast_object (x)) + deltaX);
    object_setSnappedY (cast_object (x), object_getY (cast_object (x)) + deltaY);
    
    if (glist_isParentOnScreen (x)) { glist_behaviorVisibilityChangedProceed (x, owner, 1, 1); }
    
    glist_updateLinesForObject (owner, cast_object (x));
}

static void glist_behaviorSelectedProceed (t_glist *x, t_glist *owner, int isSelected)
{
    glist_updateRectangleOnParent (x);
}

void glist_behaviorVisibilityChangedProceed (t_glist *x, t_glist *owner, int isVisible, int isMoving)
{
    if (isVisible) { glist_drawRectangleOnParent (x); }
    else {
        glist_eraseRectangleOnParent (x);
    }
        
    if (!glist_hasWindow (x)) {
    //
    t_gobj *y = NULL; 
        
    for (y = x->gl_graphics; y; y = y->g_next) { 
        if (isVisible && isMoving && (pd_class (y) == garray_class)) { continue; }
        gobj_visibilityChanged (y, x, isVisible);
    }
    //
    }
}

static int glist_behaviorMouseProceed (t_glist *x, t_glist *owner, t_mouse *m)
{
    if (glist_hasWindow (x)) { return 0; }
    else {
        int k = 0;
        int a = m->m_x;
        int b = m->m_y;
        
        int i, n = glist_objectGetNumberOf (x);
        
        for (i = n - 1; i >= 0; i--) {
        //
        t_gobj *y = glist_objectGetAt (x, i);
            
        t_rectangle t;
        
        if (gobj_hit (y, x, a, b, 0, &t)) {
            if ((k = gobj_mouse (y, x, m))) {
                break;
            }
        }
        //
        }

        return k; 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnGetRectangle (z, glist, r); }
    else {
        glist_behaviorGetRectangleProceed (x, glist, r);
    }
}

void glist_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnDisplaced (z, glist, deltaX, deltaY); }
    else {
        glist_behaviorDisplacedProceed (x, glist, deltaX, deltaY);
        glist_redrawRequired (glist);
    }
}

void glist_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_glist *x = cast_glist (z);
    
    glist_setSelected (x, (isSelected != 0));
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnSelected (z, glist, isSelected); }
    else {
        glist_behaviorSelectedProceed (x, glist, isSelected);
    }
}

void glist_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnActivated (z, glist, isActivated); }
}

void glist_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    t_glist *x = cast_glist (z);

    glist_objectRemoveAll (x);
    
    text_widgetBehavior.w_fnDeleted (z, glist);
}

void glist_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_glist *x = cast_glist (z);

    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnVisibilityChanged (z, glist, isVisible); }
    else {
        glist_behaviorVisibilityChangedProceed (x, glist, isVisible, 0);
    }
}

int glist_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_glist *x = cast_glist (z);

    if (!glist_isGraphOnParent (x)) { return text_widgetBehavior.w_fnMouse (z, glist, m); }
    else {
        return glist_behaviorMouseProceed (x, glist, m);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
