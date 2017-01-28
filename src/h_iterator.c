
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _iterator {
    int     iter_argc;
    int     iter_index;
    t_atom  *iter_argv;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int iterator_next (t_iterator *x, t_atom **a)
{
    if (x->iter_argc > 0) {
    //
    if (x->iter_index < x->iter_argc) {
    //
    int i, n, next;
    for (i = x->iter_index; i < x->iter_argc && !IS_SEMICOLON_OR_COMMA (x->iter_argv + i); i++) { }
    n = i - x->iter_index;
    next = PD_MIN (i + 1, x->iter_argc);
    (*a) = x->iter_argv + x->iter_index; 
    x->iter_index = next;
    return n;
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_iterator *iterator_new (int argc, t_atom *argv)
{
    t_iterator *x = (t_iterator *)PD_MEMORY_GET (sizeof (t_iterator));
    
    x->iter_argc  = argc;
    x->iter_index = 0;
    x->iter_argv  = argv;
    
    return x;
}

void iterator_free (t_iterator *x)
{
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
