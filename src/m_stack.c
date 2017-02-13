
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

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

static t_gstack *stack_stackHead;               /* Static. */
static t_pd     *stack_lastPopped;              /* Static. */
static t_symbol *stack_loadingAbstraction;      /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void stack_push (t_pd *x)
{
    t_gstack *p = (t_gstack *)PD_MEMORY_GET (sizeof (t_gstack));
    p->g_what = pd_getBoundX();
    p->g_next = stack_stackHead;
    p->g_loadingAbstraction  = stack_loadingAbstraction;
    stack_loadingAbstraction = NULL;
    stack_stackHead = p;
    pd_setBoundX (x);
}

void stack_pop (t_pd *x)
{
    if (!stack_stackHead || pd_getBoundX() != x) { PD_BUG; }
    else {
        t_gstack *p = stack_stackHead;
        pd_setBoundX (p->g_what);
        stack_stackHead = p->g_next;
        PD_MEMORY_FREE (p);
        stack_lastPopped = x;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void stack_proceedLoadbang (void)
{
    if (stack_lastPopped) { pd_vMessage (stack_lastPopped, sym_loadbang, ""); }
    
    stack_lastPopped = NULL;
}

int stack_setLoadingAbstraction (t_symbol *s)
{
    t_gstack *p = stack_stackHead;
    
    for (p = stack_stackHead; p; p = p->g_next) {
        if (p->g_loadingAbstraction == s) { return 1; }
    }
    
    stack_loadingAbstraction = s;
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
