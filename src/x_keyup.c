
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *keyup_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _keyup {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outlet;
    } t_keyup;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void keyup_float (t_keyup *x, t_float f)
{
    outlet_float (x->x_outlet, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *keyup_new (void)
{
    t_keyup *x = (t_keyup *)pd_new (keyup_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    pd_bind (cast_pd (x), sym__keyup);
    
    return x;
}

static void keyup_free (t_keyup *x)
{
    pd_unbind (cast_pd (x), sym__keyup);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void keyup_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_keyup,
            (t_newmethod)keyup_new,
            (t_method)keyup_free,
            sizeof (t_keyup),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
        
    class_addFloat (c, (t_method)keyup_float);

    class_setHelpName (c, sym_key);
    
    keyup_class = c;
}

void keyup_destroy (void)
{
    CLASS_FREE (keyup_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
