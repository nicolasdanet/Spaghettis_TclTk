
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
    t_pathlist *nl1 = x;
    t_pathlist *nl2 = NULL;
    
    nl2 = (t_pathlist *)(PD_MEMORY_GET (sizeof (t_pathlist)));
    nl2->nl_next   = NULL;
    nl2->nl_string = (char *)PD_MEMORY_GET (strlen (s) + 1);
    
    strcpy (nl2->nl_string, s);
    sys_unbashfilename (nl2->nl_string, nl2->nl_string);
    
    if (!nl1) { return nl2; }
    else {
        do {
            if (!strcmp (nl1->nl_string, s)) { namelist_free (nl2); return x; }     /* Avoid duplicate. */
        } while (nl1->nl_next && (nl1 = nl1->nl_next));
        
        nl1->nl_next = nl2;
    }
    
    return x;
}

/* add a colon-separated list of names to a namelist */

t_pathlist *namelist_append_files(t_pathlist *listwas, const char *s)
{
    const char *npos;
    char temp[PD_STRING];
    t_pathlist *nl = listwas, *rtn = listwas;
    
    npos = s;
    do
    {
        npos = pathlist_getNextFile (temp, sizeof(temp), npos, PATHLIST_SEPARATOR);
        if (! *temp) continue;
        nl = pathlist_newAppend(nl, temp);
    }
        while (npos);
    return (nl);
}

void namelist_free(t_pathlist *listwas)
{
    t_pathlist *nl, *nl2;
    for (nl = listwas; nl; nl = nl2)
    {
        nl2 = nl->nl_next;
        PD_MEMORY_FREE(nl->nl_string, strlen(nl->nl_string) + 1);
        PD_MEMORY_FREE(nl, sizeof(*nl));
    }
}

char *namelist_get(t_pathlist *namelist, int n)
{
    int i;
    t_pathlist *nl;
    for (i = 0, nl = namelist; i < n && nl; i++, nl = nl->nl_next)
        ;
    return (nl ? nl->nl_string : 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
