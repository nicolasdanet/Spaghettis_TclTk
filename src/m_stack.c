
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

void instance_stackPush (t_pd *x)
{
    t_stack *p = (t_stack *)PD_MEMORY_GET (sizeof (t_stack));
    
    p->g_what = instance_getBoundX();
    p->g_next = instance_get()->pd_stackHead;
    p->g_abstraction = instance_get()->pd_loadingAbstraction;
    
    instance_get()->pd_loadingAbstraction = NULL;
    instance_get()->pd_stackHead = p;
    
    instance_setBoundX (x);
}

void instance_stackPop (t_pd *x)
{
    t_stack *p = instance_get()->pd_stackHead;
    
    PD_ASSERT (p != NULL);
    PD_ASSERT (instance_getBoundX() == x);
    
    instance_setBoundX (p->g_what);
    instance_get()->pd_stackHead = p->g_next;
    
    PD_MEMORY_FREE (p);
    
    instance_get()->pd_stackPopped = x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_stackContextStore (void)
{
    PD_ASSERT (instance_get()->pd_stackCached == NULL);
    
    instance_get()->pd_stackCached = instance_getBoundX();
}

void instance_stackContextRestore (void)
{
    instance_setBoundX (instance_get()->pd_stackCached);
    
    instance_get()->pd_stackCached = NULL;
}

int instance_stackContextHasChanged (void)
{
    PD_ASSERT (instance_getBoundX() != NULL);
    
    return (instance_get()->pd_stackCached != instance_getBoundX());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_stackLoadbang (void)
{
    if (instance_get()->pd_stackPopped) {
    // 
    pd_message (instance_get()->pd_stackPopped, sym_loadbang, 0, NULL);
    // 
    }
    
    instance_get()->pd_stackPopped = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int instance_loadAbstractionIsNotAlreadyThere (t_symbol *filename)
{
    t_stack *p = instance_get()->pd_stackHead;
    
    for (p = instance_get()->pd_stackHead; p; p = p->g_next) {
        if (p->g_abstraction == filename) { return 0; }
    }
    
    instance_get()->pd_loadingAbstraction = filename;
    
    return 1;
}

void instance_loadAbstraction (t_symbol *s, int argc, t_atom *argv)
{
    char directory[PD_STRING] = { 0 }; char *name = NULL;
    
    if (canvas_fileFind (canvas_getCurrent(), s->s_name, PD_PATCH, directory, &name, PD_STRING)) {
    //
    t_symbol *filename = gensym (name);
    
    if (instance_loadAbstractionIsNotAlreadyThere (filename)) {
    //
    instance_stackContextStore();
    environment_setActiveArguments (argc, argv);
    
    buffer_fileEval (filename, gensym (directory));
    if (instance_stackContextHasChanged()) {
        instance_setNewestObject (instance_getBoundX()); 
        canvas_pop (cast_glist (instance_getBoundX()), 0); 
    }
    
    environment_resetActiveArguments();
    instance_stackContextRestore();
    //
    } else {
        error_recursiveInstantiation (filename);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
