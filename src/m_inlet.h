
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_inlet_h_
#define __m_inlet_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _inlet {
    t_pd            i_pd;           /* MUST be the first. */
    struct _inlet   *i_next;
    t_object        *i_owner;
    t_pd            *i_receiver;
    t_symbol        *i_type;
    union {
        t_symbol    *i_method;
        t_gpointer  *i_pointer;
        t_float     *i_float;
        t_symbol    **i_symbol;
        t_float     i_signal;
    } i_un;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_inlet *inlet_new                  (t_object *owner, t_pd *receiver, t_symbol *t, t_symbol *m);

void    inlet_free                  (t_inlet *x);
void    inlet_moveFirst             (t_inlet *x);
int     inlet_isSignal              (t_inlet *x);
int     inlet_getSignalIndex        (t_inlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_inlet *inlet_getNext (t_inlet *x)
{
    return x->i_next;
}

static inline t_object *inlet_getOwner (t_inlet *x)
{
    return x->i_owner;
}

static inline t_float *inlet_getValueOfSignal (t_inlet *x)
{
    return &x->i_un.i_signal;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_inlet_h_
