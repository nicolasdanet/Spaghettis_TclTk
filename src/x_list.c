
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

extern t_pd     *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *list_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _listinletelement {
    t_atom      le_atom;
    t_gpointer  le_gpointer;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void listinlet_init(t_listinlet *x)
{
    x->li_pd = list_class;
    x->li_size = x->li_numberOfPointers = 0;
    x->li_vector = 0;
}

void listinlet_clear(t_listinlet *x)
{
    int i;
    for (i = 0; i < x->li_size; i++)
    {
        if (x->li_vector[i].le_atom.a_type == A_POINTER)
            gpointer_unset(x->li_vector[i].le_atom.a_w.w_gpointer);
    }
    if (x->li_vector)
        PD_MEMORY_FREE(x->li_vector);
}

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
    x->li_numberOfPointers = 0;
    for (i = 0; i < argc; i++)
    {
        x->li_vector[i].le_atom = argv[i];
        if (x->li_vector[i].le_atom.a_type == A_POINTER)
        {
            x->li_numberOfPointers++;
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
    x->li_numberOfPointers = 0;
    SET_SYMBOL(&x->li_vector[0].le_atom, s);
    for (i = 0; i < argc; i++)
    {
        x->li_vector[i+1].le_atom = argv[i];
        if (x->li_vector[i+1].le_atom.a_type == A_POINTER)
        {
            x->li_numberOfPointers++;            
            gpointer_setByCopy(x->li_vector[i+1].le_atom.a_w.w_gpointer, &x->li_vector[i+1].le_gpointer);
            x->li_vector[i+1].le_atom.a_w.w_gpointer = &x->li_vector[i+1].le_gpointer;
        }
    }
}

void listinlet_copyAtoms(t_listinlet *x, t_atom *to)
{
    int i;
    for (i = 0; i < x->li_size; i++)
        to[i] = x->li_vector[i].le_atom;
}


void listinlet_clone(t_listinlet *x, t_listinlet *y)
{
    int i;
    y->li_pd = list_class;
    y->li_size = x->li_size;
    y->li_numberOfPointers = x->li_numberOfPointers;
    if (!(y->li_vector = (t_listinletelement *)PD_MEMORY_GET(y->li_size * sizeof(*y->li_vector))))
    {
        y->li_size = 0;
        post_error ("list_alloc: out of memory");
    }
    else for (i = 0; i < x->li_size; i++)
    {
        y->li_vector[i].le_atom = x->li_vector[i].le_atom;
        if (y->li_vector[i].le_atom.a_type == A_POINTER)
        {
            gpointer_setByCopy(y->li_vector[i].le_atom.a_w.w_gpointer, &y->li_vector[i].le_gpointer);
            y->li_vector[i].le_atom.a_w.w_gpointer = &y->li_vector[i].le_gpointer;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *list_makeObject (t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = listappend_new (s, argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_append)            { pd_newest = listappend_new (s,        argc - 1, argv + 1); }
    else if (t == sym_prepend)      { pd_newest = listprepend_new (s,       argc - 1, argv + 1); }
    else if (t == sym_split)        { pd_newest = listsplit_new (s,         argc - 1, argv + 1); }
    else if (t == sym_trim)         { pd_newest = listtrim_new (s,          argc - 1, argv + 1); }
    else if (t == sym_length)       { pd_newest = listlength_new (s,        argc - 1, argv + 1); }
    else if (t == sym_fromsymbol)   { pd_newest = listfromsymbol_new (s,    argc - 1, argv + 1); }
    else if (t == sym_tosymbol)     { pd_newest = listtosymbol_new (s,      argc - 1, argv + 1); }
    else {
        error_unexpected (sym_list, t);
    }
    //
    }
    
    return pd_newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void list_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__inlet,
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);
        
    class_addCreator ((t_newmethod)list_makeObject, &s_list, A_GIMME, A_NULL);
            
    class_addList (c, listinlet_list);
    class_addAnything (c, listinlet_anything);
    
    list_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

