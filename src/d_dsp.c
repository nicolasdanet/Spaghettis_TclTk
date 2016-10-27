
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
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

extern t_pdinstance *pd_this;

int dsp_phase;

/* ------------------ DSP call list ----------------------- */

t_int dsp_done(t_int *w)
{
    return (0);
}

void dsp_add(t_perform f, int n, ...)
{
    int newsize = pd_this->pd_dspChainSize + n+1, i;
    va_list ap;

    pd_this->pd_dspChain = PD_MEMORY_RESIZE(pd_this->pd_dspChain, 
        pd_this->pd_dspChainSize * sizeof (t_int), newsize * sizeof (t_int));
    pd_this->pd_dspChain[pd_this->pd_dspChainSize-1] = (t_int)f;
    va_start(ap, n);
    for (i = 0; i < n; i++)
        pd_this->pd_dspChain[pd_this->pd_dspChainSize + i] = va_arg(ap, t_int);
    va_end(ap);
    pd_this->pd_dspChain[newsize-1] = (t_int)dsp_done;
    pd_this->pd_dspChainSize = newsize;
}

/*
void dsp_addv(t_perform f, int n, t_int *vec)
{
    int newsize = pd_this->pd_dspChainSize + n+1, i;
    
    pd_this->pd_dspChain = PD_MEMORY_RESIZE(pd_this->pd_dspChain, 
        pd_this->pd_dspChainSize * sizeof (t_int), newsize * sizeof (t_int));
    pd_this->pd_dspChain[pd_this->pd_dspChainSize-1] = (t_int)f;
    for (i = 0; i < n; i++)
        pd_this->pd_dspChain[pd_this->pd_dspChainSize + i] = vec[i];
    pd_this->pd_dspChain[newsize-1] = (t_int)dsp_done;
    pd_this->pd_dspChainSize = newsize;
}
*/
void ugen_tick(void)
{
    if (pd_this->pd_dspChain)
    {
        t_int *ip;
        for (ip = pd_this->pd_dspChain; ip; ) ip = (*(t_perform)(*ip))(ip);
        dsp_phase++;
    }
}

/* ---------------- signals ---------------------------- */

int ilog2(int n)
{
    int r = -1;
    if (n <= 0) return(0);
    while (n)
    {
        r++;
        n >>= 1;
    }
    return (r);
}
