
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabread4_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabread4 {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabread4;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabread4_float (t_tabread4 *x, t_float f)
{
    t_garray *a = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
    
    if (!a) { error_canNotFind (sym_tabread4, x->x_name); }
    else {
    //
    int size = 0;
    t_word *data = NULL;
    int n = (int)f;
    
    garray_getData (a, &size, &data);
    
    if (size < 4)           { outlet_float (x->x_outlet, 0.0); }
    else if (n < 1)         { outlet_float (x->x_outlet, WORD_FLOAT (data + 1)); }
    else if (n >= size - 2) { outlet_float (x->x_outlet, WORD_FLOAT (data + size - 2)); }
    else {
        outlet_float (x->x_outlet, dsp_4PointsInterpolation ((f - n), data + n - 1));
    }
    //
    }
}

static void tabread4_set (t_tabread4 *x, t_symbol *s)
{
    x->x_name = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabread4_new (t_symbol *s)
{
    t_tabread4 *x = (t_tabread4 *)pd_new (tabread4_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabread4_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabread4,
            (t_newmethod)tabread4_new,
            NULL,
            sizeof (t_tabread4),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, (t_method)tabread4_float);
    
    class_addMethod (c, (t_method)tabread4_set, sym_set, A_SYMBOL, A_NULL);
    
    tabread4_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
