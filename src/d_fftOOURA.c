
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"
#include "d_fftOOURA.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int      *ooura_ip;     /* Static. */
double   *ooura_w;      /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void ooura_release (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Next squarable power of two. */

static int ooura_getNextSize (int n)
{
    int t = (int)((math_ilog2 ((int)PD_NEXT_POWER_2 (n)) / 2.0) + 0.5);
    
    return (1 << (t * 2));
}

/* Initialize the tables with a dummy fft. */

static void ooura_dummy (int n)
{
    double *t = alloca (n * sizeof (double));
    
    while (n--) { *t++ = 0.0; }
        
    rdft (n, 1, t, ooura_ip, ooura_w);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ooura_initialize (int n)
{
    static int ooura_maximum = 0;
    
    n = ooura_getNextSize (n);
    
    if (n > ooura_maximum) {
    //
    int t    = (int)sqrt (n);
    size_t a = 2 + t;
    size_t b = n / 2;
    
    PD_ASSERT (PD_IS_POWER_2 (n));
    PD_ASSERT (sqrt (n) == (double)t);
    
    ooura_release();
    
    ooura_ip = (int *)PD_MEMORY_GET (a * sizeof (int));
    ooura_w  = (double *)PD_MEMORY_GET (b * sizeof (double));
    
    ooura_maximum = n;
    
    PD_ASSERT (ooura_ip[0] == 0);
    
    ooura_dummy (n);
    //
    }
}

static void ooura_release (void)
{
    if (ooura_w)  { PD_MEMORY_FREE (ooura_w); }
    if (ooura_ip) { PD_MEMORY_FREE (ooura_ip); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void fft_initialize (void)
{
    ooura_initialize (AUDIO_DEFAULT_BLOCKSIZE);
}

void fft_release (void)
{
    ooura_release();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
