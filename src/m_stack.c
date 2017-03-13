
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

void instance_stackPush (t_glist *x)
{
    t_stack *p = (t_stack *)PD_MEMORY_GET (sizeof (t_stack));
    
    p->g_what = instance_contextGet();
    p->g_next = instance_get()->pd_stackHead;
    p->g_abstraction = instance_get()->pd_loadingAbstraction;
    
    instance_get()->pd_loadingAbstraction = NULL;
    instance_get()->pd_stackHead = p;
    
    instance_contextSet (x);
}

void instance_stackPop (t_glist *x)
{
    t_stack *p = instance_get()->pd_stackHead;
    
    PD_ASSERT (p != NULL);
    PD_ASSERT (instance_contextGet() == x);
    
    instance_contextSet (p->g_what);
    instance_get()->pd_stackHead = p->g_next;
    
    PD_MEMORY_FREE (p);
    
    instance_get()->pd_contextPopped = x;
}

void instance_stackPopUntil (t_glist *x)
{
    t_glist *glist = x;
        
    while (instance_contextGet() && (glist != instance_contextGet())) {
        glist = instance_contextGet(); 
        canvas_pop (glist, 1); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_contextStore (void)
{
    PD_ASSERT (instance_get()->pd_contextCached == NULL);
    
    instance_get()->pd_contextCached = instance_contextGet();
}

void instance_contextRestore (void)
{
    instance_contextSet (instance_get()->pd_contextCached);
    
    instance_get()->pd_contextCached = NULL;
}

int instance_contextHasChanged (void)
{
    PD_ASSERT (instance_contextGet() != NULL);
    
    return (instance_get()->pd_contextCached != instance_contextGet());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_loadbang (void)
{
    if (instance_get()->pd_contextPopped) { 
        canvas_loadbang (instance_get()->pd_contextPopped);
        instance_get()->pd_contextPopped = NULL;
    }
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
    instance_contextStore();
    environment_setActiveArguments (argc, argv);
    
    buffer_fileEval (filename, gensym (directory));
    
    if (instance_contextHasChanged()) {
        instance_setNewestObject (cast_pd (instance_contextGet())); 
        canvas_pop (instance_contextGet(), 0); 
    }
    
    environment_resetActiveArguments();
    instance_contextRestore();
    //
    } else {
        error_recursiveInstantiation (filename);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
