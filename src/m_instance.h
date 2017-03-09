
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

#define dsp_add     instance_appendToDspChain

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline t_pdinstance *instance_get (void)
{
    return pd_this;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    instance_addToRoots                     (t_glist *glist);
void    instance_removeFromRoots                (t_glist *glist);
void    instance_freeAllRoots                   (void);

void    instance_destroyAllScalarsByTemplate    (t_template *tmpl);

void    instance_initializeDspChain             (void);
void    instance_appendToDspChain               (t_perform f, int n, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_systime instance_getLogicalTime (void)
{
    return instance_get()->pd_systime;
}

static inline int instance_getDspState (void)
{
    return instance_get()->pd_dspState;
}

static inline int instance_getDspChainSize (void)
{
    return instance_get()->pd_dspChainSize;
}

static inline t_pd *instance_getBoundX (void)
{
    return s__X.s_thing;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void instance_setLogicalTime (t_systime t)
{
    instance_get()->pd_systime = t;
}

static inline void instance_setDspState (int n)
{
    instance_get()->pd_dspState = (n != 0);
}

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
#endif // __m_instance_h_
