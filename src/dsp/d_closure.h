
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_closure_h_
#define __d_closure_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _closure {
    struct _closure     *s_next;
    int                 s_type;
    } t_closure;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _space {
    t_closure           s_closure;                  /* Must be the first. */
    t_float             s_float0;
    t_float             s_float1;
    t_float             s_float2;
    t_float             s_float3;
    t_float             s_float4;
    int                 s_int0;
    int                 s_int1;
    int                 s_int2;
    int                 s_int3;
    int                 s_int4;
    void                *s_pointer0;
    } t_space;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sfvectors {
    t_closure           s_closure;                  /* Must be the first. */
    int                 s_size;
    t_sample            *(s_v[SOUNDFILE_CHANNELS]);
    } t_sfvectors;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _FFTState {
    t_closure           s_closure;                  /* Must be the first. */
    int                 s_size;
    double              *s_cache;
    } t_FFTState;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _garbage {
    t_closure           s_closure;                  /* Must be the first. */
    void                *s_ptr;
    } t_garbage;


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _blockclosure {
    t_closure           s_closure;                  /* Must be the first. */
    int                 s_reblocked;                /* True if reblocking is required. */
    int                 s_contextLength;            /* Size of the DSP chain for all the context. */
    int                 s_epilogueLength;           /* Size of the DSP chain for the epilogue. */
    int                 s_phase;                    /* Index for supermultiple block size. */
    int                 s_period;                   /* Supermultiple factor. */
    int                 s_count;                    /* Counter for submultiple block size. */
    int                 s_frequency;                /* Submultiple factor. */
    } t_blockclosure;

typedef struct _vinletclosure {
    t_closure           s_closure;                  /* Must be the first. */
    int                 s_zeroed;
    int                 s_hopSize;                  /* Size of the hop if overlapped. */
    int                 s_inSize;
    int                 s_bufferSize;               /* Handle vector size conversion in a buffer. */
    t_sample            *s_in;
    t_sample            *s_buffer;
    t_sample            *s_bufferEnd;
    t_sample            *s_bufferWrite;
    t_sample            *s_bufferRead;
    } t_vinletclosure;

typedef struct _voutletclosure {
    t_closure           s_closure;                  /* Must be the first. */
    int                 s_hopSize;                  /* Size of the hop if overlapped. */
    int                 s_outSize;
    int                 s_bufferSize;               /* Handle vector size conversion in a buffer. */
    t_sample            *s_out;
    t_sample            *s_buffer;
    t_sample            *s_bufferEnd;
    t_sample            *s_bufferWrite;
    t_sample            *s_bufferRead;
    } t_voutletclosure;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void fft_stateRelease       (t_FFTState *x);
void fft_stateInitialize    (t_FFTState *x, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_space             *space_new          (void);
t_sfvectors         *sfvectors_new      (void);
t_FFTState          *fftstate_new       (int n);

void                garbage_newRaw      (void *m);
void                garbage_newObject   (t_gobj *o);

t_blockclosure      *block_newClosure   (void);
t_vinletclosure     *vinlet_newClosure  (void);
t_voutletclosure    *voutlet_newClosure (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_closure_h_
