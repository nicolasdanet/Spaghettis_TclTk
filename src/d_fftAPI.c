
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"

void cdft(int, int, double *, int *, double *);
void rdft(int, int, double *, int *, double *);

static int ooura_maxn;
static int *ooura_bitrev;
static int ooura_bitrevsize;
static double *ooura_costab;

static int ooura_init( int n)
{
    n = (1 << math_ilog2 (n));
    if (n < 64)
        return (0);
    if (n > ooura_maxn)
    {
        if (ooura_maxn)
        {
            PD_MEMORY_FREE(ooura_bitrev);
            PD_MEMORY_FREE(ooura_costab);
        }
        ooura_bitrevsize = sizeof(int) * (2 + (1 << (math_ilog2 (n)/2)));
        ooura_bitrev = (int *)PD_MEMORY_GET(ooura_bitrevsize);
        ooura_bitrev[0] = 0;
        if (!ooura_bitrev)
        {
            post_error ("out of memory allocating FFT buffer");
            ooura_maxn = 0;
            return (0);
        }
        ooura_costab = (double *)PD_MEMORY_GET(n * sizeof(double)/2);
        if (!ooura_costab)
        {
            post_error ("out of memory allocating FFT buffer");
            PD_MEMORY_FREE(ooura_bitrev);
            ooura_maxn = 0;
            return (0);
        }
        ooura_maxn = n;
        ooura_bitrev[0] = 0;
    }
    return (1);
}

void mayer_fht(t_sample *fz, int n)
{
    post("FHT: not yet implemented");
}

void mayer_dofft(t_sample *fz1, t_sample *fz2, int n, int sgn)
{
    double *buf, *fp3;
    int i;
    t_sample *fp1, *fp2;
    buf = alloca(n * (2 * sizeof(double)));
    if (!ooura_init(2*n))
        return;
    for (i = 0, fp1 = fz1, fp2 = fz2, fp3 = buf; i < n; i++)
    {
        fp3[0] = *fp1++;
        fp3[1] = *fp2++;
        fp3 += 2;
    }
    cdft(2*n, sgn, buf, ooura_bitrev, ooura_costab);
    for (i = 0, fp1 = fz1, fp2 = fz2, fp3 = buf; i < n; i++)
    {
        *fp1++ = fp3[0];
        *fp2++ = fp3[1];
        fp3 += 2;
    }
}

void mayer_FFT(int n, t_sample *fz1, t_sample *fz2)
{
    mayer_dofft(fz1, fz2, n, -1);
}

void mayer_inverseFFT(int n, t_sample *fz1, t_sample *fz2)
{
    mayer_dofft(fz1, fz2, n, 1);
}

void mayer_realFFT(int n, t_sample *fz)
{
    double *buf, *fp3;
    int i, nover2 = n/2;
    t_sample *fp1, *fp2;
    buf = alloca(n * sizeof(double));
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

void mayer_realInverseFFT(int n, t_sample *fz)
{
    double *buf, *fp3;
    int i, nover2 = n/2;
    t_sample *fp1, *fp2;
    buf = alloca(n * sizeof(double));
    if (!ooura_init(n))
        return;
    buf[0] = fz[0];
    buf[1] = fz[nover2];
    for (i = 1, fp1 = fz+1, fp2 = fz+(n-1), fp3 = buf+2; i < nover2;
        i++, fp1++, fp2--, fp3 += 2)
            fp3[0] = *fp1, fp3[1] = *fp2;
    rdft(n, -1, buf, ooura_bitrev, ooura_costab);
    for (i = 0, fp1 = fz, fp3 = buf; i < n; i++, fp1++, fp3++)
        fz[i] = 2*buf[i];
}

    /* ancient ISPW-like version, used in fiddle~ and perhaps other externs
    here and there. */
/*
void pd_fft(t_float *buf, int npoints, int inverse)
{
    double *buf2 = (double *)alloca(2 * npoints * sizeof(double)), *bp2;
    t_float *fp;
    int i;
    if (!ooura_init(2*npoints))
        return;
    for (i = 0, bp2 = buf2, fp = buf; i < 2 * npoints; i++, bp2++, fp++)
        *bp2 = *fp;
    cdft(2*npoints, (inverse ? 1 : -1), buf2, ooura_bitrev, ooura_costab);
    for (i = 0, bp2 = buf2, fp = buf; i < 2 * npoints; i++, bp2++, fp++)
        *fp = *bp2;
}
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
