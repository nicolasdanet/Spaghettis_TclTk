
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
    t_pd            i_pd;                   /* MUST be the first. */
    struct _inlet   *i_next;
    t_object        *i_owner;
    t_pd            *i_receiver;
    t_symbol        *i_type;
    int             i_hot;
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

t_inlet *inlet_new              (t_object *owner, t_pd *receiver, t_symbol *t, t_symbol *m);

void    inlet_free              (t_inlet *x);
void    inlet_setHot            (t_inlet *x);
void    inlet_moveFirst         (t_inlet *x);
int     inlet_isSignal          (t_inlet *x);
int     inlet_getIndexAsSignal  (t_inlet *x);

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

static inline t_float *inlet_getSignal (t_inlet *x)
{
    return &x->i_un.i_signal;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define INLET_WIDTH     7
#define INLET_HEIGHT    2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Handy functions to draw inlets. */

static inline int inlet_getOffset (int i, int n, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int w = rectangle_getWidth (r);
    
    return (a + (((w - INLET_WIDTH) * i) / ((n == 1) ? 1 : (n - 1))));
}

static inline int inlet_getMiddle (int i, int n, t_rectangle *r)
{
    return (inlet_getOffset (i, n, r) + ((INLET_WIDTH - 1) / 2));
}

static inline int inlet_getClosest (int x, int n, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int w = rectangle_getWidth (r);
    int i = (((x - a) * (n - 1) + (w / 2)) / w);
    
    PD_ASSERT (n > 0); return PD_CLAMP (i, 0, n - 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_inlet_h_
