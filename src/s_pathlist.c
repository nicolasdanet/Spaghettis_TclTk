
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

static const char *pathlist_getNextFile (char *dest, size_t length, const char *src, char delimiter)
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
    l2->nl_next   = NULL;
    l2->nl_string = (char *)PD_MEMORY_GET (strlen (s) + 1);
    
    strcpy (l2->nl_string, s);
    sys_unbashfilename (l2->nl_string, l2->nl_string);
    
    if (!l1) { return l2; }
    else {
        do {
            if (!strcmp (l1->nl_string, s)) { pathlist_free (l2); return x; }       /* Avoid duplicate. */
        } while (l1->nl_next && (l1 = l1->nl_next));
        
        l1->nl_next = l2;
    }
    
    return x;
}

void pathlist_free (t_pathlist *x)
{
    t_pathlist *l1 = NULL;
    t_pathlist *l2 = NULL;
    
    for (l1 = x; l1; l1 = l2) {
    //
    l2 = l1->nl_next;
    PD_MEMORY_FREE (l1->nl_string);
    PD_MEMORY_FREE (l1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pathlist *pathlist_newAppendFiles (t_pathlist *x, const char *s, char delimiter)
{
    char t[PD_STRING];
    t_pathlist *l = x;
    const char *p = s;
    
    do {
        p = pathlist_getNextFile (t, PD_STRING, p, delimiter);
        if (*t) { 
            l = pathlist_newAppend (l, t); 
        }
    } while (p);
    
    return l;
}

/* It is rather inefficient to traverse the list. */

char *pathlist_getFileAtIndex (t_pathlist *x, int n)
{
    int i;
    t_pathlist *l = x;
    
    for (i = 0; i < n && l; i++, l = l->nl_next) { }
    
    return (l ? l->nl_string : NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
