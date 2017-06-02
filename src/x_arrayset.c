
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arrayset_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arrayset {
    t_arrayrange    x_arrayrange;       /* Must be the first. */
    } t_arrayset;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arrayset_list (t_arrayset *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!arrayrange_isValid (&x->x_arrayrange)) { error_invalid (sym_array__space__set, sym_field); }
    else {
    //
    int i, start, n;
    t_array *a = arrayrange_getRange (&x->x_arrayrange, &start, &n);
    
    n = PD_MIN (argc, n);
    
    for (i = 0; i < n; i++) {
        t_float f = atom_getFloatAtIndex (i, argc, argv);
        array_setFloatAtIndex (a, start + i, arrayrange_getFieldName (&x->x_arrayrange), f);
    }
    //
    }
    
    arrayrange_update (&x->x_arrayrange);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arrayset_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arrayset *x = (t_arrayset *)arrayrange_new (arrayset_class, argc, argv, 1, 0);
    
    if (!x) {
        error_invalidArguments (sym_array__space__set, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayset_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__set,
            (t_newmethod)arrayset_new,
            (t_method)arrayclient_free,
            sizeof (t_arrayset),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)arrayset_list);
    
    class_setHelpName (c, sym_array);
    
    arrayset_class = c;
}

void arrayset_destroy (void)
{
    CLASS_FREE (arrayset_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
