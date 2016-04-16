
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _gstack {
    t_pd            *g_what;                    /* MUST be the first. */
    t_symbol        *g_loadingAbstraction;
    struct _gstack  *g_next;
    } t_gstack;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_gstack *pd_stackHead;                  /* Shared. */
static t_pd     *pd_lastPopped;                 /* Shared. */
static t_symbol *pd_loadingAbstraction;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_push (t_pd *x)
{
    t_gstack *p = (t_gstack *)PD_MEMORY_GET (sizeof (t_gstack));
    p->g_what = s__X.s_thing;
    p->g_next = pd_stackHead;
    p->g_loadingAbstraction = pd_loadingAbstraction;
    pd_loadingAbstraction = NULL;
    pd_stackHead = p;
    s__X.s_thing = x;
}

void pd_pop (t_pd *x)
{
    if (!pd_stackHead || s__X.s_thing != x) { PD_BUG; }
    else {
        t_gstack *p = pd_stackHead;
        s__X.s_thing = p->g_what;
        pd_stackHead = p->g_next;
        PD_MEMORY_FREE (p);
        pd_lastPopped = x;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_performLoadbang (void)
{
    if (pd_lastPopped) { pd_vMessage (pd_lastPopped, gensym ("loadbang"), ""); }
    
    pd_lastPopped = NULL;
}

int pd_setLoadingAbstraction (t_symbol *s)
{
    t_gstack *p = pd_stackHead;
    
    for (p = pd_stackHead; p; p = p->g_next) {
        if (p->g_loadingAbstraction == s) { return 1; }
    }
    
    pd_loadingAbstraction = s;
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
