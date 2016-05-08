/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  These routines build a copy of the DSP portion of a graph, which is
    then sorted into a linear list of DSP operations which are added to
    the DSP duty cycle called by the scheduler.  Once that's been done,
    we delete the copy.  The DSP objects are represented by "ugenbox"
    structures which are parallel to the DSP objects in the graph and
    have vectors of siginlets and sigoutlets which record their
    interconnections.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"
#include <stdlib.h>
#include <stdarg.h>

#define MAXLOGSIG           32

extern t_pdinstance *pd_this;
extern t_class *canvas_class;

extern t_class *vinlet_class, *voutlet_class;

static int ugen_loud;

struct _vinlet;
struct _voutlet;

void vinlet_dspprolog(struct _vinlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample,  int reblock, int switched);
void voutlet_dspprolog(struct _voutlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched);
void voutlet_dspepilog(struct _voutlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched);

t_int *zero_perform(t_int *w)   /* zero out a vector */
{
    t_sample *out = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    while (n--) *out++ = 0; 
    return (w+3);
}

t_int *zero_perf8(t_int *w)
{
    t_sample *out = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    
    for (; n; n -= 8, out += 8)
    {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        out[3] = 0;
        out[4] = 0;
        out[5] = 0;
        out[6] = 0;
        out[7] = 0;
    }
    return (w+3);
}

void dsp_add_zero(t_sample *out, int n)
{
    if (n&7)
        dsp_add(zero_perform, 2, out, n);
    else        
        dsp_add(zero_perf8, 2, out, n);
}

/* ---------------------------- block~ ----------------------------- */

/* The "block~ object maintains the containing canvas's DSP computation,
calling it at a super- or sub-multiple of the containing canvas's
calling frequency.  The block~'s creation arguments specify block size
and overlap.  Block~ does no "dsp" computation in its own right, but it
adds prolog and epilog code before and after the canvas's unit generators.

A subcanvas need not have a block~ at all; if there's none, its
ugens are simply put on the list without any prolog or epilog code.

Block~ may be invoked as switch~, in which case it also acts to switch the
subcanvas on and off.  The overall order of scheduling for a subcanvas
is thus,

    inlet and outlet prologue code (1)
    block prologue (2)
    the objects in the subcanvas, including inlets and outlets
    block epilogue (2)
    outlet epilogue code (2)

where (1) means, "if reblocked" and  (2) means, "if reblocked or switched".

If we're reblocked, the inlet prolog and outlet epilog code takes care of
overlapping and buffering to deal with vector size changes.  If we're switched
but not reblocked, the inlet prolog is not needed, and the output epilog is
ONLY run when the block is switched off; in this case the epilog code simply
copies zeros to all signal outlets.
*/

static int dsp_phase;
static t_class *block_class;

typedef struct _block
{
    t_object x_obj;
    int x_vecsize;      /* size of audio signals in this block */
    int x_calcsize;     /* number of samples actually to compute */
    int x_overlap;
    int x_phase;        /* from 0 to period-1; when zero we run the block */
    int x_period;       /* submultiple of containing canvas */
    int x_frequency;    /* supermultiple of comtaining canvas */
    int x_count;        /* number of times parent block has called us */
    int x_chainonset;   /* beginning of code in DSP chain */
    int x_blocklength;  /* length of dspchain for this block */
    int x_epiloglength; /* length of epilog */
    char x_switched;    /* true if we're acting as a a switch */
    char x_switchon;    /* true if we're switched on */
    char x_reblock;     /* true if inlets and outlets are reblocking */
    int x_upsample;     /* upsampling-factor */
    int x_downsample;   /* downsampling-factor */
    int x_return;       /* stop right after this block (for one-shots) */
} t_block;

static void block_set(t_block *x, t_float fvecsize, t_float foverlap,
    t_float fupsample);

static void *block_new(t_float fvecsize, t_float foverlap,
                       t_float fupsample)
{
    t_block *x = (t_block *)pd_new(block_class);
    x->x_phase = 0;
    x->x_period = 1;
    x->x_frequency = 1;
    x->x_switched = 0;
    x->x_switchon = 1;
    block_set(x, fvecsize, foverlap, fupsample);
    return (x);
}

