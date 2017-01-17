
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

/*

    Ugen's routines build a graph from the DSP objects.
    It is sorted next to obtain a linear list of operations to perform. 
    Memory for signals is allocated according to the interconnections.
    Once that's been done, the graph is deleted (while the signals remain).
    
    Prologue and epilogue functions manage nested graphs relations.
    With resampling and reblocking it could require additional buffers.

    In case of resampling techniques, the "block~" object maintains the
    synchronisation with the parent's DSP process.
    It does NOT do any computation in its own right.
    It triggers associated ugens at a supermultiple or submultiple of the upstream.
    Note that it can also be invoked just as a switch.
    
    The overall order of scheduling is,

        - inlet and outlet prologue code (1)
        - block prologue (2)
        - the ugens in the graph, including inlets and outlets
        - block epilogue (2)
        - outlet epilogue code (2)

    where (1) means, "if reblocked" and (2) means, "if reblocked or switched".

*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _signal {
    t_float         s_sampleRate;
    int             s_vectorSize;
    int             s_hasBorrowed;
    t_sample        *s_vector;
    t_sample        *s_unused;
    struct _signal  *s_next;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _resample {
    int             r_type;
    int             r_downsample;
    int             r_upsample;
    t_sample        r_buffer;
    int             r_vectorSize;
    t_sample        *r_vector;
    } t_resample;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _vinlet {
    t_object        vi_obj;                         /* Must be the first. */
    t_resample      vi_resample;
    int             vi_hopSize;
    int             vi_bufferSize;
    t_sample        *vi_buffer;
    t_sample        *vi_bufferEnd;
    t_sample        *vi_bufferWrite;
    t_sample        *vi_bufferRead;
    t_glist         *vi_owner;
    t_outlet        *vi_outlet;
    t_inlet         *vi_inlet;
    t_signal        *vi_directSignal;
    };

struct _voutlet {
    t_object        vo_obj;                         /* Must be the first. */
    t_resample      vo_resample;
    int             vo_hopSize;
    int             vo_copyOut;
    int             vo_bufferSize;
    t_sample        *vo_buffer;
    t_sample        *vo_bufferEnd;
    t_sample        *vo_bufferRead;
    t_sample        *vo_bufferWrite;
    t_glist         *vo_owner;
    t_outlet        *vo_outlet;
    t_signal        *vo_directSignal;
    };
    
typedef struct _block {
    t_object        bk_obj;                         /* Must be the first. */
    int             bk_blockSize;
    int             bk_overlap;
    int             bk_phase;
    int             bk_period;
    int             bk_frequency;
    int             bk_downsample;
    int             bk_upsample;
    int             bk_isSwitchObject;
    int             bk_isSwitchedOn;
    int             bk_isReblocked;
    int             bk_allContextLength;
    int             bk_outletEpilogLength;
    int             bk_count;
    } t_block;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_int *(*t_perform) (t_int *w);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef int64_t t_phase;        /* Assumed -1 has all bits set (two's complement). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DSP_UNITBIT             1572864.0   // (1.5 * 2^20)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DSP_UNITBIT_MSB         0x41380000 
#define DSP_UNITBIT_LSB         0x00000000 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*
    In the style of R. Hoeldrich (ICMC 1995 Banff).
    
    The trick is based on the IEEE 754 floating-point format. 
    It uses a constant offset to get the integer and the fractional parts split over
    the fourth and fifth bytes.
    Using raw cast to 32-bit integer it is therefore possible to get or to set them
    independently and (that is the goal) efficiently.
    
    Thanks to let me know links to original paper.
    
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Notice that this approach implies limitation in the range of signals allowed. */
/* An efficient mechanism to protect to overflow should be implemented. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Double precision floating-point representation (binary and hexadecimal). */

// DSP_UNITBIT
// DSP_UNITBIT + 0.5
// DSP_UNITBIT + 0.25
// DSP_UNITBIT + 0.125
// DSP_UNITBIT + 0.0625
// DSP_UNITBIT + 0.9375

// 0 10000010011 1000 00000000 00000000     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     10000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     01000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     00100000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     00010000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000000     11110000 00000000 00000000 00000000

