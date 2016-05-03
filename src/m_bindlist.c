
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *bindlist_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bindlist_bang (t_bindlist *x)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_bang (e->e_what); }
}

static void bindlist_float (t_bindlist *x, t_float f)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_float (e->e_what, f); }
}

static void bindlist_symbol (t_bindlist *x, t_symbol *s)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_symbol (e->e_what, s); }
}

static void bindlist_pointer (t_bindlist *x, t_gpointer *gp)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_pointer (e->e_what, gp); }
}

static void bindlist_list (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_list (e->e_what, argc, argv); }
}

static void bindlist_anything (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_message (e->e_what, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void bindlist_initialize (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("bindlist"), NULL, NULL, sizeof (t_bindlist), CLASS_PURE, A_NULL);
    
    class_addBang (c, (t_method)bindlist_bang);
    class_addFloat (c, (t_method)bindlist_float);
    class_addSymbol (c, (t_method)bindlist_symbol);
    class_addPointer (c, (t_method)bindlist_pointer);
    class_addList (c, (t_method)bindlist_list);
    class_addAnything (c, (t_method)bindlist_anything);
    
    bindlist_class = c;
}

void bindlist_release (void)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
