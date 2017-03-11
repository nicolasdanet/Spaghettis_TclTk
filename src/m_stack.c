
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

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
    p->g_what = instance_getBoundX();
    p->g_next = stack_stackHead;
    p->g_loadingAbstraction  = stack_loadingAbstraction;
    stack_loadingAbstraction = NULL;
    stack_stackHead = p;
    instance_setBoundX (x);
}

void stack_pop (t_pd *x)
{
    if (!stack_stackHead || instance_getBoundX() != x) { PD_BUG; }
    else {
        t_gstack *p = stack_stackHead;
        instance_setBoundX (p->g_what);
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
    if (stack_lastPopped) { pd_message (stack_lastPopped, sym_loadbang, 0, NULL); }
    
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

static void instance_popAbstraction (t_glist *glist)
{
    instance_get()->pd_newest = cast_pd (glist);
    stack_pop (cast_pd (glist));
    
    glist->gl_isLoading = 0;
    
    canvas_resortInlets (glist);
    canvas_resortOutlets (glist);
}

void instance_loadAbstraction (t_symbol *s, int argc, t_atom *argv)
{
    char directory[PD_STRING] = { 0 }; char *name = NULL;
    
    if (canvas_fileFind (canvas_getCurrent(), s->s_name, PD_PATCH, directory, &name, PD_STRING)) {
    //
    t_symbol *f = gensym (name);
    
    if (stack_setLoadingAbstraction (f)) { error_recursiveInstantiation (f); }
    else {
        t_pd *t = instance_getBoundX();
        environment_setActiveArguments (argc, argv);
        buffer_fileEval (f, gensym (directory));
        if (instance_getBoundX() && t != instance_getBoundX()) { 
            instance_popAbstraction (cast_glist (instance_getBoundX())); 
        } else { 
            instance_setBoundX (t); 
        }
        environment_resetActiveArguments();
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
