
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *key_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _key {
    t_object    x_obj;              /* Must be the first. */
    t_outlet    *x_outlet;
    } t_key;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void key_float (t_key *x, t_float f)
{
    outlet_float (x->x_outlet, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *key_new (void)
{
    t_key *x = (t_key *)pd_new (key_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    pd_bind (cast_pd (x), sym__key);
    
    return x;
}

static void key_free (t_key *x)
{
    pd_unbind (cast_pd (x), sym__key);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void key_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_key,
            (t_newmethod)key_new,
            (t_method)key_free,
            sizeof (t_key),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
        
    class_addFloat (c, (t_method)key_float);
    
    key_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
