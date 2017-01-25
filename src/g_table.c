
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_class  *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *table_makeObject (t_symbol *name, t_float down, t_float up, t_float size, t_float flags)
{
    t_glist *x = NULL;
    t_glist *y = NULL;
    t_glist *z = canvas_getCurrent();
    t_atom a[6];
    
    if (name == &s_) { name = utils_getDefaultBindName (garray_class, sym_table); }
    if (size < 1.0)  { size = (t_float)GRAPH_DEFAULT_END; }
    
    SET_FLOAT  (a + 0, (t_float)0.0);
    SET_FLOAT  (a + 1, WINDOW_HEADER);
    SET_FLOAT  (a + 2, WINDOW_WIDTH);
    SET_FLOAT  (a + 3, WINDOW_HEIGHT);
    SET_SYMBOL (a + 4, sym_Array);
    SET_FLOAT  (a + 5, (t_float)0.0);
    
    x = canvas_new (NULL, NULL, 6, a);
    
    PD_ASSERT (x);
    
    x->gl_parent = z;

    y = canvas_newGraphOnParent (x,
            (t_float)GRAPH_DEFAULT_START,
            (t_float)up,
            (t_float)size,
            (t_float)down,
            (t_float)GRAPH_DEFAULT_X,
            (t_float)GRAPH_DEFAULT_Y,
            (t_float)GRAPH_DEFAULT_X + GRAPH_DEFAULT_WIDTH,
            (t_float)GRAPH_DEFAULT_Y + GRAPH_DEFAULT_HEIGHT);

    garray_makeObject (y, name, size, flags);

    pd_newest = cast_pd (x);
    
    canvas_pop (x, (t_float)0.0);

    return x;
}

static void *table_new (t_symbol *name, t_float size)
{
    return table_makeObject (name,
                (t_float)GRAPH_DEFAULT_DOWN,
                (t_float)GRAPH_DEFAULT_UP,
                size,
                (t_float)0.0);
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
