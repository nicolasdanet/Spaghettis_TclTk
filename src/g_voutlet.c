
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
#include "g_graphics.h"
#include "d_dsp.h"

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
    t_sample    *x_bufferRead;
    t_sample    *x_bufferWrite;
    int         x_hopSize;
    t_resample  x_resampling;
    char        x_copyOut;
    } t_voutlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outlet *voutlet_getOutlet (t_pd *x)
{
    PD_ASSERT (pd_class (x) == voutlet_class); return (((t_voutlet *)x)->x_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void voutlet_bang (t_voutlet *x)
{
    outlet_bang (x->x_outlet);
}

static void voutlet_float (t_voutlet *x, t_float f)
{
    outlet_float (x->x_outlet, f);
}

static void voutlet_symbol (t_voutlet *x, t_symbol *s)
{
    outlet_symbol (x->x_outlet, s);
}

static void voutlet_pointer (t_voutlet *x, t_gpointer *gp)
{
    outlet_pointer (x->x_outlet, gp);
}

static void voutlet_list (t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->x_outlet, argc, argv);
}

static void voutlet_anything (t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->x_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int *voutlet_perform (t_int *w)
{
    t_voutlet *x  = (t_voutlet *)(w[1]);
    t_sample *in  = (t_sample *)(w[2]);
    int n         = (int)(w[3]);
    
    t_sample *out  = x->x_bufferWrite;
    t_sample *next = out + x->x_hopSize;

    while (n--) { *out += *in; out++; in++; if (out == x->x_bufferEnd) { out = x->x_buffer; } }
    
    x->x_bufferWrite = (next >= x->x_bufferEnd) ? x->x_buffer : next;
    
    return (w + 4);
}

static t_int *voutlet_performEpilog (t_int *w)
{
    t_voutlet *x  = (t_voutlet *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n         = (int)(w[3]);
    
    t_sample *in  = x->x_bufferRead;
    
    if (x->x_resampling.r_downSample != x->x_resampling.r_upSample) { out = x->x_resampling.r_vector; }

    while (n--) { *out = *in; *in = 0.0; out++; in++; }
    if (in == x->x_bufferEnd) { in = x->x_buffer; }
    
    x->x_bufferRead = in;
    
    return (w + 4);
}

static t_int *voutlet_performEpilogWithResampling (t_int *w)
{
    t_voutlet *x  = (t_voutlet *)(w[1]);
    int n         = (int)(w[2]);
    
    t_sample *in  = x->x_bufferRead;
    t_sample *out = x->x_resampling.r_vector;

    while (n--) { *out = *in; *in = 0.0; out++; in++; }
    if (in == x->x_bufferEnd) { in = x->x_buffer; }
    
    x->x_bufferRead = in;
    
    return (w + 3);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void voutlet_dsp (t_voutlet *x, t_signal **sp)
{
    if (x->x_buffer) {
    //
    t_signal *in = sp[0];
    
    if (x->x_copyOut) { dsp_add_copy (in->s_vector, x->x_directSignal->s_vector, in->s_blockSize); }
    else if (x->x_directSignal) { signal_borrowFrom (x->x_directSignal, in); }
    else {
        dsp_add (voutlet_perform, 3, x, in->s_vector, in->s_blockSize);
    }
    //
    }
}

void voutlet_dspProlog (struct _voutlet *x,
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
    
    x->x_copyOut = (switched && !reblock);
    
    if (reblock) { x->x_directSignal = NULL; }
    else {
        PD_ASSERT (parentSignals);
        x->x_directSignal = parentSignals[object_getIndexOfSignalOutlet (x->x_outlet)];
    }
    //
    }
}

void voutlet_dspEpilog (struct _voutlet *x,
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
    t_signal *out = NULL;
        
    x->x_resampling.r_downSample = downSample;
    x->x_resampling.r_upSample   = upSample;
    
    if (reblock) {
    //
    int parentVectorSize;
    int parentVectorSizeResampled;
    int newBufferSize;
    int oldBufferSize;
    int newPeriod;
    int epilogPhase;
    int blockPhase;
    
    if (parentSignals) {
        out                         = parentSignals[object_getIndexOfSignalOutlet (x->x_outlet)];
        parentVectorSize            = out->s_blockSize;
        parentVectorSizeResampled   = parentVectorSize * upSample / downSample;
    } else {
        out                         = NULL;
        parentVectorSize            = 1;
        parentVectorSizeResampled   = 1;
    }
    
    newPeriod       = vectorSize / parentVectorSizeResampled;
    newPeriod       = PD_MAX (1, newPeriod);
    epilogPhase     = phase & (newPeriod - 1);
    blockPhase      = (phase + period - 1) & (newPeriod - 1) & (-period);
    newBufferSize   = parentVectorSizeResampled;
    
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
    
    PD_ASSERT (parentVectorSizeResampled * period <= newBufferSize);
    
    x->x_bufferWrite = x->x_buffer + parentVectorSizeResampled * blockPhase;
    
    if (x->x_bufferWrite == x->x_bufferEnd) { x->x_bufferWrite = x->x_buffer; }
    
    if (period == 1 && frequency > 1) { x->x_hopSize = parentVectorSizeResampled / frequency; }
    else { 
        x->x_hopSize = period * parentVectorSizeResampled;
    }

    if (parentSignals) {
    
        x->x_bufferRead = x->x_buffer + parentVectorSizeResampled * epilogPhase;
        
        if (upSample * downSample == 1) { 
            dsp_add (voutlet_performEpilog, 3, x, out->s_vector, parentVectorSizeResampled);
            
        } else {
            dsp_add (voutlet_performEpilogWithResampling, 2, x, parentVectorSizeResampled);
            
            resampleto_dsp (&x->x_resampling,
                out->s_vector,
                parentVectorSizeResampled,
                parentVectorSize,
                (x->x_resampling.r_type == 3) ? 1 : x->x_resampling.r_type);
        }
    }
    //
    } else if (switched) {
    //
    if (parentSignals) {
        out = parentSignals[object_getIndexOfSignalOutlet (x->x_outlet)];
        dsp_addZeroPerform (out->s_vector, out->s_blockSize);
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *voutlet_newSignal (t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new (voutlet_class);
    
    x->x_owner      = canvas_getCurrent();
    x->x_outlet     = canvas_addOutlet (x->x_owner, cast_pd (x), &s_signal);
    x->x_buffer     = (t_sample *)PD_MEMORY_GET (0);
    x->x_bufferEnd  = x->x_buffer;
    x->x_bufferSize = 0;
    
    inlet_new (cast_object (x), cast_pd (x), &s_signal, &s_signal);

    resample_init (&x->x_resampling, s);

    return x;
}

static void *voutlet_new (t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new (voutlet_class);
    
    x->x_owner      = canvas_getCurrent();
    x->x_outlet     = canvas_addOutlet (x->x_owner, cast_pd (x), &s_anything);
    x->x_bufferSize = 0;
    x->x_buffer     = NULL;
    
    inlet_new (cast_object (x), cast_pd (x), NULL, NULL);

    return x;
}

static void voutlet_free (t_voutlet *x)
{
    canvas_removeOutlet (x->x_owner, x->x_outlet);
    
    if (x->x_buffer) { PD_MEMORY_FREE (x->x_buffer); }
    
    resample_free (&x->x_resampling);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void voutlet_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_outlet,
            (t_newmethod)voutlet_new,
            (t_method)voutlet_free,
            sizeof (t_voutlet),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)voutlet_newSignal, sym_outlet__tilde__, A_DEFSYMBOL, A_NULL);
    
    class_addBang (c, voutlet_bang);
    class_addFloat (c, voutlet_float);
    class_addSymbol (c, voutlet_symbol);
    class_addPointer (c, voutlet_pointer);
    class_addList (c, voutlet_list);
    class_addAnything (c, voutlet_anything);
    
    class_addMethod (c, (t_method)voutlet_dsp, sym_dsp, A_CANT, A_NULL);
    
    class_setHelpName (c, sym_pd);
    
    voutlet_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
