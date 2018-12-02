
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_clocks_h_
#define __m_clocks_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A simple non-blocking bounded collection of pointers to manage clocks. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_clocks    *clocks_new     (void);

void        clocks_free     (t_clocks *x);
int         clocks_clean    (t_clocks *x);      /* Block other threads. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        clocks_add      (t_clocks *x, t_clock *c);
void        clocks_remove   (t_clocks *x, t_clock *c);
void        clocks_tick     (t_clocks *x, t_systime systime);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_clocks_h_