static void block_set(t_block *x, t_float fcalcsize, t_float foverlap,
    t_float fupsample)
{
    int upsample, downsample;
    int calcsize = fcalcsize;
    int overlap = foverlap;
    int dspstate = dsp_suspend();
    int vecsize;
    if (overlap < 1)
        overlap = 1;
    if (calcsize < 0)
        calcsize = 0;    /* this means we'll get it from parent later. */

    if (fupsample <= 0)
        upsample = downsample = 1;
    else if (fupsample >= 1) {
        upsample = fupsample;
        downsample   = 1;
    }
    else
    {
        downsample = 1.0 / fupsample;
        upsample   = 1;
    }

        /* vecsize is smallest power of 2 large enough to hold calcsize */
    if (calcsize)
    {
        if ((vecsize = (1 << ilog2(calcsize))) != calcsize)
            vecsize *= 2;
    }
    else vecsize = 0;
    if (vecsize && (vecsize != (1 << ilog2(vecsize))))
    {
        post_error ("block~: vector size not a power of 2");
        vecsize = 64;
    }
    if (overlap != (1 << ilog2(overlap)))
    {
        post_error ("block~: overlap not a power of 2");
        overlap = 1;
    }
    if (downsample != (1 << ilog2(downsample)))
    {
        post_error ("block~: downsampling not a power of 2");
        downsample = 1;
    }
    if (upsample != (1 << ilog2(upsample)))
    {
        post_error ("block~: upsampling not a power of 2");
        upsample = 1;
    }

    x->x_calcsize = calcsize;
    x->x_vecsize = vecsize;
    x->x_overlap = overlap;
    x->x_upsample = upsample;
    x->x_downsample = downsample;
    dsp_resume(dspstate);
}

static void *switch_new(t_float fvecsize, t_float foverlap,
                        t_float fupsample)
{
    t_block *x = (t_block *)(block_new(fvecsize, foverlap, fupsample));
    x->x_switched = 1;
    x->x_switchon = 0;
    return (x);
}

static void block_float(t_block *x, t_float f)
{
    if (x->x_switched)
        x->x_switchon = (f != 0);
}

static void block_bang(t_block *x)
{
    if (x->x_switched && !x->x_switchon && pd_this->pd_dspChain)
    {
        t_int *ip;
        x->x_return = 1;
        for (ip = pd_this->pd_dspChain + x->x_chainonset; ip; )
            ip = (*(t_perform)(*ip))(ip);
        x->x_return = 0;
    }
    else post_error ("bang to block~ or on-state switch~ has no effect");
}


#define PROLOGCALL 2
#define EPILOGCALL 2

static t_int *block_prolog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int phase = x->x_phase;
        /* if we're switched off, jump past the epilog code */
    if (!x->x_switchon)
        return (w + x->x_blocklength);
    if (phase)
    {
        phase++;
        if (phase == x->x_period) phase = 0;
        x->x_phase = phase;
        return (w + x->x_blocklength);  /* skip block; jump past epilog */
    }
    else
    {
        x->x_count = x->x_frequency;
        x->x_phase = (x->x_period > 1 ? 1 : 0);
        return (w + PROLOGCALL);        /* beginning of block is next ugen */
    }
}

static t_int *block_epilog(t_int *w)
{
    t_block *x = (t_block *)w[1];
    int count = x->x_count - 1;
    if (x->x_return)
        return (0);
    if (!x->x_reblock)
        return (w + x->x_epiloglength + EPILOGCALL);
    if (count)
    {
        x->x_count = count;
        return (w - (x->x_blocklength -
            (PROLOGCALL + EPILOGCALL)));   /* go to ugen after prolog */
    }
    else return (w + EPILOGCALL);
}

static void block_dsp(t_block *x, t_signal **sp)
{
    /* do nothing here */
}

