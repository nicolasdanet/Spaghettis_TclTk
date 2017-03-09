
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_clock_h_
#define __s_clock_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _clock {
    t_systime           c_systime;      /* Negative for unset clocks. */
    double              c_unit;         /* A positive value is in ticks, negative for number of samples. */
    t_clockfn           c_fn;
    void                *c_owner;
    struct _clock       *c_next;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error     clock_setUnitParsed                     (t_clock *x, t_float f, t_symbol *unitName);
void        clock_setUnitAsSamples                  (t_clock *x, double samples);
void        clock_setUnitAsMilliseconds             (t_clock *x, double ms);
t_error     clock_parseUnit                         (t_float f, 
                                                        t_symbol *unitName,
                                                        t_float *result,
                                                        int *isSamples);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_clock_h_
