
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
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---  array_client - common code for objects that refer to arrays -- */

#define x_sym ar_arrayclient.ac_name
#define x_struct ar_arrayclient.ac_templateIdentifier
#define x_field ar_arrayclient.ac_fieldName
#define x_gp ar_arrayclient.ac_gpointer
#define x_outlet ar_arrayclient.ac_obj.te_outlet

/* ----  array random -- output random value with array as distribution ---- */
static t_class *array_random_class;

typedef struct _array_random   /* any operation meaningful on a subrange */
{
    t_arrayrange x_r;
    unsigned int x_state;
} t_array_random;

void *arrayrandom_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_random *x = arrayrange_new(array_random_class,
        argc, argv, 0, 1);
    if (!x) { return NULL; } /* FREE & WARN ! */
    static unsigned int random_nextseed = 584926371;
    random_nextseed = random_nextseed * 435898247 + 938284287;
    x->x_state = random_nextseed;
    outlet_new(&x->x_r.ar_arrayclient.ac_obj, &s_float);
    return (x);
}

static void array_random_seed(t_array_random *x, t_float f)
{
    x->x_state = f;
}

static void array_random_bang(t_array_random *x)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i;
    
    if (!array_rangeop_getrange(&x->x_r, &firstitem, &nitem, &stride,
        &arrayonset))
            return;
    x->x_state = x->x_state * 472940017 + 832416023;
    
    arrayrange_quantile (&x->x_r, (1./4294967296.0) * (double)(x->x_state)); /* !!! */
}

static void array_random_float(t_array_random *x, t_float f)
{
    x->x_r.ar_first = f;
    array_random_bang(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arrayrandom_setup (void)
{
    array_random_class = class_new(sym_array__space__random,
        (t_newmethod)arrayrandom_new, (t_method)arrayclient_free,
            sizeof(t_array_random), 0, A_GIMME, 0);
    class_addMethod(array_random_class, (t_method)array_random_seed,
        sym_seed, A_FLOAT, 0);
    class_addFloat(array_random_class, array_random_float);
    class_addBang(array_random_class, array_random_bang);
    class_setHelpName(array_random_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
