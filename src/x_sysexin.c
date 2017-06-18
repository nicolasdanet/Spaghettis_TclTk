
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *sysexin_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _sysexin {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_sysexin;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void sysexin_list (t_sysexin *x, t_symbol *s, int argc, t_atom *argv)
{
    int byte = (int)atom_getFloatAtIndex (0, argc, argv);
    int port = (int)atom_getFloatAtIndex (1, argc, argv);
    
    outlet_float (x->x_outletRight, (t_float)port);
    outlet_float (x->x_outletLeft,  (t_float)byte);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *sysexin_new (void)
{
    t_sysexin *x = (t_sysexin *)pd_new (sysexin_class);
    
    x->x_outletLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outletRight = outlet_new (cast_object (x), &s_float);
    
    pd_bind (cast_pd (x), sym__sysexin);
    
    return x;
}

static void sysexin_free (t_sysexin *x)
{
    pd_unbind (cast_pd (x), sym__sysexin);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void sysexin_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_sysexin,
            (t_newmethod)sysexin_new,
            (t_method)sysexin_free,
            sizeof (t_sysexin),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
            
    class_addList (c, (t_method)sysexin_list);
    
    class_setHelpName (c, sym_midiout);
    
    sysexin_class = c;
}

void sysexin_destroy (void)
{
    class_free (sysexin_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