// 0x41380000 00000000
// 0x41380000 80000000
// 0x41380000 40000000
// 0x41380000 20000000
// 0x41380000 10000000
// 0x41380000 f0000000

// DSP_UNITBIT + 1.0
// DSP_UNITBIT + 2.0
// DSP_UNITBIT + 4.0

// 0 10000010011 1000 00000000 00000001     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000010     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000100     00000000 00000000 00000000 00000000 

// 0x41380001 00000000
// 0x41380002 00000000
// 0x41380004 00000000

// DSP_UNITBIT - 0.5
// DSP_UNITBIT - 0.25
// DSP_UNITBIT - 0.125
// DSP_UNITBIT - 0.0625

// DSP_UNITBIT - 1.0
// DSP_UNITBIT - 2.0
// DSP_UNITBIT - 4.0

// 0 10000010011 0111 11111111 11111111     10000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111111     11000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111111     11100000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111111     11110000 00000000 00000000 00000000

// 0 10000010011 0111 11111111 11111111     00000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111110     00000000 00000000 00000000 00000000
// 0 10000010011 0111 11111111 11111100     00000000 00000000 00000000 00000000

// 0x4137ffff 80000000
// 0x4137ffff c0000000  
// 0x4137ffff e0000000 
// 0x4137ffff f0000000

// 0x4137ffff 00000000
// 0x4137fffe 00000000
// 0x4137fffc 00000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define COSINE_TABLE_SIZE       (1 << 9)    // 512

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define COSINE_UNITBIT          (DSP_UNITBIT * COSINE_TABLE_SIZE)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define COSINE_UNITBIT_MSB      0x41c80000 
#define COSINE_UNITBIT_LSB      0x00000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// DSP_UNITBIT * 512.0
// DSP_UNITBIT * 512.0 + 256.0
// DSP_UNITBIT * 512.0 + 128.0
// ...

// 0 10000011100 1000 00000000 00000000     00000000 00000000 00000000 00000000
// 0 10000011100 1000 00000000 00000000     10000000 00000000 00000000 00000000
// 0 10000011100 1000 00000000 00000000     01000000 00000000 00000000 00000000

// 0x41c80000 00000000
// 0x41c80000 80000000
// 0x41c80000 40000000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Notice that the trick below seems broken for index with a large value. */

/* Does it worth the cost? */

extern t_float *cos_tilde_table;

