
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *namecanvas_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _namecanvas {
    t_object    x_obj;                      /* Must be the first. */
    t_symbol    *x_name;
    t_glist     *x_owner;
    } t_namecanvas;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *namecanvas_new (t_symbol *s)
{
    t_namecanvas *x = (t_namecanvas *)pd_new (namecanvas_class);
    
    x->x_owner = canvas_getCurrent();
    x->x_name = s;
    
    if (x->x_name != &s_) { pd_bind (cast_pd (x->x_owner), x->x_name); }
    
    return x;
}

static void namecanvas_free (t_namecanvas *x)
{
    if (x->x_name != &s_) { pd_unbind (cast_pd (x->x_owner), x->x_name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
        
    namecanvas_class = c;
}

void namecanvas_destroy (void)
{
    CLASS_FREE (namecanvas_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
