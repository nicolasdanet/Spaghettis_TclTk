
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_atom *atomoutlet_getAtom (t_atomoutlet *x)
{
    return &x->ao_atom;
}

t_outlet *atomoutlet_getOutlet (t_atomoutlet *x)
{
    return x->ao_outlet;
}

t_gpointer *atomoutlet_getPointer (t_atomoutlet *x)
{
    return &x->ao_gpointer;
}

t_atomtype atomoutlet_getType (t_atomoutlet *x)
{
    return atom_getType (&x->ao_atom);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int atomoutlet_isPointer (t_atomoutlet *x)
{
    return (atomoutlet_getType (x) == A_POINTER);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atomoutlet_copyAtom (t_atomoutlet *x, t_atom *a)
{
    *a = x->ao_atom;
}

t_error atomoutlet_setAtom (t_atomoutlet *x, t_atom *a)
{
    if (!atom_typesAreEqual (&x->ao_atom, a)) { return PD_ERROR; }
    else {
    //
    switch (atom_getType (&x->ao_atom)) {
        case A_FLOAT   : SET_FLOAT (&x->ao_atom,  GET_FLOAT (a));               break;
        case A_SYMBOL  : SET_SYMBOL (&x->ao_atom, GET_SYMBOL (a));              break;
        case A_POINTER : gpointer_setByCopy (GET_POINTER (a), &x->ao_gpointer); break;
    }
    //
    }
    
    return PD_ERROR_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atomoutlet_init (t_atomoutlet *x)
{
    SET_FLOAT (&x->ao_atom, 0.0);
    gpointer_init (&x->ao_gpointer); 
    x->ao_outlet = NULL;
}

void atomoutlet_release (t_atomoutlet *x)
{
    gpointer_unset (&x->ao_gpointer); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atomoutlet_makeFloat (t_atomoutlet *x, t_object *owner, t_float f, int createInlet)
{
    atomoutlet_init (x);
    SET_FLOAT (&x->ao_atom, f);
    x->ao_outlet = outlet_new (owner, &s_float);
    if (createInlet) { inlet_newFloat (owner, ADDRESS_FLOAT (&x->ao_atom)); }
}

void atomoutlet_makeSymbol (t_atomoutlet *x, t_object *owner, int createInlet)
{
    atomoutlet_init (x);
    SET_SYMBOL (&x->ao_atom, &s_symbol);
    x->ao_outlet = outlet_new (owner, &s_symbol);
    if (createInlet) { inlet_newSymbol (owner, ADDRESS_SYMBOL (&x->ao_atom)); }
}

void atomoutlet_makePointer (t_atomoutlet *x, t_object *owner, int createInlet)
{
    atomoutlet_init (x);
    SET_POINTER (&x->ao_atom, &x->ao_gpointer);
    x->ao_outlet = outlet_new (owner, &s_pointer);
    if (createInlet) { inlet_newPointer (owner, &x->ao_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
