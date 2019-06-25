
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_list.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listinlet_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void listinlet_list (t_listinlet *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listinlet_set (t_listinlet *x, int i, t_atom *a)
{
    PD_ASSERT (i >= 0 && i < x->li_size);
    
    atom_copyAtom (a, &x->li_vector[i].le_atom);
    
    if (!IS_POINTER (a)) { gpointer_unset (&x->li_vector[i].le_gpointer); }
    else {
    //
    gpointer_setByCopy (&x->li_vector[i].le_gpointer, GET_POINTER (a));
    
    SET_POINTER (&x->li_vector[i].le_atom, &x->li_vector[i].le_gpointer);
    //
    }
}

static void listinlet_resize (t_listinlet *x, int n)
{
    if (x->li_size + n >= 0) {
    //
    int i, t = x->li_size;
    int oldSize  = sizeof (t_listinletelement) * (t);
    int newSize  = sizeof (t_listinletelement) * (t + n);
    
    /* Unset gpointers that could be removed. */
    
    if (n < 0) {
    //
    for (i = t + n; i < t; i++) {
    //
    if (IS_POINTER (&x->li_vector[i].le_atom)) { gpointer_unset (GET_POINTER (&x->li_vector[i].le_atom)); }
    //
    }
    //
    }
    
    x->li_vector = (t_listinletelement *)PD_MEMORY_RESIZE (x->li_vector, oldSize, newSize);
    x->li_size   = t + n;
    
    /* Restore properly the address pointed to. */
    
    for (i = 0; i < x->li_size; i++) {
    //
    if (IS_POINTER (&x->li_vector[i].le_atom)) {
    //
    SET_POINTER (&x->li_vector[i].le_atom, &x->li_vector[i].le_gpointer);
    //
    }
    //
    }
    //
    } else { PD_BUG; }
}

static void listinlet_expand (t_listinlet *x, int toAdd)
{
    PD_ASSERT (toAdd >= 0); if (toAdd) { listinlet_resize (x, toAdd); }
}

static void listinlet_reduce (t_listinlet *x, int toRemove)
{
    PD_ASSERT (toRemove >= 0 && toRemove <= x->li_size); if (toRemove) { listinlet_resize (x, -toRemove); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listinlet_listClear (t_listinlet *x)
{
    listinlet_free (x); listinlet_init (x);
}

void listinlet_listSet (t_listinlet *x, int argc, t_atom *argv)
{
    listinlet_listClear (x); listinlet_listAppend (x, argc, argv);
}

void listinlet_listSetByCopy (t_listinlet *x, t_listinlet *toCopy)
{
    t_buffer *b = buffer_new();
    
    listinlet_listGet (toCopy, b);
    listinlet_listSet (x, buffer_getSize (b), buffer_getAtoms (b));
    
    buffer_free (b);
}

void listinlet_listAppend (t_listinlet *x, int argc, t_atom *argv)
{
    listinlet_listInsert (x, x->li_size, argc, argv);
}

void listinlet_listPrepend (t_listinlet *x, int argc, t_atom *argv)
{
    listinlet_listInsert (x, 0, argc, argv);
}

void listinlet_listSetAt (t_listinlet *x, int i, int argc, t_atom *argv)
{
    if (argc && i >= 0 && i < x->li_size) {
    //
    int j, t = PD_MIN (argc, x->li_size - i); for (j = 0; j < t; j++) { listinlet_set (x, i + j, argv + j); }
    //
    }
}

void listinlet_listInsert (t_listinlet *x, int i, int argc, t_atom *argv)
{
    if (argc && i >= 0 && i <= x->li_size) {
    //
    int j, t = x->li_size; listinlet_expand (x, argc);
    
    for (j = t - 1; j >= i; j--) { listinlet_set (x, j + argc, &x->li_vector[j].le_atom); }
    for (j = 0; j < argc; j++)   { listinlet_set (x, i + j, argv + j); }
    //
    }
}

void listinlet_listRemove (t_listinlet *x, int i, int n)
{
    if (i >= 0 && n > 0) {
    //
    int start = i;
    int end = i + n;
    
    if (start < x->li_size && end <= x->li_size) {
    //
    while (end < x->li_size) { listinlet_set (x, start, &x->li_vector[end].le_atom); end++; start++; }
    
    listinlet_reduce (x, n);
    //
    }
    //
    }
}

void listinlet_listReplace (t_listinlet *x, int i, int n, int argc, t_atom *argv)
{
    listinlet_listRemove (x, i, n); listinlet_listInsert (x, i, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int listinlet_listIsEqualTo (t_listinlet *x, int argc, t_atom *argv)
{
    if (argc != x->li_size) { return 0; }
    else {
    //
    int i;
    
    for (i = 0; i < x->li_size; i++) {
        if (!atom_areEquals (&x->li_vector[i].le_atom, argv + i)) {
            return 0;
        }
    }
    //
    }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void listinlet_listGet (t_listinlet *x, t_buffer *b)
{
    int i; for (i = 0; i < x->li_size; i++) { buffer_appendAtom (b, &x->li_vector[i].le_atom); }
}

int listinlet_listHasPointer (t_listinlet *x)
{
    int i; for (i = 0; i < x->li_size; i++) { if (IS_POINTER (&x->li_vector[i].le_atom)) { return 1; } }
    
    return 0;
}

int listinlet_listSize (t_listinlet *x)
{
    return x->li_size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void listinlet_init (t_listinlet *x)
{
    x->li_pd     = listinlet_class;
    x->li_size   = 0;
    x->li_vector = (t_listinletelement *)PD_MEMORY_GET (0);
}

void listinlet_initByClone (t_listinlet *x, t_listinlet *y)
{
    int i;
    
    y->li_pd     = listinlet_class;
    y->li_size   = x->li_size;
    y->li_vector = (t_listinletelement *)PD_MEMORY_GET (x->li_size * sizeof (t_listinletelement));

    for (i = 0; i < x->li_size; i++) { listinlet_set (y, i, &x->li_vector[i].le_atom); }
}

void listinlet_free (t_listinlet *x)
{
    int i;
    
    /* Unset all gpointers. */
    
    for (i = 0; i < x->li_size; i++) {
    //
    if (IS_POINTER (&x->li_vector[i].le_atom)) { gpointer_unset (GET_POINTER (&x->li_vector[i].le_atom)); }
    //
    }
    
    PD_ASSERT (x->li_vector); PD_MEMORY_FREE (x->li_vector);
}

void listinlet_copyAtomsUnchecked (t_listinlet *x, t_atom *a)
{
    int i; for (i = 0; i < x->li_size; i++) { a[i] = x->li_vector[i].le_atom; }
}

t_atom *listinlet_getAtomAtIndex (t_listinlet *x, int i)
{
    PD_ASSERT (i >= 0); if (i < x->li_size) { return &x->li_vector[i].le_atom; }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void listinlet_list (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    listinlet_listSet (x, argc, argv);
}

static void listinlet_anything (t_listinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)listinlet_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    class_free (listinlet_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

