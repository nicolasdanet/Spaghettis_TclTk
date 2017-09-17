
/* Copyright (c) 1997-2017 Miller Puckette and others. */

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
// MARK: -

t_object *object_setFromEntry (t_object *x, t_glist *glist, t_box *z)
{
    char *s = NULL; int size;

    box_getText (z, &s, &size);
    
    if (!object_isObject (x)) { buffer_withStringUnzeroed (object_getBuffer (x), s, size); }
    else {
    //
    t_buffer *t = buffer_new();
    
    buffer_withStringUnzeroed (t, s, size);
    
    {
    //
    int m = ((utils_getFirstAtomOfObject (x) == sym_pd) && (gobj_isCanvas (cast_gobj (x))));
    int n = ((utils_getFirstAtomOfBuffer (t) == sym_pd));
    
    if (m && n) {
        
        /* Subpatch renamed. */
        
        glist_rename (cast_glist (x), buffer_getSize (t) - 1, buffer_getAtoms (t) + 1); 
        
        object_setBuffer (x, t);
        
    } else {
    
        /* Trigger instantiation. */
        
        int w = object_getWidth (x);
        int a = object_getX (x);
        int b = object_getY (x);
        
        glist_objectRemove (glist, cast_gobj (x));
        glist_objectMake (glist, a, b, w, 0, t);
        editor_selectionRestoreLines (glist_getEditor (glist));
        
        /* Loadbang if the new object is an abstraction. */
        
        if (instance_getNewestObject() && gobj_isCanvas (cast_gobj (instance_getNewestObject()))) {
            glist_loadbang (cast_glist (instance_getNewestObject())); 
        }
        
        if (instance_getNewestObject()) { return cast_objectIfConnectable (instance_getNewestObject()); }
        else {
            return NULL;
        }
    }
    //
    }
    //
    }
    
    if (object_isMessage (x)) { message_dirty ((t_message *)x); box_retext (z); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int object_setSnappedX (t_object *x, int n)
{
    int k = object_getX (x);
    
    if (snap_hasSnapToGrid()) { n = snap_getSnapped (n); }
    
    object_setX (x, n);
    
    return (object_getX (x) - k);
}

int object_setSnappedY (t_object *x, int n)
{
    int k = object_getY (x);

    if (snap_hasSnapToGrid()) { n = snap_getSnapped (n); }
    
    object_setY (x, n);
    
    return (object_getY (x) - k);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void object_distributeAtomsOnInlets (t_object *x, int argc, t_atom *argv)
{
    if (argc) { 
        
        int count;
        t_atom *a = NULL;
        t_inlet *i = x->te_inlets;
        
        for (count = argc - 1, a = argv + 1; i && count--; a++, i = inlet_getNext (i)) {
        //
        if (IS_POINTER (a))         { pd_pointer (cast_pd (i), GET_POINTER (a)); }
        else if (IS_FLOAT (a))      { pd_float (cast_pd (i), GET_FLOAT (a)); }
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
    
        if (class_hasOverrideBangMethod (pd_class (x))) {
            (*(class_getBangMethod (pd_class (x)))) (cast_pd (x)); 
        } else {
            (*(class_getAnythingMethod (pd_class (x)))) (cast_pd (x), &s_bang, 0, NULL);
        }
    }
} 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that the linked list of inlets do NOT contains the first inlet. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    if (class_hasFirstInlet (pd_class (dest))) { if (!n) { receiver = cast_pd (dest); } else { n--; } }
    
    if (receiver == NULL) {
        t_inlet *i = NULL; for (i = dest->te_inlets; i && n; i = inlet_getNext (i), n--) { }
        if (i == NULL) { return NULL; }
        else {
            receiver = cast_pd (i);
        }
    }

    oc = outlet_addConnection (o, receiver);
    
    if (outlet_isSignal (o)) { dsp_update(); }
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
    
    if (class_hasFirstInlet (pd_class (dest))) { if (!n) { receiver = cast_pd (dest); } else { n--; } }
    
    if (receiver == NULL) {
        t_inlet *i = NULL; for (i = dest->te_inlets; i && n; i = inlet_getNext (i), n--) { }
        if (i == NULL) { return; }
        else {
            receiver = cast_pd (i);
        }
    }

    outlet_removeConnection (o, receiver);
    
    if (outlet_isSignal (o)) { dsp_update(); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int object_getNumberOfInlets (t_object *x)
{
    int n = 0;
    t_inlet *i = NULL;
    if (class_hasFirstInlet (pd_class (x))) { n++; }
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
    if (class_hasFirstInletAsSignal (pd_class (x)))  { n++; }
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
// MARK: -

int object_getIndexAsSignalOfInlet (t_object *x, int m)
{
    int n = 0;
    t_inlet *i = NULL;
    
    PD_ASSERT (m >= 0);
        
    if (class_hasFirstInletAsSignal (pd_class (x))) {
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

int object_getIndexAsSignalOfOutlet (t_object *x, int m)
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
// MARK: -

int object_isSignalInlet (t_object *x, int m)
{
    t_inlet *i = NULL;
    
    if (class_hasFirstInlet (pd_class (x))) {
        if (!m) { return (class_hasFirstInletAsSignal (pd_class (x))); }
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
// MARK: -

void object_serializeWidth (t_object *x, t_buffer *b)
{
    if (x->te_width) { buffer_vAppend (b, "ssi;", sym___hash__X, sym_f, x->te_width); }
}

t_float *object_getSignalAtIndex (t_object *x, int m)
{
    t_inlet *i = NULL;
    
    if (class_hasFirstInletAsSignal (pd_class (x))) {
        if (!m) { return class_getFirstInletSignal (cast_pd (x)); }
        m--;
    }
    
    for (i = x->te_inlets; i; i = inlet_getNext (i), m--) {
        if (inlet_isSignal (i)) { 
            if (m == 0) {
                return inlet_getSignal (i);
            }
        }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
