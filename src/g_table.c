
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

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *table_makeObject(t_symbol *s, int size, int flags, int xpix, int ypix)
{
    t_atom a[6];
    
    t_glist *gl;
    t_glist *x;
    t_glist *z = canvas_getCurrent();
    
    if (s == &s_) { s = utils_getDefaultTableName(); }
    
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

    return x;
}

static void *table_new (t_symbol *name, t_float f)
{
    return table_makeObject (name, f, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void table_setup (void)
{
    class_addCreator ((t_newmethod)table_new, sym_table, A_DEFSYMBOL, A_DEFFLOAT, A_NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
