
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

extern t_class *canvas_class;
extern t_class *scalar_class;
extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior     text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_getRectangle (t_gobj *x, t_glist *owner, int *a, int *b, int *c, int *d)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnGetRectangle) {
        (*(pd_class (x)->c_behavior->w_fnGetRectangle)) (x, owner, a, b, c, d);
    }
}

void gobj_displace (t_gobj *x, t_glist *owner, int deltaX, int deltaY)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDisplace) {
        (*(pd_class (x)->c_behavior->w_fnDisplace)) (x, owner, deltaX, deltaY);
    }
}

void gobj_select (t_gobj *x, t_glist *owner, int state)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnSelect) {
        (*(pd_class (x)->c_behavior->w_fnSelect)) (x, owner, state);
    }
}

void gobj_activate (t_gobj *x, t_glist *owner, int state)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnActivate) {
        (*(pd_class (x)->c_behavior->w_fnActivate)) (x, owner, state);
    }
}

void gobj_delete (t_gobj *x, t_glist *owner)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDelete) {
        (*(pd_class (x)->c_behavior->w_fnDelete)) (x, owner);
    }
}

int gobj_click (t_gobj *x, t_glist *owner, int a, int b, int shift, int alt, int dbl, int k)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnClick) {
        return ((*(pd_class (x)->c_behavior->w_fnClick)) (x, owner, a, b, shift, alt, dbl, k));
    } else {
        return 0;
    }
}

void gobj_save (t_gobj *x, t_buffer *buffer)
{
    if (pd_class (x)->c_fnSave) {
        (*(pd_class (x)->c_fnSave)) (x, buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gobj_hit (t_gobj *x,
    t_glist *owner, 
    int positionX,
    int positionY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    if (gobj_isVisible (x, owner)) {
    //
    int x1, y1, x2, y2;
        
    gobj_getRectangle (x, owner, &x1, &y1, &x2, &y2);
    
    if (positionX >= x1 && positionX <= x2 && positionY >= y1 && positionY <= y2) {
    //
    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2;
    
    return 1;
    //
    }
    //
    }
    
    return 0;
}

int gobj_isVisible (t_gobj *x, t_glist *owner)
{
    if (canvas_isDrawnOnParent (owner)) {
    //
    t_object *object = NULL;
            
    /* Is parent visible? */
    
    if (!gobj_isVisible (cast_gobj (owner), owner->gl_parent)) { return 0; }
    
    /* Falling outside the graph rectangle? */
    
    if (owner->gl_hasRectangle) {
            
        if (pd_class (x) == scalar_class || pd_class (x) == garray_class) { return 1; }
        else {
        //
        int a, b, c, d, e, f, g, h;
            
        gobj_getRectangle (cast_gobj (owner), owner->gl_parent, &a, &b, &c, &d);
        
        if (a > c) { int t = a; a = c; c = t; }
        if (b > d) { int t = b; b = d; d = t; }
        
        gobj_getRectangle (x, owner, &e, &f, &g, &h);

        if (e < a || e > c || g < a || g > c || f < b || f > d || h < b || h > d) { return 0; }
        //
        }
    }
    
    if (object = canvas_castToObjectIfPatchable (x)) {
    //
    if (canvas_objectIsBox (object)) {
        if (!owner->gl_hasRectangle || object->te_type != TYPE_TEXT) {      /* Compatiblity with legacy. */
            return 0; 
        }
    }
    //
    }
    //
    }

    return 1;
}

void gobj_visibilityChanged (t_gobj *x, t_glist *owner, int isVisible)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnVisible) {
        if (gobj_isVisible (x, owner)) {
            (*(pd_class (x)->c_behavior->w_fnVisible)) (x, owner, isVisible);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
