
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *dspstatus_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dspstatus {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_status;
    int         x_reentrant;
    t_outlet    *x_outlet;
    } t_dspstatus;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void dspstatus_bang (t_dspstatus *x)
{
    outlet_float (x->x_outlet, x->x_status);
}

static void dspstatus_float (t_dspstatus *x, t_float f)
{
     x->x_status = (f != 0.0);
    
     if (!x->x_reentrant) { x->x_reentrant = 1; dspstatus_bang (x); }
    
     x->x_reentrant = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dspstatus_new (void)
{
    t_dspstatus *x = (t_dspstatus *)pd_new (dspstatus_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    pd_bind (cast_pd (x), sym__dspstatus);
    
    return x;
}

static void dspstatus_free (t_dspstatus *x)
{
    pd_unbind (cast_pd (x), sym__dspstatus);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dspstatus_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_dspstatus,
            (t_newmethod)dspstatus_new,
            (t_method)dspstatus_free,
            sizeof (t_dspstatus),
            CLASS_DEFAULT,
            A_NULL);

    class_addBang (c, (t_method)dspstatus_bang);
    class_addFloat (c, (t_method)dspstatus_float);
    
    dspstatus_class = c;
}

void dspstatus_destroy (void)
{
    class_free (dspstatus_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
