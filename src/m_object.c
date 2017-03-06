
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void object_distributeOnInlets (t_object *x, int argc, t_atom *argv)
{
    if (argc) { 
        
        int count;
        t_atom *a = NULL;
        t_inlet *i = x->te_inlets;
        
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
        
    } else {
    
        if (class_hasBang (pd_class (x))) { (*(pd_class (x)->c_methodBang)) (cast_pd (x)); }
        else {
            (*(pd_class (x)->c_methodAnything)) (cast_pd (x), &s_bang, 0, NULL);
        }
    }
} 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect *object_connect (t_object *src, int m, t_object *dest, int n)
{
    t_outconnect *oc = NULL;
    t_outlet *o = NULL;
    
    PD_ASSERT (m >= 0);
    PD_ASSERT (n >= 0);
    
    for (o = src->te_outlets; o && m; o = outlet_getNext (o), m--) { }
    
    if (o != NULL) { 
    //
    t_pd *receiver = NULL;
    
    if (pd_class (dest)->c_hasFirstInlet) { if (!n) { receiver = cast_pd (dest); } else { n--; } }
    
    if (receiver == NULL) {
        t_inlet *i = NULL; for (i = dest->te_inlets; i && n; i = inlet_getNext (i), n--) { }
        if (i == NULL) { return NULL; }
        else {
            receiver = cast_pd (i);
        }
    }

    oc = outlet_addConnection (o, receiver);
    
    if (outlet_isSignal (o)) {
        dsp_update();
    }
    //
    }
    
    return oc;
}

void object_disconnect (t_object *src, int m, t_object *dest, int n)
{
    t_outlet *o = NULL;
    
    PD_ASSERT (m >= 0);
    PD_ASSERT (n >= 0);
    
    for (o = src->te_outlets; o && m; o = outlet_getNext (o), m--) { }
    
    if (o != NULL) {
    //
    t_pd *receiver = NULL;
    
    if (pd_class (dest)->c_hasFirstInlet) { if (!n) { receiver = cast_pd (dest); } else { n--; } }
    
    if (receiver == NULL) {
        t_inlet *i = NULL; for (i = dest->te_inlets; i && n; i = inlet_getNext (i), n--) { }
        if (i == NULL) { return; }
        else {
            receiver = cast_pd (i);
        }
    }

    outlet_removeConnection (o, receiver);
    
    if (outlet_isSignal (o)) {
        dsp_update(); 
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_getNumberOfInlets (t_object *x)
{
    int n = 0;
    t_inlet *i = NULL;
    if (pd_class (x)->c_hasFirstInlet) { n++; }
    for (i = x->te_inlets; i; i = inlet_getNext (i)) { n++; }
    return n;
}

int object_getNumberOfOutlets (t_object *x)
{
    int n = 0;
    t_outlet *o = NULL;
    for (o = x->te_outlets; o; o = outlet_getNext (o)) { n++; }
    return n;
}

int object_getNumberOfSignalInlets (t_object *x)
{
    int n = 0;
    t_inlet *i = NULL;
    if (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset) { n++; }
    for (i = x->te_inlets; i; i = inlet_getNext (i)) { if (inlet_isSignal (i)) { n++; } }
    return n;
}

int object_getNumberOfSignalOutlets (t_object *x)
{
    int n = 0;
    t_outlet *o = NULL;
    for (o = x->te_outlets; o; o = outlet_getNext (o)) { if (outlet_isSignal (o)) { n++; } }
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_getSignalIndexOfInlet (t_object *x, int m)
{
    int n = 0;
    t_inlet *i = NULL;
    
    PD_ASSERT (m >= 0);
        
    if (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset) {
        if (!m) { return 0; } else { n++; } 
        m--;
    }
    
    for (i = x->te_inlets; i; i = inlet_getNext (i), m--) {
        if (inlet_isSignal (i)) { 
            if (m == 0) { return n; } 
            else { 
                n++; 
            }
        }
    }
    
    return -1;
}

int object_getSignalIndexOfOutlet (t_object *x, int m)
{
    int n = 0;
    t_outlet *o = NULL;
    
    PD_ASSERT (m >= 0);
        
    for (o = x->te_outlets; o; o = outlet_getNext (o), m--) {
        if (outlet_isSignal (o)) {
            if (m == 0) { return n; }
            else {
                n++;
            }
        }
    }
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int object_isSignalInlet (t_object *x, int m)
{
    t_inlet *i = NULL;
    
    if (pd_class (x)->c_hasFirstInlet) {
        if (!m) { return (pd_class (x)->c_hasFirstInlet && pd_class (x)->c_signalOffset); }
        else {
            m--;
        }
    }
    
    for (i = x->te_inlets; i && m; i = inlet_getNext (i), m--) { }
    
    return (i && inlet_isSignal (i));
}

int object_isSignalOutlet (t_object *x, int m)
{
    t_outlet *o = NULL;
    
    for (o = x->te_outlets; o && m--; o = outlet_getNext (o)) { }
    
    return (o && outlet_isSignal (o));
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
    
    for (i = x->te_inlets; i; i = inlet_getNext (i), m--) {
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
