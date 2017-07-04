
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_atomoutlet_h_
#define __x_atomoutlet_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _atomoutlet {
    t_atom      ao_atom;
    t_gpointer  ao_gpointer;
    t_outlet    *ao_outlet;
    } t_atomoutlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_atom      *atomoutlet_getAtom         (t_atomoutlet *x);
t_outlet    *atomoutlet_getOutlet       (t_atomoutlet *x);
t_gpointer  *atomoutlet_getPointer      (t_atomoutlet *x);

t_atomtype  atomoutlet_getType          (t_atomoutlet *x);
int         atomoutlet_isPointer        (t_atomoutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    atomoutlet_copyAtom             (t_atomoutlet *x, t_atom *a);
t_error atomoutlet_setAtom              (t_atomoutlet *x, t_atom *a);
int     atomoutlet_isEqualToAtom        (t_atomoutlet *x, t_atom *a);
t_error atomoutlet_outputIfTypeMatch    (t_atomoutlet *x, t_atom *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    atomoutlet_makeFloat            (t_atomoutlet *x, t_object *o, t_float f, int inlet, int outlet);
void    atomoutlet_makeSymbol           (t_atomoutlet *x, t_object *o, int inlet, int outlet);
void    atomoutlet_makePointer          (t_atomoutlet *x, t_object *o, int inlet, int outlet);
t_error atomoutlet_makeParse            (t_atomoutlet *x, t_object *o, t_atom *a, int inlet, int outlet);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    atomoutlet_makeTypedOutlet      (t_atomoutlet *x, t_object *o, t_symbol *type, t_atom *a, int inlet);
t_error atomoutlet_makeTypedOutletParse (t_atomoutlet *x, t_object *o, t_atom *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    atomoutlet_release              (t_atomoutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_atomoutlet_h_
