
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
// ...

// 0 10000010011 1000 00000000 00000001     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000010     00000000 00000000 00000000 00000000
// 0 10000010011 1000 00000000 00000100     00000000 00000000 00000000 00000000 

// 0x41380001 00000000
// 0x41380002 00000000
// 0x41380004 00000000

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
#pragma mark -

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

void            dsp_state                   (void *dummy, t_symbol *s, int argc, t_atom *argv);
void            dsp_update                  (void);
int             dsp_suspend                 (void);
void            dsp_resume                  (int oldState);
t_int           dsp_done                    (t_int *w);
void            dsp_add                     (t_perform f, int n, ...);

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

void            dsp_addZeroPerform          (t_sample *s, int n);
void            dsp_addPlusPerform          (t_sample *src1, t_sample *src2, t_sample *dest, int n);
void            dsp_addScalarPerform        (t_float *f, t_sample *dest, int n);

void            dsp_addCopyPerform          (PD_RESTRICTED src, PD_RESTRICTED dest, int n);
void            dsp_addCopyZeroPerform      (PD_RESTRICTED src, PD_RESTRICTED dest, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            cos_tilde_initialize        (void);
void            cos_tilde_release           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            mayer_fft                   (int n, t_sample *real, t_sample *imaginary);
void            mayer_ifft                  (int n, t_sample *real, t_sample *imaginary);
void            mayer_realfft               (int n, t_sample *real);
void            mayer_realifft              (int n, t_sample *real);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_sample *resample_vector (t_resample *x)
{
    return x->r_vector;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_
