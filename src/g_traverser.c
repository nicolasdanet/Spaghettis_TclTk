
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

extern t_class *inlet_class;
extern t_class *pointerinlet_class;
extern t_class *floatinlet_class;
extern t_class *symbolinlet_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Fetch the nth outlet of an object. */
/* Return its first connection. */

static t_outconnect *traverser_outletStart  (t_object *x, t_outlet **ptr, int n)
{
    t_outlet *o = object_getOutlets (x);
    
    while (n && o) { n--; o = outlet_getNext (o); }
    
    *ptr = o;
    
    if (o) {
        return (outlet_getConnections (o)); 
    }

    return NULL;
}

/* Given a connection, fetch the object connected, the related inlet and its index. */
/* Return the next connection of the outlet (NULL if last). */

static t_outconnect *traverser_outletNext (t_outconnect *previous, t_object **dest, t_inlet **ptr, int *n)
{
    t_pd *y = connection_getReceiver (previous);

    t_class *c = pd_class (y);
    
    if (c == inlet_class || c == pointerinlet_class || c == floatinlet_class || c == symbolinlet_class) {
        t_inlet *i1 = (t_inlet *)y;
        t_inlet *i2 = NULL;
        t_object *o = inlet_getOwner (i1);
        int k = class_hasFirstInlet (pd_class (o));
        for (i2 = object_getInlets (o); i2 && i2 != i1; i2 = inlet_getNext (i2)) { k++; }
        *n    = k;
        *ptr  = i1;
        *dest = o;
        
    } else {
        *n    = 0;
        *ptr  = NULL;
        *dest = (cast_object (y));
    }
    
    return connection_getNext (previous);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void traverser_start (t_traverser *t, t_glist *glist)
{
    t->tr_owner                 = glist;
    t->tr_connectionCached      = NULL;
    t->tr_srcObject             = NULL;
    t->tr_srcIndexOfNextOutlet  = 0;
    t->tr_srcNumberOfOutlets    = 0;
}

/* Get the lines outlet per outlet, object per object. */
/* Coordinates are set at the same time. */

t_outconnect *traverser_next (t_traverser *t)
{
    t_outconnect *connection = t->tr_connectionCached;
    
    while (!connection) {
    //
    int n = t->tr_srcIndexOfNextOutlet;
    
    while (n == t->tr_srcNumberOfOutlets) {
    //
    t_gobj   *y = NULL;
    t_object *o = NULL;
    
    if (!t->tr_srcObject) { y = t->tr_owner->gl_graphics; }
    else {
        y = cast_gobj (t->tr_srcObject)->g_next;
    }
    
    for (; y; y = y->g_next) {
        if ((o = cast_objectIfConnectable (y))) { break; }          /* Only box objects are considered. */
    }
    
    if (!o) { return NULL; }
    
    t->tr_srcObject          = o;
    t->tr_srcNumberOfOutlets = object_getNumberOfOutlets (o);
    n = 0;
    
    if (canvas_isMapped (t->tr_owner)) { gobj_getRectangle (y, t->tr_owner, &t->tr_srcBox); }
    else {
        rectangle_set (&t->tr_srcBox, 0, 0, 0, 0);
    }
    //
    }
    
    t->tr_srcIndexOfOutlet     = n;
    t->tr_srcIndexOfNextOutlet = n + 1;
    connection = traverser_outletStart  (t->tr_srcObject, &t->tr_srcOutlet, n);
    //
    }
    
    t->tr_connectionCached = traverser_outletNext (connection,
        &t->tr_destObject,
        &t->tr_destInlet,
        &t->tr_destIndexOfInlet);
                                                            
    t->tr_destNumberOfInlets = object_getNumberOfInlets (t->tr_destObject);
    
    PD_ASSERT (t->tr_destNumberOfInlets);
    
    if (canvas_isMapped (t->tr_owner)) {

        gobj_getRectangle (cast_gobj (t->tr_destObject), t->tr_owner, &t->tr_destBox);
        
        {
            int w = rectangle_getWidth (&t->tr_srcBox);
            int i = t->tr_srcIndexOfOutlet;
            int j = t->tr_srcNumberOfOutlets;
        
            t->tr_cord.tr_lineStartX = rectangle_getTopLeftX (&t->tr_srcBox) + inlet_middle (w, i, j);
            t->tr_cord.tr_lineStartY = rectangle_getBottomRightY (&t->tr_srcBox);
        }
        {
            int w = rectangle_getWidth (&t->tr_destBox);
            int i = t->tr_destIndexOfInlet;
            int j = t->tr_destNumberOfInlets;
        
            t->tr_cord.tr_lineEndX = rectangle_getTopLeftX (&t->tr_destBox) + inlet_middle (w, i, j);
            t->tr_cord.tr_lineEndY = rectangle_getTopLeftY (&t->tr_destBox);
        }
        
    } else {
        rectangle_set (&t->tr_destBox, 0, 0, 0, 0); cord_init (&t->tr_cord);
    }
    
    return connection;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void traverser_disconnect (t_traverser *t)
{
    object_disconnect (t->tr_srcObject, t->tr_srcIndexOfOutlet, t->tr_destObject, t->tr_destIndexOfInlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int traverser_isLineBetween (t_traverser *t, t_object *src, int m, t_object *dest, int n)
{
    if (t->tr_srcObject == src && t->tr_destObject == dest) {
        if (t->tr_srcIndexOfOutlet == m && t->tr_destIndexOfInlet == n) { 
            return 1;
        }
    }
    
    return 0;
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
