
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

static t_symbol *keyup_sym;
static t_class *keyup_class;

typedef struct _keyup
{
    t_object x_obj;
} t_keyup;

static void *keyup_new( void)
{
    t_keyup *x = (t_keyup *)pd_new(keyup_class);
    outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, keyup_sym);
    return (x);
}

static void keyup_float(t_keyup *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, f);
}

static void keyup_free(t_keyup *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, keyup_sym);
}

void keyup_setup(void)
{
    keyup_class = class_new(sym_keyup,
        (t_newmethod)keyup_new, (t_method)keyup_free,
        sizeof(t_keyup), CLASS_NOINLET, 0);
    class_addFloat(keyup_class, keyup_float);
    keyup_sym = sym__keyup;
    class_setHelpName(keyup_class, sym_key);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
