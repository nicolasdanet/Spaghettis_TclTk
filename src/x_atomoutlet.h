
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

enum {
    ATOMOUTLET_NONE     = 0,
    ATOMOUTLET_INLET    = 1,
    ATOMOUTLET_OUTLET   = 2,
    ATOMOUTLET_BOTH     = 3
    };

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
t_error atomoutlet_broadcastIfTypeMatch (t_atomoutlet *x, t_atom *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    atomoutlet_makeFloat            (t_atomoutlet *x, t_object *o, int n, t_symbol *type, t_float f);
void    atomoutlet_makeSymbol           (t_atomoutlet *x, t_object *o, int n, t_symbol *type, t_symbol *s);
void    atomoutlet_makePointer          (t_atomoutlet *x, t_object *o, int n, t_symbol *type, t_gpointer *gp);
void    atomoutlet_make                 (t_atomoutlet *x, t_object *o, int n, t_symbol *type, t_atom *a);

void    atomoutlet_makeSymbolParsed     (t_atomoutlet *x, t_object *o, int n, t_atom *a);
void    atomoutlet_makeParsed           (t_atomoutlet *x, t_object *o, int n, t_atom *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    atomoutlet_release              (t_atomoutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_atomoutlet_h_
