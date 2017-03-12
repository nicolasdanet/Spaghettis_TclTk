
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INSTANCE_PERIOD_POLLING         47.0
#define INSTANCE_PERIOD_AUTORELEASE     SECONDS_TO_MILLISECONDS (5.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pdinstance *pd_this;                  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void instance_autoreleaseDrain (void)
{
    if (pd_isThingQuiet (sym__autorelease)) {
        pd_message (pd_getThing (sym__autorelease), sym__autorelease, 0, NULL);
    }   
}

static void instance_autoreleaseTask (void *dummy)
{
    instance_autoreleaseDrain();
    clock_delay (instance_get()->pd_autorelease, INSTANCE_PERIOD_AUTORELEASE);
}

static void instance_autoreleaseReschedule (void)
{
    clock_unset (instance_get()->pd_autorelease); 
    clock_delay (instance_get()->pd_autorelease, INSTANCE_PERIOD_AUTORELEASE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void instance_pollingTask (void *dummy)
{
    if (pd_isThingQuiet (sym__polling)) {
        pd_message (pd_getThing (sym__polling), sym__polling, 0, NULL);
    }  
    
    clock_delay (instance_get()->pd_polling, INSTANCE_PERIOD_POLLING);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_int instance_dspDone (t_int *dummy)
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_rootsAdd (t_glist *glist)
{
    glist->gl_next = instance_get()->pd_roots; instance_get()->pd_roots = glist;
}

void instance_rootsRemove (t_glist *glist)
{
    if (glist == instance_get()->pd_roots) { instance_get()->pd_roots = glist->gl_next; }
    else {
        t_glist *z = NULL;
        for (z = instance_get()->pd_roots; z->gl_next != glist; z = z->gl_next) { }
        z->gl_next = glist->gl_next;
    }
}

/* At release, close remaining (i.e. NOT dirty) and invisible patches. */

void instance_rootsFreeAll (void)
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

void instance_dspStart (void)
{
    t_glist *glist;

    ugen_dspInitialize();
    
    for (glist = instance_getRoots(); glist; glist = glist->gl_next) { canvas_dspProceed (glist, 1, NULL); }
    
    instance_setDspState (1);
}


void instance_dspStop (void)
{
    PD_ASSERT (instance_getDspState());
    
    ugen_dspRelease();
    
    instance_setDspState (0);
}

void instance_dspChainInitialize (void)
{
    PD_ASSERT (instance_get()->pd_dspChain == NULL);
    
    instance_get()->pd_dspChainSize = 1;
    instance_get()->pd_dspChain     = (t_int *)PD_MEMORY_GET (sizeof (t_int));
    instance_get()->pd_dspChain[0]  = (t_int)instance_dspDone;
}

void instance_dspChainRelease (void)
{
    if (instance_get()->pd_dspChain != NULL) {
    //
    PD_MEMORY_FREE (instance_get()->pd_dspChain);
    
    instance_get()->pd_dspChain = NULL;
    //
    }
}

void instance_dspChainAppend (t_perform f, int n, ...)
{
    int size = instance_get()->pd_dspChainSize + n + 1;
    
    PD_ASSERT (instance_get()->pd_dspChain != NULL);
    
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

void instance_signalAdd (t_signal *s)
{
    s->s_next = instance_get()->pd_signals;
    
    instance_get()->pd_signals = s;
}

void instance_signalFreeAll (void)
{
    t_signal *s = NULL;
    
    while ((s = instance_get()->pd_signals)) {
    //
    instance_get()->pd_signals = s->s_next;
    
    if (!s->s_hasBorrowed) { PD_MEMORY_FREE (s->s_vector); }
    else {
    //
    PD_ASSERT (s->s_unused); PD_MEMORY_FREE (s->s_unused);
    //
    }
    
    PD_MEMORY_FREE (s);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Enqueue the clock sorted up by logical time. */

void instance_clockAdd (t_clock *c)
{
    t_systime systime = clock_getLogicalTime (c);
    
    if (instance_get()->pd_clocks && clock_getLogicalTime (instance_get()->pd_clocks) <= systime) {
    
        t_clock *m = instance_get()->pd_clocks;
        t_clock *n = instance_get()->pd_clocks->c_next;
        
        while (m) {
            if (!n || clock_getLogicalTime (n) > systime) {
                m->c_next = c; c->c_next = n; return;
            }
            m = n;
            n = m->c_next;
        }
        
    } else {
        c->c_next = instance_get()->pd_clocks; instance_get()->pd_clocks = c;
    }
}

void instance_clockUnset (t_clock *c)
{
    if (c == instance_get()->pd_clocks) { instance_get()->pd_clocks = c->c_next; }
    else {
        t_clock *t = instance_get()->pd_clocks;
        while (t->c_next != c) { t = t->c_next; } t->c_next = c->c_next;
    }
}

void instance_clockTick (t_systime t)
{
    while (instance_get()->pd_clocks && clock_getLogicalTime (instance_get()->pd_clocks) < t) {
    //
    t_clock *c = instance_get()->pd_clocks;
    instance_get()->pd_systime = clock_getLogicalTime (c);
    clock_unset (c);
    clock_execute (c);
    //
    }
    
    instance_get()->pd_systime = t;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_pollingRun (void)
{
    instance_get()->pd_polling = clock_new ((void *)NULL, (t_method)instance_pollingTask);
    
    clock_delay (instance_get()->pd_polling, INSTANCE_PERIOD_POLLING);
}

void instance_pollingStop (void)
{
    clock_free (instance_get()->pd_polling); instance_get()->pd_polling = NULL;
}

void instance_pollingRegister (t_pd *x)
{
    pd_bind (x, sym__polling);
}

void instance_pollingUnregister (t_pd *x)
{
    pd_unbind (x, sym__polling);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void instance_autoreleaseRun (void)
{
    instance_get()->pd_autorelease = clock_new ((void *)NULL, (t_method)instance_autoreleaseTask);
    clock_delay (instance_get()->pd_autorelease, INSTANCE_PERIOD_AUTORELEASE);
}

void instance_autoreleaseStop (void)
{
    instance_autoreleaseDrain();
    clock_free (instance_get()->pd_autorelease); instance_get()->pd_autorelease = NULL;
}

void instance_autoreleaseRegister (t_pd *x)
{
    pd_bind (x, sym__autorelease);
    
    if (instance_get()->pd_autorelease) { instance_autoreleaseReschedule(); }
    else {
        instance_autoreleaseDrain();    /* While quitting the application. */
    }
}

/* Function that might be called at last by registered object while autoreleased. */

void instance_autoreleaseProceed (t_pd *x)
{
    pd_unbind (x, sym__autorelease); pd_free (x);
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

/* Called if no method of the object factory match. */
/* Note it can be called recursively with wrong externals. */

static void instance_factory (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    instance_get()->pd_newest = NULL;
    
    if (!instance_get()->pd_loadingExternal) {

        /* First search an external and redo the creation. */
        
        if (loader_load (canvas_getCurrent(), s)) {
            instance_get()->pd_loadingExternal = 1;
            pd_message (x, s, argc, argv);
            instance_get()->pd_loadingExternal = 0;
        
        /* Otherwise look for an abstraction. */
        
        } else {
            instance_loadAbstraction (s, argc, argv);
        }
        
    } else {
        error_canNotFind (sym_loader, sym_class);   /* External MUST provide a properly named class. */
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_pdinstance *instance_new()
{
    t_pdinstance *x = (t_pdinstance *)PD_MEMORY_GET (sizeof (t_pdinstance));
    
    x->pd_objectMaker = class_new (sym_objectmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    x->pd_canvasMaker = class_new (sym_canvasmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    
    class_addAnything (x->pd_objectMaker, (t_method)instance_factory);
        
    class_addMethod (x->pd_canvasMaker, (t_method)canvas_new,      sym_canvas, A_GIMME, A_NULL);
    class_addMethod (x->pd_canvasMaker, (t_method)template_create, sym_struct, A_GIMME, A_NULL);
    
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
    
    CLASS_FREE (x->pd_canvasMaker);
    CLASS_FREE (x->pd_objectMaker);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* The factories are local to the instance. */
/* Requiered global to all instances in the future? */

void instance_initialize (void)
{
    pd_this = instance_new();
    
    instance_setBoundN (&(instance_get()->pd_canvasMaker));
}

void instance_release (void)
{
    instance_setBoundA (NULL);
    instance_setBoundN (NULL);
    instance_setBoundX (NULL);
    
    instance_free (pd_this);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
