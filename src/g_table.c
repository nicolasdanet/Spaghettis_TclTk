
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;    /* OK - this should go into a .h file now :) */
extern t_class *garray_class;

/* -- "table" - classic "array define" object by Guenter Geiger --*/

static int tabcount = 0;

void *table_donew(t_symbol *s, int size, int flags, int xpix, int ypix)
{
    t_atom a[9];
    t_glist *gl;
    t_glist *x, *z = canvas_getCurrent();
    if (s == &s_)
    {
         char  tabname[255];
         t_symbol *t = sym_table; 
         sprintf(tabname, "%s%d", t->s_name, tabcount++);
         s = gensym (tabname); 
    }
    if (size < 1)
        size = 100;
    SET_FLOAT(a, 0);
    SET_FLOAT(a+1, 50);
    SET_FLOAT(a+2, xpix + 100);
    SET_FLOAT(a+3, ypix + 100);
    SET_SYMBOL(a+4, s);
    SET_FLOAT(a+5, 0);
    x = canvas_new (NULL, NULL, 6, a);

    x->gl_parent = z;

        /* create a graph for the table */
    gl = canvas_newGraph((t_glist*)x, 0, -1, (size > 1 ? size-1 : 1), 1,
        50, ypix+50, xpix+50, 50);

    garray_makeObject(gl, s, &s_float, size, flags);

    pd_newest = &x->gl_obj.te_g.g_pd;     /* mimic action of canvas_pop() */
    stack_pop(&x->gl_obj.te_g.g_pd);
    x->gl_isLoading = 0;

    return (x);
}

static void *table_new(t_symbol *s, t_float f)
{
    return (table_donew(s, f, 0, 500, 300));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void table_setup(void)
{
    class_addCreator((t_newmethod)table_new, sym_table,
        A_DEFSYMBOL, A_DEFFLOAT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
