
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pd *pd_new (t_class *c)
{
    t_pd *x;
    
    PD_ASSERT (c != NULL);
    PD_ASSERT (!class_isAbstract (c));
    PD_ASSERT (class_getInstanceSize (c) > 0);

    x = (t_pd *)PD_MEMORY_GET (class_getInstanceSize (c));
    
    *x = c;
    
    return x;
}

void pd_free (t_pd *x)
{
    t_class *c = pd_class (x);

    PD_ASSERT (c != NULL);
    PD_ASSERT (!class_isAbstract (c));
    PD_ASSERT (class_getInstanceSize (c) > 0);
    
    if (class_hasFreeMethod (c)) { (*(class_getFreeMethod (c))) (x); }

    if (class_isBox (c)) {
        while (object_getOutlets (cast_object (x))) { outlet_free (object_getOutlets (cast_object (x))); }
        while (object_getInlets (cast_object (x)))  { inlet_free (object_getInlets (cast_object (x)));   }
        
        if (cast_object (x)->te_buffer) { buffer_free (cast_object (x)->te_buffer); }
    }

    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pd_bang (t_pd *x)
{
    (*(class_getBangMethod (pd_class (x)))) (x);
}

void pd_float (t_pd *x, t_float f)
{
    (*(class_getFloatMethod (pd_class (x)))) (x, f);
}

void pd_symbol (t_pd *x, t_symbol *s)
{
    (*(class_getSymbolMethod (pd_class (x)))) (x, s);
}

void pd_list (t_pd *x, int argc, t_atom *argv)
{
    (*(class_getListMethod (pd_class (x)))) (x, &s_list, argc, argc ? argv : NULL);
}

void pd_pointer (t_pd *x, t_gpointer *gp)
{
    (*(class_getPointerMethod (pd_class (x)))) (x, gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
