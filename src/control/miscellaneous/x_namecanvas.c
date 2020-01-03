
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *namecanvas_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _namecanvas {
    t_object    x_obj;              /* Must be the first. */
    t_symbol    *x_name;
    t_glist     *x_owner;
    } t_namecanvas;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void namecanvas_bind (t_namecanvas *x, t_symbol *s)
{
    x->x_name = s; if (x->x_name != &s_) { pd_bind (cast_pd (x->x_owner), x->x_name); }
}

static void namecanvas_unbind (t_namecanvas *x)
{
    if (x->x_name != &s_) { pd_unbind (cast_pd (x->x_owner), x->x_name); x->x_name = &s_; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Only used in invisible patches to force to unbind. */

void namecanvas_set (t_namecanvas *x, t_symbol *s, int argc, t_atom *argv)
{
    namecanvas_unbind (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *namecanvas_new (t_symbol *s)
{
    t_namecanvas *x = (t_namecanvas *)pd_new (namecanvas_class);
    
    x->x_owner = instance_contextGetCurrent();
    
    namecanvas_bind (x, s);
    
    return x;
}

static void namecanvas_free (t_namecanvas *x)
{
    namecanvas_unbind (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void namecanvas_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_namecanvas,
            (t_newmethod)namecanvas_new,
            (t_method)namecanvas_free,
            sizeof (t_namecanvas),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFSYMBOL,
            A_NULL);

    class_addMethod (c, (t_method)namecanvas_set, sym_set, A_GIMME, A_NULL);

    namecanvas_class = c;
}

void namecanvas_destroy (void)
{
    class_free (namecanvas_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
