
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
void atoms_copy(int argc, t_atom *from, t_atom *to)
{
    int i;
    for (i = 0; i < argc; i++)
        to[i] = from[i];
}

/* ------------- fake class to divert inlets to ----------------- */

t_class *alist_class;

void alist_init(t_alist *x)
{
    x->l_pd = alist_class;
    x->l_n = x->l_npointer = 0;
    x->l_vec = 0;
}

void alist_clear(t_alist *x)
{
    int i;
    for (i = 0; i < x->l_n; i++)
    {
        if (x->l_vec[i].l_a.a_type == A_POINTER)
            gpointer_unset(x->l_vec[i].l_a.a_w.w_gpointer);
    }
    if (x->l_vec)
        PD_MEMORY_FREE(x->l_vec);
}

void alist_list(t_alist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    alist_clear(x);
    if (!(x->l_vec = (t_listelem *)PD_MEMORY_GET(argc * sizeof(*x->l_vec))))
    {
        x->l_n = 0;
        post_error ("list_alloc: out of memory");
        return;
    }
    x->l_n = argc;
    x->l_npointer = 0;
    for (i = 0; i < argc; i++)
    {
        x->l_vec[i].l_a = argv[i];
        if (x->l_vec[i].l_a.a_type == A_POINTER)
        {
            x->l_npointer++;
            gpointer_setByCopy(x->l_vec[i].l_a.a_w.w_gpointer, &x->l_vec[i].l_p);
            x->l_vec[i].l_a.a_w.w_gpointer = &x->l_vec[i].l_p;
        }
    }
}

static void alist_anything(t_alist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    alist_clear(x);
    if (!(x->l_vec = (t_listelem *)PD_MEMORY_GET((argc+1) * sizeof(*x->l_vec))))
    {
        x->l_n = 0;
        post_error ("list_alloc: out of memory");
        return;
    }
    x->l_n = argc+1;
    x->l_npointer = 0;
    SET_SYMBOL(&x->l_vec[0].l_a, s);
    for (i = 0; i < argc; i++)
    {
        x->l_vec[i+1].l_a = argv[i];
        if (x->l_vec[i+1].l_a.a_type == A_POINTER)
        {
            x->l_npointer++;            
            gpointer_setByCopy(x->l_vec[i+1].l_a.a_w.w_gpointer, &x->l_vec[i+1].l_p);
            x->l_vec[i+1].l_a.a_w.w_gpointer = &x->l_vec[i+1].l_p;
        }
    }
}

void alist_toatoms(t_alist *x, t_atom *to)
{
    int i;
    for (i = 0; i < x->l_n; i++)
        to[i] = x->l_vec[i].l_a;
}


void alist_clone(t_alist *x, t_alist *y)
{
    int i;
    y->l_pd = alist_class;
    y->l_n = x->l_n;
    y->l_npointer = x->l_npointer;
    if (!(y->l_vec = (t_listelem *)PD_MEMORY_GET(y->l_n * sizeof(*y->l_vec))))
    {
        y->l_n = 0;
        post_error ("list_alloc: out of memory");
    }
    else for (i = 0; i < x->l_n; i++)
    {
        y->l_vec[i].l_a = x->l_vec[i].l_a;
        if (y->l_vec[i].l_a.a_type == A_POINTER)
        {
            gpointer_setByCopy(y->l_vec[i].l_a.a_w.w_gpointer, &y->l_vec[i].l_p);
            y->l_vec[i].l_a.a_w.w_gpointer = &y->l_vec[i].l_p;
        }
    }
}

static void alist_setup(void)
{
    alist_class = class_new(sym_list__space__inlet,
        0, 0, sizeof(t_alist), 0, 0);
    class_addList(alist_class, alist_list);
    class_addAnything(alist_class, alist_anything);
}

/* ------------- list append --------------------- */

t_class *list_append_class;

typedef struct _list_append
{
    t_object x_obj;
    t_alist x_alist;
} t_list_append;

static void *list_append_new(t_symbol *s, int argc, t_atom *argv)
{
    t_list_append *x = (t_list_append *)pd_new(list_append_class);
    alist_init(&x->x_alist);
    alist_list(&x->x_alist, 0, argc, argv);
    outlet_new(&x->x_obj, &s_list);
    inlet_new(&x->x_obj, &x->x_alist.l_pd, 0, 0);
    return (x);
}

static void list_append_list(t_list_append *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_alist.l_n + argc;
    ATOMS_ALLOCA(outv, outc);
    atoms_copy(argc, argv, outv);
    if (x->x_alist.l_npointer)
    {
        t_alist y;
        alist_clone(&x->x_alist, &y);
        alist_toatoms(&y, outv+argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
        alist_clear(&y);
    }
    else
    {
        alist_toatoms(&x->x_alist, outv+argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}

static void list_append_anything(t_list_append *x, t_symbol *s,
    int argc, t_atom *argv)
{
    t_atom *outv;
    int n, outc = x->x_alist.l_n + argc + 1;
    ATOMS_ALLOCA(outv, outc);
    SET_SYMBOL(outv, s);
    atoms_copy(argc, argv, outv + 1);
    if (x->x_alist.l_npointer)
    {
        t_alist y;
        alist_clone(&x->x_alist, &y);
        alist_toatoms(&y, outv + 1 + argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
        alist_clear(&y);
    }
    else
    {
        alist_toatoms(&x->x_alist, outv + 1 + argc);
        outlet_list(x->x_obj.te_outlet, &s_list, outc, outv);
    }
    ATOMS_FREEA(outv, outc);
}

static void list_append_free(t_list_append *x)
{
    alist_clear(&x->x_alist);
}

static void list_append_setup(void)
{
    list_append_class = class_new(sym_list__space__append,
        (t_newmethod)list_append_new, (t_method)list_append_free,
        sizeof(t_list_append), 0, A_GIMME, 0);
    class_addList(list_append_class, list_append_list);
    class_addAnything(list_append_class, list_append_anything);
    class_setHelpName(list_append_class, &s_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *list_new(t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = list_append_new(s, argc, argv);
    else
    {
        t_symbol *s2 = argv[0].a_w.w_symbol;
        if (s2 == sym_append)
            pd_newest = list_append_new(s, argc-1, argv+1);
        else if (s2 == sym_prepend)
            pd_newest = list_prepend_new(s, argc-1, argv+1);
        else if (s2 == sym_split)
            pd_newest = list_split_new(atom_getFloatAtIndex(1, argc, argv));
        else if (s2 == sym_trim)
            pd_newest = list_trim_new();
        else if (s2 == sym_length)
            pd_newest = list_length_new();
        else if (s2 == sym_fromsymbol)
            pd_newest = list_fromsymbol_new();
        else if (s2 == sym_tosymbol)
            pd_newest = list_tosymbol_new();
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
    list_append_setup();
    class_addCreator((t_newmethod)list_new, &s_list, A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

