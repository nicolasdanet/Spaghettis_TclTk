
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

static t_int instance_dspDone (t_int *w)
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_pdinstance *instance_new()
{
    t_pdinstance *x = (t_pdinstance *)PD_MEMORY_GET (sizeof (t_pdinstance));
    
    return x;
}

static void instance_free (t_pdinstance *x)
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
    glist->gl_next = instance_get()->pd_roots; instance_get()->pd_roots = glist;
}

void instance_removeFromRoots (t_glist *glist)
{
    if (glist == instance_get()->pd_roots) { instance_get()->pd_roots = glist->gl_next; }
    else {
        t_glist *z = NULL;
        for (z = instance_get()->pd_roots; z->gl_next != glist; z = z->gl_next) { }
        z->gl_next = glist->gl_next;
    }
}

/* At release close remaining (i.e. NOT dirty) and invisible patches. */

void instance_freeAllRoots (void)
{    
    while (1) {

        t_glist *glist = instance_get()->pd_roots;
        
        if (glist == NULL) { break; }
        else {
            pd_free (cast_pd (glist)); 
            if (glist == instance_get()->pd_roots) { 
                PD_BUG; break;                                      /* Not removed? */
            }   
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_destroyAllScalarsByTemplate (t_template *template)
{
    t_glist *glist = instance_get()->pd_roots;
    
    while (glist) {

        t_symbol *s = utils_stripTemplateIdentifier (template_getTemplateIdentifier (template));
        t_atom t;
        SET_SYMBOL (&t, s); 
        pd_message (cast_pd (glist), sym_destroy, 1, &t);
        glist = glist->gl_next;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_initializeDspChain (void)
{
    instance_get()->pd_dspChainSize = 1;
    instance_get()->pd_dspChain     = (t_int *)PD_MEMORY_GET (sizeof (t_int));
    instance_get()->pd_dspChain[0]  = (t_int)instance_dspDone;
}

void instance_appendToDspChain (t_perform f, int n, ...)
{
    int size = instance_get()->pd_dspChainSize + n + 1;
    
    size_t newSize = sizeof (t_int) * size;
    size_t oldSize = sizeof (t_int) * instance_get()->pd_dspChainSize;
    
    instance_get()->pd_dspChain = PD_MEMORY_RESIZE (instance_get()->pd_dspChain, oldSize, newSize);
    
    {
    //
    int i;
    va_list ap;

    instance_get()->pd_dspChain[instance_get()->pd_dspChainSize - 1] = (t_int)f;

    va_start (ap, n);
    
    for (i = 0; i < n; i++) { 
        instance_get()->pd_dspChain[instance_get()->pd_dspChainSize + i] = va_arg (ap, t_int);
    }
    
    va_end (ap);
    //
    }
    
    instance_get()->pd_dspChain[size - 1] = (t_int)instance_dspDone;
    instance_get()->pd_dspChainSize = size;
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
        t_pd *t = instance_getBoundX();
        environment_setActiveArguments (argc, argv);
        buffer_fileEval (gensym (name), gensym (directory));
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
#pragma mark -

void instance_initialize (void)
{
    pd_this = instance_new();
    
    PD_ASSERT (!pd_objectMaker);
    PD_ASSERT (!pd_canvasMaker);
    
    pd_objectMaker = class_new (sym_objectmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    pd_canvasMaker = class_new (sym_canvasmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    
    class_addAnything (pd_objectMaker, (t_method)instance_newAnything);
        
    class_addMethod (pd_canvasMaker, (t_method)canvas_new,      sym_canvas, A_GIMME, A_NULL);
    class_addMethod (pd_canvasMaker, (t_method)template_create, sym_struct, A_GIMME, A_NULL);
    
    instance_setBoundN (&pd_canvasMaker);
}

void instance_release (void)
{
    instance_setBoundA (NULL);
    instance_setBoundN (NULL);
    instance_setBoundX (NULL);
    
    CLASS_FREE (pd_objectMaker);
    CLASS_FREE (pd_canvasMaker);
    
    instance_free (pd_this);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
