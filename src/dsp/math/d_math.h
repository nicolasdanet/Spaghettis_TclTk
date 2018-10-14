
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_math_h_
#define __d_math_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _unop_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    };

struct _binop_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    };

struct _binopscalar_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    t_float     x_scalar;
    t_outlet    *x_outlet;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *unop_tilde_functionData           (t_gobj *z, int flags);
t_buffer *binop_tilde_functionData          (t_gobj *z, int flags);
t_buffer *binopScalar_tilde_functionData    (t_gobj *z, int flags);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void unop_tilde_signals                     (struct _unop_tilde *x, t_float f);
void binop_tilde_signals                    (struct _binop_tilde *x, t_symbol *s, int argc, t_atom *argv);
void binopScalar_tilde_signals              (struct _binopscalar_tilde *x, t_float f);
void binopScalar_tilde_restore              (struct _binopscalar_tilde *x, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_math_h_
