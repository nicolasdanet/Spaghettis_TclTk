
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
    
    for (count = argc - 1, a = argv + 1; i && count--; a++, i = i->i_next) {
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
    
    for (o = src->te_outlet; o && m; o = o->o_next, m--) { }
    
    if (o != NULL) { 
    //
    t_pd *to = NULL;
    t_inlet *i = NULL;
    t_outconnect *oc1 = NULL;
    t_outconnect *oc2 = NULL;
    
    if (pd_class (dest)->c_hasFirstInlet) { if (!n) { to = cast_pd (dest); } else { n--; } }
    
    if (to == NULL) {
        for (i = dest->te_inlet; i && n; i = i->i_next, n--) { }
        if (i == NULL) { return NULL; }
        else {
            to = cast_pd (i);
        }
    }

    oc1 = (t_outconnect *)PD_MEMORY_GET (sizeof (t_outconnect));
    oc1->oc_next = NULL;
    oc1->oc_to = to;

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
    
    for (o = src->te_outlet; o && m; o = o->o_next, m--) { }
    
    if (o != NULL) {
    //
    t_pd *to = NULL;
    t_inlet *i = NULL;
    t_outconnect *oc1 = NULL;
    t_outconnect *oc2 = NULL;
    
    if (pd_class (dest)->c_hasFirstInlet) { if (!n) { to = cast_pd (dest); } else { n--; } }
    
    if (to == NULL) {
        for (i = dest->te_inlet; i && n; i = i->i_next, n--) { }
        if (i == NULL) { return; }
        to = cast_pd (i);
    }

    oc1 = o->o_connections;
    
    if (oc1 == NULL) { PD_BUG; return; }
    
    if (oc1->oc_to == to) {
        o->o_connections = oc1->oc_next; PD_MEMORY_FREE (oc1);
        
    } else {
        while ((oc2 = oc1->oc_next)) {
            if (oc2->oc_to != to) { oc1 = oc2; }
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
    for (i = x->te_inlet; i; i = i->i_next) { n++; }
    return n;
}

int object_numberOfOutlets (t_object *x)
{
    int n = 0;
    t_outlet *o = NULL;
    for (o = x->te_outlet; o; o = o->o_next) { n++; }
    return n;
}

int object_numberOfSignalInlets (t_object *x)
{
    int n = 0;
    t_inlet *i = NULL;
    if (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset) { n++; }
    for (i = x->te_inlet; i; i = i->i_next) { if (i->i_from == &s_signal) { n++; } }
    return n;
}

int object_numberOfSignalOutlets (t_object *x)
{
    int n = 0;
    t_outlet *o = NULL;
    for (o = x->te_outlet; o; o = o->o_next) { if (o->o_symbol == &s_signal) { n++; } }
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
    
    for (i = x->te_inlet; i; i = i->i_next, m--) {
        if (i->i_from == &s_signal) { 
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
        
    for (o = x->te_outlet; o; o = o->o_next, m--) {
        if (o->o_symbol == &s_signal) {
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
    
    for (i = x->te_inlet; i && m; i = i->i_next, m--) { }
    
    return (i && (i->i_from == &s_signal));
}

int object_isSignalOutlet (t_object *x, int m)
{
    t_outlet *o = NULL;
    
    for (o = x->te_outlet; o && m--; o = o->o_next) { }
    
    return (o && (o->o_symbol == &s_signal));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_getIndexOfSignalInlet (t_inlet *x)
{
    int n = 0;
    t_inlet *i = NULL;
    
    PD_ASSERT (x->i_from == &s_signal);
    
    for (i = x->i_owner->te_inlet; i && i != x; i = i->i_next) { 
        if (i->i_from == &s_signal) { n++; }
    }
    
    return n;
}

int object_getIndexOfSignalOutlet (t_outlet *x)
{
    int n = 0;
    t_outlet *o = NULL;
    
    PD_ASSERT (x->o_symbol == &s_signal);
    
    for (o = x->o_owner->te_outlet; o && o != x; o = o->o_next) {
        if (o->o_symbol == &s_signal) { n++; }
    }
    
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Fetch the nth outlet of an object. */
/* Return its first connection. */

t_outconnect *object_traverseOutletStart (t_object *x, t_outlet **ptr, int n)
{
    t_outlet *o = x->te_outlet;
    
    while (n && o) { n--; o = o->o_next; }
    
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
    t_pd *y = last->oc_to;

    t_class *c = pd_class (y);
    
    if (c == inlet_class || c == pointerinlet_class || c == floatinlet_class || c == symbolinlet_class) {
        t_inlet *i1 = (t_inlet *)y;
        t_inlet *i2 = NULL;
        t_object *o = i1->i_owner;
        int k = pd_class (o)->c_hasFirstInlet;
        for (i2 = o->te_inlet; i2 && i2 != i1; i2 = i2->i_next) { k++; }
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

void object_moveInletFirst (t_object *x, t_inlet *i)
{
    if (x->te_inlet != i) {
    //
    t_inlet *i2 = NULL;
        
    for (i2 = x->te_inlet; i2; i2 = i2->i_next) {
        if (i2->i_next == i) {
            i2->i_next = i->i_next;
            i->i_next = x->te_inlet;
            x->te_inlet = i;
            break;
        }
    }
    //
    }
}

void object_moveOutletFirst (t_object *x, t_outlet *o)
{
    if (x->te_outlet != o) {
    //
    t_outlet *o2 = NULL;
    
    for (o2 = x->te_outlet; o2; o2 = o2->o_next) {
        if (o2->o_next == o) {
            o2->o_next = o->o_next;
            o->o_next = x->te_outlet;
            x->te_outlet = o;
            break;
        }
    }
    //
    }
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
    
    for (i = x->te_inlet; i; i = i->i_next, m--) {
        if (i->i_from == &s_signal) {
            if (m == 0) { return &i->i_un.i_signal; }
        }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
