
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __d_fft_ooura_h_
#define __d_fft_ooura_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define FFT_MINIMUM         4
#define FFT_MAXIMUM         65536

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int      *ooura_ip; 
extern double   *ooura_w;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void cdft       (int, int, double *, int *, double *);
void rdft       (int, int, double *, int *, double *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void ooura_initialize (int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void ooura_complexFFT (PD_RESTRICTED real, PD_RESTRICTED imaginary, int n, int type)
{
    double *t = alloca (n * 2 * sizeof (double));
    int i;
    
    for (i = 0; i < n; i++) {
    //
    t[(i * 2) + 0] = (double)real[i]; 
    t[(i * 2) + 1] = (double)imaginary[i];
    //
    }
    
    cdft (n * 2, type, t, ooura_ip, ooura_w);
    
    for (i = 0; i < n; i++) {
    //
    real[i]      = (t_sample)t[(i * 2) + 0]; 
    imaginary[i] = (t_sample)t[(i * 2) + 1];
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void fft_realFFT (int n, PD_RESTRICTED s)
{
    double *t = alloca (n * sizeof (double));
    int i, half = n / 2;
    
    for (i = 0; i < n; i++) { t[i] = (double)s[i]; }
        
    rdft (n, 1, t, ooura_ip, ooura_w);
    
    s[0]     = (t_sample)t[0];
    s[half]  = (t_sample)t[1];
    
    for (i = 1; i < half; i++) {
    //
    s[i]     = (t_sample)t[(i * 2) + 0];
    s[n - i] = (t_sample)t[(i * 2) + 1];
    //
    }
}

static inline void fft_realInverseFFT (int n, PD_RESTRICTED s)
{
    double *t = alloca (n * sizeof (double));
    int i, half = n / 2;
    
    t[0] = (double)s[0];
    t[1] = (double)s[half];
    
    for (i = 1; i < half; i++) {
    //
    t[(i * 2) + 0] = (double)s[i];
    t[(i * 2) + 1] = (double)s[n - i];
    //
    }
            
    rdft (n, -1, t, ooura_ip, ooura_w);
    
    for (i = 0; i < n; i++) { s[i] = (t_sample)(2.0 * t[i]); }
}

static inline void fft_complexFFT (int n, PD_RESTRICTED real, PD_RESTRICTED imaginary)
{
    ooura_complexFFT (real, imaginary, n, -1);
}

static inline void fft_complexInverseFFT (int n, PD_RESTRICTED real, PD_RESTRICTED imaginary)
{
    ooura_complexFFT (real, imaginary, n, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void fft_setSize (int size)
{
    ooura_initialize (size);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_fft_ooura_h_
