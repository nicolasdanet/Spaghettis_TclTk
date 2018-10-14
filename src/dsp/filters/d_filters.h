
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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
    t_float     x_f;
    t_sample    x_real;
    t_outlet    *x_outlet;
    };

struct _complex_raw_tilde {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_f;
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

void        real_raw_restore            (struct _real_raw_tilde *x, t_float f);
void        real_raw_signals            (struct _real_raw_tilde *x, t_symbol *s, int argc, t_atom *argv);

void        complex_raw_restore         (struct _complex_raw_tilde *x, t_symbol *s, int argc, t_atom *argv);
void        complex_raw_signals         (struct _complex_raw_tilde *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_filters_h_
