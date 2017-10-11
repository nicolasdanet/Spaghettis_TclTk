
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arguments_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arguments {
    t_object    x_obj;                      /* Must be the first. */
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    } t_arguments;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arguments_bang (t_arguments *x)
{
    t_environment *e = glist_getEnvironment (x->x_owner);
    
    PD_ASSERT (e);
    
    outlet_list (x->x_outlet, environment_getNumberOfArguments (e), environment_getArguments (e));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *arguments_new (void)
{
    t_arguments *x = (t_arguments *)pd_new (arguments_class);
    
    x->x_owner  = instance_contextGetCurrent();
    x->x_outlet = outlet_new (cast_object (x), &s_anything);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arguments_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_arguments,
            (t_newmethod)arguments_new,
            NULL,
            sizeof (t_arguments),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)arguments_bang);
    
    arguments_class = c;
}

void arguments_destroy (void)
{
    class_free (arguments_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
