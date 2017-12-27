
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
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
    t_rand48    x_tag;
    t_buffer    *x_previous;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabreceive;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_output (t_tabreceive *x, t_garray *garray)
{
    int n = garray_getSize (garray);
    int i, k = 0;
    
    if (n != buffer_getSize (x->x_previous)) { k = 1; buffer_resize (x->x_previous, n); }
    
    for (i = 0; i < n; i++) {
        t_atom *a = buffer_getAtomAtIndex (x->x_previous, i);
        t_float v = garray_getDataAtIndex (garray, i);
        t_float f = atom_getFloat (a);
        if (f != v) { k = 1; }
        SET_FLOAT (a, v);
    }
    
    if (k) { outlet_list (x->x_outlet, n, buffer_getAtoms (x->x_previous)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_bang (t_tabreceive *x)
{
    t_garray *garray = (t_garray *)symbol_getThingByClass (x->x_name, garray_class);
    
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
    t_garray *garray = (t_garray *)symbol_getThingByClass (x->x_name, garray_class);
    
    if (garray && (x->x_tag != garray_getTag (garray))) {
        x->x_tag = garray_getTag (garray);
        tabreceive_output (x, garray);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabreceive_new (t_symbol *s)
{
    t_tabreceive *x = (t_tabreceive *)pd_new (tabreceive_class);
    
    x->x_name     = s;
    x->x_previous = buffer_new();
    x->x_outlet   = outlet_new (cast_object (x), &s_list);
    
    instance_pollingRegister (cast_pd (x));
    
    return x;
}

static void tabreceive_free (t_tabreceive *x)
{
    instance_pollingUnregister (cast_pd (x));
    
    buffer_free (x->x_previous);
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
