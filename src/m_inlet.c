
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

static void inlet_errorUnexpected (t_inlet *x, t_symbol *s)
{
    error_unexpected (sym_inlet, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void inlet_bang (t_inlet *x)
{
    if (x->i_from == &s_bang)               { pd_vMessage (x->i_destination, x->i_un.i_to, ""); }
    else if (x->i_from == NULL)             { pd_bang (x->i_destination); }
    else if (x->i_from == &s_list)          { inlet_list (x, &s_bang, 0, NULL); }
    else {
        error_unexpected (class_getName (pd_class (x)), &s_bang);
    }
}

static void inlet_float (t_inlet *x, t_float f)
{
    if (x->i_from == &s_float)              { pd_vMessage (x->i_destination, x->i_un.i_to, "f", f); }
    else if (x->i_from == &s_signal)        { x->i_un.i_signal = f; }
    else if (x->i_from == NULL)             { pd_float (x->i_destination, f); }
    else if (x->i_from == &s_list) {
        t_atom a;
        SET_FLOAT (&a, f);
        inlet_list (x, &s_float, 1, &a);
    } else { 
        error_unexpected (class_getName (pd_class (x)), &s_float);
    }
}

static void inlet_symbol (t_inlet *x, t_symbol *s)
{
    if (x->i_from == &s_symbol)             { pd_vMessage (x->i_destination, x->i_un.i_to, "s", s); }
    else if (x->i_from == NULL)             { pd_symbol (x->i_destination, s); }
    else if (x->i_from == &s_list) {
        t_atom a;
        SET_SYMBOL (&a, s);
        inlet_list (x, &s_symbol, 1, &a);
    } else { 
        error_unexpected (class_getName (pd_class (x)), &s_symbol);
    }
}

static void inlet_pointer (t_inlet *x, t_gpointer *gp)
{
    if (x->i_from == &s_pointer)            { pd_vMessage (x->i_destination, x->i_un.i_to, "p", gp); }
    else if (x->i_from == NULL)             { pd_pointer (x->i_destination, gp); }
    else if (x->i_from == &s_list) {
        t_atom a;
        SET_POINTER (&a, gp);
        inlet_list (x, &s_pointer, 1, &a);

    } else {
        error_unexpected (class_getName (pd_class (x)), &s_pointer);
    }
}

static void inlet_list (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_from == &s_list)               { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == &s_float)         { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == &s_symbol)        { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == &s_pointer)       { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == NULL)             { pd_list (x->i_destination, argc, argv);  }
    else if (!argc)                         { inlet_bang (x); }
    else if (argc == 1 && IS_FLOAT (argv))  { inlet_float (x, atom_getFloat (argv));   }
    else if (argc == 1 && IS_SYMBOL (argv)) { inlet_symbol (x, atom_getSymbol (argv)); }
    else { 
        error_unexpected (class_getName (pd_class (x)), &s_list);
    }
}

static void inlet_anything (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_from == s)                     { pd_message (x->i_destination, x->i_un.i_to, argc, argv); }
    else if (x->i_from == NULL)             { pd_message (x->i_destination, s, argc, argv); }
    else {
        error_unexpected (class_getName (pd_class (x)), s);
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

t_inlet *inlet_newFloat (t_object *owner, t_float *fp)
{
    t_inlet *x = (t_inlet *)pd_new (floatinlet_class);
    t_inlet *yA = NULL;
    t_inlet *yB = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    x->i_from = &s_float;
    x->i_un.i_float = fp;
    x->i_next = NULL;
    
    if ((yA = owner->te_inlet)) { while ((yB = yA->i_next)) { yA = yB; } yA->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

t_inlet *inlet_newPointer (t_object *owner, t_gpointer *gp)
{
    t_inlet *x = (t_inlet *)pd_new (pointerinlet_class);
    t_inlet *yA = NULL;
    t_inlet *yB = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    x->i_from = &s_pointer;
    x->i_un.i_pointer = gp;
    x->i_next = NULL;
    
    if ((yA = owner->te_inlet)) { while ((yB = yA->i_next)) { yA = yB; } yA->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

t_inlet *inlet_newSymbol (t_object *owner, t_symbol **sp)
{
    t_inlet *x = (t_inlet *)pd_new (symbolinlet_class);
    t_inlet *yA = NULL;
    t_inlet *yB = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    x->i_from = &s_symbol;
    x->i_un.i_symbol = sp;
    x->i_next = NULL;
    
    if ((yA = owner->te_inlet)) { while ((yB = yA->i_next)) { yA = yB; } yA->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

t_inlet *inlet_newSignal (t_object *owner)
{
    return inlet_new (owner, cast_pd (owner), &s_signal, &s_signal);
}

t_inlet *inlet_newSignalDefault (t_object *owner, t_float f)
{
    t_inlet *x = inlet_new (owner, cast_pd (owner), &s_signal, &s_signal);
    
    x->i_un.i_signal = f;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *inlet_new (t_object *owner, t_pd *destination, t_symbol *s1, t_symbol *s2)
{
    t_inlet *x = (t_inlet *)pd_new (inlet_class);
    t_inlet *yA = NULL;
    t_inlet *yB = NULL;
    
    x->i_owner = owner;
    x->i_destination = destination;
    
    if (s1 == &s_signal) { x->i_un.i_signal = (t_float)0.0; }
    else { 
        x->i_un.i_to = s2;
    }
    
    x->i_from = s1;
    x->i_next = NULL;
    
    if ((yA = owner->te_inlet)) { while ((yB = yA->i_next)) { yA = yB; } yA->i_next = x; } 
    else { 
        owner->te_inlet = x;
    }
    
    return x;
}

void inlet_free (t_inlet *x)
{
    t_object *y = x->i_owner;
    t_inlet *xB = NULL;
    
    if (y->te_inlet == x) { y->te_inlet = x->i_next; }
    else {
        for (xB = y->te_inlet; xB; xB = xB->i_next) {
            if (xB->i_next == x) {
                xB->i_next = x->i_next; break;
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
    class_addAnything (floatinlet_class,    (t_method)inlet_errorUnexpected);
    
    class_addSymbol (symbolinlet_class,     (t_method)inlet_forSymbol);
    class_addAnything (symbolinlet_class,   (t_method)inlet_errorUnexpected);
    
    class_addPointer (pointerinlet_class,   (t_method)inlet_forPointer);
    class_addAnything (pointerinlet_class,  (t_method)inlet_errorUnexpected);  
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
