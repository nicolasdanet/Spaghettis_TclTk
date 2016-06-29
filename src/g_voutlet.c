
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* graphical inlets and outlets, both for control and signals.  */

/* This code is highly inefficient; messages actually have to be forwarded
by inlets and outlets.  The outlet is in even worse shape than the inlet;
in order to avoid having a "signal" method in the class, the oulet actually
sprouts an inlet, which forwards the message to the "outlet" object, which
sends it on to the outlet proper.  Another way to do it would be to have
separate classes for "signal" and "control" outlets, but this would complicate
life elsewhere. */


#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"
#include <string.h>
void signal_setborrowed(t_signal *sig, t_signal *sig2);
void signal_makereusable(t_signal *sig);

/* ------------------------- voutlet -------------------------- */

t_class *voutlet_class;     /* Shared. */

typedef struct _voutlet
{
    t_object x_obj;
    t_glist *x_canvas;
    t_outlet *x_parentoutlet;
    int x_bufsize;
    t_sample *x_buf;         /* signal buffer; zero if not a signal */
    t_sample *x_endbuf;
    t_sample *x_empty;       /* next to read out of buffer in epilog code */
    t_sample *x_write;       /* next to write in to buffer */
    int x_hop;              /* hopsize */
        /* vice versa from the inlet, if we don't block, this holds the
        parent's outlet signal, valid between the prolog and the dsp setup
        routines.  */
    t_signal *x_directsignal;
        /* and here's a flag indicating that we aren't blocked but have to
        do a copy (because we're switched). */
    char x_justcopyout;
  t_resample x_updown;
} t_voutlet;

static void *voutlet_new(t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new(voutlet_class);
    x->x_canvas = canvas_getCurrent();
    x->x_parentoutlet = canvas_addOutlet (x->x_canvas, &x->x_obj.te_g.g_pd, 0);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, 0, 0);
    x->x_bufsize = 0;
    x->x_buf = 0;
    return (x);
}

static void voutlet_bang(t_voutlet *x)
{
    outlet_bang(x->x_parentoutlet);
}

static void voutlet_pointer(t_voutlet *x, t_gpointer *gp)
{
    outlet_pointer(x->x_parentoutlet, gp);
}

static void voutlet_float(t_voutlet *x, t_float f)
{
    outlet_float(x->x_parentoutlet, f);
}

static void voutlet_symbol(t_voutlet *x, t_symbol *s)
{
    outlet_symbol(x->x_parentoutlet, s);
}

static void voutlet_list(t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_parentoutlet, s, argc, argv);
}

static void voutlet_anything(t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_parentoutlet, s, argc, argv);
}

static void voutlet_free(t_voutlet *x)
{
    canvas_removeOutlet (x->x_canvas, x->x_parentoutlet);
    if (x->x_buf)
        PD_MEMORY_FREE(x->x_buf);
    resample_free(&x->x_updown);
}

t_outlet *voutlet_getit(t_pd *x)
{
    if (pd_class(x) != voutlet_class) { PD_BUG; }
    return (((t_voutlet *)x)->x_parentoutlet);
}

/* ------------------------- signal outlet -------------------------- */

int voutlet_issignal(t_voutlet *x)
{
    return (x->x_buf != 0);
}

    /* LATER optimize for non-overlapped case where the "+=" isn't needed */
t_int *voutlet_perform(t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_sample *out = x->x_write, *outwas = out;
#if 0
    if (tot < 5) post("-in %lx out %lx n %d", in, out, n);
    if (tot < 5) post("-buf %lx endbuf %lx", x->x_buf, x->x_endbuf);
#endif
    while (n--)
    {
        *out++ += *in++;
        if (out == x->x_endbuf) out = x->x_buf;
    }
    outwas += x->x_hop;
    if (outwas >= x->x_endbuf) outwas = x->x_buf;
    x->x_write = outwas;
    return (w+4);
}

    /* epilog code for blocking: write buffer to parent patch */
static t_int *voutlet_doepilog(t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);

    int n = (int)(w[3]);
    t_sample *in = x->x_empty;
    if (x->x_updown.r_downSample != x->x_updown.r_upSample)
        out = x->x_updown.r_vector;

#if 0
    if (tot < 5) post("outlet in %lx out %lx n %lx", in, out, n), tot++;
#endif
    for (; n--; in++) *out++ = *in, *in = 0;
    if (in == x->x_endbuf) in = x->x_buf;
    x->x_empty = in;
    return (w+4);
}

static t_int *voutlet_doepilog_resampling(t_int *w)
{
    t_voutlet *x = (t_voutlet *)(w[1]);
    int n = (int)(w[2]);
    t_sample *in  = x->x_empty;
    t_sample *out = x->x_updown.r_vector;

#if 0
    if (tot < 5) post("outlet in %lx out %lx n %lx", in, out, n), tot++;
#endif
    for (; n--; in++) *out++ = *in, *in = 0;
    if (in == x->x_endbuf) in = x->x_buf;
    x->x_empty = in;
    return (w+3);
}



        /* prolog for outlets -- store pointer to the outlet on the
        parent, which, if "reblock" is false, will want to refer
        back to whatever we see on our input during the "dsp" method
        called later.  */
