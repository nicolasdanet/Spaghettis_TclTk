
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

#if PD_WINDOWS
    #define NAMELIST_SEPARATOR ';'
#else
    #define NAMELIST_SEPARATOR ':'
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static const char *namelist_getNextFile (char *dest, size_t length, const char *src, char delimiter)
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

t_namelist *namelist_newAppend (t_namelist *x, const char *s)
{
    t_namelist *nl1 = x;
    t_namelist *nl2 = NULL;
    
    nl2 = (t_namelist *)(PD_MEMORY_GET (sizeof (t_namelist)));
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

t_namelist *namelist_append_files(t_namelist *listwas, const char *s)
{
    const char *npos;
    char temp[PD_STRING];
    t_namelist *nl = listwas, *rtn = listwas;
    
    npos = s;
    do
    {
        npos = namelist_getNextFile (temp, sizeof(temp), npos, NAMELIST_SEPARATOR);
        if (! *temp) continue;
        nl = namelist_newAppend(nl, temp);
    }
        while (npos);
    return (nl);
}

void namelist_free(t_namelist *listwas)
{
    t_namelist *nl, *nl2;
    for (nl = listwas; nl; nl = nl2)
    {
        nl2 = nl->nl_next;
        PD_MEMORY_FREE(nl->nl_string, strlen(nl->nl_string) + 1);
        PD_MEMORY_FREE(nl, sizeof(*nl));
    }
}

char *namelist_get(t_namelist *namelist, int n)
{
    int i;
    t_namelist *nl;
    for (i = 0, nl = namelist; i < n && nl; i++, nl = nl->nl_next)
        ;
    return (nl ? nl->nl_string : 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
