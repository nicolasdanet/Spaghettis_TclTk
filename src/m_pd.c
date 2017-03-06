
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *bindlist_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd *pd_new (t_class *c)
{
    t_pd *x;
    
    PD_ASSERT (c != NULL);
    PD_ASSERT (c->c_type != CLASS_ABSTRACT);
    PD_ASSERT (c->c_size > 0);

    x = (t_pd *)PD_MEMORY_GET (c->c_size);
    
    *x = c;
    
    return x;
}

void pd_free (t_pd *x)
{
    t_class *c = pd_class (x);

    PD_ASSERT (c != NULL);
    PD_ASSERT (c->c_type != CLASS_ABSTRACT);
    PD_ASSERT (c->c_size > 0);
    
    if (c->c_methodFree) { (*(c->c_methodFree))(x); }

    if (c->c_isBox) {
    //
    while (object_getFirstOutlet (cast_object (x))) { outlet_free (object_getFirstOutlet (cast_object (x))); }
    while (object_getFirstInlet (cast_object (x)))  { inlet_free (object_getFirstInlet (cast_object (x)));   }
    
    if (object_getBuffer (cast_object (x))) { 
        buffer_free (object_getBuffer (cast_object (x))); 
    }
    //
    }

    if (c->c_size) { PD_MEMORY_FREE (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bang (t_pd *x)
{
    (*(pd_class (x)->c_methodBang)) (x);
}

void pd_float (t_pd *x, t_float f)
{
    (*(pd_class (x)->c_methodFloat)) (x, f);
}

void pd_symbol (t_pd *x, t_symbol *s)
{
    (*(pd_class (x)->c_methodSymbol)) (x, s);
}

void pd_list (t_pd *x, int argc, t_atom *argv)
{
    (*(pd_class (x)->c_methodList)) (x, &s_list, argc, argv);
}

void pd_pointer (t_pd *x, t_gpointer *gp)
{
    (*(pd_class (x)->c_methodPointer)) (x, gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
