
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_pd_h_
#define __m_pd_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define getbytes                PD_MEMORY_GET
#define t_resizebytes           PD_MEMORY_RESIZE
#define error                   post_error

#define SETFLOAT                SET_FLOAT

#define s_n                     s_vectorSize
#define s_sr                    s_sampleRate
#define s_vec                   s_vector

#define ob_pd                   te_g.g_pd

#define class_addmethod         class_addMethod
#define class_addlist           class_addList
#define pd_findbyclass          symbol_getThingByClass
#define garray_getfloatwords    garray_getData

#define CLASS_MAINSIGNALIN      CLASS_SIGNAL

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_CPP

static inline void outlet_list (t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x, argc, argv);
}

#endif // PD_CPP

static inline void freebytes (void *x, size_t size)
{
    PD_MEMORY_FREE (x);
}

static inline t_symbol *atom_getsymbolarg (int n, int argc, t_atom *argv)
{
    return atom_getSymbolAtIndex (n, argc, argv);
}

static inline t_float atom_getfloatarg (int n, int argc, t_atom *argv)
{
    return atom_getFloatAtIndex (n, argc, argv);
}

static inline void pd_error (void *object, const char *fmt, const char *s)
{
    post_error (fmt, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../d_fft.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef double FFTFLT;
typedef double t_floatarg;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ooura_bitrev ooura_ip
#define ooura_costab ooura_w

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int ooura_init (int n)
{
    fft_setSize (n); return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

// ====================================

static inline void mayer_realfft (int n, t_float *fz)
{
    FFTFLT *buf, *fp3;
    int i, nover2 = n/2;
    t_float *fp1, *fp2;
    buf = (FFTFLT *)alloca(n * sizeof(FFTFLT));
    if (!ooura_init(n))
        return;
    for (i = 0, fp1 = fz, fp3 = buf; i < n; i++, fp1++, fp3++)
        buf[i] = fz[i];
    rdft(n, 1, buf, ooura_bitrev, ooura_costab);
    fz[0] = buf[0];
    fz[nover2] = buf[1];
    for (i = 1, fp1 = fz+1, fp2 = fz+(n-1), fp3 = buf+2; i < nover2;
        i++, fp1++, fp2--, fp3 += 2)
            *fp1 = fp3[0], *fp2 = fp3[1];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_pd_h_
