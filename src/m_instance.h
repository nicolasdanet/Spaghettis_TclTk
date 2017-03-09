
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_instance_h_
#define __m_instance_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    t_systime   pd_systime;
    int         pd_dspState;
    int         pd_dspChainSize;
    t_int       *pd_dspChain;
    t_clock     *pd_clocks;
    t_signal    *pd_signals;
    t_glist     *pd_roots;
    t_clock     *pd_polling;
    t_clock     *pd_autorelease;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    instance_destroyAllScalarsByTemplate    (t_template *tmpl);
void    instance_addToRoots                     (t_glist *glist);
void    instance_removeFromRoots                (t_glist *glist);
void    instance_freeAllRoots                   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void instance_setBoundN (t_pd *x)
{
    s__N.s_thing = x;
}

static inline void instance_setBoundX (t_pd *x)
{
    s__X.s_thing = x;
}

static inline void instance_setBoundA (t_pd *x)
{
    s__A.s_thing = x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_pd *instance_getBoundX (void)
{
    return s__X.s_thing;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_instance_h_
