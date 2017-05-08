
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_dspcontext     *ugen_context;                  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_phase          ugen_dspPhase;                  /* Static. */
static int              ugen_buildIdentifier;           /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sigoutconnect {
    int                     oc_index;
    struct _ugenbox         *oc_to;
    struct _sigoutconnect   *oc_next;
    } t_sigoutconnect;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sigoutlet {
    int                     o_numberOfConnections;
    t_sigoutconnect         *o_connections;
    t_signal                *o_signal;
    } t_sigoutlet;
    
typedef struct _siginlet {
    int                     i_numberOfConnections;
    int                     i_numberConnected;
    t_signal                *i_signal;
    } t_siginlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _ugenbox {
    int                     u_done;
    int                     u_phase;
    int                     u_inSize;
    int                     u_outSize;
    t_siginlet              *u_in;
    t_sigoutlet             *u_out;
    t_object                *u_owner;
    struct _ugenbox         *u_next;
    } t_ugenbox;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _dspcontext {
    int                     dc_numberOfInlets;
    int                     dc_numberOfOutlets;
    t_float                 dc_sampleRate;
    int                     dc_blockSize;
    t_ugenbox               *dc_ugens;
    struct _dspcontext      *dc_parentContext;
    t_signal                **dc_signals;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_dsp)       (void *x, void *signals);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define UGEN_DSP(x, s, a)   ((*(t_dsp)class_getMethod (pd_class (x), (s)))((x), (a)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void ugen_graphMainRecursive (t_dspcontext *, t_ugenbox *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ugen_dspInitialize (void)
{
    ugen_dspRelease();
    
    PD_ASSERT (ugen_context == NULL);
    
    instance_dspChainInitialize();
    
    ugen_buildIdentifier++;
}

void ugen_dspTick (void)
{
    t_int *t = instance_getDspChain();
    
    if (t) { while (t) { t = (*(t_perform)(*t))(t); } ugen_dspPhase++; }
}

void ugen_dspRelease (void)
{
    instance_dspChainRelease();
    instance_signalFreeAll();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int ugen_getBuildIdentifier (void)
{
    return ugen_buildIdentifier;
}

t_phase ugen_getPhase (void)
{
    return ugen_dspPhase;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_ugenbox *ugen_graphFetchUgen (t_dspcontext *context, t_object *o)
{
    t_ugenbox *u = NULL; for (u = context->dc_ugens; u && u->u_owner != o; u = u->u_next) { }; return u;
}

static void ugen_graphConnectUgens (t_ugenbox *u1, int m, t_ugenbox *u2, int n)
{
    t_sigoutlet *o = u1->u_out + m;
    t_siginlet  *i = u2->u_in  + n;
    
    t_sigoutconnect *c = (t_sigoutconnect *)PD_MEMORY_GET (sizeof (t_sigoutconnect));
    
    c->oc_to    = u2;
    c->oc_index = n;
    c->oc_next  = o->o_connections;
    
    o->o_connections = c;

    o->o_numberOfConnections++;
    i->i_numberOfConnections++;
}

static t_block *ugen_graphGetBlockIfContainsAny (t_dspcontext *context)
{
    t_block *block = NULL;
    t_ugenbox *u = NULL;
    
    for (u = context->dc_ugens; u; u = u->u_next) {

        t_object *o = u->u_owner;
        
        if (pd_class (o) == block_class) {
            if (block) { error_unexpected (sym_dsp, sym_block__tilde__); }
            else {
                block = (t_block *)o;
            }
        }
    }
    
    return block;
}

static int ugen_graphIsUgenReady (t_ugenbox *u)
{
    if (u->u_done == 1) { return 0; }
    else {
        int i;
        for (i = 0; i < u->u_inSize; i++) {
            if (u->u_in[i].i_numberConnected < u->u_in[i].i_numberOfConnections) {
                return 0;
            }
        }
    }
    
    /* All the parents of the ugen are done. */
    
    return 1;
}

static void ugen_graphProlog (t_dspcontext *context, t_blockproperties *p)
{
    t_signal **i = context->dc_signals;
    t_signal **o = i ? i + context->dc_numberOfInlets : NULL;
    t_ugenbox *u = NULL;
    
    for (u = context->dc_ugens; u; u = u->u_next) {
        t_object *object = u->u_owner;
        if (pd_class (object) == vinlet_class)  { vinlet_dspProlog ((t_vinlet *)object, i, p);   }
        if (pd_class (object) == voutlet_class) { voutlet_dspProlog ((t_voutlet *)object, o, p); }
    }
}

static void ugen_graphEpilog (t_dspcontext *context, t_blockproperties *p)
{
    t_signal **i = context->dc_signals;
    t_signal **o = i ? i + context->dc_numberOfInlets : NULL;
    t_ugenbox *u = NULL;
        
    for (u = context->dc_ugens; u; u = u->u_next) {
        t_object *object = u->u_owner;
        if (pd_class (object) == voutlet_class) { voutlet_dspEpilog ((t_voutlet *)object, o, p); }
    }
}

static void ugen_graphMainRecursiveChild (t_dspcontext *context, t_ugenbox *u)
{
    int i;
    
    for (i = 0; i < u->u_outSize; i++) {
    //
    t_signal *parentSignal = u->u_out[i].o_signal;
    
    t_sigoutconnect *c = NULL;
    
    for (c = u->u_out[i].o_connections; c; c = c->oc_next) {
    //
    t_ugenbox  *child       = c->oc_to;
    t_siginlet *childInlet  = &child->u_in[c->oc_index];
    t_signal   *childSignal = childInlet->i_signal;
    
    /* Use out signal in place of in signal. */
    
    if (childSignal == NULL) { childInlet->i_signal = parentSignal; }
    else {
    //
    t_signal *s = signal_new (parentSignal->s_vectorSize, parentSignal->s_sampleRate);   
         
    PD_ASSERT (signal_isCompatibleWith (parentSignal, childSignal));
    PD_ABORT (!signal_isCompatibleWith (parentSignal, childSignal));
    
    /* Accumulate content of in signals. */
    
    dsp_addPlusPerform (parentSignal->s_vector,
        childSignal->s_vector,
        s->s_vector,
        parentSignal->s_vectorSize);
    
    childInlet->i_signal = s;
    //
    }

    childInlet->i_numberConnected++;
    
    if (ugen_graphIsUgenReady (child)) { 
        ugen_graphMainRecursive (context, child); 
    }
    //
    }
    //
    }
}

static void ugen_graphMainRecursiveEmptyInlets (t_dspcontext *context, t_ugenbox *u)
{
    int i;
    
    for (i = 0; i < u->u_inSize; i++) {
    //
    if (u->u_in[i].i_numberOfConnections == 0) {
    //
    t_signal *s = signal_new (context->dc_blockSize, context->dc_sampleRate);
    t_float *f  = object_getSignalValueAtIndex (u->u_owner, i);
    
    if (f) { dsp_addScalarPerform (f, s->s_vector, s->s_vectorSize); }
    else {
        dsp_addZeroPerform (s->s_vector, s->s_vectorSize);
    }
    
    u->u_in[i].i_signal = s;
    //
    }
    //
    }
}

static void ugen_graphMainRecursive (t_dspcontext *context, t_ugenbox *u)
{
    t_signal **signals = (t_signal **)PD_MEMORY_GET ((u->u_inSize + u->u_outSize) * sizeof (t_signal *));
    t_signal **p = signals;
    int i;
    
    /* Create signals for inlets without any connections. */
    /* At this point all signals for inlets are created. */
    
    ugen_graphMainRecursiveEmptyInlets (context, u);
    
    for (i = 0; i < u->u_inSize; i++)  { PD_ASSERT (u->u_in[i].i_signal != NULL); }
    for (i = 0; i < u->u_inSize; i++)  { *p++ = u->u_in[i].i_signal; }
    
    /* Create all signals for outlets. */
    
    for (i = 0; i < u->u_outSize; i++) {
        *p++ = u->u_out[i].o_signal = signal_new (context->dc_blockSize, context->dc_sampleRate);
    }
    
    /* Call the ugen dsp method. */
    
    UGEN_DSP (u->u_owner, sym_dsp, signals);

    u->u_done = 1; PD_MEMORY_FREE (signals);
    
    /* Propagate to childs (depth-first). */
    
    ugen_graphMainRecursiveChild (context, u);
}

/* Topological sort. */

static void ugen_graphMain (t_dspcontext *context)
{
    t_ugenbox *u = NULL;
    
    /* Will start from ugens with no connection in. */
    
    for (u = context->dc_ugens; u; u = u->u_next) {
        if (ugen_graphIsUgenReady (u)) { ugen_graphMainRecursive (context, u); }
    }

    /* Check for DSP loops. */
    
    for (u = context->dc_ugens; u; u = u->u_next) {
        if (!u->u_done) {
            error_dspLoop(); break; 
        }
    }
}

static void ugen_graphDelete (t_dspcontext *context)
{
    while (context->dc_ugens) {
    //
    t_ugenbox *u = context->dc_ugens;
    t_sigoutlet *o = u->u_out;
    int i;
    
    for (i = 0; i < u->u_outSize; i++) {
    //
    t_sigoutconnect *c = o->o_connections;
    
    while (c) {
        t_sigoutconnect *t = c->oc_next;
        PD_MEMORY_FREE (c);
        c = t;
    }
    
    o++;
    //
    }
    
    context->dc_ugens = u->u_next;
    
    PD_MEMORY_FREE (u->u_out);
    PD_MEMORY_FREE (u->u_in);
    PD_MEMORY_FREE (u);
    //
    }
    
    PD_ASSERT (ugen_context == context);
    
    ugen_context = context->dc_parentContext;
    
    PD_MEMORY_FREE (context);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_dspcontext *ugen_graphStart (int isTopLevel, t_signal **sp, int m, int n)
{
    t_dspcontext *context = (t_dspcontext *)PD_MEMORY_GET (sizeof (t_dspcontext));

    PD_ASSERT (!isTopLevel || ugen_context == NULL);
    
    /* Note that an abstraction can be opened as a toplevel patch. */
    
    context->dc_numberOfInlets  = isTopLevel ? 0 : m;
    context->dc_numberOfOutlets = isTopLevel ? 0 : n;
    context->dc_ugens           = NULL;
    context->dc_parentContext   = ugen_context;
    context->dc_signals         = sp;

    ugen_context = context;
    
    /* Just in case. */
    
    #if PD_WITH_DEBUG
    
        if (!isTopLevel) {
        //
        int i;
        for (i = 0; i < (m + n); i++) {
        //
        t_signal *s = *(sp + i);
        PD_ASSERT (s->s_vectorSize == context->dc_parentContext->dc_blockSize);
        PD_ASSERT (s->s_sampleRate == context->dc_parentContext->dc_sampleRate);
        //
        }
        //
        }
    
    #endif  // PD_WITH_DEBUG
    
    return context;
}

void ugen_graphAdd (t_dspcontext *context, t_object *o)
{
    t_ugenbox *x = (t_ugenbox *)PD_MEMORY_GET (sizeof (t_ugenbox));

    x->u_inSize  = object_getNumberOfSignalInlets (o);
    x->u_outSize = object_getNumberOfSignalOutlets (o);
    x->u_in      = PD_MEMORY_GET (x->u_inSize * sizeof (t_siginlet));
    x->u_out     = PD_MEMORY_GET (x->u_outSize * sizeof (t_sigoutlet));
    x->u_owner   = o;
    x->u_next    = context->dc_ugens;
    
    context->dc_ugens = x;
}

void ugen_graphConnect (t_dspcontext *context, t_object *o1, int m, t_object *o2, int n)
{
    t_ugenbox *u1 = ugen_graphFetchUgen (context, o1);
    t_ugenbox *u2 = ugen_graphFetchUgen (context, o2);
    
    m = object_getSignalIndexOfOutlet (o1, m);
    n = object_getSignalIndexOfInlet (o2, n);
    
    if (!u1 || !u2 || n < 0) { PD_BUG; }
    else if (m < 0 || m >= u1->u_outSize || n >= u2->u_inSize) { PD_BUG; }
    else {
        ugen_graphConnectUgens (u1, m, u2, n);
    }
}

/* Period (submultiple) is roughly the number of parent's blocks used to filled the child. */
/* Frequency (supermultiple) is roughly the number of child's iterations used to fill the parent. */
/* Note that it is respectively divided and multiplied in case of overlap. */
/* In that case the period represents the hop size. */

void ugen_graphClose (t_dspcontext *context)
{
    t_dspcontext *parentContext = context->dc_parentContext;
    t_float parentSampleRate    = parentContext ? parentContext->dc_sampleRate : audio_getSampleRate();
    int parentBlockSize         = parentContext ? parentContext->dc_blockSize  : AUDIO_DEFAULT_BLOCKSIZE;
    int chainBegin;
    int chainEnd;
    int chainEpilog; 
    
    t_block *block = ugen_graphGetBlockIfContainsAny (context);
    
    t_blockproperties p;
    
    p.bp_switchable = 0;
    p.bp_reblocked  = parentContext ? 0 : 1;
    p.bp_blockSize  = parentBlockSize;
    p.bp_sampleRate = parentSampleRate;
    p.bp_period     = 1;
    p.bp_frequency  = 1;
    p.bp_downsample = 1;
    p.bp_upsample   = 1;
    
    if (block) { block_getProperties (block, parentBlockSize, parentSampleRate, &p); }

    context->dc_sampleRate = p.bp_sampleRate;
    context->dc_blockSize  = p.bp_blockSize;
    
    ugen_graphProlog (context, &p);
    
    chainBegin = instance_getDspChainSize();
    
    if (block && (p.bp_switchable || p.bp_reblocked)) { dsp_add (block_performProlog, 1, block); }   

    ugen_graphMain (context);

    if (block && (p.bp_switchable || p.bp_reblocked)) { dsp_add (block_performEpilog, 1, block); }
    
    chainEnd = instance_getDspChainSize();

    ugen_graphEpilog (context, &p);

    chainEpilog = instance_getDspChainSize();
    
    if (block) { block_setPerformsLength (block, chainEnd - chainBegin, chainEpilog - chainEnd); }

    ugen_graphDelete (context);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
