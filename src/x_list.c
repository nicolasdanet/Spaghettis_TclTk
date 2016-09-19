
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

void list_init(t_list *x)
{
    x->l_pd = list_class;
    x->l_size = x->l_numberOfPointers = 0;
    x->l_vector = 0;
}

void list_clear(t_list *x)
{
    int i;
    for (i = 0; i < x->l_size; i++)
    {
        if (x->l_vector[i].le_atom.a_type == A_POINTER)
            gpointer_unset(x->l_vector[i].le_atom.a_w.w_gpointer);
    }
    if (x->l_vector)
        PD_MEMORY_FREE(x->l_vector);
}

void list_list(t_list *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    list_clear(x);
    if (!(x->l_vector = (t_listelement *)PD_MEMORY_GET(argc * sizeof(*x->l_vector))))
    {
        x->l_size = 0;
        post_error ("list_alloc: out of memory");
        return;
    }
    x->l_size = argc;
    x->l_numberOfPointers = 0;
    for (i = 0; i < argc; i++)
    {
        x->l_vector[i].le_atom = argv[i];
        if (x->l_vector[i].le_atom.a_type == A_POINTER)
        {
            x->l_numberOfPointers++;
            gpointer_setByCopy(x->l_vector[i].le_atom.a_w.w_gpointer, &x->l_vector[i].le_gpointer);
            x->l_vector[i].le_atom.a_w.w_gpointer = &x->l_vector[i].le_gpointer;
        }
    }
}

static void list_anything(t_list *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    list_clear(x);
    if (!(x->l_vector = (t_listelement *)PD_MEMORY_GET((argc+1) * sizeof(*x->l_vector))))
    {
        x->l_size = 0;
        post_error ("list_alloc: out of memory");
        return;
    }
    x->l_size = argc+1;
    x->l_numberOfPointers = 0;
    SET_SYMBOL(&x->l_vector[0].le_atom, s);
    for (i = 0; i < argc; i++)
    {
        x->l_vector[i+1].le_atom = argv[i];
        if (x->l_vector[i+1].le_atom.a_type == A_POINTER)
        {
            x->l_numberOfPointers++;            
            gpointer_setByCopy(x->l_vector[i+1].le_atom.a_w.w_gpointer, &x->l_vector[i+1].le_gpointer);
            x->l_vector[i+1].le_atom.a_w.w_gpointer = &x->l_vector[i+1].le_gpointer;
        }
    }
}

void list_copyAtoms(t_list *x, t_atom *to)
{
    int i;
    for (i = 0; i < x->l_size; i++)
        to[i] = x->l_vector[i].le_atom;
}


void list_clone(t_list *x, t_list *y)
{
    int i;
    y->l_pd = list_class;
    y->l_size = x->l_size;
    y->l_numberOfPointers = x->l_numberOfPointers;
    if (!(y->l_vector = (t_listelement *)PD_MEMORY_GET(y->l_size * sizeof(*y->l_vector))))
    {
        y->l_size = 0;
        post_error ("list_alloc: out of memory");
    }
    else for (i = 0; i < x->l_size; i++)
    {
        y->l_vector[i].le_atom = x->l_vector[i].le_atom;
        if (y->l_vector[i].le_atom.a_type == A_POINTER)
        {
            gpointer_setByCopy(y->l_vector[i].le_atom.a_w.w_gpointer, &y->l_vector[i].le_gpointer);
            y->l_vector[i].le_atom.a_w.w_gpointer = &y->l_vector[i].le_gpointer;
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
            
    class_addList (c, list_list);
    class_addAnything (c, list_anything);
    
    list_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

