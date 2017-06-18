
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_getRectangle (t_gobj *x, t_glist *owner, t_rectangle *r)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnGetRectangle) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnGetRectangle)) (x, owner, r);
        }
    }
}

void gobj_displaced (t_gobj *x, t_glist *owner, int deltaX, int deltaY)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnDisplaced) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnDisplaced)) (x, owner, deltaX, deltaY);
        }
    }
}

void gobj_selected (t_gobj *x, t_glist *owner, int isSelected)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnSelected) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnSelected)) (x, owner, isSelected);
        }
    }
}

void gobj_activated (t_gobj *x, t_glist *owner, int isActivated)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnActivated) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnActivated)) (x, owner, isActivated);
        }
    }
}

void gobj_deleted (t_gobj *x, t_glist *owner)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnDeleted) {
            (*(class_getWidgetBehavior (pd_class (x))->w_fnDeleted)) (x, owner);
        }
    }
}

void gobj_visibilityChanged (t_gobj *x, t_glist *owner, int isVisible)
{
    if (class_hasWidgetBehavior (pd_class (x))) {
        if (class_getWidgetBehavior (pd_class (x))->w_fnVisibilityChanged) {
            if (gobj_isVisible (x, owner)) {
                (*(class_getWidgetBehavior (pd_class (x))->w_fnVisibilityChanged)) (x, owner, isVisible);
            }
        }
    }
}

int gobj_mouse (t_gobj *x, t_glist *owner, t_mouse *m)
{
    if (class_hasWidgetBehavior (pd_class (x))) { 
        if (class_getWidgetBehavior (pd_class (x))->w_fnMouse) {
            return (*(class_getWidgetBehavior (pd_class (x))->w_fnMouse)) (x, owner, m);
        }
    } 

    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gobj_save (t_gobj *x, t_buffer *buffer)
{
    if (class_hasSaveFunction (pd_class (x))) {
        (*(class_getSaveFunction (pd_class (x)))) (x, buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int gobj_hit (t_gobj *x, t_glist *owner, int a, int b, int n, t_rectangle *r)
{
    if (gobj_isVisible (x, owner)) {
    //
    t_rectangle t1, t2;
    
    gobj_getRectangle (x, owner, &t1);
    
    rectangle_setCopy (&t2, &t1); rectangle_enlargeHeight (&t2, n);
    
    if (!rectangle_isNothing (&t2) && rectangle_containsPoint (&t2, a, b)) {
        rectangle_setCopy (r, &t1);
        return 1;
    }
    //
    }
    
    return 0;
}

/* Always true if NOT a GOP. */

int gobj_isVisible (t_gobj *x, t_glist *owner)
{
    if (glist_hasParent (owner) && !glist_isWindowable (owner)) {
    //
    t_object *object = NULL;
            
    /* Is parent visible? */
    
    if (!gobj_isVisible (cast_gobj (owner), glist_getParent (owner))) { return 0; }
    
    if (pd_class (x) == garray_class) { return 1; }                             /* Always true. */
    if (pd_class (x) == scalar_class && glist_isArray (owner)) { return 1; }    /* Ditto. */
    else {
    //
    {
        /* In GOP the only regular box type shown is comment. */
        
        if ((object = cast_objectIfConnectable (x))) {
            if (object_isViewAsBox (object)) {
                if (!object_isComment (object)) {
                    return 0; 
                }
            }
        }
    }
    {
        /* Falling outside the graph rectangle? */
        
        t_rectangle r1, r2;
        gobj_getRectangle (cast_gobj (owner), glist_getParent (owner), &r1);
        gobj_getRectangle (x, owner, &r2);
        
        if (!rectangle_containsRectangle (&r1, &r2)) {
            return 0; 
        }
    }
    //
    }
    //
    }

    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* True if object is really drawn as a box in the patch. */

int object_isViewAsBox (t_object *x)
{
    return ((class_getWidgetBehavior (pd_class (x)) == &text_widgetBehavior)
        || ((pd_class (x) == canvas_class) && !glist_isGraphOnParent (cast_glist (x))));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
