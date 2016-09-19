
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

extern t_pd *pd_newest;

/* the "list" object family.

    list append - append a list to another
    list prepend - prepend a list to another
    list split - first n elements to first outlet, rest to second outlet 
    list trim - trim off "list" selector
    list length - output number of items in list
    list fromsymbol - "explode" a symbol into a list of character codes

Need to think more about:
    list foreach - spit out elements of a list one by one (also in reverse?)
    list array - get items from a named array as a list
    list reverse - permute elements of a list back to front
    list pack - synonym for 'pack'
    list unpack - synonym for 'unpack'
    list cat - build a list by accumulating elements

Probably don't need:
    list first - output first n elements.
    list last - output last n elements
    list nth - nth item in list, counting from zero
*/

/* -------------- utility functions: storage, copying  -------------- */
    /* List element for storage.  Keep an atom and, in case it's a pointer,
        an associated 'gpointer' to protect against stale pointers. */


/* ------------- fake class to divert inlets to ----------------- */

t_class *alist_class;

void alist_init(t_list *x)
{
    x->l_pd = alist_class;
    x->l_size = x->l_numberOfPointers = 0;
    x->l_vector = 0;
}

void alist_clear(t_list *x)
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

void alist_list(t_list *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    alist_clear(x);
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

static void alist_anything(t_list *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    alist_clear(x);
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

void alist_toatoms(t_list *x, t_atom *to)
{
    int i;
    for (i = 0; i < x->l_size; i++)
        to[i] = x->l_vector[i].le_atom;
}


void alist_clone(t_list *x, t_list *y)
{
    int i;
    y->l_pd = alist_class;
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

static void alist_setup(void)
{
    alist_class = class_new(sym_list__space__inlet,
        0, 0, sizeof(t_list), 0, 0);
    class_addList(alist_class, alist_list);
    class_addAnything(alist_class, alist_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *list_new(t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = listappend_new(s, argc, argv);
    else
    {
        t_symbol *s2 = argv[0].a_w.w_symbol;
        if (s2 == sym_append)
            pd_newest = listappend_new(s, argc-1, argv+1);
        else if (s2 == sym_prepend)
            pd_newest = listprepend_new(s, argc-1, argv+1);
        else if (s2 == sym_split)
            pd_newest = listsplit_new(s, argc-1, argv+1);
        else if (s2 == sym_trim)
            pd_newest = listtrim_new(s, argc-1, argv+1);
        else if (s2 == sym_length)
            pd_newest = listlength_new(s, argc-1, argv+1);
        else if (s2 == sym_fromsymbol)
            pd_newest = listfromsymbol_new(s, argc-1, argv+1);
        else if (s2 == sym_tosymbol)
            pd_newest = listtosymbol_new(s, argc-1, argv+1);
        else 
        {
            post_error ("list %s: unknown function", s2->s_name);
            pd_newest = 0;
        }
    }
    return (pd_newest);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void x_list_setup(void)
{
    alist_setup();
    class_addCreator((t_newmethod)list_new, &s_list, A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

