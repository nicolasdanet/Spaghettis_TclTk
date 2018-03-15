
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "x_mica.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void *mica_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    t_pd *newest = NULL;
    
    instance_setNewestObject (NULL);
    
    if (!argc || !IS_SYMBOL (argv)) { }
    else {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
    
    if (t == sym_set)           { }
    else if (t == sym_get)      { }
    else if (t == sym_map)      { }
    else if (t == sym_index)    { }
    else if (t == sym_item)     { }
    else if (t == sym_info)     { }
    else if (t == sym_interval) { }
    else if (t == sym_spell)    { }
    else {
        error_unexpected (sym_mica, t);
    }
    //
    }
    
    instance_setNewestObject (newest);
    
    return newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void mica_setup (void)
{
    #if PD_WITH_BELLE
    
    class_addCreator ((t_newmethod)mica_makeObject, sym_mica, A_GIMME, A_NULL);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
