
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

static t_class *tabreceive_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabreceive {
    t_object    x_obj;                  /* Must be the first. */
    t_seed      x_tag;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabreceive;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_output (t_tabreceive *x, t_garray *garray)
{
    t_atom *a = NULL;
    int i, n = garray_getSize (garray);
    
    PD_ATOMS_ALLOCA (a, n);
    
    for (i = 0; i < n; i++) { t_float f = garray_getDataAtIndex (garray, i); SET_FLOAT (a + i, f); }
    
    outlet_list (x->x_outlet, n, a);
    
    PD_ATOMS_FREEA (a, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_bang (t_tabreceive *x)
{
    t_garray *garray = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
    
    if (garray) { tabreceive_output (x, garray); }
}

static void tabreceive_set (t_tabreceive *x, t_symbol *s)
{
    x->x_name = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_polling (t_tabreceive *x)
{
    t_garray *garray = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
    
    if (garray && (x->x_tag != garray_getTag (garray))) {
        x->x_tag = garray_getTag (garray); tabreceive_output (x, garray);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabreceive_new (t_symbol *s)
{
    t_tabreceive *x = (t_tabreceive *)pd_new (tabreceive_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    instance_pollingRegister (cast_pd (x));
    
    return x;
}

static void tabreceive_free (t_tabreceive *x)
{
    instance_pollingUnregister (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabreceive_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabreceive,
            (t_newmethod)tabreceive_new,
            (t_method)tabreceive_free,
            sizeof (t_tabreceive),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
    
    class_addBang (c, (t_method)tabreceive_bang);
    class_addPolling (c, (t_method)tabreceive_polling);
    
    class_addMethod (c, (t_method)tabreceive_set, sym_set, A_SYMBOL, A_NULL);
    
    tabreceive_class = c;
}

void tabreceive_destroy (void)
{
    class_free (tabreceive_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
