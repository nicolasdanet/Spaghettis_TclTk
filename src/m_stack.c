
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

/* A canvas is bind to a context (the symbol #X). */
/* Most of the lines of a file can be considered as messages to the current context. */
/* Nested canvas are handled with a stack mechanism. */
/* Contexts are pushed and popped to go down and up in the tree. */
/* Note that abstractions cannot be recursively instantiated. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void instance_contextStore (void)
{
    instance_get()->pd_stack.s_cache = instance_contextGetCurrent();
}

static void instance_contextRestore (void)
{
    instance_contextSetCurrent (instance_get()->pd_stack.s_cache);
    
    instance_get()->pd_stack.s_cache = NULL;
}

static t_glist *instance_contextGetStored (void)
{
    return instance_get()->pd_stack.s_cache;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_stackPush (t_glist *x)
{
    t_stackelement *e = instance_get()->pd_stack.s_stack + (instance_get()->pd_stack.s_stackIndex++);
    
    PD_ABORT (instance_get()->pd_stack.s_stackIndex >= INSTANCE_STACK_SIZE);    /* Resize? */
    
    e->s_context = instance_contextGetCurrent();
    e->s_abstraction = instance_get()->pd_loadingAbstraction;
    
    instance_get()->pd_loadingAbstraction = NULL;
    
    instance_contextSetCurrent (x);
}

void instance_stackPop (t_glist *x)
{
    t_stackelement *e = instance_get()->pd_stack.s_stack + (--instance_get()->pd_stack.s_stackIndex);
    
    PD_ASSERT (instance_get()->pd_stack.s_stackIndex >= 0);
    PD_ASSERT (instance_contextGetCurrent() == x);
    
    instance_contextSetCurrent (e->s_context);
    
    instance_get()->pd_stack.s_popped = x;
}

void instance_stackPopPatch (t_glist *glist, int visible)
{
    instance_stackPop (glist);
    
    canvas_resortInlets (glist); canvas_resortOutlets (glist);
    
    glist_loadEnd (glist);
    
    if (visible) { canvas_visible (glist, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int instance_loadAbstractionIsValid (t_symbol *filename)
{
    int i;
    
    for (i = 0; i < instance_get()->pd_stack.s_stackIndex; i++) {
    //
    t_stackelement *e = instance_get()->pd_stack.s_stack + i;
    if (e->s_abstraction == filename) { return 0; }
    //
    }
    
    instance_get()->pd_loadingAbstraction = filename;
    
    return 1;
}

void instance_loadAbstraction (t_symbol *s, int argc, t_atom *argv)
{
    char directory[PD_STRING] = { 0 }; char *name = NULL;
    
    if (glist_fileFind (instance_contextGetCurrent(), s->s_name, PD_PATCH, directory, &name, PD_STRING)) {
    //
    t_symbol *filename = gensym (name);
    
    if (instance_loadAbstractionIsValid (filename)) {
    //
    instance_environmentSetArguments (argc, argv);          /* Get an environment (unique dollar zero). */
    
    buffer_fileEval (filename, gensym (directory));
    
    if (instance_contextGetCurrent() != instance_contextGetStored()) {
    
        instance_setNewestObject (cast_pd (instance_contextGetCurrent()));
        
        instance_stackPopPatch (instance_contextGetCurrent(), 0); 
    }
    
    instance_environmentResetArguments();
    //
    } else {
        error_recursiveInstantiation (filename);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void instance_loadPatchLoadbang (void)
{
    if (instance_get()->pd_stack.s_popped) { 
        canvas_loadbang (instance_get()->pd_stack.s_popped);
        instance_get()->pd_stack.s_popped = NULL;
    }
}

static void instance_loadPatchProceed (t_symbol *name, t_symbol *directory, char *s, int visible)
{
    instance_contextStore();
    instance_contextSetCurrent (NULL);          /* The root canvas do NOT have parent. */
        
    if (s) { buffer_fileEvalByString (name, directory, s); }
    else   { buffer_fileEval (name, directory); }
    
    if (instance_contextGetCurrent() != NULL) { 
        instance_stackPopPatch (instance_contextGetCurrent(), visible); 
    }
    
    PD_ASSERT (instance_contextGetCurrent() == NULL);
    
    instance_loadPatchLoadbang();
    instance_contextRestore();
}

void instance_loadPatch (t_symbol *name, t_symbol *directory)
{
    instance_loadPatchProceed (name, directory, NULL, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Load invisible patches (mainly used for built-in templates). */

void instance_loadInvisible (t_symbol *name, t_symbol *directory, char *s)
{
    instance_loadPatchProceed (name, directory, s, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Context of the stack temporary bypassed to eval the buffer. */

void instance_loadSnippet (t_glist *glist, t_buffer *b)
{
    instance_contextStore();
    instance_contextSetCurrent (glist);
    buffer_eval (b, NULL, 0, NULL);
    instance_contextRestore();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
