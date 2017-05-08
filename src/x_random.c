
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Notice that it is a strictly homebrew and untested PRNG. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *random_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _random {
    t_object        x_obj;          /* Must be the first. */
    t_float         x_range;
    unsigned int    x_state;
    t_outlet        *x_outlet;
    } t_random;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int random_makeSeed (void)
{
    static unsigned int random_seed = 1489853723;
    
    random_seed = random_seed * 435898247 + 938284287;
    
    return (random_seed & PD_INT_MAX);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void random_bang (t_random *x)                       /* Weird LCG kept for compatbility. */
{
    int range = PD_MAX (1, (int)x->x_range);
    int k = 0;
        
    x->x_state = x->x_state * 472940017 + 832416023;
    k = (int)((double)range * (double)x->x_state * (1.0 / 4294967296.0));
    k = PD_MIN (k, range - 1);
    
    outlet_float (x->x_outlet, (t_float)k);
}

static void random_seed (t_random *x, t_float f)
{
    x->x_state = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *random_new (t_float f)
{
    t_random *x = (t_random *)pd_new (random_class);
    
    x->x_range  = f;
    x->x_state  = (unsigned int)random_makeSeed();
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    inlet_newFloat (cast_object (x), &x->x_range);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void random_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_random,
            (t_newmethod)random_new,
            NULL,
            sizeof (t_random),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addBang (c, (t_method)random_bang);
    
    class_addMethod (c, (t_method)random_seed, sym_seed, A_FLOAT, A_NULL);
    
    random_class = c;
}

void random_destroy (void)
{
    CLASS_FREE (random_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