static inline t_float dsp_getCosineAt (double index)
{
    t_float f1, f2, f;
    t_rawcast64 z;
    int i;
        
    z.z_d = index + DSP_UNITBIT;
    
    i = (int)(z.z_i[PD_RAWCAST64_MSB] & (COSINE_TABLE_SIZE - 1));   /* Integer part. */
    
    z.z_i[PD_RAWCAST64_MSB] = DSP_UNITBIT_MSB;
    
    f = z.z_d - DSP_UNITBIT;  /* Fractional part. */
    
    /* Linear interpolation. */
    
    f1 = cos_tilde_table[i + 0];
    f2 = cos_tilde_table[i + 1];
    
    return (f1 + f * (f2 - f1));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define dsp_getSineAt(index)    dsp_getCosineAt ((double)(index) - (COSINE_TABLE_SIZE / 4.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_sample *resample_vector (t_resample *x)
{
    return x->r_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_float dsp_4PointsInterpolationWithFloats (t_float f, double a, double b, double c, double d)
{
    double t = c - b;
    
    return (t_float)(b + f * (t - 0.1666667 * (1.0 - f) * ((d - a - 3.0 * t) * f + (d + 2.0 * a - 3.0 * b))));
}

static inline t_float dsp_4PointsInterpolationWithWords (t_float f, t_word *data)
{
    double a = (double)WORD_FLOAT (data + 0);
    double b = (double)WORD_FLOAT (data + 1);
    double c = (double)WORD_FLOAT (data + 2);
    double d = (double)WORD_FLOAT (data + 3);
    
    return dsp_4PointsInterpolationWithFloats (f, a, b, c, d);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            dsp_state                   (void *dummy, t_symbol *s, int argc, t_atom *argv);
void            dsp_update                  (void);
int             dsp_suspend                 (void);
void            dsp_resume                  (int oldState);
t_int           dsp_done                    (t_int *w);
void            dsp_add                     (t_perform f, int n, ...);
int             dsp_isRunning               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_signal        *signal_new                 (int blockSize, t_float sampleRate);
t_signal        *signal_borrow              (t_signal *s, t_signal *toBeBorrowed);

int             signal_isCompatibleWith     (t_signal *s1, t_signal *s2);
void            signal_clean                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            ugen_dspInitialize          (void);
void            ugen_dspTick                (void);
void            ugen_dspRelease             (void);
int             ugen_getBuildIdentifier     (void);
t_phase         ugen_getPhase               (void);

t_dspcontext    *ugen_graphStart            (int isTopLevel, t_signal **sp, int m, int n);

void            ugen_graphAdd               (t_dspcontext *context, t_object *o);
void            ugen_graphConnect           (t_dspcontext *context, t_object *o1, int m, t_object *o2, int n);
void            ugen_graphClose             (t_dspcontext *context);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_dspPerform           (t_glist *glist, int isTopLevel, t_signal **sp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            vinlet_dsp                  (t_vinlet *x, t_signal **sp);
void            vinlet_dspProlog            (t_vinlet *x,
                                                t_signal **signals,
                                                int switchable,
                                                int reblocked,
                                                int blockSize,
                                                int period,
                                                int frequency,
                                                int downsample,
                                                int upsample);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            voutlet_dsp                 (t_voutlet *x, t_signal **sp);
void            voutlet_dspProlog           (t_voutlet *x,
                                                t_signal **signals,
                                                int switchable,
                                                int reblocked,
                                                int blockSize,
                                                int period,
                                                int frequency,
                                                int downsample,
                                                int upsample);
                                                            
void            voutlet_dspEpilog           (t_voutlet *x,
                                                t_signal **signals,
                                                int switchable,
                                                int reblocked,
                                                int blockSize,
                                                int period,
                                                int frequency,
                                                int downsample,
                                                int upsample);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            resample_init               (t_resample *x, t_symbol *type);
void            resample_free               (t_resample *x);
void            resample_setRatio           (t_resample *x, int downsample, int upsample);
int             resample_isRequired         (t_resample *x);
void            resample_toDsp              (t_resample *x, t_sample *s, int vectorSize, int resampledSize);
t_sample        *resample_fromDsp           (t_resample *x, t_sample *s, int vectorSize, int resampledSize);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_int           *block_performProlog        (t_int *w);
t_int           *block_performEpilog        (t_int *w);

t_float         block_getRatio              (t_block *x);
void            block_setPerformLength      (t_block *x, int allContextLength, int epilogLength);
void            block_getParameters         (t_block *x, 
                                                int *switchable,
                                                int *reblocked,
                                                int *blockSize,
                                                t_float *sampleRate,
                                                int *period,
                                                int *frequency,
                                                int *downsample,
                                                int *upsample,
                                                int parentBlockSize,
                                                t_float parentSampleRate);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    dsp_addZeroPerform                  (t_sample *s, int n);
void    dsp_addScalarPerform                (t_float *f, t_sample *dest, int n);

void    dsp_addPlusPerform                  (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void    dsp_addSubtractPerform              (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void    dsp_addMultiplyPerform              (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void    dsp_addDividePerform                (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void    dsp_addMaximumPerform               (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void    dsp_addMinimumPerform               (t_sample *src1, t_sample *src2, t_sample *dest, int n);

void    dsp_addPlusScalarPerform            (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void    dsp_addSubtractScalarPerform        (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void    dsp_addMultiplyScalarPerform        (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void    dsp_addDivideScalarPerform          (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void    dsp_addMaximumScalarPerform         (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);
void    dsp_addMinimumScalarPerform         (PD_RESTRICTED src, t_float *f, PD_RESTRICTED dest, int n);

void    dsp_addCopyPerform                  (PD_RESTRICTED src, PD_RESTRICTED dest, int n);
void    dsp_addCopyZeroPerform              (PD_RESTRICTED src, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    cos_tilde_initialize                (void);
void    cos_tilde_release                   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    mayer_fft                           (int n, t_sample *real, t_sample *imaginary);
void    mayer_ifft                          (int n, t_sample *real, t_sample *imaginary);
void    mayer_realfft                       (int n, t_sample *real);
void    mayer_realifft                      (int n, t_sample *real);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_
