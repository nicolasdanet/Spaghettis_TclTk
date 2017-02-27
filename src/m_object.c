
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *inlet_class;
extern t_class *pointerinlet_class;
extern t_class *floatinlet_class;
extern t_class *symbolinlet_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void object_distributeOnInlets (t_object *x, int argc, t_atom *argv)
{
    if (!argc) { pd_empty (cast_pd (x)); }
    else {
    //
    int count;
    t_atom *a = NULL;
    t_inlet *i = x->te_inlet;
    
    for (count = argc - 1, a = argv + 1; i && count--; a++, i = inlet_getNext (i)) {
    //
    if (IS_POINTER (a))        { pd_pointer (cast_pd (i), GET_POINTER (a)); }
    else if (IS_FLOAT (a))     { pd_float (cast_pd (i), GET_FLOAT (a)); }
    else {
        pd_symbol (cast_pd (i), GET_SYMBOL (a));
    }
    //
    }
    
    if (IS_POINTER (argv))      { pd_pointer (cast_pd (x), GET_POINTER (argv)); }
    else if (IS_FLOAT (argv))   { pd_float (cast_pd (x), GET_FLOAT (argv)); }
    else {
        pd_symbol (cast_pd (x), GET_SYMBOL (argv));
    }
    //
    }
} 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect *object_connect (t_object *src, int m, t_object *dest, int n)
{
    t_outlet *o = NULL;

    PD_ASSERT (m >= 0);
    PD_ASSERT (n >= 0);
    
    for (o = src->te_outlet; o && m; o = outlet_getNext (o), m--) { }
    
    if (o != NULL) { 
    //
    t_pd *to = NULL;
    t_inlet *i = NULL;
    t_outconnect *oc1 = NULL;
    t_outconnect *oc2 = NULL;
    
    if (pd_class (dest)->c_hasFirstInlet) { if (!n) { to = cast_pd (dest); } else { n--; } }
    
    if (to == NULL) {
        for (i = dest->te_inlet; i && n; i = inlet_getNext (i), n--) { }
        if (i == NULL) { return NULL; }
        else {
            to = cast_pd (i);
        }
    }

    oc1 = (t_outconnect *)PD_MEMORY_GET (sizeof (t_outconnect));
    oc1->oc_next = NULL;
    oc1->oc_receiver = to;

    if ((oc2 = o->o_connections)) { while (oc2->oc_next) { oc2 = oc2->oc_next; } oc2->oc_next = oc1; }
    else {
        o->o_connections = oc1;
    }
    
    if (outlet_isSignal (o)) { dsp_update(); }

    return oc1;
    //
    }
    
    return NULL;
}

