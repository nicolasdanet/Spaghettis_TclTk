
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_alloca.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listinlet_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _listinletelement {
    t_atom      le_atom;
    t_gpointer  le_gpointer;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void listinlet_list (t_listinlet *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void listinlet_cachePointer (t_listinlet *x, int i, t_atom *a)
{
    if (IS_POINTER (a)) {
        gpointer_setByCopy (&x->li_vector[i].le_gpointer, GET_POINTER (a));
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

void listinlet_setList (t_listinlet *x, int argc, t_atom *argv)
{
    listinlet_list (x, NULL, argc, argv);
}

int listinlet_hasPointer (t_listinlet *x)
{
    return x->li_hasPointer;
}

int listinlet_getSize (t_listinlet *x)
{
    return x->li_size;
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

void listinlet_copyListUnchecked (t_listinlet *x, t_atom *a)
{
    int i;
    for (i = 0; i < x->li_size; i++) { a[i] = x->li_vector[i].le_atom; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void listinlet_list (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
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
    utils_anythingToList (cast_pd (x), (t_listmethod)listinlet_list, s, argc, argv);
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

    class_addList (c, (t_method)listinlet_list);
    class_addAnything (c, (t_method)listinlet_anything);
    
    listinlet_class = c;
}

void listinlet_destroy (void)
{
    CLASS_FREE (listinlet_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

