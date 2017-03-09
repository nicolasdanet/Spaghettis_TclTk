
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

void        instance_destroyScalarsByTemplate           (t_template *tmpl);
void        instance_addToRoots                         (t_glist *glist);
void        instance_removeFromRoots                    (t_glist *glist);
void        instance_freeAllRoots                       (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_instance_h_