void block_tilde_setup(void)
{
    block_class = class_new(gensym ("block~"), (t_newmethod)block_new, 0,
            sizeof(t_block), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addCreator((t_newmethod)switch_new, gensym ("switch~"),
        A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(block_class, (t_method)block_set, gensym ("set"), 
        A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addMethod(block_class, (t_method)block_dsp, sym_dsp, A_CANT, 0);
    class_addFloat(block_class, block_float);
    class_addBang(block_class, block_bang);
}

/* ------------------ DSP call list ----------------------- */

static t_int dsp_done(t_int *w)
{
    return (0);
}

void dsp_add(t_perform f, int n, ...)
{
    int newsize = pd_this->pd_dspChainSize + n+1, i;
    va_list ap;

    pd_this->pd_dspChain = PD_MEMORY_RESIZE(pd_this->pd_dspChain, 
        pd_this->pd_dspChainSize * sizeof (t_int), newsize * sizeof (t_int));
    pd_this->pd_dspChain[pd_this->pd_dspChainSize-1] = (t_int)f;
    va_start(ap, n);
    for (i = 0; i < n; i++)
        pd_this->pd_dspChain[pd_this->pd_dspChainSize + i] = va_arg(ap, t_int);
    va_end(ap);
    pd_this->pd_dspChain[newsize-1] = (t_int)dsp_done;
    pd_this->pd_dspChainSize = newsize;
}

    /* at Guenter's suggestion, here's a vectorized version */
void dsp_addv(t_perform f, int n, t_int *vec)
{
    int newsize = pd_this->pd_dspChainSize + n+1, i;
    
    pd_this->pd_dspChain = PD_MEMORY_RESIZE(pd_this->pd_dspChain, 
        pd_this->pd_dspChainSize * sizeof (t_int), newsize * sizeof (t_int));
    pd_this->pd_dspChain[pd_this->pd_dspChainSize-1] = (t_int)f;
    for (i = 0; i < n; i++)
        pd_this->pd_dspChain[pd_this->pd_dspChainSize + i] = vec[i];
    pd_this->pd_dspChain[newsize-1] = (t_int)dsp_done;
    pd_this->pd_dspChainSize = newsize;
}

void ugen_tick(void)
{
    if (pd_this->pd_dspChain)
    {
        t_int *ip;
        for (ip = pd_this->pd_dspChain; ip; ) ip = (*(t_perform)(*ip))(ip);
        dsp_phase++;
    }
}

/* ---------------- signals ---------------------------- */

int ilog2(int n)
{
    int r = -1;
    if (n <= 0) return(0);
    while (n)
    {
        r++;
        n >>= 1;
    }
    return (r);
}

    /* list of signals which can be reused, sorted by buffer size */
static t_signal *signal_freelist[MAXLOGSIG+1];
    /* list of reusable "borrowed" signals (which don't own sample buffers) */
static t_signal *signal_freeborrowed;

    /* call this when DSP is stopped to free all the signals */
void signal_cleanup(void)
{
    t_signal **svec, *sig, *sig2;
    int i;
    while (sig = pd_this->pd_signals)
    {
        pd_this->pd_signals = sig->s_nextUsed;
        if (!sig->s_isBorrowed)
            PD_MEMORY_FREE(sig->s_vector);
        PD_MEMORY_FREE(sig);
    }
    for (i = 0; i <= MAXLOGSIG; i++)
        signal_freelist[i] = 0;
    signal_freeborrowed = 0;
}

    /* mark the signal "reusable." */
void signal_makereusable(t_signal *sig)
{
    int logn = ilog2(sig->s_vectorSize);
#if 1
    t_signal *s5;
    for (s5 = signal_freeborrowed; s5; s5 = s5->s_nextFree)
    {
        if (s5 == sig)
        {
            PD_BUG;
            return;
        }
    }
    for (s5 = signal_freelist[logn]; s5; s5 = s5->s_nextFree)
    {
        if (s5 == sig)
        {
            PD_BUG;
            return;
        }
    }
#endif
    if (ugen_loud) post("free %lx: %d", sig, sig->s_isBorrowed);
    if (sig->s_isBorrowed)
    {
            /* if the signal is borrowed, decrement the borrowed-from signal's
                reference count, possibly marking it reusable too */
        t_signal *s2 = sig->s_borrowedFrom;
        if ((s2 == sig) || !s2) { PD_BUG; }
        s2->s_count--;
        if (!s2->s_count)
            signal_makereusable(s2);
        sig->s_nextFree = signal_freeborrowed;
        signal_freeborrowed = sig;
    }
    else
    {
            /* if it's a real signal (not borrowed), put it on the free list
                so we can reuse it. */
        if (signal_freelist[logn] == sig) { PD_BUG; }
        sig->s_nextFree = signal_freelist[logn];
        signal_freelist[logn] = sig;
    }
}

    /* reclaim or make an audio signal.  If n is zero, return a "borrowed"
    signal whose buffer and size will be obtained later via
    signal_setborrowed(). */

t_signal *signal_new(int n, t_float sr)
{
    int logn, n2, vecsize = 0;
    t_signal *ret, **whichlist;
    t_sample *fp;
    logn = ilog2(n);
    if (n)
    {
        if ((vecsize = (1<<logn)) != n)
            vecsize *= 2;
        if (logn > MAXLOGSIG) { PD_BUG; }
        whichlist = signal_freelist + logn;
    }
    else
        whichlist = &signal_freeborrowed;

        /* first try to reclaim one from the free list */
    if (ret = *whichlist)
        *whichlist = ret->s_nextFree;
    else
    {
            /* LATER figure out what to do for out-of-space here! */
        ret = (t_signal *)PD_MEMORY_GET(sizeof *ret);
        if (n)
        {
            ret->s_vector = (t_sample *)PD_MEMORY_GET(vecsize * sizeof (*ret->s_vector));
            ret->s_isBorrowed = 0;
        }
        else
        {
            ret->s_vector = 0;
            ret->s_isBorrowed = 1;
        }
        ret->s_nextUsed = pd_this->pd_signals;
        pd_this->pd_signals = ret;
    }
    ret->s_blockSize = n;
    ret->s_vectorSize = vecsize;
    ret->s_sampleRate = sr;
    ret->s_count = 0;
    ret->s_borrowedFrom = 0;
    if (ugen_loud) post("new %lx: %d", ret, ret->s_isBorrowed);
    return (ret);
}

static t_signal *signal_newlike(const t_signal *sig)
{
    return (signal_new(sig->s_blockSize, sig->s_sampleRate));
}

void signal_setborrowed(t_signal *sig, t_signal *sig2)
{
    if (!sig->s_isBorrowed || sig->s_borrowedFrom) { PD_BUG; }
    if (sig == sig2) { PD_BUG; }
    sig->s_borrowedFrom = sig2;
    sig->s_vector = sig2->s_vector;
    sig->s_blockSize = sig2->s_blockSize;
    sig->s_vectorSize = sig2->s_vectorSize;
}

int signal_compatible(t_signal *s1, t_signal *s2)
{
    return (s1->s_blockSize == s2->s_blockSize && s1->s_sampleRate == s2->s_sampleRate);
}

/* ------------------ ugen ("unit generator") sorting ----------------- */

typedef struct _ugenbox
{
    struct _siginlet *u_in;
    int u_nin;
    struct _sigoutlet *u_out;
    int u_nout;
    int u_phase;
    struct _ugenbox *u_next;
    t_object *u_obj;
    int u_done;
} t_ugenbox;

typedef struct _siginlet
{
    int i_nconnect;
    int i_ngot;
    t_signal *i_signal;
} t_siginlet;

typedef struct _sigoutconnect
{
    t_ugenbox *oc_who;
    int oc_inno;
    struct _sigoutconnect *oc_next;
} t_sigoutconnect;

typedef struct _sigoutlet
{
    int o_nconnect;
    int o_nsent;
    t_signal *o_signal;
    t_sigoutconnect *o_connections;
} t_sigoutlet;


struct _dspcontext
{
    struct _ugenbox *dc_ugenlist;
    struct _dspcontext *dc_parentcontext;
    int dc_ninlets;
    int dc_noutlets;
    t_signal **dc_iosigs;
    t_float dc_srate;
    int dc_vecsize;         /* vector size, power of two */
    int dc_calcsize;        /* number of elements to calculate */
    char dc_toplevel;       /* true if "iosigs" is invalid. */
    char dc_reblock;        /* true if we have to reblock inlets/outlets */
    char dc_switched;       /* true if we're switched */
    
};

static int ugen_sortno = 0;
static t_dspcontext *ugen_currentcontext;

void ugen_stop(void)
{
    t_signal *s;
    int i;
    if (pd_this->pd_dspChain)
    {
        PD_MEMORY_FREE(pd_this->pd_dspChain);
        pd_this->pd_dspChain = 0;
    }
    signal_cleanup();
    
}

void ugen_start(void)
{
    ugen_stop();
    ugen_sortno++;
    pd_this->pd_dspChain = (t_int *)PD_MEMORY_GET(sizeof(*pd_this->pd_dspChain));
    pd_this->pd_dspChain[0] = (t_int)dsp_done;
    pd_this->pd_dspChainSize = 1;
    if (ugen_currentcontext) { PD_BUG; }
}

int ugen_getsortno(void)
{
    return (ugen_sortno);
}

    /* start building the graph for a canvas */
t_dspcontext *ugen_start_graph(int toplevel, t_signal **sp,
    int ninlets, int noutlets)
{
    t_dspcontext *dc = (t_dspcontext *)PD_MEMORY_GET(sizeof(*dc));
    t_float parent_srate, srate;
    int parent_vecsize, vecsize;

    if (ugen_loud) post("ugen_start_graph...");

    /* protect against invalid numsignals
     * this might happen if we have an abstraction with inlet~/outlet~ opened as a toplevel patch
     */
    if(toplevel)
        ninlets=noutlets=0;

    dc->dc_ugenlist = 0;
    dc->dc_toplevel = toplevel;
    dc->dc_iosigs = sp;
    dc->dc_ninlets = ninlets;
    dc->dc_noutlets = noutlets;
    dc->dc_parentcontext = ugen_currentcontext;
    ugen_currentcontext = dc;
    return (dc);
}

    /* first the canvas calls this to create all the boxes... */
void ugen_add(t_dspcontext *dc, t_object *obj)
{
    t_ugenbox *x = (t_ugenbox *)PD_MEMORY_GET(sizeof *x);
    int i;
    t_sigoutlet *uout;
    t_siginlet *uin;
    
    x->u_next = dc->dc_ugenlist;
    dc->dc_ugenlist = x;
    x->u_obj = obj;
    x->u_nin = object_numberOfSignalInlets(obj);
    x->u_in = PD_MEMORY_GET(x->u_nin * sizeof (*x->u_in));
    for (uin = x->u_in, i = x->u_nin; i--; uin++)
        uin->i_nconnect = 0;
    x->u_nout = object_numberOfSignalOutlets(obj);
    x->u_out = PD_MEMORY_GET(x->u_nout * sizeof (*x->u_out));
    for (uout = x->u_out, i = x->u_nout; i--; uout++)
        uout->o_connections = 0, uout->o_nconnect = 0;
}

    /* and then this to make all the connections. */
void ugen_connect(t_dspcontext *dc, t_object *x1, int outno, t_object *x2,
    int inno)
{
    t_ugenbox *u1, *u2;
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc;
    int sigoutno = object_indexOfSignalOutlet(x1, outno);
    int siginno = object_indexOfSignalInlet(x2, inno);
    if (ugen_loud)
        post("%s -> %s: %d->%d",
            class_getName(x1->te_g.g_pd),
                class_getName(x2->te_g.g_pd), outno, inno);
    for (u1 = dc->dc_ugenlist; u1 && u1->u_obj != x1; u1 = u1->u_next);
    for (u2 = dc->dc_ugenlist; u2 && u2->u_obj != x2; u2 = u2->u_next);
    if (!u1 || !u2 || siginno < 0)
    {
        post_error ("signal outlet connect to nonsignal inlet (ignored)");
        return;
    }
    if (sigoutno < 0 || sigoutno >= u1->u_nout || siginno >= u2->u_nin)
    {
        PD_BUG;
    }
    uout = u1->u_out + sigoutno;
    uin = u2->u_in + siginno;

        /* add a new connection to the outlet's list */
    oc = (t_sigoutconnect *)PD_MEMORY_GET(sizeof *oc);
    oc->oc_next = uout->o_connections;
    uout->o_connections = oc;
    oc->oc_who = u2;
    oc->oc_inno = siginno;
        /* update inlet and outlet counts  */
    uout->o_nconnect++;
    uin->i_nconnect++;
}

    /* get the index of a ugenbox or -1 if it's not on the list */
static int ugen_index(t_dspcontext *dc, t_ugenbox *x)
{
    int ret;
    t_ugenbox *u;
    for (u = dc->dc_ugenlist, ret = 0; u; u = u->u_next, ret++)
        if (u == x) return (ret);
    return (-1);
}

    /* put a ugenbox on the chain, recursively putting any others on that
    this one might uncover. */
static void ugen_doit(t_dspcontext *dc, t_ugenbox *u)
{
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc, *oc2;
    t_class *class = pd_class(&u->u_obj->te_g.g_pd);
    int i, n;
        /* suppress creating new signals for the outputs of signal
        inlets and subpatchs; except in the case we're an inlet and "blocking"
        is set.  We don't yet know if a subcanvas will be "blocking" so there
        we delay new signal creation, which will be handled by calling
        signal_setborrowed in the ugen_done_graph routine below. */
    int nonewsigs = (class == canvas_class || 
        (class == vinlet_class) && !(dc->dc_reblock));
        /* when we encounter a subcanvas or a signal outlet, suppress freeing
        the input signals as they may be "borrowed" for the super or sub
        patch; same exception as above, but also if we're "switched" we
        have to do a copy rather than a borrow.  */
    int nofreesigs = (class == canvas_class || 
        (class == voutlet_class) &&  !(dc->dc_reblock || dc->dc_switched));
    t_signal **insig, **outsig, **sig, *s1, *s2, *s3;
    t_ugenbox *u2;
    
    if (ugen_loud) post("doit %s %d %d", class_getName(class), nofreesigs,
        nonewsigs);
    for (i = 0, uin = u->u_in; i < u->u_nin; i++, uin++)
    {
        if (!uin->i_nconnect)
        {
            t_float *scalar;
            s3 = signal_new(dc->dc_calcsize, dc->dc_srate);
            /* post("%s: unconnected signal inlet set to zero",
                class_getName(u->u_obj->te_g.g_pd)); */
            if (scalar = object_getSignalValueAtIndex(u->u_obj, i))
                dsp_add_scalarcopy(scalar, s3->s_vector, s3->s_blockSize);
            else
                dsp_add_zero(s3->s_vector, s3->s_blockSize);
            uin->i_signal = s3;
            s3->s_count = 1;
        }
    }
    insig = (t_signal **)PD_MEMORY_GET((u->u_nin + u->u_nout) * sizeof(t_signal *));
    outsig = insig + u->u_nin;
    for (sig = insig, uin = u->u_in, i = u->u_nin; i--; sig++, uin++)
    {
        int newrefcount;
        *sig = uin->i_signal;
        newrefcount = --(*sig)->s_count;
            /* if the reference count went to zero, we free the signal now,
            unless it's a subcanvas or outlet; these might keep the
            signal around to send to objects connected to them.  In this
            case we increment the reference count; the corresponding decrement
            is in sig_makereusable(). */
        if (nofreesigs)
            (*sig)->s_count++;
        else if (!newrefcount)
            signal_makereusable(*sig);
    }
    for (sig = outsig, uout = u->u_out, i = u->u_nout; i--; sig++, uout++)
    {
            /* similarly, for outlets of subcanvases we delay creating
            them; instead we create "borrowed" ones so that the refcount
            is known.  The subcanvas replaces the fake signal with one showing
            where the output data actually is, to avoid having to copy it.
            For any other object, we just allocate a new output vector;
            since we've already freed the inputs the objects might get called
            "in place." */
        if (nonewsigs)
        {
            *sig = uout->o_signal =
                signal_new(0, dc->dc_srate);
        }
        else
            *sig = uout->o_signal = signal_new(dc->dc_calcsize, dc->dc_srate);
        (*sig)->s_count = uout->o_nconnect;
    }
        /* now call the DSP scheduling routine for the ugen.  This
        routine must fill in "borrowed" signal outputs in case it's either
        a subcanvas or a signal inlet. */
        
    mess1(&u->u_obj->te_g.g_pd, sym_dsp, insig);
    
        /* if any output signals aren't connected to anyone, free them
        now; otherwise they'll either get freed when the reference count
        goes back to zero, or even later as explained above. */

    for (sig = outsig, uout = u->u_out, i = u->u_nout; i--; sig++, uout++)
    {
        if (!(*sig)->s_count)
            signal_makereusable(*sig);
    }
    if (ugen_loud)
    {
        if (u->u_nin + u->u_nout == 0) post("put %s %d", 
            class_getName(u->u_obj->te_g.g_pd), ugen_index(dc, u));
        else if (u->u_nin + u->u_nout == 1) post("put %s %d (%lx)", 
            class_getName(u->u_obj->te_g.g_pd), ugen_index(dc, u), sig[0]);
        else if (u->u_nin + u->u_nout == 2) post("put %s %d (%lx %lx)", 
            class_getName(u->u_obj->te_g.g_pd), ugen_index(dc, u),
                sig[0], sig[1]);
        else post("put %s %d (%lx %lx %lx ...)", 
            class_getName(u->u_obj->te_g.g_pd), ugen_index(dc, u),
                sig[0], sig[1], sig[2]);
    }

        /* pass it on and trip anyone whose last inlet was filled */
    for (uout = u->u_out, i = u->u_nout; i--; uout++)
    {
        s1 = uout->o_signal;
        for (oc = uout->o_connections; oc; oc = oc->oc_next)
        {
            u2 = oc->oc_who;
            uin = &u2->u_in[oc->oc_inno];
                /* if there's already someone here, sum the two */
            if (s2 = uin->i_signal)
            {
                s1->s_count--;
                s2->s_count--;
                if (!signal_compatible(s1, s2))
                {
                    post_error ("%s: incompatible signal inputs",
                        class_getName(u->u_obj->te_g.g_pd));
                    return;
                }
                s3 = signal_newlike(s1);
                dsp_add_plus(s1->s_vector, s2->s_vector, s3->s_vector, s1->s_blockSize);
                uin->i_signal = s3;
                s3->s_count = 1;
                if (!s1->s_count) signal_makereusable(s1);
                if (!s2->s_count) signal_makereusable(s2);
            }
            else uin->i_signal = s1;
            uin->i_ngot++;
                /* if we didn't fill this inlet don't bother yet */
            if (uin->i_ngot < uin->i_nconnect)
                goto notyet;
                /* if there's more than one, check them all */
            if (u2->u_nin > 1)
            {
                for (uin = u2->u_in, n = u2->u_nin; n--; uin++)
                    if (uin->i_ngot < uin->i_nconnect) goto notyet;
            }
                /* so now we can schedule the ugen.  */
            ugen_doit(dc, u2);
        notyet: ;
        }
    }
    PD_MEMORY_FREE(insig);
    u->u_done = 1;
}

    /* once the DSP graph is built, we call this routine to sort it.
    This routine also deletes the graph; later we might want to leave the
    graph around, in case the user is editing the DSP network, to save having
    to recreate it all the time.  But not today.  */

void ugen_done_graph(t_dspcontext *dc)
{
    t_ugenbox *u, *u2;
    t_sigoutlet *uout;
    t_siginlet *uin;
    t_sigoutconnect *oc, *oc2;
    int i, n;
    t_block *blk;
    t_dspcontext *parent_context = dc->dc_parentcontext;
    t_float parent_srate;
    int parent_vecsize;
    int period, frequency, phase, vecsize, calcsize;
    t_float srate;
    int chainblockbegin;    /* DSP chain onset before block prolog code */
    int chainblockend;      /* and after block epilog code */
    int chainafterall;      /* and after signal outlet epilog */
    int reblock = 0, switched;
    int downsample = 1, upsample = 1;
    /* debugging printout */
    
    if (ugen_loud)
    {
        post("ugen_done_graph...");
        for (u = dc->dc_ugenlist; u; u = u->u_next)
        {
            post("ugen: %s", class_getName(u->u_obj->te_g.g_pd));
            for (uout = u->u_out, i = 0; i < u->u_nout; uout++, i++)
                for (oc = uout->o_connections; oc; oc = oc->oc_next)
            {
                post("... out %d to %s, index %d, inlet %d", i,
                    class_getName(oc->oc_who->u_obj->te_g.g_pd),
                        ugen_index(dc, oc->oc_who), oc->oc_inno);
            }
        }
    }
    
        /* search for an object of class "block~" */
    for (u = dc->dc_ugenlist, blk = 0; u; u = u->u_next)
    {
        t_pd *zz = &u->u_obj->te_g.g_pd;
        if (pd_class(zz) == block_class)
        {
            if (blk)
                post_error ("conflicting block~ objects in same page");
            else blk = (t_block *)zz;
        }
    }

        /* figure out block size, calling frequency, sample rate */
    if (parent_context)
    {
        parent_srate = parent_context->dc_srate;
        parent_vecsize = parent_context->dc_vecsize;
    }
    else
    {
        parent_srate = audio_getSampleRate();
        parent_vecsize = AUDIO_DEFAULT_BLOCKSIZE;
    }
    if (blk)
    {
        int realoverlap;
        vecsize = blk->x_vecsize;
        if (vecsize == 0)
            vecsize = parent_vecsize;
        calcsize = blk->x_calcsize;
        if (calcsize == 0)
            calcsize = vecsize;
        realoverlap = blk->x_overlap;
        if (realoverlap > vecsize) realoverlap = vecsize;
        downsample = blk->x_downsample;
        upsample   = blk->x_upsample;
        if (downsample > parent_vecsize)
            downsample = parent_vecsize;
        period = (vecsize * downsample)/
            (parent_vecsize * realoverlap * upsample);
        frequency = (parent_vecsize * realoverlap * upsample)/
            (vecsize * downsample);
        phase = blk->x_phase;
        srate = parent_srate * realoverlap * upsample / downsample;
        if (period < 1) period = 1;
        if (frequency < 1) frequency = 1;
        blk->x_frequency = frequency;
        blk->x_period = period;
        blk->x_phase = dsp_phase & (period - 1);
        if (! parent_context || (realoverlap != 1) ||
            (vecsize != parent_vecsize) || 
                (downsample != 1) || (upsample != 1))
                    reblock = 1;
        switched = blk->x_switched;
    }
    else
    {
        srate = parent_srate;
        vecsize = parent_vecsize;
        calcsize = (parent_context ? parent_context->dc_calcsize : vecsize);
        downsample = upsample = 1;
        period = frequency = 1;
        phase = 0;
        if (!parent_context) reblock = 1;
        switched = 0;
    }
    dc->dc_reblock = reblock;
    dc->dc_switched = switched;
    dc->dc_srate = srate;
    dc->dc_vecsize = vecsize;
    dc->dc_calcsize = calcsize;
    
        /* if we're reblocking or switched, we now have to create output
        signals to fill in for the "borrowed" ones we have now.  This
        is also possibly true even if we're not blocked/switched, in
        the case that there was a signal loop.  But we don't know this
        yet.  */

    if (dc->dc_iosigs && (switched || reblock))
    {
        t_signal **sigp;
        for (i = 0, sigp = dc->dc_iosigs + dc->dc_ninlets; i < dc->dc_noutlets;
            i++, sigp++)
        {
            if ((*sigp)->s_isBorrowed && !(*sigp)->s_borrowedFrom)
            {
                signal_setborrowed(*sigp,
                    signal_new(parent_vecsize, parent_srate));
                (*sigp)->s_count++;

                if (ugen_loud) post("set %lx->%lx", *sigp,
                    (*sigp)->s_borrowedFrom);
            }
        }
    }

    if (ugen_loud)
        post("reblock %d, switched %d", reblock, switched);

        /* schedule prologs for inlets and outlets.  If the "reblock" flag
        is set, an inlet will put code on the DSP chain to copy its input
        into an internal buffer here, before any unit generators' DSP code
        gets scheduled.  If we don't "reblock", inlets will need to get
        pointers to their corresponding inlets/outlets on the box we're inside,
        if any.  Outlets will also need pointers, unless we're switched, in
        which case outlet epilog code will kick in. */
        
    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
        t_pd *zz = &u->u_obj->te_g.g_pd;
        t_signal **insigs = dc->dc_iosigs, **outsigs = dc->dc_iosigs;
        if (outsigs) outsigs += dc->dc_ninlets;

        if (pd_class(zz) == vinlet_class)
            vinlet_dspprolog((struct _vinlet *)zz, 
                dc->dc_iosigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
        else if (pd_class(zz) == voutlet_class)
            voutlet_dspprolog((struct _voutlet *)zz, 
                outsigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
    }    
    chainblockbegin = pd_this->pd_dspChainSize;

    if (blk && (reblock || switched))   /* add the block DSP prolog */
    {
        dsp_add(block_prolog, 1, blk);
        blk->x_chainonset = pd_this->pd_dspChainSize - 1;
    }   
        /* Initialize for sorting */
    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
        u->u_done = 0;
        for (uout = u->u_out, i = u->u_nout; i--; uout++)
            uout->o_nsent = 0;
        for (uin = u->u_in, i = u->u_nin; i--; uin++)
            uin->i_ngot = 0, uin->i_signal = 0;
   }
    
        /* Do the sort */

    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
            /* check that we have no connected signal inlets */
        if (u->u_done) continue;
        for (uin = u->u_in, i = u->u_nin; i--; uin++)
            if (uin->i_nconnect) goto next;

        ugen_doit(dc, u);
    next: ;
    }

        /* check for a DSP loop, which is evidenced here by the presence
        of ugens not yet scheduled. */
        
    for (u = dc->dc_ugenlist; u; u = u->u_next)
        if (!u->u_done) 
    {
        t_signal **sigp;
        post_error ("DSP loop detected (some tilde objects not scheduled)");
                /* this might imply that we have unfilled "borrowed" outputs
                which we'd better fill in now. */
        for (i = 0, sigp = dc->dc_iosigs + dc->dc_ninlets; i < dc->dc_noutlets;
            i++, sigp++)
        {
            if ((*sigp)->s_isBorrowed && !(*sigp)->s_borrowedFrom)
            {
                t_signal *s3 = signal_new(parent_vecsize, parent_srate);
                signal_setborrowed(*sigp, s3);
                (*sigp)->s_count++;
                dsp_add_zero(s3->s_vector, s3->s_blockSize);
                if (ugen_loud)
                    post("oops, belatedly set %lx->%lx", *sigp,
                        (*sigp)->s_borrowedFrom);
            }
        }
        break;   /* don't need to keep looking. */
    }

    if (blk && (reblock || switched))    /* add block DSP epilog */
        dsp_add(block_epilog, 1, blk);
    chainblockend = pd_this->pd_dspChainSize;

        /* add epilogs for outlets.  */

    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
        t_pd *zz = &u->u_obj->te_g.g_pd;
        if (pd_class(zz) == voutlet_class)
        {
            t_signal **iosigs = dc->dc_iosigs;
            if (iosigs) iosigs += dc->dc_ninlets;
            voutlet_dspepilog((struct _voutlet *)zz, 
                iosigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
        }
    }

    chainafterall = pd_this->pd_dspChainSize;
    if (blk)
    {
        blk->x_blocklength = chainblockend - chainblockbegin;
        blk->x_epiloglength = chainafterall - chainblockend;
        blk->x_reblock = reblock;
    }

    if (ugen_loud)
    {
        t_int *ip;
        if (!dc->dc_parentcontext)
            for (i = pd_this->pd_dspChainSize, ip = pd_this->pd_dspChain; 
                i--; ip++)
                    post("chain %lx", *ip);
        post("... ugen_done_graph done.");
    }
        /* now delete everything. */
    while (dc->dc_ugenlist)
    {
        for (uout = dc->dc_ugenlist->u_out, n = dc->dc_ugenlist->u_nout;
            n--; uout++)
        {
            oc = uout->o_connections;
            while (oc)
            {
                oc2 = oc->oc_next;
                PD_MEMORY_FREE(oc);
                oc = oc2;
            }
        }
        PD_MEMORY_FREE(dc->dc_ugenlist->u_out);
        PD_MEMORY_FREE(dc->dc_ugenlist->u_in);
        u = dc->dc_ugenlist;
        dc->dc_ugenlist = u->u_next;
        PD_MEMORY_FREE(u);
    }
    if (ugen_currentcontext == dc)
        ugen_currentcontext = dc->dc_parentcontext;
    else { PD_BUG; }
    PD_MEMORY_FREE(dc);

}

t_signal *ugen_getiosig(int index, int inout)
{
    if (!ugen_currentcontext) { PD_BUG; }
    if (ugen_currentcontext->dc_toplevel) return (0);
    if (inout) index += ugen_currentcontext->dc_ninlets;
    return (ugen_currentcontext->dc_iosigs[index]);
}

/* ------------------------ samplerate~~ -------------------------- */

static t_class *samplerate_tilde_class;

typedef struct _samplerate
{
    t_object x_obj;
    t_float x_sr;
    t_glist *x_canvas;
} t_samplerate;

static void *canvas_getblock (t_class *blockclass, t_glist **canvasp)
{
    t_glist *canvas = *canvasp;
    t_gobj *g;
    void *ret = 0;
    for (g = canvas->gl_graphics; g; g = g->g_next)
    {
        if (g->g_pd == blockclass)
            ret = g;
    }
    *canvasp = canvas->gl_parent;
    return(ret);
}

static void samplerate_tilde_bang(t_samplerate *x)
{
    t_float srate = audio_getSampleRate();
    t_glist *canvas = x->x_canvas;
    while (canvas)
    {
        t_block *b = (t_block *)canvas_getblock(block_class, &canvas);
        if (b) 
            srate *= (t_float)(b->x_upsample) / (t_float)(b->x_downsample); 
    }
    outlet_float(x->x_obj.te_outlet, srate);
}

static void *samplerate_tilde_new(t_symbol *s)
{
    t_samplerate *x = (t_samplerate *)pd_new(samplerate_tilde_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_canvas = canvas_getCurrent();
    return (x);
}

static void samplerate_tilde_setup(void)
{
    samplerate_tilde_class = class_new(gensym ("samplerate~"),
        (t_newmethod)samplerate_tilde_new, 0, sizeof(t_samplerate), 0, 0);
    class_addBang(samplerate_tilde_class, samplerate_tilde_bang);
}

/* -------------------- setup routine -------------------------- */

void d_ugen_setup(void) 
{
    block_tilde_setup();
    samplerate_tilde_setup();
}

