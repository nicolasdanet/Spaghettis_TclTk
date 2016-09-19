
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd *pd_newest;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *list_makeObject (t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = listappend_new (s, argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_append)            { pd_newest = listappend_new (s,        argc - 1, argv + 1); }
    else if (t == sym_prepend)      { pd_newest = listprepend_new (s,       argc - 1, argv + 1); }
    else if (t == sym_split)        { pd_newest = listsplit_new (s,         argc - 1, argv + 1); }
    else if (t == sym_trim)         { pd_newest = listtrim_new (s,          argc - 1, argv + 1); }
    else if (t == sym_length)       { pd_newest = listlength_new (s,        argc - 1, argv + 1); }
    else if (t == sym_fromsymbol)   { pd_newest = listfromsymbol_new (s,    argc - 1, argv + 1); }
    else if (t == sym_tosymbol)     { pd_newest = listtosymbol_new (s,      argc - 1, argv + 1); }
    else {
        error_unexpected (sym_list, t);
    }
    //
    }
    
    return pd_newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void list_setup (void)
{
    class_addCreator ((t_newmethod)list_makeObject, &s_list, A_GIMME, A_NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

