
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist *pathlist_newAppend (t_pathlist *x, const char *s)
{
    if (*s) {
    //
    t_pathlist *l1 = x;
    t_pathlist *l2 = NULL;
    
    l2 = (t_pathlist *)PD_MEMORY_GET (sizeof (t_pathlist));
    l2->pl_next   = NULL;
    l2->pl_string = (char *)PD_MEMORY_GET (strlen (s) + 1);
    
    strcpy (l2->pl_string, s); path_backslashToSlashIfNecessary (l2->pl_string);
    
    if (!l1) { return l2; }
    else {
        do {
            if (!strcmp (l1->pl_string, s)) { pathlist_free (l2); return x; }       /* Avoid duplicate. */
        } while (l1->pl_next && (l1 = l1->pl_next));
        
        l1->pl_next = l2;
    }
    //
    }
    
    return x;
}

void pathlist_free (t_pathlist *x)
{
    t_pathlist *l1 = NULL;
    t_pathlist *l2 = NULL;
    
    for (l1 = x; l1; l1 = l2) {
    //
    l2 = l1->pl_next;
    PD_MEMORY_FREE (l1->pl_string);
    PD_MEMORY_FREE (l1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char *pathlist_getPath (t_pathlist *x)
{
    return (x ? x->pl_string : NULL);
}

t_pathlist *pathlist_getNext (t_pathlist *x)
{
    return (x ? x->pl_next : NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
