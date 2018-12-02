
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabread_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabread {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabread;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabread_float (t_tabread *x, t_float f)
{
    t_garray *a = (t_garray *)symbol_getThingByClass (x->x_name, garray_class);
    
    if (!a) { error_canNotFind (sym_tabread, x->x_name); }
    else {
        outlet_float (x->x_outlet, garray_getDataAtIndex (a, (int)f));
    }
}

static void tabread_set (t_tabread *x, t_symbol *s)
{
    x->x_name = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *tabread_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_tabread *x = (t_tabread *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym_set);
    buffer_appendSymbol (b, x->x_name);
    
    return b;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabread_new (t_symbol *s)
{
    t_tabread *x = (t_tabread *)pd_new (tabread_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabread_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabread,
            (t_newmethod)tabread_new,
            NULL,
            sizeof (t_tabread),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
        
    class_addFloat (c, (t_method)tabread_float);
    
    class_addMethod (c, (t_method)tabread_set, sym_set, A_SYMBOL, A_NULL);
    
    class_setDataFunction (c, tabread_functionData);

    tabread_class = c;
}

void tabread_destroy (void)
{
    class_free (tabread_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
