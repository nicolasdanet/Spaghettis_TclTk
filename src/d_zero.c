/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  These routines build a copy of the DSP portion of a graph, which is
    then sorted into a linear list of DSP operations which are added to
    the DSP duty cycle called by the scheduler.  Once that's been done,
    we delete the copy.  The DSP objects are represented by "ugenbox"
    structures which are parallel to the DSP objects in the graph and
    have vectors of siginlets and sigoutlets which record their
    interconnections.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"

t_int *zero_perform(t_int *w)   /* zero out a vector */
{
    t_sample *out = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    while (n--) *out++ = 0; 
    return (w+3);
}

t_int *zero_perf8(t_int *w)
{
    t_sample *out = (t_sample *)(w[1]);
    int n = (int)(w[2]);
    
    for (; n; n -= 8, out += 8)
    {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        out[3] = 0;
        out[4] = 0;
        out[5] = 0;
        out[6] = 0;
        out[7] = 0;
    }
    return (w+3);
}

void dsp_add_zero(t_sample *out, int n)
{
    if (n&7)
        dsp_add(zero_perform, 2, out, n);
    else        
        dsp_add(zero_perf8, 2, out, n);
}

