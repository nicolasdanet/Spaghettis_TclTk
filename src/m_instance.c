
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INSTANCE_MAXIMUM_RECURSION   1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pdinstance *pd_this;                          /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd *pd_newest;                                /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd pd_objectMaker;                            /* Static. */
t_pd pd_canvasMaker;                            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int instance_recursiveDepth;             /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_pdinstance *pdinstance_new()
{
    t_pdinstance *x = (t_pdinstance *)PD_MEMORY_GET (sizeof (t_pdinstance));
    
    return x;
}

static void pdinstance_free (t_pdinstance *x)
{
    PD_ASSERT (x->pd_dspChain       == NULL);
    PD_ASSERT (x->pd_clocks         == NULL);
    PD_ASSERT (x->pd_signals        == NULL);
    PD_ASSERT (x->pd_roots          == NULL);
    PD_ASSERT (x->pd_polling        == NULL);
    PD_ASSERT (x->pd_autorelease    == NULL);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_addToRoots (t_glist *glist)
{
    glist->gl_next = pd_this->pd_roots; pd_this->pd_roots = glist;
}

void instance_removeFromRoots (t_glist *glist)
{
    if (glist == pd_this->pd_roots) { pd_this->pd_roots = glist->gl_next; }
    else {
        t_glist *z = NULL;
        for (z = pd_this->pd_roots; z->gl_next != glist; z = z->gl_next) { }
        z->gl_next = glist->gl_next;
    }
}

/* At release close remaining (i.e. NOT dirty) and invisible patches. */

void instance_freeAllRoots (void)
{    
    while (1) {
    //
    t_glist *glist = pd_this->pd_roots;
    
    if (glist == NULL) { break; }
    else {
        pd_free (cast_pd (glist)); 
        if (glist == pd_this->pd_roots) { PD_BUG; break; }      /* Not removed? */
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void instance_popAbstraction (t_glist *glist)
{
    pd_newest = cast_pd (glist);
    stack_pop (cast_pd (glist));
    
    glist->gl_isLoading = 0;
    
    canvas_resortInlets (glist);
    canvas_resortOutlets (glist);
}

static void instance_newAnything (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    int f;
    char directory[PD_STRING] = { 0 };
    char *name = NULL;
    t_error err = PD_ERROR_NONE;
    
    if (instance_recursiveDepth > INSTANCE_MAXIMUM_RECURSION) { PD_BUG; return; }

    pd_newest = NULL;
    
    if (loader_load (canvas_getCurrent(), s->s_name)) {
        instance_recursiveDepth++;
        pd_message (x, s, argc, argv);
        instance_recursiveDepth--;
        return;
    }
    
    err = (f = canvas_openFile (canvas_getCurrent(), s->s_name, PD_PATCH, directory, &name, PD_STRING)) < 0;
    
    if (err) { pd_newest = NULL; }
    else {
    //
    close (f);
    
    if (stack_setLoadingAbstraction (s)) { error_recursiveInstantiation (s); }
    else {
        t_pd *t = pd_getBoundX();
        environment_setActiveArguments (argc, argv);
        buffer_fileEval (gensym (name), gensym (directory));
        if (pd_getBoundX() && t != pd_getBoundX()) { instance_popAbstraction (cast_glist (pd_getBoundX())); }
        else { 
            pd_setBoundX (t); 
        }
        environment_resetActiveArguments();
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_initialize (void)
{
    pd_this = pdinstance_new();
    
    PD_ASSERT (!pd_objectMaker);
    
    pd_objectMaker = class_new (sym_objectmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    pd_canvasMaker = class_new (sym_canvasmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    
    class_addAnything (pd_objectMaker, (t_method)instance_newAnything);
        
    class_addMethod (pd_canvasMaker, (t_method)canvas_new,      sym_canvas, A_GIMME, A_NULL);
    class_addMethod (pd_canvasMaker, (t_method)template_create, sym_struct, A_GIMME, A_NULL);
    
    pd_setBoundN (&pd_canvasMaker);
}

void instance_release (void)
{
    pd_setBoundA (NULL);
    pd_setBoundN (NULL);
    pd_setBoundX (NULL);
    
    CLASS_FREE (pd_objectMaker);
    CLASS_FREE (pd_canvasMaker);
    
    pdinstance_free (pd_this);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
