
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

static t_class *tabwrite_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabwrite {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_index;
    t_symbol    *x_name;
    } t_tabwrite;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabwrite_float (t_tabwrite *x, t_float f)
{
    t_garray *a = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
    
    if (!a) { error_canNotFind (sym_tabwrite, x->x_name); }
    else {
        garray_setDataAtIndex (a, x->x_index, f);
        garray_redraw (a);
    }
}

static void tabwrite_set (t_tabwrite *x, t_symbol *s)
{
    x->x_name = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabwrite_new (t_symbol *s)
{
    t_tabwrite *x = (t_tabwrite *)pd_new (tabwrite_class);
    
    x->x_name = s;
    
    inlet_newFloat (cast_object (x), &x->x_index);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabwrite_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabwrite,
            (t_newmethod)tabwrite_new,
            NULL,
            sizeof (t_tabwrite),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, (t_method)tabwrite_float);
    
    class_addMethod (c, (t_method)tabwrite_set, sym_set, A_SYMBOL, A_NULL);
    
    tabwrite_class = c;
}

void tabwrite_destroy (void)
{
    class_free (tabwrite_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
