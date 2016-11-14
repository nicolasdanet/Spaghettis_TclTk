
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance     *pd_this;
extern t_class          *canvas_class;
extern t_class          *vinlet_class; 
extern t_class          *voutlet_class;
extern t_class          *block_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_dspcontext     *ugen_context;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int ugen_dspPhase;                       /* Shared. */
static int ugen_buildIdentifier;                /* Shared. */

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
    int                     dc_isTopLevel;
    t_ugenbox               *dc_ugens;
    struct _dspcontext      *dc_parentContext;
    t_signal                **dc_ioSignals;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ugen_dspInitialize (void)
{
    ugen_dspRelease();
    
    PD_ASSERT (ugen_context == NULL);
    
    pd_this->pd_dspChainSize = 1;
    pd_this->pd_dspChain     = (t_int *)PD_MEMORY_GET (pd_this->pd_dspChainSize * sizeof (t_int));
    pd_this->pd_dspChain[0]  = (t_int)dsp_done;
    
    ugen_buildIdentifier++;
}

void ugen_dspTick (void)
{
    if (pd_this->pd_dspChain) {
    //
    t_int *t = pd_this->pd_dspChain; while (t) { t = (*(t_perform)(*t))(t); } ugen_dspPhase++;
    //
    }
}

void ugen_dspRelease (void)
{
    if (pd_this->pd_dspChain) {
    //
    PD_MEMORY_FREE (pd_this->pd_dspChain); pd_this->pd_dspChain = NULL;
    //
    }
    
    signal_clean();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int ugen_getBuildIdentifier (void)
{
    return ugen_buildIdentifier;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_ugenbox *ugen_graphFetchUgen (t_dspcontext *context, t_object *o)
{
    t_ugenbox *u = NULL; for (u = context->dc_ugens; u && u->u_owner != o; u = u->u_next); return u;
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

static void ugen_graphCreateMissingSignalsForOutlets (t_dspcontext *context,
    int blockSize,
    t_float sampleRate, 
    int zeroed)
{
    if (context->dc_ioSignals) {
    //
    int i;
    t_signal **t = context->dc_ioSignals + context->dc_numberOfInlets;
    
    for (i = 0; i < context->dc_numberOfOutlets; i++) {
    //
    t_signal *s = (*t);
    
    if (s->s_isVectorBorrowed && !s->s_borrowedFrom) {
        t_signal *o = signal_new (blockSize, sampleRate);
        signal_borrow (s, o);
        s->s_count++;
        if (zeroed) {
            dsp_addZeroPerform (o->s_vector, o->s_vectorSize);
        }
    }
    
    t++;
    //
    }
    //
    }
}

static void ugen_graphDspProlog (t_dspcontext *context, 
    int switchable,
    int reblocked, 
    int blockSize,
    int period, 
    int frequency,
    int downsample,
    int upsample)
{
    t_signal **i = context->dc_ioSignals;
    t_signal **o = i ? i + context->dc_numberOfInlets : NULL;
    t_ugenbox *u = NULL;
    
    for (u = context->dc_ugens; u; u = u->u_next) {
    //
    t_object *object = u->u_owner;

    if (pd_class (object) == vinlet_class) {
    
        vinlet_dspProlog ((t_vinlet *)object, 
            i,
            switchable,
            reblocked,
            blockSize,
            ugen_dspPhase,
            period,
            frequency,
            downsample,
            upsample);
    }
    
    if (pd_class (object) == voutlet_class) {
    
        voutlet_dspProlog ((t_voutlet *)object, 
            o,
            switchable,
            reblocked,
            blockSize,
            ugen_dspPhase,
            period,
            frequency,
            downsample,
            upsample);
    }
    //
    }
}

static void ugen_graphDspEpilog (t_dspcontext *context,
    int switchable,
    int reblocked, 
    int blockSize,
    int period, 
    int frequency,
    int downsample,
    int upsample)
{
    t_signal **i = context->dc_ioSignals;
    t_signal **o = i ? i + context->dc_numberOfInlets : NULL;
    t_ugenbox *u = NULL;
        
    for (u = context->dc_ugens; u; u = u->u_next) {
    //
    t_object *object = u->u_owner;
    
    if (pd_class (object) == voutlet_class) {

        voutlet_dspEpilog ((t_voutlet *)object,
            o,
            switchable,
            reblocked,
            blockSize,
            ugen_dspPhase,
            period,
            frequency,
            downsample,
            upsample);
    }
    //
    }
}

static void ugen_graphDspMainRecursive (t_dspcontext *context, int switchable, int reblocked, t_ugenbox *u)
{
    t_sigoutlet *uout;
    t_siginlet *uin;

    t_signal **signals = NULL;
    t_signal **p = NULL;
    int i;
        
    int doNotCreateSignals = (pd_class (u->u_owner) == canvas_class);
    int doNotFreeSignals   = (pd_class (u->u_owner) == canvas_class);
    
    if (pd_class (u->u_owner) == vinlet_class)  { doNotCreateSignals = !(reblocked); }
    if (pd_class (u->u_owner) == voutlet_class) { doNotFreeSignals   = !(switchable || reblocked); }
        
    for (i = 0; i < u->u_inSize; i++) {
    //
    if (u->u_in[i].i_numberOfConnections == 0) {
    //
    t_signal *s = signal_new (context->dc_blockSize, context->dc_sampleRate);
    t_float *f  = object_getSignalValueAtIndex (u->u_owner, i);
    
    if (f) { dsp_add_scalarcopy (f, s->s_vector, s->s_vectorSize); }
    else {
        dsp_addZeroPerform (s->s_vector, s->s_vectorSize);
    }
    
    u->u_in[i].i_signal = s;
    s->s_count = 1;
    //
    }
    //
    }
    
    signals = (t_signal **)PD_MEMORY_GET ((u->u_inSize + u->u_outSize) * sizeof (t_signal *));
    
    p = signals;
    
    for (i = 0; i < u->u_inSize; i++) {
        int newrefcount;
        *p = u->u_in[i].i_signal;
        newrefcount = --(*p)->s_count;

        if (doNotFreeSignals)
            (*p)->s_count++;
        else if (!newrefcount)
            signal_free(*p);
        
        p++;
    }
    
    for (p = signals + u->u_inSize, uout = u->u_out, i = u->u_outSize; i--; p++, uout++)
    {
        if (doNotCreateSignals)
        {
            *p = uout->o_signal =
                signal_new(0, context->dc_sampleRate);
        }
        else
            *p = uout->o_signal = signal_new(context->dc_blockSize, context->dc_sampleRate);
        (*p)->s_count = uout->o_numberOfConnections;
    }
        /* now call the DSP scheduling routine for the ugen.  This
        routine must fill in "borrowed" signal outputs in case it's either
        a subcanvas or a signal inlet. */
        
    mess1(&u->u_owner->te_g.g_pd, sym_dsp, signals);
    
        /* if any output signals aren't connected to anyone, free them
        now; otherwise they'll either get freed when the reference count
        goes back to zero, or even later as explained above. */

    for (p = signals + u->u_inSize, uout = u->u_out, i = u->u_outSize; i--; p++, uout++)
    {
        if (!(*p)->s_count)
            signal_free(*p);
    }

        /* pass it on and trip anyone whose last inlet was filled */
    for (uout = u->u_out, i = u->u_outSize; i--; uout++)
    {
        t_signal *s1;
        t_signal *s2;
        t_signal *s3;
        s1 = uout->o_signal;
        t_sigoutconnect *oc;
        for (oc = uout->o_connections; oc; oc = oc->oc_next)
        {
            t_ugenbox *u2;
            u2 = oc->oc_to;
            uin = &u2->u_in[oc->oc_index];
                /* if there's already someone here, sum the two */
            if (s2 = uin->i_signal)
            {
                s1->s_count--;
                s2->s_count--;
                //if (!signal_compatible(s1, s2))
                if (!(s1->s_vectorSize == s2->s_vectorSize && s1->s_sampleRate == s2->s_sampleRate))
                {
                    post_error ("%s: incompatible signal inputs",
                        class_getNameAsString(u->u_owner->te_g.g_pd));
                    return;
                }
                s3 = signal_new(s1->s_vectorSize, s1->s_sampleRate);
                //s3 = signal_newlike(s1);
                dsp_addPlusPerform(s1->s_vector, s2->s_vector, s3->s_vector, s1->s_vectorSize);
                uin->i_signal = s3;
                s3->s_count = 1;
                if (!s1->s_count) signal_free(s1);
                if (!s2->s_count) signal_free(s2);
            }
            else uin->i_signal = s1;
            uin->i_numberConnected++;
                /* if we didn't fill this inlet don't bother yet */
            if (uin->i_numberConnected < uin->i_numberOfConnections)
                goto notyet;
                /* if there's more than one, check them all */
            if (u2->u_inSize > 1)
            {
                int n;
                for (uin = u2->u_in, n = u2->u_inSize; n--; uin++)
                    if (uin->i_numberConnected < uin->i_numberOfConnections) goto notyet;
            }
                /* so now we can schedule the ugen.  */
            ugen_graphDspMainRecursive(context, switchable, reblocked, u2);
        notyet: ;
        }
    }
    
    PD_MEMORY_FREE (signals);
    
    u->u_done = 1;
}

/* Topological sort. */

static void ugen_graphDspMain (t_dspcontext *context,
    int switchable,
    int reblocked,
    int parentBlockSize,
    t_float parentSampleRate)
{
    t_ugenbox *u = NULL;
    
    /* Start from ugens with no connection in. */
    
    for (u = context->dc_ugens; u; u = u->u_next) {

        int i, k = (u->u_done == 0);
        
        if (k) {
            for (i = 0; i < u->u_inSize; i++) {     
                if (u->u_in[i].i_numberOfConnections != 0) { k = 0; break; }
            }
            if (k) { ugen_graphDspMainRecursive (context, switchable, reblocked, u); }
        }
    }

    /* Check for DSP loops. */
    
    for (u = context->dc_ugens; u; u = u->u_next) {

        if (!u->u_done) {
            error_dspLoop();
            ugen_graphCreateMissingSignalsForOutlets (context, parentBlockSize, parentSampleRate, 1);
            break;
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

/* Note that an abstraction can be opened as a toplevel patch. */

t_dspcontext *ugen_graphStart (int isTopLevel, t_signal **sp, int m, int n)
{
    t_dspcontext *context = (t_dspcontext *)PD_MEMORY_GET (sizeof (t_dspcontext));

    context->dc_numberOfInlets  = isTopLevel ? 0 : m;
    context->dc_numberOfOutlets = isTopLevel ? 0 : n;
    context->dc_isTopLevel      = isTopLevel;
    context->dc_ugens           = NULL;
    context->dc_parentContext   = ugen_context;
    context->dc_ioSignals       = sp;
        
    ugen_context = context;
    
    return context;
}

void ugen_graphAdd (t_dspcontext *context, t_object *o)
{
    t_ugenbox *x = (t_ugenbox *)PD_MEMORY_GET (sizeof (t_ugenbox));

    x->u_inSize  = object_numberOfSignalInlets (o);
    x->u_outSize = object_numberOfSignalOutlets (o);
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
    
    m = object_indexAsSignalOutlet (o1, m);
    n = object_indexAsSignalInlet (o2, n);
    
    if (!u1 || !u2 || n < 0) { PD_BUG; }
    else if (m < 0 || m >= u1->u_outSize || n >= u2->u_inSize) { PD_BUG; }
    else {
        ugen_graphConnectUgens (u1, m, u2, n);
    }
}

/* Period is roughly the number of parent's blocks required to filled the child. */
/* Frequency is roughly the number of child's iterations required to equaled the parent. */
/* Note that it is respectively divided and multiplied in case of overlap. */

void ugen_graphClose (t_dspcontext *context)
{
    t_dspcontext *parentContext = context->dc_parentContext;
    t_float parentSampleRate    = parentContext ? parentContext->dc_sampleRate : audio_getSampleRate();
    int parentBlockSize         = parentContext ? parentContext->dc_blockSize  : AUDIO_DEFAULT_BLOCKSIZE;
    t_float sampleRate          = parentSampleRate;
    int blockSize               = parentBlockSize;
    int period                  = 1;
    int frequency               = 1;
    int downsample              = 1;
    int upsample                = 1;
    int switchable              = 0;
    int reblocked               = parentContext ? 0 : 1;
    int chainBegin;
    int chainEnd;
    int chainEpilog; 
    
    t_block *block = ugen_graphGetBlockIfContainsAny (context);
        
    if (block) {
    //
    block_getParameters (block,
        &switchable,
        &reblocked,
        &blockSize,
        &sampleRate,
        &period,
        &frequency,
        &downsample,
        &upsample,
        ugen_dspPhase,
        parentBlockSize,
        parentSampleRate);
    //
    }

    context->dc_sampleRate = sampleRate;
    context->dc_blockSize  = blockSize;
    
    if (switchable || reblocked) {
    //
    ugen_graphCreateMissingSignalsForOutlets (context, parentBlockSize, parentSampleRate, 0);
    //
    }
    
    ugen_graphDspProlog (context,
        switchable,
        reblocked,
        blockSize,
        period,
        frequency,
        downsample,
        upsample);
    
    chainBegin = pd_this->pd_dspChainSize;
    
    if (block && (switchable || reblocked)) { dsp_add (block_performProlog, 1, block); }   

    ugen_graphDspMain (context, switchable, reblocked, parentBlockSize, parentSampleRate);

    if (block && (switchable || reblocked)) { dsp_add (block_performEpilog, 1, block); }
    
    chainEnd = pd_this->pd_dspChainSize;

    ugen_graphDspEpilog (context,
        switchable,
        reblocked,
        blockSize,
        period,
        frequency,
        downsample,
        upsample);

    chainEpilog = pd_this->pd_dspChainSize;
    
    if (block) { block_setPerformLength (block, chainEnd - chainBegin, chainEpilog - chainEnd); }

    ugen_graphDelete (context);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
