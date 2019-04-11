
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_filters_h_
#define __d_filters_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _real_raw_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_sample    x_real;
    t_outlet    *x_outlet;
    };

struct _complex_raw_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_sample    x_real;
    t_sample    x_imaginary;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer    *real_raw_functionData      (t_gobj *z, int flags);
t_buffer    *complex_raw_functionData   (t_gobj *z, int flags);

void        real_raw_initialize         (void *lhs, void *rhs);
void        complex_raw_initialize      (void *lhs, void *rhs);

void        real_raw_initializer        (t_gobj *x);
void        complex_raw_initializer     (t_gobj *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_filters_h_
