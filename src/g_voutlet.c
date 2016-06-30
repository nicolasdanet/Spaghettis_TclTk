
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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *voutlet_class;                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _voutlet {
    t_object    x_obj;
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    t_signal    *x_directSignal;
    int         x_bufferSize;
    t_sample    *x_buffer;
    t_sample    *x_bufferEnd;
    t_sample    *x_bufferEmpty;
    t_sample    *x_bufferWrite;
    int         x_hopSize;
    t_resample  x_resampling;
    char        x_justCopyOut;
    } t_voutlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outlet *voutlet_getOutlet(t_pd *x)
{
    if (pd_class(x) != voutlet_class) { PD_BUG; }
    return (((t_voutlet *)x)->x_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void voutlet_bang(t_voutlet *x)
{
    outlet_bang(x->x_outlet);
}

static void voutlet_pointer(t_voutlet *x, t_gpointer *gp)
{
    outlet_pointer(x->x_outlet, gp);
}

static void voutlet_float(t_voutlet *x, t_float f)
{
    outlet_float(x->x_outlet, f);
}

static void voutlet_symbol(t_voutlet *x, t_symbol *s)
{
    outlet_symbol(x->x_outlet, s);
}

static void voutlet_list(t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_outlet, s, argc, argv);
}

static void voutlet_anything(t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* LATER optimize for non-overlapped case where the "+=" isn't needed */
t_int *voutlet_perform(t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_sample *out = x->x_bufferWrite, *outwas = out;

    while (n--)
    {
        *out++ += *in++;
        if (out == x->x_bufferEnd) out = x->x_buffer;
    }
    outwas += x->x_hopSize;
    if (outwas >= x->x_bufferEnd) outwas = x->x_buffer;
    x->x_bufferWrite = outwas;
    return (w+4);
}

    /* epilog code for blocking: write buffer to parent patch */
static t_int *voutlet_doepilog(t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);

    int n = (int)(w[3]);
    t_sample *in = x->x_bufferEmpty;
    if (x->x_resampling.r_downSample != x->x_resampling.r_upSample)
        out = x->x_resampling.r_vector;

    for (; n--; in++) *out++ = *in, *in = 0;
    if (in == x->x_bufferEnd) in = x->x_buffer;
    x->x_bufferEmpty = in;
    return (w+4);
}

static t_int *voutlet_doepilog_resampling(t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    int n = (int)(w[2]);
    t_sample *in  = x->x_bufferEmpty;
    t_sample *out = x->x_resampling.r_vector;

    for (; n--; in++) *out++ = *in, *in = 0;
    if (in == x->x_bufferEnd) in = x->x_buffer;
    x->x_bufferEmpty = in;
    return (w+3);
}


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

        /* prolog for outlets -- store pointer to the outlet on the
        parent, which, if "reblock" is false, will want to refer
        back to whatever we see on our input during the "dsp" method
        called later.  */
void voutlet_dspprolog(struct _voutlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched)
{
        /* no buffer means we're not a signal outlet */
    if (!x->x_buffer)
        return;
    x->x_resampling.r_downSample=downsample;
    x->x_resampling.r_upSample=upsample;
    x->x_justCopyOut = (switched && !reblock);
    if (reblock)
    {
        x->x_directSignal = 0;
    }
    else
    {
        if (!parentsigs) { PD_BUG; }
        x->x_directSignal =
            parentsigs[object_getIndexOfSignalOutlet(x->x_outlet)];
    }
}

static void voutlet_dsp(t_voutlet *x, t_signal **sp)
{
    t_signal *insig;
    if (!x->x_buffer) return;
    insig = sp[0];
    if (x->x_justCopyOut)
        dsp_add_copy(insig->s_vector, x->x_directSignal->s_vector, insig->s_blockSize);
    else if (x->x_directSignal)
    {
            /* if we're just going to make the signal available on the
            parent patch, hand it off to the parent signal. */
        /* this is done elsewhere--> sp[0]->s_count++; */
        signal_setborrowed(x->x_directSignal, sp[0]);
    }
    else
        dsp_add(voutlet_perform, 3, x, insig->s_vector, insig->s_blockSize);
}

        /* set up epilog DSP code.  If we're reblocking, this is the
        time to copy the samples out to the containing object's outlets.
        If we aren't reblocking, there's nothing to do here.  */
void voutlet_dspepilog(struct _voutlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched)
{
    if (!x->x_buffer) return;  /* this shouldn't be necesssary... */
    x->x_resampling.r_downSample=downsample;
    x->x_resampling.r_upSample=upsample;
    if (reblock)
    {
        t_signal *insig, *outsig;
        int parentvecsize, bufsize, oldbufsize;
        int re_parentvecsize;
        int bigperiod, epilogphase, blockphase;
        if (parentsigs)
        {
            outsig = parentsigs[object_getIndexOfSignalOutlet(x->x_outlet)];
            parentvecsize = outsig->s_vectorSize;
            re_parentvecsize = parentvecsize * upsample / downsample;
        }
        else
        {
            outsig = 0;
            parentvecsize = 1;
            re_parentvecsize = 1;
        }
        bigperiod = myvecsize/re_parentvecsize;
        if (!bigperiod) bigperiod = 1;
        epilogphase = phase & (bigperiod - 1);
        blockphase = (phase + period - 1) & (bigperiod - 1) & (- period);
        bufsize = re_parentvecsize;
        if (bufsize < myvecsize) bufsize = myvecsize;
        if (bufsize != (oldbufsize = x->x_bufferSize))
        {
            t_sample *buf = x->x_buffer;
            PD_MEMORY_FREE(buf);
            buf = (t_sample *)PD_MEMORY_GET(bufsize * sizeof(*buf));
            memset((char *)buf, 0, bufsize * sizeof(*buf));
            x->x_bufferSize = bufsize;
            x->x_bufferEnd = buf + bufsize;
            x->x_buffer = buf;
        }
        if (re_parentvecsize * period > bufsize) { PD_BUG; }
        x->x_bufferWrite = x->x_buffer + re_parentvecsize * blockphase;
        if (x->x_bufferWrite == x->x_bufferEnd) x->x_bufferWrite = x->x_buffer;
        if (period == 1 && frequency > 1)
            x->x_hopSize = re_parentvecsize / frequency;
        else x->x_hopSize = period * re_parentvecsize;
        /* post("phase %d, block %d, parent %d", phase & 63,
            parentvecsize * blockphase, parentvecsize * epilogphase); */
        if (parentsigs)
        {
            /* set epilog pointer and schedule it */
            x->x_bufferEmpty = x->x_buffer + re_parentvecsize * epilogphase;
            if (upsample * downsample == 1)
                dsp_add(voutlet_doepilog, 3, x, outsig->s_vector,
                    re_parentvecsize);
            else
            {
                int method = (x->x_resampling.r_type == 3?
                    (0 ? 0 : 1) : x->x_resampling.r_type);
                dsp_add(voutlet_doepilog_resampling, 2, x, re_parentvecsize);
                resampleto_dsp(&x->x_resampling, outsig->s_vector, re_parentvecsize,
                    parentvecsize, method);
            }
        }
    }
        /* if we aren't blocked but we are switched, the epilog code just
        copies zeros to the output.  In this case the blocking code actually
        jumps over the epilog if the block is running. */
    else if (switched)
    {
        if (parentsigs)
        {
            t_signal *outsig =
                parentsigs[object_getIndexOfSignalOutlet(x->x_outlet)];
            dsp_add_zero(outsig->s_vector, outsig->s_blockSize);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *voutlet_newsig(t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new(voutlet_class);
    x->x_owner = canvas_getCurrent();
    x->x_outlet = canvas_addOutlet (x->x_owner,
        &x->x_obj.te_g.g_pd, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_bufferEnd = x->x_buffer = (t_sample *)PD_MEMORY_GET(0);
    x->x_bufferSize = 0;

    resample_init(&x->x_resampling, s);

    return (x);
}

static void *voutlet_new(t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new(voutlet_class);
    x->x_owner = canvas_getCurrent();
    x->x_outlet = canvas_addOutlet (x->x_owner, &x->x_obj.te_g.g_pd, 0);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, 0, 0);
    x->x_bufferSize = 0;
    x->x_buffer = 0;
    return (x);
}

static void voutlet_free(t_voutlet *x)
{
    canvas_removeOutlet (x->x_owner, x->x_outlet);
    if (x->x_buffer)
        PD_MEMORY_FREE(x->x_buffer);
    resample_free(&x->x_resampling);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void voutlet_setup(void)
{
    voutlet_class = class_new(sym_outlet, (t_newmethod)voutlet_new,
        (t_method)voutlet_free, sizeof(t_voutlet), CLASS_NOINLET, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)voutlet_newsig, sym_outlet__tilde__, A_DEFSYMBOL, 0);
    class_addBang(voutlet_class, voutlet_bang);
    class_addPointer(voutlet_class, voutlet_pointer);
    class_addFloat(voutlet_class, (t_method)voutlet_float);
    class_addSymbol(voutlet_class, voutlet_symbol);
    class_addList(voutlet_class, voutlet_list);
    class_addAnything(voutlet_class, voutlet_anything);
    class_addMethod(voutlet_class, (t_method)voutlet_dsp, 
        sym_dsp, A_CANT, 0);
    class_setHelpName(voutlet_class, sym_pd);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
