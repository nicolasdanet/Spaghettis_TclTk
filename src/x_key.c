
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* ---------------------- key and its relatives ------------------ */

static t_symbol *key_sym;
static t_class *key_class;

typedef struct _key
{
    t_object x_obj;
} t_key;

static void *key_new( void)
{
    t_key *x = (t_key *)pd_new(key_class);
    outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, key_sym);
    return (x);
}

static void key_float(t_key *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, f);
}

static void key_free(t_key *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, key_sym);
}

void key_setup(void)
{
    key_class = class_new (sym_key,
        (t_newmethod)key_new, (t_method)key_free,
        sizeof(t_key), CLASS_NOINLET, 0);
    class_addFloat(key_class, key_float);
    key_sym = sym__key;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
