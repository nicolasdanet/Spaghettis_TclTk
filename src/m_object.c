
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define OBJECT_MAXIMUM_ITERATION            1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int object_stackCount;               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _inlet
{
    t_pd                i_pd;               /* MUST be the first. */
    struct _inlet       *i_next;
    t_object            *i_owner;
    t_pd                *i_destination;
    t_symbol            *i_from;
    union {
        t_symbol        *i_to;
        t_gpointer      *i_pointer;
        t_float         *i_float;
        t_symbol        **i_symbol;
        t_float         i_signal;
    } i_un;
};

struct _outconnect
{
    struct _outconnect  *oc_next;
    t_pd                *oc_to;
};

struct _outlet
{
    struct _outlet      *o_next;
    t_object            *o_owner;
    t_outconnect        *o_connections;
    t_symbol            *o_symbol;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *inlet_class;                /* Shared. */
static t_class *pointerinlet_class;         /* Shared. */
static t_class *floatinlet_class;           /* Shared. */
static t_class *symbolinlet_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void inlet_forList (t_inlet *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void object_errorUnexpected (t_inlet *x, t_symbol *s)
{
    post_error (PD_TRANSLATE ("inlet: unexpected '%s'"), s->s_name);    // --
}

static void object_errorStackOverflow (t_outlet *x)
{
    post_error (PD_TRANSLATE ("inlet: stack overflow"));    // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void inlet_forBang (t_inlet *x)
{
    if (x->i_from == &s_bang)               { pd_vMessage (x->i_destination, x->i_un.i_to, ""); }
    else if (x->i_from == NULL)             { pd_bang (x->i_destination); }
    else if (x->i_from == &s_list)          { inlet_forList (x, &s_bang, 0, NULL); }
    else {
        object_errorUnexpected (x, &s_bang);
    }
}

static void inlet_forPointer (t_inlet *x, t_gpointer *gp)
{
    if (x->i_from == &s_pointer)            { pd_vMessage (x->i_destination, x->i_un.i_to, "p", gp); }
    else if (x->i_from == NULL)             { pd_pointer (x->i_destination, gp); }
    else if (x->i_from == &s_list) {
        t_atom a;
        SET_POINTER (&a, gp);
        inlet_forList (x, &s_pointer, 1, &a);

    } else {
        object_errorUnexpected (x, &s_pointer);
    }
}

static void inlet_forFloat (t_inlet *x, t_float f)
{
    if (x->i_from == &s_float)              { pd_vMessage (x->i_destination, x->i_un.i_to, "f", f); }
    else if (x->i_from == &s_signal)        { x->i_un.i_signal = f; }
    else if (x->i_from == NULL)             { pd_float (x->i_destination, f); }
    else if (x->i_from == &s_list) {
        t_atom a;
        SET_FLOAT (&a, f);
        inlet_forList (x, &s_float, 1, &a);
    } else { 
        object_errorUnexpected (x, &s_float);
    }
}

static void inlet_forSymbol (t_inlet *x, t_symbol *s)
{
    if (x->i_from == &s_symbol)             { pd_vMessage (x->i_destination, x->i_un.i_to, "s", s); }
    else if (x->i_from == NULL)             { pd_symbol (x->i_destination, s); }
    else if (x->i_from == &s_list) {
        t_atom a;
        SET_SYMBOL (&a, s);
        inlet_forList (x, &s_symbol, 1, &a);
    } else { 
        object_errorUnexpected (x, &s_symbol);
    }
}

static void inlet_forList (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_from == &s_list)               { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == &s_float)         { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == &s_symbol)        { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == &s_pointer)       { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == NULL)             { pd_list (x->i_destination, argc, argv);  }
    else if (!argc)                         { inlet_forBang (x); }
    else if (argc == 1 && IS_FLOAT (argv))  { inlet_forFloat (x, atom_getFloat (argv));   }
    else if (argc == 1 && IS_SYMBOL (argv)) { inlet_forSymbol (x, atom_getSymbol (argv)); }
    else { 
        object_errorUnexpected (x, &s_list);
    }
}

static void inlet_forAnything (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_from == s)                     { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == NULL)             { pd_message (x->i_destination, s, argc, argv); }
    else {
        object_errorUnexpected (x, s);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void floatinlet_forFloat (t_inlet *x, t_float f)
{
    *(x->i_un.i_float) = f;
}

static void symbolinlet_forSymbol (t_inlet *x, t_symbol *s)
{
    *(x->i_un.i_symbol) = s;
}

static void pointerinlet_forPointer (t_inlet *x, t_gpointer *gp)
{
    gpointer_unset (x->i_un.i_pointer); 
    *(x->i_un.i_pointer) = *gp;
    if (gp->gp_stub) { gp->gp_stub->gs_count++; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *inlet_new (t_object *owner, t_pd *destination, t_symbol *s1, t_symbol *s2)
{
    t_inlet *x = (t_inlet *)pd_new (inlet_class);
    t_inlet *y1 = NULL;
    t_inlet *y2 = NULL;
    
    x->i_owner = owner;
    x->i_destination = destination;
    
    if (s1 == &s_signal) { x->i_un.i_signal = 0.0; }
    else { 
        x->i_un.i_to = s2;
    }
    
    x->i_from = s1;
    x->i_next = NULL;
    
    if (y1 = owner->te_inlet) { while (y2 = y1->i_next) { y1 = y2; } y1->i_next = x; } 
    else { 
        owner->te_inlet = x;
    }
    
    return x;
}

void inlet_free (t_inlet *x)
{
    t_object *y = x->i_owner;
    t_inlet *x2 = NULL;
    
    if (y->te_inlet == x) { y->te_inlet = x->i_next; }
    else {
        for (x2 = y->te_inlet; x2; x2 = x2->i_next) {
            if (x2->i_next == x) {
                x2->i_next = x->i_next; break;
            }
        }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *inlet_newSignal (t_object *owner, t_float f)
{
    t_inlet *x = inlet_new (owner, cast_pd (owner), &s_signal, &s_signal);
    
    x->i_un.i_signal = f;
    
    return x;
}

t_inlet *inlet_newPointer (t_object *owner, t_gpointer *gp)
{
    t_inlet *x = (t_inlet *)pd_new (pointerinlet_class);
    t_inlet *y1 = NULL;
    t_inlet *y2 = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    x->i_from = &s_pointer;
    x->i_un.i_pointer = gp;
    x->i_next = NULL;
    
    if (y1 = owner->te_inlet) { while (y2 = y1->i_next) { y1 = y2; } y1->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

t_inlet *inlet_newFloat (t_object *owner, t_float *fp)
{
    t_inlet *x = (t_inlet *)pd_new (floatinlet_class);
    t_inlet *y1 = NULL;
    t_inlet *y2 = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    x->i_from = &s_float;
    x->i_un.i_float = fp;
    x->i_next = NULL;
    
    if (y1 = owner->te_inlet) { while (y2 = y1->i_next) { y1 = y2; } y1->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

t_inlet *inlet_newSymbol (t_object *owner, t_symbol **sp)
{
    t_inlet *x = (t_inlet *)pd_new (symbolinlet_class);
    t_inlet *y1 = NULL;
    t_inlet *y2 = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    x->i_from = &s_symbol;
    x->i_un.i_symbol = sp;
    x->i_next = NULL;
    
    if (y1 = owner->te_inlet) { while (y2 = y1->i_next) { y1 = y2; } y1->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outlet *outlet_new (t_object *owner, t_symbol *s)
{
    t_outlet *x = (t_outlet *)PD_MEMORY_GET (sizeof (t_outlet));
    t_outlet *y1 = NULL;
    t_outlet *y2 = NULL;
    
    x->o_next  = NULL;
    x->o_owner = owner;

    if (y1 = owner->te_outlet) { while (y2 = y1->o_next) { y1 = y2; } y1->o_next = x; }
    else {
        owner->te_outlet = x;
    }
    
    x->o_connections = NULL;
    x->o_symbol = s;
    
    return x;
}

void outlet_free (t_outlet *x)
{
    t_object *y = x->o_owner;
    t_outlet *x2 = NULL;
    
    if (y->te_outlet == x) { y->te_outlet = x->o_next; }
    else {
        for (x2 = y->te_outlet; x2; x2 = x2->o_next) {
            if (x2->o_next == x) { x2->o_next = x->o_next; break; }
        }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int outlet_isSignal (t_outlet *x)
{
    return (x->o_symbol == &s_signal);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void outlet_bang (t_outlet *x)
{
    t_outconnect *oc = NULL;
    
    if (++object_stackCount >= OBJECT_MAXIMUM_ITERATION)  { object_errorStackOverflow (x); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_bang (oc->oc_to); }
    }
    
    --object_stackCount;
}

void outlet_pointer (t_outlet *x, t_gpointer *gp)
{
    t_outconnect *oc = NULL;
    t_gpointer gpointer;
    
    if (++object_stackCount >= OBJECT_MAXIMUM_ITERATION)  { object_errorStackOverflow (x); }
    else {
        gpointer = *gp;
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_pointer (oc->oc_to, &gpointer); }
    }
    
    --object_stackCount;
}

void outlet_float (t_outlet *x, t_float f)
{
    t_outconnect *oc = NULL;
    
    if (++object_stackCount >= OBJECT_MAXIMUM_ITERATION)  { object_errorStackOverflow (x); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_float (oc->oc_to, f); }
    }
    
    --object_stackCount;
}

void outlet_symbol (t_outlet *x, t_symbol *s)
{
    t_outconnect *oc = NULL;
    
    if (++object_stackCount >= OBJECT_MAXIMUM_ITERATION)  { object_errorStackOverflow (x); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_symbol (oc->oc_to, s); }
    }
    
    --object_stackCount;
}

void outlet_list (t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc = NULL;
    
    if (++object_stackCount >= OBJECT_MAXIMUM_ITERATION)  { object_errorStackOverflow (x); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_list (oc->oc_to, argc, argv); }
    }
    
    --object_stackCount;
}

void outlet_anything (t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc = NULL;
    
    if (++object_stackCount >= OBJECT_MAXIMUM_ITERATION)  { object_errorStackOverflow (x); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_message (oc->oc_to, s, argc, argv); }
    }
    
    --object_stackCount;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void object_initialize (void)
{
    inlet_class         = class_new (gensym ("inlet"), NULL, NULL, sizeof (t_inlet), CLASS_PURE, A_NULL);
    pointerinlet_class  = class_new (gensym ("inlet"), NULL, NULL, sizeof (t_inlet), CLASS_PURE, A_NULL);
    floatinlet_class    = class_new (gensym ("inlet"), NULL, NULL, sizeof (t_inlet), CLASS_PURE, A_NULL);
    symbolinlet_class   = class_new (gensym ("inlet"), NULL, NULL, sizeof (t_inlet), CLASS_PURE, A_NULL);
    
    class_addBang (inlet_class,             (t_method)inlet_forBang);
    class_addPointer (inlet_class,          (t_method)inlet_forPointer);
    class_addFloat (inlet_class,            (t_method)inlet_forFloat);
    class_addSymbol (inlet_class,           (t_method)inlet_forSymbol);
    class_addList (inlet_class,             (t_method)inlet_forList);
    class_addAnything (inlet_class,         (t_method)inlet_forAnything);
    
    class_addFloat (floatinlet_class,       (t_method)floatinlet_forFloat);
    class_addSymbol (symbolinlet_class,     (t_method)symbolinlet_forSymbol);
    class_addPointer (pointerinlet_class,   (t_method)pointerinlet_forPointer);
        
    class_addAnything (floatinlet_class,    (t_method)object_errorUnexpected);
    class_addAnything (symbolinlet_class,   (t_method)object_errorUnexpected);
    class_addAnything (pointerinlet_class,  (t_method)object_errorUnexpected);
}

void object_release (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void object_list (t_object *x, t_symbol *s, int argc, t_atom *argv)
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
        while (oc2 = oc1->oc_next) {
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

int object_indexOfSignalInlet (t_object *x, int m)
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

int object_indexOfSignalOutlet (t_object *x, int m)
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
    int n = 0;
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
/* Return its first connection.  */

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
        t_inlet *i1 = cast_inlet (y);
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
    if (x->te_width) { buffer_vAppend (b, "ssf;", &s__X, gensym ("f"), (t_float)x->te_width); }
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
