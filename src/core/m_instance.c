
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"
#include "../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define INSTANCE_TIME_CLOCKS    1000

#define INSTANCE_TIME_CHAIN     PD_SECONDS_TO_MILLISECONDS (30)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pdinstance *pd_this;  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_new             (void *, t_symbol *, int, t_atom *);
void canvas_closeProceed    (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int dspthread_isChainSafeToDelete (t_dspthread *, t_chain *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *atomic_pointerSwap    (void *, t_pointerAtomic *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_glist *instance_contextGetCurrent (void)
{
    return cast_glist (instance_getBoundX());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_rootsAdd (t_glist *glist)
{
    glist_setNext (glist, instance_get()->pd_roots); instance_get()->pd_roots = glist;
    
    instance_registerAdd (cast_gobj (glist), NULL);
}

void instance_rootsRemove (t_glist *glist)
{
    instance_registerRemove (cast_gobj (glist));
    
    if (glist == instance_get()->pd_roots) { instance_get()->pd_roots = glist_getNext (glist); }
    else {
        t_glist *z = NULL;
        for (z = instance_get()->pd_roots; glist_getNext (z) != glist; z = glist_getNext (z)) { }
        glist_setNext (z, glist_getNext (glist));
    }
}

/* At release, close remaining (i.e. NOT dirty) and invisible patches. */

void instance_rootsFreeAll (void)
{    
    while (1) {

        t_glist *glist = instance_get()->pd_roots;
        
        if (glist == NULL) { break; }
        else {
            canvas_closeProceed (glist);
            if (glist == instance_get()->pd_roots) {    /* Not removed? */
                PD_BUG; break;
            }   
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_registerAdd (t_gobj *o, t_glist *owner)
{
    register_add (instance_get()->pd_register, gobj_getUnique (o), o, owner);
}

t_error instance_registerRemove (t_gobj *o)
{
    t_error err = register_remove (instance_get()->pd_register, gobj_getUnique (o));
    
    PD_ASSERT (!err); PD_UNUSED (err);
    
    return err;
}

void instance_registerRename (t_gobj *o, t_id newUnique)
{
    register_rename (instance_get()->pd_register, gobj_getUnique (o), newUnique);
}

int instance_registerContains (t_id u)
{
    return register_contains (instance_get()->pd_register, u);
}

t_gobj *instance_registerGetObject (t_id u)
{
    return register_getObject (instance_get()->pd_register, u);
}

t_glist *instance_registerGetOwner (t_id u)
{
    return register_getOwner (instance_get()->pd_register, u);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_clocksAdd (t_clock *c)
{
    clocks_add (instance_get()->pd_clocks, c);
}

void instance_clocksRemove (t_clock *c)
{
    clocks_remove (instance_get()->pd_clocks, c);
}

void instance_clocksTick (t_systime t)
{
    PD_ASSERT (sys_isMainThread());
    
    clocks_tick (instance_get()->pd_clocks, t);
}

void instance_clocksClean (void)
{
    int n = PD_ATOMIC_INT32_INCREMENT (&instance_get()->pd_clocksCount);
    
    if (n > INSTANCE_TIME_CLOCKS) {
    //
    if (clocks_clean (instance_get()->pd_clocks)) {
        PD_ATOMIC_INT32_WRITE (0, &instance_get()->pd_clocksCount);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_chain *instance_chainGetCurrent (void)
{
    return (t_chain *)PD_ATOMIC_POINTER_READ (&instance_get()->pd_chain);
}

static void instance_chainSetCurrent (t_chain *chain)
{
    t_chain *oldChain = (t_chain *)atomic_pointerSwap ((void *)chain, &instance_get()->pd_chain);
    
    if (oldChain) { chain_release (oldChain); }
}

t_chain *instance_chainGetTemporary (void)
{
    t_chain *chain = instance_get()->pd_build; PD_ASSERT (chain); return chain;
}

static void instance_chainStartTemporary (void)
{
    PD_ATOMIC_INT32_WRITE (1, &instance_get()->pd_chainRetain);

    PD_ASSERT (instance_get()->pd_build == NULL); instance_get()->pd_build = chain_new();
}

static void instance_chainPushTemporary (void)
{
    instance_chainSetCurrent (instance_chainGetTemporary()); instance_get()->pd_build = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int instance_isChainSafeToDelete (t_chain *chain)
{
    if (PD_ATOMIC_INT32_READ (&instance_get()->pd_chainRetain)) { return 0; }
    
    return dspthread_isChainSafeToDelete (instance_get()->pd_dsp, chain);
}

void instance_chainSetInitialized (void)
{
    PD_ATOMIC_INT32_WRITE (0, &instance_get()->pd_chainRetain);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error instance_dspCreate (void)
{
    return dspthread_create (instance_get()->pd_dsp);
}

void instance_dspStart (void)
{
    t_glist *glist;

    PD_ASSERT (instance_ugenGetContext() == NULL);
    
    instance_chainStartTemporary();
    
    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) { 
        canvas_dspProceed (glist, 1, NULL); 
    }
    
    instance_chainPushTemporary();
    
    dspthread_run (instance_get()->pd_dsp);
}

void instance_dspStop (void)
{
    dspthread_stop (instance_get()->pd_dsp);
}

void instance_dspClean (void)
{
    if (instance_chainGetCurrent()) {
    //
    t_systime t = dspthread_time (instance_get()->pd_dsp);
    
    if (t && scheduler_getMillisecondsSince (t) > INSTANCE_TIME_CHAIN) {
    //
    if (PD_ATOMIC_INT32_READ (&instance_get()->pd_chainRetain) == 0) {
    //
    instance_chainSetCurrent (NULL);
    //
    }
    //
    }
    //
    }
}

void instance_dspFree (void)
{
    instance_chainSetCurrent (NULL); dspthread_free (instance_get()->pd_dsp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_audioCloseTask (void *dummy)
{
    dsp_setState (0); error_unexpected (sym_audio, sym_shutdown);
}

void instance_audioCloseWithError (void)
{
    clock_delay (instance_get()->pd_stop, 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_deselectAllObjects (void)
{
    t_glist *glist = instance_get()->pd_roots;
    
    while (glist) { glist_deselectAllRecursive (glist); glist = glist_getNext (glist); }
}

void instance_destroyAllScalarsByTemplate (t_template *tmpl)
{
    t_glist *glist = instance_get()->pd_roots;
    
    PD_ASSERT (tmpl);
    
    instance_undoUnsetRecursive();  /* Edge cases when de-encapsulating templates for scalars. */
    
    while (glist) {

        if (!template_isPrivate (template_getTemplateIdentifier (tmpl))) {
            glist_objectRemoveAllByTemplate (glist, tmpl); 
        }
    
        glist = glist_getNext (glist);
    }
    
    instance_undoSetRecursive();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_setDefaultCoordinates (t_glist *glist, int a, int b)
{
    instance_get()->pd_locate.p_glist = glist;
    instance_get()->pd_locate.p_x     = a;
    instance_get()->pd_locate.p_y     = b;
}

int instance_getDefaultX (t_glist *glist)
{
    t_glist *t = instance_get()->pd_locate.p_glist;
    
    if (t == glist) { return instance_get()->pd_locate.p_x; }
    else {
        t_point pt = glist_getPositionMiddle (glist);
        return point_getX (&pt);
    }
}

int instance_getDefaultY (t_glist *glist)
{
    t_glist *t = instance_get()->pd_locate.p_glist;
    
    if (t == glist) { return instance_get()->pd_locate.p_y; }
    else {
        t_point pt = glist_getPositionMiddle (glist);
        return point_getY (&pt);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_openedWindowInEditModeCountCheck (t_glist *glist)
{
    if (!glist_isAbstraction (glist) && glist_isWindowable (glist) && glist_isOnScreen (glist)) {
    //
    if (glist_hasEditMode (glist)) { instance_get()->pd_openedWindowInEditMode++; }
    //
    }
}

static void instance_openedWindowInEditModeCountRecursive (t_glist *glist)
{
    t_gobj *y = NULL;
    
    instance_openedWindowInEditModeCountCheck (glist);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_isCanvas (y)) { instance_openedWindowInEditModeCountRecursive (cast_glist (y)); }
    }
}

static void instance_openedWindowInEditModeCount (void)
{
    t_glist *glist = instance_get()->pd_roots;
    
    instance_get()->pd_openedWindowInEditMode = 0;
    
    while (glist) { instance_openedWindowInEditModeCountRecursive (glist); glist = glist_getNext (glist); }
}

static int instance_openedWindowInEditMode (void)
{
    if (instance_get()->pd_openedWindowInEditMode == -1) { instance_openedWindowInEditModeCount(); }
    
    PD_ASSERT (instance_get()->pd_openedWindowInEditMode >= 0);
    
    return instance_get()->pd_openedWindowInEditMode;
}

int instance_hasOpenedWindowInEditMode (void)
{
    return (instance_openedWindowInEditMode() != 0);
}

void instance_openedWindowInEditModeReset (void)
{
    instance_get()->pd_openedWindowInEditMode = -1;
    
    if (symbol_hasThingQuiet (sym__editmode)) {
    //
    pd_message (symbol_getThing (sym__editmode), sym_set, 0, NULL);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error instance_overflowPush (void)
{
    int count   = ++instance_get()->pd_overflowCount;
    t_error err = (count >= INSTANCE_OVERFLOW);
    
    if (err && !instance_get()->pd_overflow) { instance_get()->pd_overflow = 1; error_stackOverflow(); }
    
    err |= instance_get()->pd_overflow;
    
    return err;
}

void instance_overflowPop (void)
{
    int count = --instance_get()->pd_overflowCount; if (count == 0) { instance_get()->pd_overflow = 0; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Called if no method of the maker object match. */

static void instance_factory (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    instance_get()->pd_newest = NULL;
    
    /* Note that it can be a recursive call. */
    
    if (!instance_get()->pd_isLoadingExternal) {

        /* First search an external. */
        
        if (loader_load (instance_contextGetCurrent(), s)) {
            instance_get()->pd_isLoadingExternal = 1;
            pd_message (x, s, argc, argv);              /* Try again. */
            instance_get()->pd_isLoadingExternal = 0;
        
        /* Otherwise look for an abstraction. */
        
        } else {
            instance_loadAbstraction (s, argc, argv);
        }
        
    } else {
        error_canNotFind (sym_loader, sym_class);       /* External MUST provide a properly named class. */
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_pdinstance *instance_new()
{
    t_pdinstance *x = (t_pdinstance *)PD_MEMORY_GET (sizeof (t_pdinstance));
    
    x->pd_stack.s_stack = (t_stackelement *)PD_MEMORY_GET (INSTANCE_STACK * sizeof (t_stackelement));
    
    x->pd_environment.env_directory = &s_;
    x->pd_environment.env_fileName  = &s_;
    
    x->pd_objectMaker = class_new (sym_objectmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    x->pd_canvasMaker = class_new (sym_canvasmaker, NULL, NULL, 0, CLASS_ABSTRACT, A_NULL);
    
    x->pd_clocks      = clocks_new();
    x->pd_register    = register_new();
    x->pd_pool        = buffer_new();
    x->pd_dsp         = dspthread_new();
    x->pd_stop        = clock_new ((void *)x, (t_method)instance_audioCloseTask);
    
    x->pd_openedWindowInEditMode = -1;
    
    class_addAnything (x->pd_objectMaker, (t_method)instance_factory);
    
    class_addMethod (x->pd_canvasMaker, (t_method)canvas_new, sym_canvas, A_GIMME, A_NULL);
    
    return x;
}

static void instance_free (t_pdinstance *x)
{
    PD_ASSERT (x->pd_roots       == NULL);
    PD_ASSERT (x->pd_polling     == NULL);
    PD_ASSERT (x->pd_autorelease == NULL);
    PD_ASSERT (x->pd_paint       == NULL);
    PD_ASSERT (x->pd_pending     == NULL);
    
    PD_ASSERT (buffer_getSize (x->pd_pool) == x->pd_poolCount);
    
    clock_free (x->pd_stop);
    
    buffer_free (x->pd_pool);
    register_free (x->pd_register);
    clocks_free (x->pd_clocks);
    
    class_free (x->pd_canvasMaker);
    class_free (x->pd_objectMaker);
    
    PD_ASSERT (x->pd_stack.s_stackIndex == 0);
    
    if (x->pd_environment.env_argv) { PD_MEMORY_FREE (x->pd_environment.env_argv); }
    
    PD_MEMORY_FREE (x->pd_stack.s_stack);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
