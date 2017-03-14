
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_instance_h_
#define __m_instance_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _stackelement {
    t_glist         *g_context;
    t_symbol        *g_abstraction;
    } t_stackelement;

typedef struct _stack       {
    int             stack_index;
    t_stackelement  *stack_array;
    t_glist         *stack_popped;
    t_glist         *stack_cached;
    } t_stack;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    t_systime       pd_systime;
    t_stack         pd_stack;
    int             pd_dspState;
    int             pd_dspChainSize;
    int             pd_loadingExternal;
    t_symbol        *pd_loadingAbstraction;
    t_int           *pd_dspChain;
    t_clock         *pd_clocks;
    t_signal        *pd_signals;
    t_glist         *pd_roots;
    t_clock         *pd_polling;
    t_clock         *pd_autorelease;
    t_pd            *pd_newest;
    t_class         *pd_objectMaker;
    t_class         *pd_canvasMaker;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INSTANCE_STACK_SIZE     1024

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define dsp_add     instance_dspChainAppend

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline t_pdinstance *instance_get (void)
{
    return pd_this;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* The #N symbol is bound to the patcher factory. */
/* The #X symbol is bound to the context currently active. */
/* The #A symbol can be used to serialize things. */

static inline void instance_setBoundN (t_pd *x)
{
    s__N.s_thing = x;
}

static inline void instance_setBoundA (t_pd *x)
{
    s__A.s_thing = x;
}

static inline void instance_setBoundX (t_pd *x)
{
    s__X.s_thing = x;
}

static inline t_pd *instance_getBoundX (void)
{
    return s__X.s_thing;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    instance_rootsAdd                       (t_glist *glist);
void    instance_rootsRemove                    (t_glist *glist);
void    instance_rootsFreeAll                   (void);

void    instance_dspStart                       (void);
void    instance_dspStop                        (void);
void    instance_dspChainInitialize             (void);
void    instance_dspChainRelease                (void);
void    instance_dspChainAppend                 (t_perform f, int n, ...);

void    instance_signalAdd                      (t_signal *s);
void    instance_signalFreeAll                  (void);

void    instance_clockAdd                       (t_clock *c);
void    instance_clockUnset                     (t_clock *c);
void    instance_clockTick                      (t_systime systime);

void    instance_pollingRun                     (void);
void    instance_pollingStop                    (void);
void    instance_pollingRegister                (t_pd *x);
void    instance_pollingUnregister              (t_pd *x);

void    instance_autoreleaseRun                 (void);
void    instance_autoreleaseStop                (void);
void    instance_autoreleaseRegister            (t_pd *x);
void    instance_autoreleaseProceed             (t_pd *x);

void    instance_destroyAllScalarsByTemplate    (t_template *tmpl);

void    instance_loadAbstraction                (t_symbol *s, int argc, t_atom *argv);
void    instance_loadPatch                      (t_symbol *name, t_symbol *directory);
void    instance_loadInvisible                  (t_symbol *name, t_symbol *directory, char *s);

void    instance_stackPush                      (t_glist *glist);
void    instance_stackPop                       (t_glist *glist);
void    instance_stackEval                      (t_glist *glist, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void instance_contextSetCurrent (t_glist *glist)
{
    return instance_setBoundX (cast_pd (glist));
}

static inline t_glist *instance_contextGetCurrent (void)
{
    return cast_glist (instance_getBoundX());
}

static inline void instance_contextStore (void)
{
    instance_get()->pd_stack.stack_cached = instance_contextGetCurrent();
}

static inline void instance_contextRestore (void)
{
    instance_contextSetCurrent (instance_get()->pd_stack.stack_cached);
    
    instance_get()->pd_stack.stack_cached = NULL;
}

static inline int instance_contextHasChanged (void)
{
    return (instance_get()->pd_stack.stack_cached != instance_contextGetCurrent());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int instance_isMakerObject (t_pd *x)
{
    return (x == &(instance_get()->pd_objectMaker));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_pd *instance_getMakerObject (void)
{
    return &(instance_get()->pd_objectMaker);
}

static inline t_class *instance_getMakerObjectClass (void)
{
    return (instance_get()->pd_objectMaker);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_systime instance_getLogicalTime (void)
{
    return instance_get()->pd_systime;
}

static inline int instance_getDspState (void)
{
    return instance_get()->pd_dspState;
}

static inline int instance_getDspChainSize (void)
{
    return instance_get()->pd_dspChainSize;
}

static inline t_int *instance_getDspChain (void)
{
    return instance_get()->pd_dspChain;
}

static inline t_glist *instance_getRoots (void)
{
    return instance_get()->pd_roots;
}

static inline t_pd *instance_getNewestObject (void)
{
    return instance_get()->pd_newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void instance_setDspState (int n)
{
    instance_get()->pd_dspState = (n != 0);
}

static inline void instance_setNewestObject (t_pd *x)
{
    instance_get()->pd_newest = x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_instance_h_
