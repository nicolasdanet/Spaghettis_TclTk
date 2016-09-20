
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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *listinlet_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _listinletelement {
    t_atom      le_atom;
    t_gpointer  le_gpointer;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void listinlet_cachePointer (t_listinlet *x, int i, t_atom *a)
{
    if (IS_POINTER (a)) {
        gpointer_setByCopy (GET_POINTER (a), &x->li_vector[i].le_gpointer);
        SET_POINTER (&x->li_vector[i].le_atom, &x->li_vector[i].le_gpointer);
        x->li_hasPointer = 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listinlet_init (t_listinlet *x)
{
    x->li_pd          = listinlet_class;
    x->li_size        = 0;
    x->li_hasPointer  = 0;
    x->li_vector      = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listinlet_clear (t_listinlet *x)
{
    int i;
    
    for (i = 0; i < x->li_size; i++) {
    //
    if (IS_POINTER (&x->li_vector[i].le_atom)) { gpointer_unset (GET_POINTER (&x->li_vector[i].le_atom)); }
    //
    }
    
    if (x->li_vector) { PD_MEMORY_FREE (x->li_vector); }
    
    listinlet_init (x);
}

void listinlet_clone (t_listinlet *x, t_listinlet *y)
{
    int i;
    
    y->li_pd         = x->li_pd;
    y->li_size       = x->li_size;
    y->li_hasPointer = x->li_hasPointer;
    y->li_vector     = (t_listinletelement *)PD_MEMORY_GET (x->li_size * sizeof (t_listinletelement));

    for (i = 0; i < x->li_size; i++) {
    //
    y->li_vector[i].le_atom = x->li_vector[i].le_atom;
    listinlet_cachePointer (y, i, &x->li_vector[i].le_atom);
    //
    }
}

void listinlet_copy (t_listinlet *x, t_atom *a)
{
    int i;
    for (i = 0; i < x->li_size; i++) { a[i] = x->li_vector[i].le_atom; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listinlet_list (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    
    listinlet_clear (x);
    
    x->li_size   = argc;
    x->li_vector = (t_listinletelement *)PD_MEMORY_GET (x->li_size * sizeof (t_listinletelement));
    
    for (i = 0; i < x->li_size; i++) {
    //
    x->li_vector[i].le_atom = argv[i];
    listinlet_cachePointer (x, i, argv + i);
    //
    }
}

static void listinlet_anything (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    
    ATOMS_ALLOCA (t, argc + 1);
    
    atom_copyAtomsUnchecked (argc, argv, t + 1);
    SET_SYMBOL (t, s);
    listinlet_list (x, &s_list, argc + 1, t);
    
    ATOMS_FREEA (t, argc + 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listinlet_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_listinlet,
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);

    class_addList (c, listinlet_list);
    class_addAnything (c, listinlet_anything);
    
    listinlet_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

