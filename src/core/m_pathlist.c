
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_pathlist *pathlist_removeAtIndex (t_pathlist *x, int n)
{
    if (n == 0) { return pathlist_removeFirst (x); }
    else {
    //
    t_pathlist *l1 = NULL;
    t_pathlist *l2 = NULL;
    t_pathlist *t  = NULL;
    
    int i = 0;
    
    for (l1 = x; l1; l1 = l2, i++) {
        if (i == (n - 1)) {
            PD_ASSERT (l1->pl_next);
            t = l1->pl_next; l1->pl_next = l1->pl_next->pl_next;
            PD_MEMORY_FREE (t->pl_string);
            PD_MEMORY_FREE (t);
            break;
        }
    
        l2 = l1->pl_next;
    }
    
    return x;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist *pathlist_newAppend (t_pathlist *x, const char *s)
{
    if (*s) {
    //
    int k = -1;
    
    t_pathlist *l1 = x;
    t_pathlist *l2 = NULL;
    
    l2 = (t_pathlist *)PD_MEMORY_GET (sizeof (t_pathlist));
    l2->pl_next   = NULL;
    l2->pl_string = (char *)PD_MEMORY_GET (strlen (s) + 1);
    
    strcpy (l2->pl_string, s); path_backslashToSlashIfNecessary (l2->pl_string);
    
    if (!l1) { return l2; }
    else {
    //
    int i = 0;
    
    do { if (!strcmp (l1->pl_string, s)) { k = i; } i++; } while (l1->pl_next && (l1 = l1->pl_next));
        
    l1->pl_next = l2;
    //
    }
    
    /* Avoid duplicate. */
    
    if (k >= 0) { x = pathlist_removeAtIndex (x, k); }
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

t_pathlist *pathlist_removeFirst (t_pathlist *x)
{
    t_pathlist *l = NULL;
    
    if (x) {
        l = x->pl_next;
        PD_MEMORY_FREE (x->pl_string);
        PD_MEMORY_FREE (x);
    }
    
    return l;
}

int pathlist_getSize (t_pathlist *x)
{
    int n = 0;
    
    t_pathlist *l1 = NULL;
    t_pathlist *l2 = NULL;
    
    for (l1 = x; l1; l1 = l2) { l2 = l1->pl_next; n++; }
    
    return n;
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
