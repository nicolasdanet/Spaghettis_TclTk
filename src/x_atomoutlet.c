
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
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

int atomoutlet_isEqualTo (t_atomoutlet *x, t_atom *a)
{
    if (atom_typesAreEqual (&x->ao_atom, a)) {
    //
    if (IS_FLOAT (&x->ao_atom) && (GET_FLOAT (&x->ao_atom) == GET_FLOAT (a)))    { return 1; }
    if (IS_SYMBOL (&x->ao_atom) && (GET_SYMBOL (&x->ao_atom) == GET_SYMBOL (a))) { return 1; }
    
    PD_ASSERT (IS_FLOAT (&x->ao_atom) || IS_SYMBOL (&x->ao_atom));      /* Others not implemented yet. */
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void atomoutlet_copyAtom (t_atomoutlet *x, t_atom *a)
{
    *a = *atomoutlet_getAtom (x);
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
        default         : PD_BUG; return PD_ERROR;
    }
    //
    }
    
    return PD_ERROR_NONE;
}

t_error atomoutlet_outputAtom (t_atomoutlet *x, t_atom *a)
{
    if (!atom_typesAreEqual (&x->ao_atom, a)) { return PD_ERROR; }
    else {
    //
    switch (atomoutlet_getType (&x->ao_atom)) {
        case A_FLOAT    : outlet_float (x->ao_outlet, GET_FLOAT (a));     break;
        case A_SYMBOL   : outlet_symbol (x->ao_outlet, GET_SYMBOL (a));   break;
        case A_POINTER  : outlet_pointer (x->ao_outlet, GET_POINTER (a)); break;
        default         : PD_BUG; return PD_ERROR;
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

t_error atomoutlet_makeParse (t_atomoutlet *x, t_object *owner, t_atom *a, int createInlet, int createOutlet)
{
    t_error err = PD_ERROR_NONE;
    t_symbol *t = atom_getSymbol (a);
        
    if (t == sym_p || t == &s_pointer) { atomoutlet_makePointer (x, owner, createInlet, createOutlet); }
    else if (t == sym_s || t == &s_symbol) { atomoutlet_makeSymbol (x, owner, createInlet, createOutlet); }
    else {
        atomoutlet_makeFloat (x, owner, atom_getFloat (a), createInlet, createOutlet);
        if (!IS_FLOAT (a) && t != sym_f && t != &s_float) {
            err = PD_ERROR;
        }
    }
    
    return err;
}

void atomoutlet_makeFloat (t_atomoutlet *x, t_object *owner, t_float f, int createInlet, int createOutlet)
{
    atomoutlet_init (x);
    SET_FLOAT (&x->ao_atom, f);
    if (createOutlet) { x->ao_outlet = outlet_new (owner, &s_float); }
    if (createInlet)  { inlet_newFloat (owner, ADDRESS_FLOAT (&x->ao_atom)); }
}

void atomoutlet_makeSymbol (t_atomoutlet *x, t_object *owner, int createInlet, int createOutlet)
{
    atomoutlet_init (x);
    SET_SYMBOL (&x->ao_atom, &s_symbol);
    if (createOutlet) { x->ao_outlet = outlet_new (owner, &s_symbol); }
    if (createInlet)  { inlet_newSymbol (owner, ADDRESS_SYMBOL (&x->ao_atom)); }
}

void atomoutlet_makePointer (t_atomoutlet *x, t_object *owner, int createInlet, int createOutlet)
{
    atomoutlet_init (x);
    SET_POINTER (&x->ao_atom, &x->ao_gpointer);
    if (createOutlet) { x->ao_outlet = outlet_new (owner, &s_pointer); }
    if (createInlet)  { inlet_newPointer (owner, &x->ao_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error atomoutlet_makeTypedOutletParse (t_atomoutlet *x, t_object *owner, t_atom *a)
{
    t_symbol *t = atom_getSymbol (a);
    
    if (t == sym_b || t == &s_bang) {
        x->ao_outlet = outlet_new (owner, &s_bang); 
        SET_SYMBOL (&x->ao_atom, &s_bang); 
        
    } else if (t == sym_p || t == &s_pointer) {
        x->ao_outlet = outlet_new (owner, &s_pointer);
        SET_SYMBOL (&x->ao_atom, &s_pointer);
        
    } else if (t == sym_l || t == &s_list) {
        x->ao_outlet = outlet_new (owner, &s_list);
        SET_SYMBOL (&x->ao_atom, &s_list); 
 
    } else if (t == sym_s || t == &s_symbol) {
        x->ao_outlet = outlet_new (owner, &s_symbol);
        SET_SYMBOL (&x->ao_atom, &s_symbol); 
    
    } else if (t == sym_a || t == &s_anything) {
        x->ao_outlet = outlet_new (owner, &s_anything);
        SET_SYMBOL (&x->ao_atom, &s_anything);
        
    } else {
        x->ao_outlet = outlet_new (owner, &s_float);
        SET_FLOAT (&x->ao_atom, 0.0);
        
        if (t != sym_f && t != &s_float) { 
            return PD_ERROR;
        }
    }
    
    return PD_ERROR_NONE;
}

void atomoutlet_makeTypedOutlet (t_atomoutlet *x, t_object *owner, t_symbol *type, t_atom *a, int createInlet)
{
    atomoutlet_init (x);
    
    if (IS_SYMBOL (a)) { SET_SYMBOL (&x->ao_atom, atom_getSymbol (a)); }
    else {
        SET_FLOAT (&x->ao_atom, atom_getFloat (a));
    }
    
    if (createInlet) {
    //
    if (IS_SYMBOL (a)) { inlet_newSymbol (owner, ADDRESS_SYMBOL (&x->ao_atom)); }
    else {
        inlet_newFloat (owner, ADDRESS_FLOAT (&x->ao_atom));        
    }
    //
    }
    
    x->ao_outlet = outlet_new (owner, type);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
