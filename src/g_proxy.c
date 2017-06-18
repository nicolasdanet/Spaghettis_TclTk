
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *proxy_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Listener destroyed. */
/* No sender thus free it now. */
/* Sender alive thus free it later. */
 
void proxy_release (t_proxy *x)
{
    x->x_owner = NULL;
    
    if (!x->x_bound) { pd_free (cast_pd (x)); } 
    else {
        instance_autoreleaseRegister (cast_pd (x)); 
    }    
}

/* Sender destroyed. */
/* No listener thus free it now. */
/* Listener alive thus unbind it. */

static void proxy_signoff (t_proxy *x)  
{
    if (!x->x_owner) { instance_autoreleaseProceed (cast_pd (x)); }     
    else {
        pd_unbind (cast_pd (x), x->x_bound); x->x_bound = NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void proxy_autorelease (t_proxy *x)
{
    instance_autoreleaseProceed (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void proxy_anything (t_proxy *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_proxy *proxy_newWithBound (t_pd *owner, t_symbol *bindTo)
{
    t_proxy *x = (t_proxy *)pd_new (proxy_class);
    
    PD_ASSERT (owner);
    PD_ASSERT (bindTo);
    
    x->x_owner = owner;
    x->x_bound = bindTo;
    
    pd_bind (cast_pd (x), x->x_bound);
    
    return x;
}

t_proxy *proxy_new (t_pd *owner)
{
    char t[PD_STRING] = { 0 }; t_error err = string_sprintf (t, PD_STRING, ".x%lx", owner);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    return proxy_newWithBound (owner, gensym (t));
}

static void proxy_free (t_proxy *x)
{
    if (x->x_bound) { pd_unbind (cast_pd (x), x->x_bound); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void proxy_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_proxy,
            NULL,
            (t_method)proxy_free,
            sizeof (t_proxy), 
            CLASS_NOBOX,
            A_NULL);
        
    class_addAnything (c, (t_method)proxy_anything);
    
    class_addAutorelease (c, (t_method)proxy_autorelease);
    
    class_addMethod (c, (t_method)proxy_signoff, sym__signoff, A_NULL);
    
    proxy_class = c;
}

void proxy_destroy (void)
{
    class_free (proxy_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
