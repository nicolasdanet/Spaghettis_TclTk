
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_list.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Called by the t_listmethod of the object maker class. */

static void *list_makeObject (t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    t_pd *newest = NULL;
    
    instance_setNewestObject (NULL);
    
    if (!argc || !IS_SYMBOL (argv)) { newest = listappend_new (s,       argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_append)            { newest = listappend_new (s,       argc - 1, argv + 1); }
    else if (t == sym_prepend)      { newest = listprepend_new (s,      argc - 1, argv + 1); }
    else if (t == sym_split)        { newest = listsplit_new (s,        argc - 1, argv + 1); }
    else if (t == sym_trim)         { newest = listtrim_new (s,         argc - 1, argv + 1); }
    else if (t == sym_length)       { newest = listlength_new (s,       argc - 1, argv + 1); }
    else if (t == sym_fromsymbol)   { newest = listfromsymbol_new (s,   argc - 1, argv + 1); }
    else if (t == sym_tosymbol)     { newest = listtosymbol_new (s,     argc - 1, argv + 1); }
    else {
        error_unexpected (&s_list, t);
    }
    //
    }
    
    instance_setNewestObject (newest);
    
    return newest;  /* Unused but kept due to t_newmethod signature. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void list_setup (void)
{
    class_addCreator ((t_newmethod)list_makeObject, &s_list, A_GIMME, A_NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

