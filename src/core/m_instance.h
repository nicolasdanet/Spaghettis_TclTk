
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_instance_h_
#define __m_instance_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* In the future, what should belongs to it and what should be kept global? */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _stackelement {
    t_glist         *s_context;
    t_symbol        *s_loadedAbstraction;
    } t_stackelement;

typedef struct _stack {
    int             s_stackIndex;
    t_stackelement  *s_stack;
    t_glist         *s_contextCached;
    t_glist         *s_contextPopped;
    } t_stack;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _position {
    int             p_x;
    int             p_y;
    t_glist         *p_glist;
    } t_position;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _pdinstance {
    t_stack         pd_stack;
    t_environment   pd_environment;
    t_position      pd_locate;
    uint64_t        pd_pollingCount;
    uint64_t        pd_autoreleaseCount;
    t_int32Atomic   pd_clocksCount;
    int             pd_overflow;
    int             pd_overflowCount;
    int             pd_isLoadingExternal;
    int             pd_isUndoRecursive;
    int             pd_hasPending;
    t_int32Atomic   pd_chainRetain;
    t_pointerAtomic pd_chain;
    t_chain         *pd_build;
    t_symbol        *pd_loadingAbstraction;
    t_dspcontext    *pd_ugenContext;
    t_clocks        *pd_clocks;
    t_glist         *pd_roots;
    t_clock         *pd_polling;
    t_clock         *pd_autorelease;
    t_gobj          *pd_pending;
    t_pd            *pd_newest;
    t_class         *pd_objectMaker;
    t_class         *pd_canvasMaker;
    t_register      *pd_register;
    t_dspthread     *pd_dsp;
    } t_pdinstance;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define INSTANCE_STACK          1024        /* Arbitrary. */
#define INSTANCE_OVERFLOW       1000        /* Arbitrary. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline t_pdinstance *instance_get (void)
{
    return pd_this;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* The #N symbol is bound to the patch factory. */
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
// MARK: -

void    instance_rootsAdd                       (t_glist *glist);
void    instance_rootsRemove                    (t_glist *glist);
void    instance_rootsFreeAll                   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    instance_registerAdd                    (t_gobj *o, t_glist *owner);
t_error instance_registerRemove                 (t_gobj *o);
void    instance_registerRename                 (t_gobj *o, t_id newUnique);

int     instance_registerContains               (t_id u);

t_gobj  *instance_registerGetObject             (t_id u);
t_glist *instance_registerGetOwner              (t_id u);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error instance_dspCreate                      (void);
void    instance_dspStart                       (void);
void    instance_dspStop                        (void);
void    instance_dspClean                       (void);
void    instance_dspFree                        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_chain *instance_chainGetCurrent               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    instance_clocksClean                    (void);
void    instance_clocksAdd                      (t_clock *c);
void    instance_clocksRemove                   (t_clock *c);
void    instance_clocksTick                     (t_systime systime);

void    instance_pollingRun                     (void);
void    instance_pollingStop                    (void);
void    instance_pollingRegister                (t_pd *x);
void    instance_pollingUnregister              (t_pd *x);

void    instance_autoreleaseRun                 (void);
void    instance_autoreleaseStop                (void);
void    instance_autoreleaseRegister            (t_pd *x);
void    instance_autoreleaseProceed             (t_pd *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    instance_deselectAllObjects             (void);
void    instance_destroyAllScalarsByTemplate    (t_template *tmpl);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    instance_patchNew                       (t_symbol *name, t_symbol *directory);
t_error instance_patchOpen                      (t_symbol *name, t_symbol *directory);

void    instance_loadAbstraction                (t_symbol *name, int argc, t_atom *argv);
void    instance_loadInvisible                  (t_symbol *name, const char *s);
void    instance_loadSnippet                    (t_glist *glist, t_buffer *b);

void    instance_stackPush                      (t_glist *glist);
void    instance_stackPop                       (t_glist *glist);
void    instance_stackPopPatch                  (t_glist *glist, int visible);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_environment   *instance_environmentFetchIfAny (void);

void    instance_environmentSetFile             (t_symbol *name, t_symbol *directory);
void    instance_environmentSetArguments        (int argc, t_atom *argv);
void    instance_environmentResetFile           (void);
void    instance_environmentResetArguments      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    instance_setDefaultCoordinates          (t_glist *glist, int a, int b);
int     instance_getDefaultX                    (t_glist *glist);
int     instance_getDefaultY                    (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error instance_overflowPush                   (void);
void    instance_overflowPop                    (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_gobj  *instance_pendingFetch                  (t_gobj *y);

int     instance_pendingRequired                (t_gobj *y);
void    instance_pendingAdd                     (t_gobj *y);
void    instance_pendingRelease                 (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void instance_contextSetCurrent (t_glist *glist)
{
    return instance_setBoundX (cast_pd (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void instance_ugenSetContext (t_dspcontext *context)
{
    instance_get()->pd_ugenContext = context;
}

static inline t_dspcontext *instance_ugenGetContext (void)
{
    return instance_get()->pd_ugenContext;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_pd *instance_getMakerObject (void)
{
    return &(instance_get()->pd_objectMaker);
}

static inline t_class *instance_getMakerObjectClass (void)
{
    return (instance_get()->pd_objectMaker);
}

static inline int instance_isMakerObject (t_pd *x)
{
    return (x == instance_getMakerObject());
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static inline void instance_setNewestObject (t_pd *x)
{
    instance_get()->pd_newest = x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int instance_undoIsRecursive (void)
{
    return (instance_get()->pd_isUndoRecursive > 0);
}

static inline void instance_undoSetRecursive (void)
{
    instance_get()->pd_isUndoRecursive++;
}

static inline void instance_undoUnsetRecursive (void)
{
    instance_get()->pd_isUndoRecursive--;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int instance_hasPending (void)
{
    return (instance_get()->pd_hasPending != 0);
}

static inline void instance_pendingBegin (void)
{
    instance_get()->pd_hasPending++;
}

static inline void instance_pendingEnd (void)
{
    instance_get()->pd_hasPending--; if (instance_get()->pd_hasPending == 0) { instance_pendingRelease(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_instance_h_
