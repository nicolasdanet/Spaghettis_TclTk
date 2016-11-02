
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_dsp_h_
#define __d_dsp_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _signal {
    t_float         s_sampleRate;
    int             s_count;
    int             s_isBorrowed;
    int             s_blockSize;
    t_sample        *s_vector;
    struct _signal  *s_borrowedFrom;
    struct _signal  *s_nextReusable;
    struct _signal  *s_nextUsed;
    };

typedef struct _resample {
    int             r_type;
    int             r_downSample;
    int             r_upSample;
    int             r_vectorSize;
    int             r_coefficientsSize;
    int             r_bufferSize;
    t_sample        *r_vector;
    t_sample        *r_coefficients;
    t_sample        *r_buffer;
    } t_resample;

typedef struct _block {
    t_object        x_obj;                  /* Must be the first. */
    int             x_vecsize;
    int             x_calcsize;
    int             x_overlap;
    int             x_phase;
    int             x_period;
    int             x_frequency;
    int             x_count;
    int             x_chainonset;  
    int             x_blocklength;
    int             x_epiloglength;
    char            x_switched;
    char            x_switchon;
    char            x_reblock;
    int             x_upsample;
    int             x_downsample;
    int             x_return;
    } t_block;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_int *(*t_perform)(t_int *args);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int           dsp_done                    (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_signal        *signal_new                 (int blockSize, t_float sampleRate);

void            signal_borrow               (t_signal *s, t_signal *toBeBorrowed);
void            signal_free                 (t_signal *s);
void            signal_clean                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            ugen_start                  (void);
void            ugen_tick                   (void);
void            ugen_stop                   (void);
int             ugen_getBuildIdentifier     (void);

t_dspcontext    *ugen_start_graph           (int, t_signal **, int, int);
void            ugen_add                    (t_dspcontext *, t_object *);
void            ugen_connect                (t_dspcontext *, t_object *, int, t_object *, int);
void            ugen_done_graph             (t_dspcontext *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        dsp_addZeroPerform      (t_sample *s, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int   *plus_perform       (t_int *args);
t_int   *copy_perform       (t_int *args);

void    dsp_add_plus        (t_sample *in1, t_sample *in2, t_sample *out, int n);
void    dsp_add_copy        (t_sample *in, t_sample *out, int n);
void    dsp_add_scalarcopy  (t_float *in, t_sample *out, int n);

void    dsp_add             (t_perform f, int n, ...);
void    pd_fft              (t_float *buffer, int npoints, int inverse);

void    mayer_fht           (t_sample *fz, int n);
void    mayer_fft           (int n, t_sample *real, t_sample *imag);
void    mayer_ifft          (int n, t_sample *real, t_sample *imag);
void    mayer_realfft       (int n, t_sample *real);
void    mayer_realifft      (int n, t_sample *real);

void    resample_init       (t_resample *x, t_symbol *type);
void    resample_free       (t_resample *x);
void    resample_dsp        (t_resample *x, t_sample *in, int insize, t_sample *out, int outsize, int m);
void    resamplefrom_dsp    (t_resample *x, t_sample *in, int insize, int outsize, int m);
void    resampleto_dsp      (t_resample *x, t_sample *out, int insize, int outsize, int m);

t_int   *block_dspProlog    (t_int *w);
t_int   *block_dspEpilog    (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        vinlet_dspProlog    (t_vinlet *x,
                                    t_signal **parentSignals,
                                    int vectorSize,
                                    int size,
                                    int phase,
                                    int period,
                                    int frequency,
                                    int downSample,
                                    int upSample,
                                    int reblock,
                                    int switched);

void        voutlet_dspProlog   (t_voutlet *x,
                                    t_signal **parentSignals,
                                    int vectorSize,
                                    int size,
                                    int phase,
                                    int period,
                                    int frequency,
                                    int downSample,
                                    int upSample,
                                    int reblock,
                                    int switched);
                                                            
void        voutlet_dspEpilog   (t_voutlet *x,
                                    t_signal **parentSignals,
                                    int vectorSize,
                                    int size,
                                    int phase,
                                    int period,
                                    int frequency,
                                    int downSample,
                                    int upSample,
                                    int reblock,
                                    int switched);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_
