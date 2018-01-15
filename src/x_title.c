
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *title_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _title {
    t_object    x_obj;                  /* Must be the first. */
    t_glist     *x_owner;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_title;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *title_getName (t_title *x)
{
    if (glist_isTop (x->x_owner)) { return symbol_removeExtension (x->x_name); }
    
    return x->x_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void title_bang (t_title *x)
{
    outlet_symbol (x->x_outlet, title_getName (x));
}

static void title_polling (t_title *x)
{
    t_symbol *t = glist_getName (x->x_owner); if (x->x_name != t) { x->x_name = t; title_bang (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *title_new (void)
{
    t_title *x = (t_title *)pd_new (title_class);
    
    x->x_owner  = instance_contextGetCurrent();
    x->x_name   = glist_getName (x->x_owner);
    x->x_outlet = outlet_new (cast_object (x), &s_symbol);
    
    instance_pollingRegister (cast_pd (x));
    
    return x;
}

static void title_free (t_title *x)
{
    instance_pollingUnregister (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void title_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_title,
            (t_newmethod)title_new,
            (t_method)title_free,
            sizeof (t_title),
            CLASS_DEFAULT,
            A_NULL);

    class_addBang (c, (t_method)title_bang);
    class_addPolling (c, (t_method)title_polling);
    
    title_class = c;
}

void title_destroy (void)
{
    class_free (title_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