void object_disconnect (t_object *src, int m, t_object *dest, int n)
{
    t_outlet *o = NULL;
    
    PD_ASSERT (m >= 0);
    PD_ASSERT (n >= 0);
    
    for (o = src->te_outlet; o && m; o = outlet_getNext (o), m--) { }
    
    if (o != NULL) {
    //
    t_pd *to = NULL;
    t_inlet *i = NULL;
    t_outconnect *oc1 = NULL;
    t_outconnect *oc2 = NULL;
    
    if (pd_class (dest)->c_hasFirstInlet) { if (!n) { to = cast_pd (dest); } else { n--; } }
    
    if (to == NULL) {
        for (i = dest->te_inlet; i && n; i = inlet_getNext (i), n--) { }
        if (i == NULL) { return; }
        to = cast_pd (i);
    }

    oc1 = o->o_connections;
    
    if (oc1 == NULL) { PD_BUG; return; }
    
    if (oc1->oc_receiver == to) {
        o->o_connections = oc1->oc_next; PD_MEMORY_FREE (oc1);
        
    } else {
        while ((oc2 = oc1->oc_next)) {
            if (oc2->oc_receiver != to) { oc1 = oc2; }
            else {
                oc1->oc_next = oc2->oc_next; PD_MEMORY_FREE (oc2); break;
            }
        }
    }

    if (outlet_isSignal (o)) { dsp_update(); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_numberOfInlets (t_object *x)
{
    int n = 0;
    t_inlet *i = NULL;
    if (pd_class (x)->c_hasFirstInlet) { n++; }
    for (i = x->te_inlet; i; i = inlet_getNext (i)) { n++; }
    return n;
}

int object_numberOfOutlets (t_object *x)
{
    int n = 0;
    t_outlet *o = NULL;
    for (o = x->te_outlet; o; o = outlet_getNext (o)) { n++; }
    return n;
}

int object_numberOfSignalInlets (t_object *x)
{
    int n = 0;
    t_inlet *i = NULL;
    if (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset) { n++; }
    for (i = x->te_inlet; i; i = inlet_getNext (i)) { if (inlet_isSignal (i)) { n++; } }
    return n;
}

int object_numberOfSignalOutlets (t_object *x)
{
    int n = 0;
    t_outlet *o = NULL;
    for (o = x->te_outlet; o; o = outlet_getNext (o)) { if (outlet_isSignal (o)) { n++; } }
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_indexAsSignalInlet (t_object *x, int m)
{
    int n = 0;
    t_inlet *i = NULL;
    
    PD_ASSERT (m >= 0);
        
    if (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset) {
        if (!m) { return 0; } else { n++; } 
        m--;
    }
    
    for (i = x->te_inlet; i; i = inlet_getNext (i), m--) {
        if (inlet_isSignal (i)) { 
            if (m == 0) { return n; } 
            else { 
                n++; 
            }
        }
    }
    
    return -1;
}

int object_indexAsSignalOutlet (t_object *x, int m)
{
    int n = 0;
    t_outlet *o = NULL;
    
    PD_ASSERT (m >= 0);
        
    for (o = x->te_outlet; o; o = outlet_getNext (o), m--) {
        if (outlet_isSignal (o)) {
            if (m == 0) { return n; }
            else {
                n++;
            }
        }
    }
    
    return -1;
}

int object_isSignalInlet (t_object *x, int m)
{
    t_inlet *i = NULL;
    
    if (pd_class (x)->c_hasFirstInlet) {
        if (!m) { return (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset); }
        else {
            m--;
        }
    }
    
    for (i = x->te_inlet; i && m; i = inlet_getNext (i), m--) { }
    
    return (i && inlet_isSignal (i));
}

int object_isSignalOutlet (t_object *x, int m)
{
    t_outlet *o = NULL;
    
    for (o = x->te_outlet; o && m--; o = outlet_getNext (o)) { }
    
    return (o && outlet_isSignal (o));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Fetch the nth outlet of an object. */
/* Return its first connection. */

t_outconnect *object_traverseOutletStart (t_object *x, t_outlet **ptr, int n)
{
    t_outlet *o = x->te_outlet;
    
    while (n && o) { n--; o = outlet_getNext (o); }
    
    *ptr = o;
    
    if (o) { return (o->o_connections); }
    else {
        return NULL;
    }
}

/* Given a connection, fetch the object connected, the related inlet and its index. */
/* Return the next connection of the outlet (NULL if last). */

t_outconnect *object_traverseOutletNext (t_outconnect *last, t_object **dest, t_inlet **ptr, int *n)
{
    t_pd *y = last->oc_receiver;

    t_class *c = pd_class (y);
    
    if (c == inlet_class || c == pointerinlet_class || c == floatinlet_class || c == symbolinlet_class) {
        t_inlet *i1 = (t_inlet *)y;
        t_inlet *i2 = NULL;
        t_object *o = inlet_getOwner (i1);
        int k = pd_class (o)->c_hasFirstInlet;
        for (i2 = o->te_inlet; i2 && i2 != i1; i2 = inlet_getNext (i2)) { k++; }
        *n    = k;
        *ptr  = i1;
        *dest = o;
        
    } else {
        *n    = 0;
        *ptr  = NULL;
        *dest = (cast_object (y));
    }
    
    return last->oc_next;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void object_saveWidth (t_object *x, t_buffer *b)
{
    if (x->te_width) { buffer_vAppend (b, "ssi;", sym___hash__X, sym_f, x->te_width); }
}

t_float *object_getSignalValueAtIndex (t_object *x, int m)
{
    t_inlet *i = NULL;
    
    if (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset) {
        if (!m) {
            if (pd_class (x)->c_signalOffset > 0) {
                return (t_float *)(((char *)x) + pd_class (x)->c_signalOffset);
            } else {
                PD_BUG; return NULL;
            }
        }
        m--;
    }
    
    for (i = x->te_inlet; i; i = inlet_getNext (i), m--) {
        if (inlet_isSignal (i)) { 
            if (m == 0) { 
                return inlet_getSignalValue (i); 
            } 
        }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
