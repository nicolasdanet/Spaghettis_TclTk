
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
    t_stackelement *e = instance_get()->pd_stack.stack_array + (instance_get()->pd_stack.stack_index++);
    
    PD_ABORT (instance_get()->pd_stack.stack_index >= INSTANCE_STACK_SIZE);     /* Resize? */
    
    e->stack_context = instance_contextGetCurrent();
    e->stack_abstraction = instance_get()->pd_loadingAbstraction;
    
    instance_get()->pd_loadingAbstraction = NULL;
    
    instance_contextSetCurrent (x);
}

void instance_stackPop (t_glist *x)
{
    t_stackelement *e = instance_get()->pd_stack.stack_array + (--instance_get()->pd_stack.stack_index);
    
    PD_ASSERT (instance_get()->pd_stack.stack_index >= 0);
    PD_ASSERT (instance_contextGetCurrent() == x);
    
    instance_contextSetCurrent (e->stack_context);
    
    instance_get()->pd_stack.stack_popped = x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Stack context temporary bypassed to eval the buffer. */

void instance_stackEval (t_glist *glist, t_buffer *b)
{
    instance_contextStore();
    instance_contextSetCurrent (glist);
    
    buffer_eval (b, NULL, 0, NULL);
    
    instance_contextRestore();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int instance_loadAbstractionIsValid (t_symbol *filename)
{
    int i;
    
    for (i = 0; i < instance_get()->pd_stack.stack_index; i++) {
    //
    t_stackelement *e = instance_get()->pd_stack.stack_array + i;
    if (e->stack_abstraction == filename) { return 0; }
    //
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
    
    if (instance_loadAbstractionIsValid (filename)) {
    //
    instance_contextStore();
    instance_environmentSetArguments (argc, argv);
    
    buffer_fileEval (filename, gensym (directory));
    
    if (instance_contextGetCurrent() != instance_contextGetStore()) {
        instance_setNewestObject (cast_pd (instance_contextGetCurrent())); 
        canvas_pop (instance_contextGetCurrent(), 0); 
    }
    
    instance_environmentResetArguments();
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
#pragma mark -

static void instance_loadPatchLoadbang (void)
{
    if (instance_get()->pd_stack.stack_popped) { 
        canvas_loadbang (instance_get()->pd_stack.stack_popped);
        instance_get()->pd_stack.stack_popped = NULL;
    }
}

static void instance_loadPatchProceed (t_symbol *name, t_symbol *directory, char *s, int visible)
{
    instance_contextStore();
    instance_contextSetCurrent (NULL);          /* The root canvas do NOT have parent. */
        
    if (s) { buffer_fileEvalByString (name, directory, s); }
    else   { buffer_fileEval (name, directory); }
    
    if (instance_contextGetCurrent() != NULL) { canvas_pop (instance_contextGetCurrent(), visible); }
    
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
