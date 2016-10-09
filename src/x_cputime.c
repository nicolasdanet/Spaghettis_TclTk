
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"

#if defined (__APPLE__) || defined (__FreeBSD__)
#define CLOCKHZ CLK_TCK
#endif
#if defined (__linux__) || defined (__CYGWIN__) || defined (ANDROID)
#define CLOCKHZ sysconf(_SC_CLK_TCK)
#endif
#if defined (__FreeBSD_kernel__) || defined(__GNU__) || defined(__OpenBSD__)
#include <time.h>
#define CLOCKHZ CLOCKS_PER_SEC
#endif

static t_class *cputime_class;

typedef struct _cputime
{
    t_object x_obj;
#ifdef _WIN32
    LARGE_INTEGER x_kerneltime;
    LARGE_INTEGER x_usertime;
    int x_warned;
#else
    struct tms x_setcputime;
#endif /* _WIN32 */
} t_cputime;

static void cputime_bang(t_cputime *x)
{
#ifdef _WIN32
    FILETIME ignorethis, ignorethat;
    BOOL retval;
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&x->x_kerneltime, (FILETIME *)&x->x_usertime);
    if (!retval)
    {
        if (!x->x_warned)
            post("cputime is apparently not supported on your platform");
        x->x_warned = 1;
        x->x_kerneltime.QuadPart = 0;
        x->x_usertime.QuadPart = 0;
    }
#else
    times(&x->x_setcputime);
#endif /* _WIN32 */
}

static void cputime_bang2(t_cputime *x)
{
#ifndef _WIN32
    t_float elapsedcpu;
    struct tms newcputime;
    times(&newcputime);
    elapsedcpu = 1000 * (
        newcputime.tms_utime + newcputime.tms_stime -
            x->x_setcputime.tms_utime - x->x_setcputime.tms_stime) / CLOCKHZ;
    outlet_float(x->x_obj.te_outlet, elapsedcpu);
#else
    t_float elapsedcpu;
    FILETIME ignorethis, ignorethat;
    LARGE_INTEGER usertime, kerneltime;
    BOOL retval;
    
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&kerneltime, (FILETIME *)&usertime);
    if (retval)
        elapsedcpu = 0.0001 *
            ((kerneltime.QuadPart - x->x_kerneltime.QuadPart) +
                (usertime.QuadPart - x->x_usertime.QuadPart));
    else elapsedcpu = 0;
    outlet_float(x->x_obj.te_outlet, elapsedcpu);
#endif /* NOT _WIN32 */
}

static void *cputime_new(void)
{
    t_cputime *x = (t_cputime *)pd_new(cputime_class);
    outlet_new(&x->x_obj, &s_float);

    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_bang, sym_inlet2);
#ifdef _WIN32
    x->x_warned = 0;
#endif
    cputime_bang(x);
    return (x);
}

void cputime_setup(void)
{
    cputime_class = class_new(sym_cputime, (t_newmethod)cputime_new, 0,
        sizeof(t_cputime), 0, 0);
    class_addBang(cputime_class, cputime_bang);
    class_addMethod(cputime_class, (t_method)cputime_bang2, sym_inlet2, 0);
}