void voutlet_dspprolog(struct _voutlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample, int reblock, int switched)
{
        /* no buffer means we're not a signal outlet */
    if (!x->x_buf)
        return;
    x->x_updown.r_downSample=downsample;
    x->x_updown.r_upSample=upsample;
    x->x_justcopyout = (switched && !reblock);
    if (reblock)
    {
        x->x_directsignal = 0;
    }
    else
    {
        if (!parentsigs) { PD_BUG; }
        x->x_directsignal =
            parentsigs[object_getIndexOfSignalOutlet(x->x_parentoutlet)];
    }
}

static void voutlet_dsp(t_voutlet *x, t_signal **sp)
{
    t_signal *insig;
    if (!x->x_buf) return;
    insig = sp[0];
    if (x->x_justcopyout)
        dsp_add_copy(insig->s_vector, x->x_directsignal->s_vector, insig->s_blockSize);
    else if (x->x_directsignal)
    {
            /* if we're just going to make the signal available on the
            parent patch, hand it off to the parent signal. */
        /* this is done elsewhere--> sp[0]->s_count++; */
        signal_setborrowed(x->x_directsignal, sp[0]);
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
    if (!x->x_buf) return;  /* this shouldn't be necesssary... */
    x->x_updown.r_downSample=downsample;
    x->x_updown.r_upSample=upsample;
    if (reblock)
    {
        t_signal *insig, *outsig;
        int parentvecsize, bufsize, oldbufsize;
        int re_parentvecsize;
        int bigperiod, epilogphase, blockphase;
        if (parentsigs)
        {
            outsig = parentsigs[object_getIndexOfSignalOutlet(x->x_parentoutlet)];
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
        if (bufsize != (oldbufsize = x->x_bufsize))
        {
            t_sample *buf = x->x_buf;
            PD_MEMORY_FREE(buf);
            buf = (t_sample *)PD_MEMORY_GET(bufsize * sizeof(*buf));
            memset((char *)buf, 0, bufsize * sizeof(*buf));
            x->x_bufsize = bufsize;
            x->x_endbuf = buf + bufsize;
            x->x_buf = buf;
        }
        if (re_parentvecsize * period > bufsize) { PD_BUG; }
        x->x_write = x->x_buf + re_parentvecsize * blockphase;
        if (x->x_write == x->x_endbuf) x->x_write = x->x_buf;
        if (period == 1 && frequency > 1)
            x->x_hop = re_parentvecsize / frequency;
        else x->x_hop = period * re_parentvecsize;
        /* post("phase %d, block %d, parent %d", phase & 63,
            parentvecsize * blockphase, parentvecsize * epilogphase); */
        if (parentsigs)
        {
            /* set epilog pointer and schedule it */
            x->x_empty = x->x_buf + re_parentvecsize * epilogphase;
            if (upsample * downsample == 1)
                dsp_add(voutlet_doepilog, 3, x, outsig->s_vector,
                    re_parentvecsize);
            else
            {
                int method = (x->x_updown.r_type == 3?
                    (0 ? 0 : 1) : x->x_updown.r_type);
                dsp_add(voutlet_doepilog_resampling, 2, x, re_parentvecsize);
                resampleto_dsp(&x->x_updown, outsig->s_vector, re_parentvecsize,
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
                parentsigs[object_getIndexOfSignalOutlet(x->x_parentoutlet)];
            dsp_add_zero(outsig->s_vector, outsig->s_blockSize);
        }
    }
}

static void *voutlet_newsig(t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new(voutlet_class);
    x->x_canvas = canvas_getCurrent();
    x->x_parentoutlet = canvas_addOutlet (x->x_canvas,
        &x->x_obj.te_g.g_pd, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_endbuf = x->x_buf = (t_sample *)PD_MEMORY_GET(0);
    x->x_bufsize = 0;

    resample_init(&x->x_updown);

    /* this should be though over: 
     * it might prove hard to provide consistency between labeled up- & downsampling methods
     * maybe indeces would be better...
     *
     * up till now we provide several upsampling methods and 1 single downsampling method (no filtering !)
     */
    if (s == sym_hold)x->x_updown.r_type=1;        /* up: sample and hold */
    else if (s == sym_linear)x->x_updown.r_type=2; /* up: linear interpolation */
    else if (s == sym_pad)x->x_updown.r_type=0;    /* up: zero pad */
    else x->x_updown.r_type=3;                           /* up: zero-padding; down: ignore samples inbetween */

    return (x);
}


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
