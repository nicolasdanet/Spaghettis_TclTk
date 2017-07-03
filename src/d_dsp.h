
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_dsp_h_
#define __d_dsp_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*

    Ugen's routines build a temporary graph from the DSP objects.
    It is sorted next to obtain a linear list of operations to perform. 
    Memory for signals is allocated according to the interconnections.
    Once that's been done, the graph is deleted (while the signals remain).
    
    Prologue and epilogue functions manage nested graphs relations.
    With resampling and reblocking it could require additional buffers.

    The "block~" object maintains the synchronisation with the parent's DSP process.
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

#include "d_resample.h"
#include "d_perform.h"
#include "d_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _signal {
    t_float         s_sampleRate;
    int             s_vectorSize;
    int             s_hasBorrowed;
    t_sample        *s_vector;
    t_sample        *s_unused;
    struct _signal  *s_next;
    };

struct _vinlet {
    t_object        vi_obj;                 /* Must be the first. */
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
    t_object        vo_obj;                 /* Must be the first. */
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef int64_t t_phase;                    /* Assumed -1 has all bits set (two's complement). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void            dsp_setState                (int n);
int             dsp_getState                (void);

void            dsp_update                  (void);
int             dsp_suspend                 (void);
void            dsp_resume                  (int oldState);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_signal        *signal_new                 (int blockSize, t_float sampleRate);
t_signal        *signal_borrow              (t_signal *s, t_signal *toBeBorrowed);

int             signal_isCompatibleWith     (t_signal *s1, t_signal *s2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void            ugen_dspInitialize          (void);
void            ugen_dspTick                (void);
void            ugen_dspRelease             (void);
int             ugen_getBuildIdentifier     (void);
t_phase         ugen_getPhase               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_dspcontext    *ugen_graphStart            (int isTopLevel, t_signal **sp, int m, int n);

void            ugen_graphAdd               (t_dspcontext *context, t_object *o);
void            ugen_graphConnect           (t_dspcontext *context, t_object *o1, int m, t_object *o2, int n);
void            ugen_graphClose             (t_dspcontext *context);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void            canvas_dspProceed           (t_glist *glist, int isTopLevel, t_signal **sp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void            vinlet_dsp                  (t_vinlet *x, t_signal **sp);
void            vinlet_dspProlog            (t_vinlet *x, t_signal **signals, t_blockproperties *properties);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void            voutlet_dsp                 (t_voutlet *x, t_signal **sp);
void            voutlet_dspProlog           (t_voutlet *x, t_signal **signals, t_blockproperties *properties);
void            voutlet_dspEpilog           (t_voutlet *x, t_signal **signals, t_blockproperties *properties);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_dsp_h_
