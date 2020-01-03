
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    if (x->iter_with && i < x->iter_argc) { return n + 1; }
    else {
        return n;
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void iterator_skip (t_iterator *x, int n)
{
    x->iter_index += n;
    
    PD_ASSERT (x->iter_index <= x->iter_argc);
}

int iterator_get (t_iterator *x)
{
    return x->iter_index;
}

void iterator_set (t_iterator *x, int argc, t_atom *argv)
{
    x->iter_argc  = argc;
    x->iter_index = 0;
    x->iter_argv  = argv;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_iterator *iterator_new (int argc, t_atom *argv, int includeEndInCount)
{
    t_iterator *x = (t_iterator *)PD_MEMORY_GET (sizeof (t_iterator));
    
    x->iter_with  = (includeEndInCount != 0) ? 1 : 0;
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
