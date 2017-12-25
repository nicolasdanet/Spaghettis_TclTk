
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_time_h_
#define __s_time_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef uint64_t t_time;
typedef uint64_t t_nano;
typedef uint64_t t_stamp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

uint64_t    time_makeRandomSeed         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Do NOT call time_addNanoseconds or time_elapsedNanoseconds at initialization time. */

/* < http://www.parashift.com/c++-faq/static-init-order.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        time_set                    (t_time *t);
void        time_addNanoseconds         (t_time *t, t_nano ns);
t_error     time_elapsedNanoseconds     (const t_time *t0, const t_time *t1, t_nano *elapsed);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        nano_sleep                  (t_nano ns);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://en.wikipedia.org/wiki/Network_Time_Protocol > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        stamp_set                   (t_stamp *stamp);
void        stamp_addNanoseconds        (t_stamp *stamp, t_nano ns);
t_error     stamp_elapsedNanoseconds    (const t_stamp *t0, const t_stamp *t1, t_nano *elapsed);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_time_h_

