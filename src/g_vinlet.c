
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
    int             x_bufferSize;
    t_float         *x_buffer;
    t_float         *x_bufferEnd;
    int             x_hopSize;
    t_resample      x_resampling;
    } t_vinlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *vinlet_getInlet (t_pd *x)
{
    PD_ASSERT (pd_class (x) == vinlet_class); return (((t_vinlet *)x)->x_inlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

/* ------------------------- signal inlet -------------------------- */
int vinlet_issignal(t_vinlet *x)
{
    return (x->x_buffer != 0);
}

t_int *vinlet_perform(t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float *in = x->x_read;

    while (n--) *out++ = *in++;
    if (in == x->x_bufferEnd) in = x->x_buffer;
    x->x_read = in;
    return (w+4);
}

static void vinlet_dsp(t_vinlet *x, t_signal **sp)
{
    t_signal *outsig;
        /* no buffer means we're not a signal inlet */
    if (!x->x_buffer)
        return;
    outsig = sp[0];
    if (x->x_directSignal)
    {
        signal_setborrowed(sp[0], x->x_directSignal);
    }
    else
    {
        dsp_add(vinlet_perform, 3, x, outsig->s_vector, outsig->s_vectorSize);
        x->x_read = x->x_buffer;
    }
}

    /* prolog code: loads buffer from parent patch */
t_int *vinlet_doprolog(t_int *w)
{
    t_vinlet *x = (t_vinlet *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float *out = x->x_fill;
    if (out == x->x_bufferEnd)
    {
      t_float *f1 = x->x_buffer, *f2 = x->x_buffer + x->x_hopSize;
        int nshift = x->x_bufferSize - x->x_hopSize;
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
    if (!x->x_buffer)
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
        if (!x->x_buffer) return;

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
        if (bufsize != (oldbufsize = x->x_bufferSize))
        {
            t_float *buf = x->x_buffer;
            PD_MEMORY_FREE(buf);
            buf = (t_float *)PD_MEMORY_GET(bufsize * sizeof(*buf));
            memset((char *)buf, 0, bufsize * sizeof(*buf));
            x->x_bufferSize = bufsize;
            x->x_bufferEnd = buf + bufsize;
            x->x_buffer = buf;
        }
        if (parentsigs)
        {
            x->x_hopSize = period * re_parentvecsize;

            x->x_fill = x->x_bufferEnd -
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
        else memset((char *)(x->x_buffer), 0, bufsize * sizeof(*x->x_buffer));
        x->x_directSignal = 0;
    }
    else
    {
            /* no reblocking; in this case our output signal is "borrowed"
            and merely needs to be pointed to the real one. */
        x->x_directSignal = parentsigs[object_getIndexOfSignalInlet(x->x_inlet)];
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vinlet_newSignal (t_symbol *s)
{
    t_vinlet *x = (t_vinlet *)pd_new (vinlet_class);
    
    x->x_owner          = canvas_getCurrent();
    x->x_inlet          = canvas_addInlet (x->x_owner, cast_pd (x), &s_signal);
    x->x_buffer         = (t_float *)PD_MEMORY_GET (0);
    x->x_bufferEnd      = x->x_buffer;
    x->x_bufferSize     = 0;
    x->x_directSignal   = NULL;
    
    outlet_new (cast_object (x), &s_signal);
    
    resample_init (&x->x_resampling, s);

    return x;
}

static void *vinlet_new (t_symbol *s)
{
    t_vinlet *x = (t_vinlet *)pd_new (vinlet_class);
    
    x->x_owner      = canvas_getCurrent();
    x->x_inlet      = canvas_addInlet (x->x_owner, cast_pd (x), NULL);
    x->x_bufferSize = 0;
    x->x_buffer     = NULL;
    
    outlet_new (cast_object (x), &s_anything);
    
    return x;
}

static void vinlet_free (t_vinlet *x)
{
    canvas_removeInlet (x->x_owner, x->x_inlet);
    
    if (x->x_buffer) { PD_MEMORY_FREE (x->x_buffer); }
    
    resample_free (&x->x_resampling);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vinlet_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_inlet,
            (t_newmethod)vinlet_new,
            (t_method)vinlet_free,
            sizeof (t_vinlet),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)vinlet_newSignal, sym_inlet__tilde__, A_DEFSYMBOL, A_NULL);
    
    class_addBang (c, vinlet_bang);
    class_addPointer (c, vinlet_pointer);
    class_addFloat (c, vinlet_float);
    class_addSymbol (c, vinlet_symbol);
    class_addList (c, vinlet_list);
    class_addAnything (c, vinlet_anything);
    
    class_addMethod (c, (t_method)vinlet_dsp, sym_dsp, A_CANT, A_NULL);
    
    class_setHelpName (c, sym_pd);
    
    vinlet_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
