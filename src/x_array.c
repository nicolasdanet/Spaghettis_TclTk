
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *array_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
    
    pd_newest = NULL;
    
    if (t == sym_size)          { pd_newest = arraysize_new (s,     argc - 1, argv + 1); }
    else if (t == sym_sum)      { pd_newest = arraysum_new (s,      argc - 1, argv + 1); }
    else if (t == sym_get)      { pd_newest = arrayget_new (s,      argc - 1, argv + 1); }
    else if (t == sym_set)      { pd_newest = arrayset_new (s,      argc - 1, argv + 1); }
    else if (t == sym_quantile) { pd_newest = arrayquantile_new (s, argc - 1, argv + 1); }
    else if (t == sym_random)   { pd_newest = arrayrandom_new (s,   argc - 1, argv + 1); }
    else if (t == sym_max)      { pd_newest = arraymax_new (s,      argc - 1, argv + 1); }
    else if (t == sym_min)      { pd_newest = arraymin_new (s,      argc - 1, argv + 1); }
    else {
        error_unexpected (sym_array, t);
    }
    
    return pd_newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void array_setup (void)
{
    class_addCreator ((t_newmethod)array_makeObject, sym_array, A_GIMME, A_NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
