
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

t_class *vinlet_class;                              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vinlet {
    t_object        x_obj;
    t_glist         *x_owner;
    t_inlet         *x_inlet;
    t_signal        *x_directSignal;
    t_float         *x_fill;
    t_float         *x_read;
    int             x_signalSize;
    t_float         *x_signal;
    t_float         *x_signalEnd;
    int             x_hopSize;
    t_resample      x_resampling;
    } t_vinlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vinlet_new(t_symbol *s)
{
    t_vinlet *x = (t_vinlet *)pd_new(vinlet_class);
    x->x_owner = canvas_getCurrent();
    x->x_inlet = canvas_addInlet (x->x_owner, &x->x_obj.te_g.g_pd, 0);
    x->x_signalSize = 0;
    x->x_signal = 0;
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void vinlet_bang(t_vinlet *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

static void vinlet_pointer(t_vinlet *x, t_gpointer *gp)
{
    outlet_pointer(x->x_obj.te_outlet, gp);
}

static void vinlet_float(t_vinlet *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, f);
}

static void vinlet_symbol(t_vinlet *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.te_outlet, s);
}

static void vinlet_list(t_vinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_obj.te_outlet, s, argc, argv);
}

static void vinlet_anything(t_vinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_obj.te_outlet, s, argc, argv);
}

static void vinlet_free(t_vinlet *x)
{
    canvas_removeInlet (x->x_owner, x->x_inlet);
    if (x->x_signal)
        PD_MEMORY_FREE(x->x_signal);
    resample_free(&x->x_resampling);
}

t_inlet *vinlet_getit(t_pd *x)
{
    if (pd_class(x) != vinlet_class) { PD_BUG; }
    return (((t_vinlet *)x)->x_inlet);
}

/* ------------------------- signal inlet -------------------------- */
int vinlet_issignal(t_vinlet *x)
{
    return (x->x_signal != 0);
}

t_int *vinlet_perform(t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float *in = x->x_read;

    while (n--) *out++ = *in++;
    if (in == x->x_signalEnd) in = x->x_signal;
    x->x_read = in;
    return (w+4);
}

static void vinlet_dsp(t_vinlet *x, t_signal **sp)
{
    t_signal *outsig;
        /* no buffer means we're not a signal inlet */
    if (!x->x_signal)
        return;
    outsig = sp[0];
    if (x->x_directSignal)
    {
        signal_setborrowed(sp[0], x->x_directSignal);
    }
    else
    {
        dsp_add(vinlet_perform, 3, x, outsig->s_vector, outsig->s_vectorSize);
        x->x_read = x->x_signal;
    }
}

    /* prolog code: loads buffer from parent patch */
t_int *vinlet_doprolog(t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float *out = x->x_fill;
    if (out == x->x_signalEnd)
    {
      t_float *f1 = x->x_signal, *f2 = x->x_signal + x->x_hopSize;
        int nshift = x->x_signalSize - x->x_hopSize;
        out -= x->x_hopSize;
        while (nshift--) *f1++ = *f2++;
    }

    while (n--) *out++ = *in++;
    x->x_fill = out;
    return (w+4);
}

        /* set up prolog DSP code  */
