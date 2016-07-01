
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
    int             x_bufferSize;
    t_sample        *x_buffer;
    t_sample        *x_bufferEnd;
    t_sample        *x_bufferWrite;
    t_sample        *x_bufferRead;
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

static void vinlet_bang (t_vinlet *x)
{
    outlet_bang (cast_object (x)->te_outlet);
}

static void vinlet_pointer (t_vinlet *x, t_gpointer *gp)
{
    outlet_pointer (cast_object (x)->te_outlet, gp);
}

static void vinlet_float (t_vinlet *x, t_float f)
{
    outlet_float (cast_object (x)->te_outlet, f);
}

static void vinlet_symbol (t_vinlet *x, t_symbol *s)
{
    outlet_symbol (cast_object (x)->te_outlet, s);
}

static void vinlet_list (t_vinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (cast_object (x)->te_outlet, s, argc, argv);
}

static void vinlet_anything (t_vinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (cast_object (x)->te_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int *vinlet_perform (t_int *w)
{
    t_vinlet *x   = (t_vinlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n         = (int)(w[3]);
    
    t_sample *in  = x->x_bufferRead;

    while (n--) { *out++ = *in++; }
    if (in == x->x_bufferEnd) { in = x->x_buffer; }
    
    x->x_bufferRead = in;
    
    return (w + 4);
}

static t_int *vinlet_performProlog (t_int *w)
{
    t_vinlet *x   = (t_vinlet *)(w[1]);
    t_sample *in  = (t_sample *)(w[2]);
    int n         = (int)(w[3]);
    
    t_sample *out = x->x_bufferWrite;
    
    if (out == x->x_bufferEnd) {
        t_sample *f1 = x->x_buffer;
        t_sample *f2 = x->x_buffer + x->x_hopSize;
        int shift    = x->x_bufferSize - x->x_hopSize;
        out -= x->x_hopSize;
        while (shift--) { *f1++ = *f2++; }
    }

    while (n--) { *out++ = *in++; }
    
    x->x_bufferWrite = out;
    
    return (w + 4);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vinlet_dsp (t_vinlet *x, t_signal **sp)
{
    if (x->x_buffer) {
    //
    t_signal *out = sp[0];
            
    if (x->x_directSignal) { signal_setborrowed (out, x->x_directSignal); }
    else {
        dsp_add (vinlet_perform, 3, x, out->s_vector, out->s_vectorSize);
        x->x_bufferRead = x->x_buffer;
    }
    //
    }
}

void vinlet_dspProlog (struct _vinlet *x,
    t_signal **parentSignals,
    int vectorSize,
    int size,
    int phase,
    int period,
    int frequency,
    int downSample,
    int upSample,
    int reblock,
    int switched)
{
    if (x->x_buffer) {
    //
    x->x_resampling.r_downSample = downSample;
    x->x_resampling.r_upSample   = upSample;

    if (!reblock) { x->x_directSignal = parentSignals[object_getIndexOfSignalInlet (x->x_inlet)]; }
    else {
    //
    t_signal *signalIn = NULL;
    
    int newBufferSize;
    int oldBufferSize;
    int parentVectorSize;
    int parentVectorSizeResampled;

    int prologPhase = (phase - 1) & (period - 1);
    
    if (parentSignals) {
        signalIn                    = parentSignals[object_getIndexOfSignalInlet (x->x_inlet)];
        parentVectorSize            = signalIn->s_vectorSize;
        parentVectorSizeResampled   = parentVectorSize * upSample / downSample;
        
    } else {
        signalIn                    = NULL;
        parentVectorSize            = 1;
        parentVectorSizeResampled   = 1;
    }

    newBufferSize = parentVectorSizeResampled;
    
    if (newBufferSize < vectorSize) { newBufferSize = vectorSize; }
    if (newBufferSize != (oldBufferSize = x->x_bufferSize)) {
        t_sample *t = x->x_buffer;
        PD_MEMORY_FREE (t);
        t = (t_sample *)PD_MEMORY_GET (newBufferSize * sizeof (t_sample));
        memset ((char *)t, 0, newBufferSize * sizeof (t_sample));
        x->x_bufferSize = newBufferSize;
        x->x_bufferEnd  = t + newBufferSize;
        x->x_buffer     = t;
    }
    
    if (!parentSignals) { memset ((char *)x->x_buffer, 0, newBufferSize * sizeof (t_sample)); }
    else {
    //
    x->x_hopSize = period * parentVectorSizeResampled;
    x->x_bufferWrite = x->x_bufferEnd - (x->x_hopSize - prologPhase * parentVectorSizeResampled);

    if (upSample * downSample == 1) {
        dsp_add (vinlet_performProlog, 3, x, signalIn->s_vector, parentVectorSizeResampled);
        
    } else {
        resamplefrom_dsp (&x->x_resampling, 
            signalIn->s_vector,
            parentVectorSize,
            parentVectorSizeResampled,
            (x->x_resampling.r_type == 3) ? 0 : x->x_resampling.r_type);
            
        dsp_add (vinlet_performProlog, 3, x, x->x_resampling.r_vector, parentVectorSizeResampled);
    }

    /* Free signal with a zero reference count. */
        
    if (!signalIn->s_count) { signal_makereusable (signalIn); }
    //
    }
    
    x->x_directSignal = NULL;
    //
    }
    //
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
    x->x_buffer         = (t_sample *)PD_MEMORY_GET (0);
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
