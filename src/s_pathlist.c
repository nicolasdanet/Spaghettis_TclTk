
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static const char *pathlist_getNextFileDelimited (char *dest, size_t length, const char *src, char delimiter)
{
    size_t i;

    for (i = 0; i < (length - 1) && src[i] && src[i] != delimiter; i++) { dest[i] = src[i]; }
    
    dest[i] = 0;

    if (i && src[i] != 0) { return (src + i + 1); }
    else {
        return NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pathlist *pathlist_newAppend (t_pathlist *x, const char *s)
{
    t_pathlist *l1 = x;
    t_pathlist *l2 = NULL;
    
    l2 = (t_pathlist *)(PD_MEMORY_GET (sizeof (t_pathlist)));
    l2->pl_next   = NULL;
    l2->pl_string = (char *)PD_MEMORY_GET (strlen (s) + 1);
    
    strcpy (l2->pl_string, s);
    path_backslashToSlashIfNecessary (l2->pl_string, l2->pl_string);
    
    if (!l1) { return l2; }
    else {
        do {
            if (!strcmp (l1->pl_string, s)) { pathlist_free (l2); return x; }       /* Avoid duplicate. */
        } while (l1->pl_next && (l1 = l1->pl_next));
        
        l1->pl_next = l2;
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
#pragma mark -

char *pathlist_getFileAtIndex (t_pathlist *x, int n)
{
    int i;
    t_pathlist *l = x;
    
    for (i = 0; i < n && l; i++, l = l->pl_next) { }
    
    return (l ? l->pl_string : NULL);
}

char *pathlist_getFile (t_pathlist *x)
{
    return (x ? x->pl_string : NULL);
}

t_pathlist *pathlist_getNext (t_pathlist *x)
{
    return (x ? x->pl_next : NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pathlist *pathlist_newAppendFiles (t_pathlist *x, t_symbol *s, char delimiter)
{
    t_pathlist *l = x;
    
    if (s && *s->s_name) {
    //
    char t[PD_STRING];
    const char *p = s->s_name;
    
    do {
        p = pathlist_getNextFileDelimited (t, PD_STRING, p, delimiter);
        if (*t) { 
            l = pathlist_newAppend (l, t); 
        }
    } while (p);
    //
    }
    
    return l;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
