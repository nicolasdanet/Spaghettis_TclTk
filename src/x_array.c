
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *array_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *t  = atom_getSymbolAtIndex (0, argc, argv);
    t_pd *newest = NULL;
    
    instance_setNewestObject (NULL);
    
    if (t == sym_size)          { newest = arraysize_new (s,        argc - 1, argv + 1); }
    else if (t == sym_sum)      { newest = arraysum_new (s,         argc - 1, argv + 1); }
    else if (t == sym_get)      { newest = arrayget_new (s,         argc - 1, argv + 1); }
    else if (t == sym_set)      { newest = arrayset_new (s,         argc - 1, argv + 1); }
    else if (t == sym_quantile) { newest = arrayquantile_new (s,    argc - 1, argv + 1); }
    else if (t == sym_random)   { newest = arrayrandom_new (s,      argc - 1, argv + 1); }
    else if (t == sym_max)      { newest = arraymax_new (s,         argc - 1, argv + 1); }
    else if (t == sym_min)      { newest = arraymin_new (s,         argc - 1, argv + 1); }
    else {
        error_unexpected (sym_array, t);
    }
    
    instance_setNewestObject (newest);
    
    return newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void array_setup (void)
{
    class_addCreator ((t_newmethod)array_makeObject, sym_array, A_GIMME, A_NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
