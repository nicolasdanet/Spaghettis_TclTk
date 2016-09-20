
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

static void listinlet_cachePointerAtIndex (t_listinlet *x, int i, t_gpointer *gp)
{
    gpointer_setByCopy (gp, &x->li_vector[i].le_gpointer);
    
    SET_POINTER (&x->li_vector[i].le_atom, &x->li_vector[i].le_gpointer);
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
    if (IS_POINTER (&x->li_vector[i].le_atom)) { gpointer_unset (&x->li_vector[i].le_gpointer); }
    //
    }
    
    if (x->li_vector) { PD_MEMORY_FREE (x->li_vector); }
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
        
    if (IS_POINTER (&x->li_vector[i].le_atom)) {
        listinlet_cachePointerAtIndex (y, i, &x->li_vector[i].le_gpointer);
    }
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

void listinlet_list(t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    listinlet_clear(x);
    if (!(x->li_vector = (t_listinletelement *)PD_MEMORY_GET(argc * sizeof(*x->li_vector))))
    {
        x->li_size = 0;
        post_error ("list_alloc: out of memory");
        return;
    }
    x->li_size = argc;
    x->li_hasPointer = 0;
    for (i = 0; i < argc; i++)
    {
        x->li_vector[i].le_atom = argv[i];
        if (x->li_vector[i].le_atom.a_type == A_POINTER)
        {
            x->li_hasPointer++;
            gpointer_setByCopy(x->li_vector[i].le_atom.a_w.w_gpointer, &x->li_vector[i].le_gpointer);
            x->li_vector[i].le_atom.a_w.w_gpointer = &x->li_vector[i].le_gpointer;
        }
    }
}

static void listinlet_anything(t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    listinlet_clear(x);
    if (!(x->li_vector = (t_listinletelement *)PD_MEMORY_GET((argc+1) * sizeof(*x->li_vector))))
    {
        x->li_size = 0;
        post_error ("list_alloc: out of memory");
        return;
    }
    x->li_size = argc+1;
    x->li_hasPointer = 0;
    SET_SYMBOL(&x->li_vector[0].le_atom, s);
    for (i = 0; i < argc; i++)
    {
        x->li_vector[i+1].le_atom = argv[i];
        if (x->li_vector[i+1].le_atom.a_type == A_POINTER)
        {
            x->li_hasPointer++;            
            gpointer_setByCopy(x->li_vector[i+1].le_atom.a_w.w_gpointer, &x->li_vector[i+1].le_gpointer);
            x->li_vector[i+1].le_atom.a_w.w_gpointer = &x->li_vector[i+1].le_gpointer;
        }
    }
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

