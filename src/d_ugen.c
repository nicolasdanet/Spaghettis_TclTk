
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

static t_dspcontext     *ugen_currentcontext;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int dsp_phase;
static int ugen_sortno;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ugen_tick(void)
{
    if (pd_this->pd_dspChain)
    {
        t_int *ip;
        for (ip = pd_this->pd_dspChain; ip; ) ip = (*(t_perform)(*ip))(ip);
        dsp_phase++;
    }
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

void ugen_stop(void)
{
    t_signal *s;
    int i;
    if (pd_this->pd_dspChain)
    {
        PD_MEMORY_FREE(pd_this->pd_dspChain);
        pd_this->pd_dspChain = 0;
    }
    
    signal_clean();
}

int ugen_getsortno(void)
{
    return (ugen_sortno);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
        signal_borrowFrom in the ugen_done_graph routine below. */
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
    
    if (0) post("doit %s %d %d", class_getNameAsString(class), nofreesigs,
        nonewsigs);
    for (i = 0, uin = u->u_in; i < u->u_nin; i++, uin++)
    {
        if (!uin->i_nconnect)
        {
            t_float *scalar;
            s3 = signal_new(dc->dc_calcsize, dc->dc_srate);
            /* post("%s: unconnected signal inlet set to zero",
                class_getNameAsString(u->u_obj->te_g.g_pd)); */
            if (scalar = object_getSignalValueAtIndex(u->u_obj, i))
                dsp_add_scalarcopy(scalar, s3->s_vector, s3->s_blockSize);
            else
                dsp_addZeroPerform(s3->s_vector, s3->s_blockSize);
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
            signal_free(*sig);
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
            signal_free(*sig);
    }
    if (0)
    {
        if (u->u_nin + u->u_nout == 0) post("put %s %d", 
            class_getNameAsString(u->u_obj->te_g.g_pd), ugen_index(dc, u));
        else if (u->u_nin + u->u_nout == 1) post("put %s %d (%lx)", 
            class_getNameAsString(u->u_obj->te_g.g_pd), ugen_index(dc, u), sig[0]);
        else if (u->u_nin + u->u_nout == 2) post("put %s %d (%lx %lx)", 
            class_getNameAsString(u->u_obj->te_g.g_pd), ugen_index(dc, u),
                sig[0], sig[1]);
        else post("put %s %d (%lx %lx %lx ...)", 
            class_getNameAsString(u->u_obj->te_g.g_pd), ugen_index(dc, u),
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
                //if (!signal_compatible(s1, s2))
                if (!(s1->s_blockSize == s2->s_blockSize && s1->s_sampleRate == s2->s_sampleRate))
                {
                    post_error ("%s: incompatible signal inputs",
                        class_getNameAsString(u->u_obj->te_g.g_pd));
                    return;
                }
                s3 = signal_new(s1->s_blockSize, s1->s_sampleRate);
                //s3 = signal_newlike(s1);
                dsp_add_plus(s1->s_vector, s2->s_vector, s3->s_vector, s1->s_blockSize);
                uin->i_signal = s3;
                s3->s_count = 1;
                if (!s1->s_count) signal_free(s1);
                if (!s2->s_count) signal_free(s2);
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* start building the graph for a canvas */
t_dspcontext *ugen_start_graph(int toplevel, t_signal **sp,
    int ninlets, int noutlets)
{
    t_dspcontext *dc = (t_dspcontext *)PD_MEMORY_GET(sizeof(*dc));
    t_float parent_srate, srate;
    int parent_vecsize, vecsize;

    if (0) post("ugen_start_graph...");

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
    if (0)
        post("%s -> %s: %d->%d",
            class_getNameAsString(x1->te_g.g_pd),
                class_getNameAsString(x2->te_g.g_pd), outno, inno);
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
    
    if (0)
    {
        post("ugen_done_graph...");
        for (u = dc->dc_ugenlist; u; u = u->u_next)
        {
            post("ugen: %s", class_getNameAsString(u->u_obj->te_g.g_pd));
            for (uout = u->u_out, i = 0; i < u->u_nout; uout++, i++)
                for (oc = uout->o_connections; oc; oc = oc->oc_next)
            {
                post("... out %d to %s, index %d, inlet %d", i,
                    class_getNameAsString(oc->oc_who->u_obj->te_g.g_pd),
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
                signal_borrowFrom(*sigp,
                    signal_new(parent_vecsize, parent_srate));
                (*sigp)->s_count++;

                if (0) post("set %lx->%lx", *sigp,
                    (*sigp)->s_borrowedFrom);
            }
        }
    }

    if (0)
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
            vinlet_dspProlog((t_vinlet *)zz, 
                dc->dc_iosigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
        else if (pd_class(zz) == voutlet_class)
            voutlet_dspProlog((t_voutlet *)zz, 
                outsigs, vecsize, calcsize, dsp_phase, period, frequency,
                    downsample, upsample, reblock, switched);
    }    
    chainblockbegin = pd_this->pd_dspChainSize;

    if (blk && (reblock || switched))   /* add the block DSP prolog */
    {
        dsp_add(block_dspProlog, 1, blk);
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
                signal_borrowFrom(*sigp, s3);
                (*sigp)->s_count++;
                dsp_addZeroPerform(s3->s_vector, s3->s_blockSize);
                if (0)
                    post("oops, belatedly set %lx->%lx", *sigp,
                        (*sigp)->s_borrowedFrom);
            }
        }
        break;   /* don't need to keep looking. */
    }

    if (blk && (reblock || switched))    /* add block DSP epilog */
        dsp_add(block_dspEpilog, 1, blk);
    chainblockend = pd_this->pd_dspChainSize;

        /* add epilogs for outlets.  */

    for (u = dc->dc_ugenlist; u; u = u->u_next)
    {
        t_pd *zz = &u->u_obj->te_g.g_pd;
        if (pd_class(zz) == voutlet_class)
        {
            t_signal **iosigs = dc->dc_iosigs;
            if (iosigs) iosigs += dc->dc_ninlets;
            voutlet_dspEpilog((t_voutlet *)zz, 
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

    if (0)
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

/*
t_signal *ugen_getiosig(int index, int inout)
{
    if (!ugen_currentcontext) { PD_BUG; }
    if (ugen_currentcontext->dc_toplevel) return (0);
    if (inout) index += ugen_currentcontext->dc_ninlets;
    return (ugen_currentcontext->dc_iosigs[index]);
}
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