void vinlet_dspprolog(struct _vinlet *x, t_signal **parentsigs,
    int myvecsize, int calcsize, int phase, int period, int frequency,
    int downsample, int upsample,  int reblock, int switched)
{
    t_signal *insig, *outsig;
        /* no buffer means we're not a signal inlet */
    if (!x->x_signal)
        return;
    x->x_resampling.r_downSample = downsample;
    x->x_resampling.r_upSample   = upsample;

        /* if the "reblock" flag is set, arrange to copy data in from the
        parent. */
    if (reblock)
    {
        int parentvecsize, bufsize, oldbufsize, prologphase;
        int re_parentvecsize; /* resampled parentvectorsize */
            /* this should never happen: */
        if (!x->x_signal) return;

            /* the prolog code counts from 0 to period-1; the
            phase is backed up by one so that AFTER the prolog code
            runs, the "x_fill" phase is in sync with the "x_read" phase. */
        prologphase = (phase - 1) & (period - 1);
        if (parentsigs)
        {
            insig = parentsigs[object_getIndexOfSignalInlet(x->x_inlet)];
            parentvecsize = insig->s_vectorSize;
            re_parentvecsize = parentvecsize * upsample / downsample;
        }
        else
        {
            insig = 0;
            parentvecsize = 1;
            re_parentvecsize = 1;
        }

        bufsize = re_parentvecsize;
        if (bufsize < myvecsize) bufsize = myvecsize;
        if (bufsize != (oldbufsize = x->x_signalSize))
        {
            t_float *buf = x->x_signal;
            PD_MEMORY_FREE(buf);
            buf = (t_float *)PD_MEMORY_GET(bufsize * sizeof(*buf));
            memset((char *)buf, 0, bufsize * sizeof(*buf));
            x->x_signalSize = bufsize;
            x->x_signalEnd = buf + bufsize;
            x->x_signal = buf;
        }
        if (parentsigs)
        {
            x->x_hopSize = period * re_parentvecsize;

            x->x_fill = x->x_signalEnd -
              (x->x_hopSize - prologphase * re_parentvecsize);

            if (upsample * downsample == 1)
                    dsp_add(vinlet_doprolog, 3, x, insig->s_vector,
                        re_parentvecsize);
            else {
              int method = (x->x_resampling.r_type == 3?
                (0 ? 0 : 1) : x->x_resampling.r_type);
              resamplefrom_dsp(&x->x_resampling, insig->s_vector, parentvecsize,
                re_parentvecsize, method);
              dsp_add(vinlet_doprolog, 3, x, x->x_resampling.r_vector,
                re_parentvecsize);
        }

            /* if the input signal's reference count is zero, we have
               to free it here because we didn't in ugen_doit(). */
            if (!insig->s_count)
                signal_makereusable(insig);
        }
        else memset((char *)(x->x_signal), 0, bufsize * sizeof(*x->x_signal));
        x->x_directSignal = 0;
    }
    else
    {
            /* no reblocking; in this case our output signal is "borrowed"
            and merely needs to be pointed to the real one. */
        x->x_directSignal = parentsigs[object_getIndexOfSignalInlet(x->x_inlet)];
    }
}

static void *vinlet_newsig(t_symbol *s)
{
    t_vinlet *x = (t_vinlet *)pd_new(vinlet_class);
    x->x_owner = canvas_getCurrent();
    x->x_inlet = canvas_addInlet (x->x_owner, &x->x_obj.te_g.g_pd, &s_signal);
    x->x_signalEnd = x->x_signal = (t_float *)PD_MEMORY_GET(0);
    x->x_signalSize = 0;
    x->x_directSignal = 0;
    outlet_new(&x->x_obj, &s_signal);

    resample_init(&x->x_resampling);

    /* this should be though over: 
     * it might prove hard to provide consistency between labeled up- & downsampling methods
     * maybe indeces would be better...
     *
     * up till now we provide several upsampling methods and 1 single downsampling method (no filtering !)
     */
    if (s == sym_hold)
        x->x_resampling.r_type=1;       /* up: sample and hold */
    else if (s == sym_linear)
        x->x_resampling.r_type=2;       /* up: linear interpolation */
    else if (s == sym_pad)
        x->x_resampling.r_type=0;       /* up: zero-padding */
    else x->x_resampling.r_type=3;      /* sample/hold unless version<0.44 */

    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vinlet_setup (void)
{
    vinlet_class = class_new (sym_inlet, (t_newmethod)vinlet_new,
        (t_method)vinlet_free, sizeof(t_vinlet), CLASS_NOINLET, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)vinlet_newsig, sym_inlet__tilde__, A_DEFSYMBOL, 0);
    class_addBang(vinlet_class, vinlet_bang);
    class_addPointer(vinlet_class, vinlet_pointer);
    class_addFloat(vinlet_class, vinlet_float);
    class_addSymbol(vinlet_class, vinlet_symbol);
    class_addList(vinlet_class, vinlet_list);
    class_addAnything(vinlet_class, vinlet_anything);
    class_addMethod(vinlet_class, (t_method)vinlet_dsp, 
        sym_dsp, A_CANT, 0);
    class_setHelpName(vinlet_class, sym_pd);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
