
/* 
    Copyright (c) 1996-2001 Takuya OOURA.

    You may use, copy, modify and distribute this code for any purpose
    (include commercial use) and without fee.
    
    Please refer to this package when you modify this code.
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void cdft (int, int, double *, int *, double *);
void rdft (int, int, double *, int *, double *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      *ooura_ip;      /* Shared. */
static double   *ooura_w;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int ooura_initialize (int n)
{
    static int ooura_maximum = 0;
    
    n = (1 << math_ilog2 (n));
    if (n < 64)
        return (0);
    if (n > ooura_maximum)
    {
        if (ooura_maximum)
        {
            PD_MEMORY_FREE(ooura_ip);
            PD_MEMORY_FREE(ooura_w);
        }
        size_t bitrevsize = sizeof(int) * (2 + (1 << (math_ilog2 (n)/2)));
        ooura_ip = (int *)PD_MEMORY_GET(bitrevsize);
        ooura_ip[0] = 0;
        if (!ooura_ip)
        {
            post_error ("out of memory allocating FFT buffer");
            ooura_maximum = 0;
            return (0);
        }
        ooura_w = (double *)PD_MEMORY_GET(n * sizeof(double)/2);
        if (!ooura_w)
        {
            post_error ("out of memory allocating FFT buffer");
            PD_MEMORY_FREE(ooura_ip);
            ooura_maximum = 0;
            return (0);
        }
        ooura_maximum = n;
        ooura_ip[0] = 0;
    }
    return (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ooura_complexFFT (t_sample *fz1, t_sample *fz2, int n, int sgn)
{
    double *buf, *fp3;
    int i;
    t_sample *fp1, *fp2;
    buf = alloca(n * (2 * sizeof(double)));
    if (!ooura_initialize(2*n))
        return;
    for (i = 0, fp1 = fz1, fp2 = fz2, fp3 = buf; i < n; i++)
    {
        fp3[0] = *fp1++;
        fp3[1] = *fp2++;
        fp3 += 2;
    }
    cdft(2*n, sgn, buf, ooura_ip, ooura_w);
    for (i = 0, fp1 = fz1, fp2 = fz2, fp3 = buf; i < n; i++)
    {
        *fp1++ = fp3[0];
        *fp2++ = fp3[1];
        fp3 += 2;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void fft_complexFFT (int n, t_sample *real, t_sample *imaginary)
{
    ooura_complexFFT (real, imaginary, n, -1);
}

void fft_complexInverseFFT (int n, t_sample *real, t_sample *imaginary)
{
    ooura_complexFFT (real, imaginary, n, 1);
}

void fft_realFFT(int n, t_sample *fz)
{
    double *buf, *fp3;
    int i, nover2 = n/2;
    t_sample *fp1, *fp2;
    buf = alloca(n * sizeof(double));
    if (!ooura_initialize(n))
        return;
    for (i = 0, fp1 = fz, fp3 = buf; i < n; i++, fp1++, fp3++)
        buf[i] = fz[i];
    rdft(n, 1, buf, ooura_ip, ooura_w);
    fz[0] = buf[0];
    fz[nover2] = buf[1];
    for (i = 1, fp1 = fz+1, fp2 = fz+(n-1), fp3 = buf+2; i < nover2;
        i++, fp1++, fp2--, fp3 += 2)
            *fp1 = fp3[0], *fp2 = fp3[1];
}

void fft_realInverseFFT (int n, t_sample *fz)
{
    double *buf, *fp3;
    int i, nover2 = n/2;
    t_sample *fp1, *fp2;
    buf = alloca(n * sizeof(double));
    if (!ooura_initialize(n))
        return;
    buf[0] = fz[0];
    buf[1] = fz[nover2];
    for (i = 1, fp1 = fz+1, fp2 = fz+(n-1), fp3 = buf+2; i < nover2;
        i++, fp1++, fp2--, fp3 += 2)
            fp3[0] = *fp1, fp3[1] = *fp2;
    rdft(n, -1, buf, ooura_ip, ooura_w);
    for (i = 0, fp1 = fz, fp3 = buf; i < n; i++, fp1++, fp3++)
        fz[i] = 2*buf[i];
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void fft_initialize (void)
{

}

void fft_release (void)
{

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
