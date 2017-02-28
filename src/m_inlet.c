
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *inlet_class;                   /* Shared. */
t_class *pointerinlet_class;            /* Shared. */
t_class *floatinlet_class;              /* Shared. */
t_class *symbolinlet_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void inlet_list (t_inlet *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void inlet_unexpected (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    error_unexpected (class_getName (pd_class (x->i_owner)), s);
}

static void inlet_add (t_inlet *x, t_object *owner)
{
    t_inlet *i1 = NULL;
    t_inlet *i2 = NULL;
    
    PD_ASSERT (owner);
    
    if ((i1 = owner->te_inlets)) { while ((i2 = i1->i_next)) { i1 = i2; } i1->i_next = x; } 
    else { 
        owner->te_inlets = x;
    }
}

int inlet_isSignal (t_inlet *x)
{
    return (x->i_type == &s_signal);
}

void inlet_moveFirst (t_inlet *x)
{
    t_object *owner = x->i_owner;
    
    if (owner->te_inlets != x) {
    //
    t_inlet *i = NULL;
        
    for (i = owner->te_inlets; i; i = i->i_next) {
        if (i->i_next == x) {
            i->i_next = x->i_next;
            x->i_next = owner->te_inlets;
            owner->te_inlets = x;
            return;
        }
    }
    
    PD_BUG;
    //
    }
}

int inlet_getSignalIndex (t_inlet *x)
{
    t_inlet *i = NULL;
    int n = 0;
        
    PD_ASSERT (inlet_isSignal (x));
    
    for (i = x->i_owner->te_inlets; i && i != x; i = i->i_next) { 
        if (inlet_isSignal (i)) { n++; }
    }
    
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* For background compatibility the broadcasting rules below must be changed with care. */

static void inlet_bang (t_inlet *x)
{
    if (x->i_type == &s_bang)        { pd_vMessage (x->i_receiver, x->i_un.i_method, ""); }
    else if (x->i_type == NULL)      { pd_bang (x->i_receiver); }
    else if (x->i_type == &s_list)   { inlet_list (x, &s_bang, 0, NULL); }
    else {
        inlet_unexpected (x, &s_bang, 0, NULL);
    }
}

static void inlet_float (t_inlet *x, t_float f)
{
    if (x->i_type == &s_float)       { pd_vMessage (x->i_receiver, x->i_un.i_method, "f", f); }
    else if (x->i_type == &s_signal) { x->i_un.i_signal = f; }
    else if (x->i_type == NULL)      { pd_float (x->i_receiver, f); }
    else if (x->i_type == &s_list)   { t_atom a; SET_FLOAT (&a, f); inlet_list (x, &s_float, 1, &a); }
    else { 
        inlet_unexpected (x, &s_float, 0, NULL);
    }
}

static void inlet_symbol (t_inlet *x, t_symbol *s)
{
    if (x->i_type == &s_symbol)      { pd_vMessage (x->i_receiver, x->i_un.i_method, "s", s); }
    else if (x->i_type == NULL)      { pd_symbol (x->i_receiver, s); }
    else if (x->i_type == &s_list)   { t_atom a; SET_SYMBOL (&a, s); inlet_list (x, &s_symbol, 1, &a); }
    else { 
        inlet_unexpected (x, &s_symbol, 0, NULL);
    }
}

static void inlet_pointer (t_inlet *x, t_gpointer *gp)
{
    if (x->i_type == &s_pointer)     { pd_vMessage (x->i_receiver, x->i_un.i_method, "p", gp); }
    else if (x->i_type == NULL)      { pd_pointer (x->i_receiver, gp); }
    else if (x->i_type == &s_list)   { t_atom a; SET_POINTER (&a, gp); inlet_list (x, &s_pointer, 1, &a); }
    else {
        inlet_unexpected (x, &s_pointer, 0, NULL);
    }
}

static void inlet_list (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_type == &s_list)               { pd_message (x->i_receiver, x->i_un.i_method, argc, argv); }
    else if (x->i_type == &s_float)         { pd_message (x->i_receiver, x->i_un.i_method, argc, argv); }
    else if (x->i_type == &s_symbol)        { pd_message (x->i_receiver, x->i_un.i_method, argc, argv); }
    else if (x->i_type == &s_pointer)       { pd_message (x->i_receiver, x->i_un.i_method, argc, argv); }
    else if (x->i_type == NULL)             { pd_list (x->i_receiver, argc, argv); }
    else if (!argc)                         { inlet_bang (x); }
    else if (argc == 1 && IS_FLOAT (argv))  { inlet_float (x, atom_getFloat (argv));   }
    else if (argc == 1 && IS_SYMBOL (argv)) { inlet_symbol (x, atom_getSymbol (argv)); }
    else { 
        inlet_unexpected (x, &s_list, argc, argv);
    }
}

static void inlet_anything (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_type == s)         { pd_message (x->i_receiver, x->i_un.i_method, argc, argv); }
    else if (x->i_type == NULL) { pd_message (x->i_receiver, s, argc, argv); }
    else {
        inlet_unexpected (x, s, argc, argv);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void inlet_forFloat (t_inlet *x, t_float f)
{
    *(x->i_un.i_float) = f;
}

static void inlet_forSymbol (t_inlet *x, t_symbol *s)
{
    *(x->i_un.i_symbol) = s;
}

static void inlet_forPointer (t_inlet *x, t_gpointer *gp)
{
    gpointer_setByCopy (x->i_un.i_pointer, gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Typed inlet that just store the incoming value. */

t_inlet *inlet_newSignalDefault (t_object *owner, t_float f)
{
    t_inlet *x = inlet_new (owner, NULL, &s_signal, NULL);
    
    x->i_un.i_signal = f;
    
    return x;
}

t_inlet *inlet_newSignal (t_object *owner)
{
    return inlet_newSignalDefault (owner, (t_float)0.0);
}

t_inlet *inlet_newFloat (t_object *owner, t_float *fp)
{
    t_inlet *x = (t_inlet *)pd_new (floatinlet_class);
    
    x->i_owner          = owner;
    x->i_receiver       = NULL;
    x->i_type           = &s_float;
    x->i_un.i_float     = fp;
    x->i_next           = NULL;
    
    inlet_add (x, owner);
    
    return x;
}

t_inlet *inlet_newPointer (t_object *owner, t_gpointer *gp)
{
    t_inlet *x = (t_inlet *)pd_new (pointerinlet_class);
    
    x->i_owner          = owner;
    x->i_receiver       = NULL;
    x->i_type           = &s_pointer;
    x->i_un.i_pointer   = gp;
    x->i_next           = NULL;
    
    inlet_add (x, owner);
    
    return x;
}

t_inlet *inlet_newSymbol (t_object *owner, t_symbol **sp)
{
    t_inlet *x = (t_inlet *)pd_new (symbolinlet_class);
    
    x->i_owner          = owner;
    x->i_receiver       = NULL;
    x->i_type           = &s_symbol;
    x->i_un.i_symbol    = sp;
    x->i_next           = NULL;
    
    inlet_add (x, owner);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Proxy inlet that call a receiver's method. */
/* Broadcasting rules are defined according to its type. */
/* A signal inlet is always typed. */

t_inlet *inlet_new (t_object *owner, t_pd *receiver, t_symbol *type, t_symbol *method)
{
    t_inlet *x = (t_inlet *)pd_new (inlet_class);
    
    x->i_owner    = owner;
    x->i_receiver = receiver;
    x->i_type     = type;
    x->i_next     = NULL;
    
    if (type != &s_signal) { x->i_un.i_method = method; }
    else {
        x->i_un.i_signal = (t_float)0.0; 
    }
    
    inlet_add (x, owner);
    
    return x;
}

void inlet_free (t_inlet *x)
{
    t_object *y = x->i_owner;
    t_inlet *i  = NULL;
    
    if (y->te_inlets == x) { y->te_inlets = x->i_next; }
    else {
        for (i = y->te_inlets; i; i = i->i_next) {
            if (i->i_next == x) {
                i->i_next = x->i_next;
                break;
            }
        }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void inlet_setup (void)
{
    inlet_class = class_new (sym_inlet,
                            NULL,
                            NULL,
                            sizeof (t_inlet),
                            CLASS_NOBOX,
                            A_NULL);
    
    floatinlet_class = class_new (sym_floatinlet,
                            NULL,
                            NULL,
                            sizeof (t_inlet),
                            CLASS_NOBOX,
                            A_NULL);
    
    symbolinlet_class = class_new (sym_symbolinlet, 
                            NULL,
                            NULL,
                            sizeof (t_inlet),
                            CLASS_NOBOX,
                            A_NULL);
    
    pointerinlet_class = class_new (sym_pointerinlet,
                            NULL,
                            NULL,
                            sizeof (t_inlet),
                            CLASS_NOBOX,
                            A_NULL);
    
    class_addBang (inlet_class,             (t_method)inlet_bang);
    class_addFloat (inlet_class,            (t_method)inlet_float);
    class_addSymbol (inlet_class,           (t_method)inlet_symbol);
    class_addPointer (inlet_class,          (t_method)inlet_pointer);
    class_addList (inlet_class,             (t_method)inlet_list);
    class_addAnything (inlet_class,         (t_method)inlet_anything);
    
    class_addFloat (floatinlet_class,       (t_method)inlet_forFloat);
    class_addAnything (floatinlet_class,    (t_method)inlet_unexpected);
    
    class_addSymbol (symbolinlet_class,     (t_method)inlet_forSymbol);
    class_addAnything (symbolinlet_class,   (t_method)inlet_unexpected);
    
    class_addPointer (pointerinlet_class,   (t_method)inlet_forPointer);
    class_addAnything (pointerinlet_class,  (t_method)inlet_unexpected);  
}

void inlet_destroy (void)
{
    CLASS_FREE (inlet_class);
    CLASS_FREE (pointerinlet_class);
    CLASS_FREE (floatinlet_class);
    CLASS_FREE (symbolinlet_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
