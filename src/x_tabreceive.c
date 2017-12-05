
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
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

static void tabreceive_set (t_tabreceive *x, t_symbol *s)
{
    x->x_name = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_polling (t_tabreceive *x)
{
    t_garray *a = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
    
    if (a && (x->x_tag != garray_getTag (a))) {
    //
    post_log ("?");
    
    x->x_tag = garray_getTag (a);
    //
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
    
    class_addMethod (c, (t_method)tabreceive_set, sym_set, A_SYMBOL, A_NULL);
    class_addPolling (c, (t_method)tabreceive_polling);
    
    tabreceive_class = c;
}

void tabreceive_destroy (void)
{
    class_free (tabreceive_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
