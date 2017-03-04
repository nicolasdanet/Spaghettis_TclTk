
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *vline_tilde_class;                          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vline_tilde_segment {
    double                          s_timeTarget;
    double                          s_timeStart;
    t_sample                        s_target;
    struct _vline_tilde_segment     *s_next;
    } t_vline_tilde_segment;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _vline_tilde {
    t_object                        x_obj;                  /* Must be the first. */
    t_systime                       x_systime;
    t_sample                        x_target;
    double                          x_current;
    double                          x_increment;
    double                          x_timeTarget;
    double                          x_timePrevious;
    double                          x_timeBlock;
    double                          x_millisecondsPerSample;
    t_float                         x_timeRamp;
    t_float                         x_delay;
    t_vline_tilde_segment           *x_segments;
    t_outlet                        *x_outlet;
    } t_vline_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define VLINE_TIME_NONE             PD_DBL_MAX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vline_tilde_stop (t_vline_tilde *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int vline_tilde_isBeforeSegment (t_vline_tilde *x, double start, t_vline_tilde_segment *s)
{
    if (s->s_timeStart > start)       { return 1; }
    else if (s->s_timeStart == start) {
    //
    PD_ASSERT (x->x_timeRamp >= 0.0);
    if (x->x_timeRamp == 0.0)                   { return 1; }    /* Before if new one is instantaneous. */
    else if (s->s_timeTarget > s->s_timeStart)  { return 1; }    /* Before if old one isn't. */
    //
    }
    
    return 0;
}

static int vline_tilde_replaceFirstSegment (t_vline_tilde *x, double start)
{
    if (!x->x_segments) { return 1; }
    
    return vline_tilde_isBeforeSegment (x, start, x->x_segments);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vline_tilde_float (t_vline_tilde *x, t_float f)
{
    if (x->x_delay < 0.0) { x->x_current = f; vline_tilde_stop (x); }
    else {
    //
    t_vline_tilde_segment *segment = (t_vline_tilde_segment *)PD_MEMORY_GET (sizeof (t_vline_tilde_segment));
    t_vline_tilde_segment *deleted = NULL;
    
    double now = scheduler_getMillisecondsSince (x->x_systime);
    double start = now + (double)x->x_delay;
    
    x->x_timeRamp = (t_float)PD_MAX (0.0, x->x_timeRamp);
    
    if (vline_tilde_replaceFirstSegment (x, start)) { deleted = x->x_segments; x->x_segments = segment; }
    else {

        t_vline_tilde_segment *s1 = NULL;
        t_vline_tilde_segment *s2 = NULL;
        int k = 0;
        
        for ((s1 = x->x_segments); (s2 = s1->s_next); (s1 = s2)) {
        //
        if (vline_tilde_isBeforeSegment (x, start, s2)) {
            deleted = s2; s1->s_next = segment;
            k = 1; break;
        }
        //
        }
        
        if (!k) { s1->s_next = segment; PD_ASSERT (deleted == NULL); }
    }
    
    while (deleted) {

        t_vline_tilde_segment *t = deleted->s_next;
        PD_MEMORY_FREE (deleted);
        deleted = t;
    }
    
    segment->s_next       = NULL;
    segment->s_target     = (t_sample)f;
    segment->s_timeStart  = start;
    segment->s_timeTarget = start + (double)x->x_timeRamp;
    //
    }
    
    x->x_timeRamp = (t_float)0.0;
    x->x_delay    = (t_float)0.0;
}

static void vline_tilde_stop (t_vline_tilde *x)
{
    t_vline_tilde_segment *s1 = NULL;
    t_vline_tilde_segment *s2 = NULL;
    
    for (s1 = x->x_segments; s1; s1 = s2) { s2 = s1->s_next; PD_MEMORY_FREE (s1); }
    
    x->x_increment  = 0.0;
    x->x_timeRamp   = (t_float)0.0;
    x->x_delay      = (t_float)0.0;
    x->x_segments   = NULL;
    
    x->x_target     = (t_sample)x->x_current;
    x->x_timeTarget = VLINE_TIME_NONE;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *vline_tilde_perform (t_int *w)
{
    t_vline_tilde *x = (t_vline_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    t_vline_tilde_segment *s = x->x_segments;
    double millisecondsPerSample = x->x_millisecondsPerSample;
    double f = x->x_current;
    double increment = x->x_increment;
    double timeSample, timeNow = scheduler_getMillisecondsSince (x->x_systime);
    
    if (timeNow != x->x_timePrevious) {
    //
    x->x_timePrevious = timeNow;
    x->x_timeBlock    = timeNow - (PD_MAX (AUDIO_DEFAULT_BLOCKSIZE, n) * millisecondsPerSample);
    //
    }
    
    timeSample = x->x_timeBlock;
    
    x->x_timeBlock += n * millisecondsPerSample;
    
    while (n--) {
    //
    double timeNextSample = timeSample + millisecondsPerSample;
    
    while (s && s->s_timeStart < timeNextSample) {

        if (x->x_timeTarget <= timeNextSample) { f = x->x_target; increment = 0.0; }
        if (s->s_timeTarget <= s->s_timeStart) { f = s->s_target; increment = 0.0; }
        else {
            double incrementPerMillisecond = (s->s_target - f) / (s->s_timeTarget - s->s_timeStart);
            double rampAlreadyDone = incrementPerMillisecond * (timeNextSample - s->s_timeStart);
            f += rampAlreadyDone;
            increment = incrementPerMillisecond * millisecondsPerSample;
        }
        
        x->x_increment      = increment;
        x->x_target         = s->s_target;
        x->x_timeTarget     = s->s_timeTarget;
        
        x->x_segments = s->s_next; PD_MEMORY_FREE (s); s = x->x_segments;
    }
    
    if (x->x_timeTarget <= timeNextSample) {
        f = x->x_target; x->x_timeTarget = VLINE_TIME_NONE; x->x_increment = 0.0; increment = 0.0;
    }
    
    *out++ = (t_sample)f; f += increment; timeSample = timeNextSample;
    //
    }
    
    x->x_current = f;
    
    return (w + 4);
}

static void vline_tilde_dsp (t_vline_tilde *x, t_signal **sp)
{
    x->x_millisecondsPerSample = 1000.0 / sp[0]->s_sampleRate;
    
    dsp_add (vline_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vline_tilde_new (void)
{
    t_vline_tilde *x = (t_vline_tilde *)pd_new (vline_tilde_class);
    
    x->x_systime        = scheduler_getLogicalTime();
    x->x_timeTarget     = VLINE_TIME_NONE;
    x->x_timePrevious   = scheduler_getMillisecondsSince (x->x_systime);
    x->x_timeBlock      = x->x_timePrevious;
    x->x_outlet         = outlet_new (cast_object (x), &s_signal);
    
    inlet_newFloat (cast_object (x), &x->x_timeRamp);
    inlet_newFloat (cast_object (x), &x->x_delay);

    return x;
}

static void vline_tilde_free (t_vline_tilde *x)
{
    vline_tilde_stop (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vline_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_vline__tilde__,
            (t_newmethod)vline_tilde_new, 
            (t_method)vline_tilde_free,
            sizeof (t_vline_tilde),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addDSP (c, (t_method)vline_tilde_dsp);
    class_addFloat (c, (t_method)vline_tilde_float);

    class_addMethod (c, (t_method)vline_tilde_stop, sym_stop, A_NULL);
    
    vline_tilde_class = c;
}

void vline_tilde_destroy (void)
{
    CLASS_FREE (vline_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
