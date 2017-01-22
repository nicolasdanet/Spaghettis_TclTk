
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *guiconnect_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _guiconnect {
    t_object    x_obj;                  /* Must be the first. */
    t_pd        *x_owner;
    t_symbol    *x_bound;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void guiconnect_release (t_guiconnect *x)
{
    x->x_owner = NULL;
    
    if (!x->x_bound) { pd_free (cast_pd (x)); }
    else {
        autorelease_add (cast_pd (x));
    }    
}

static void guiconnect_signoff (t_guiconnect *x)
{
    if (x->x_owner) { pd_unbind (cast_pd (x), x->x_bound); x->x_bound = NULL; }
}

static void guiconnect_autorelease (t_guiconnect *x)
{
    autorelease_perform (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void guiconnect_anything (t_guiconnect *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_owner) { pd_message (x->x_owner, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *guiconnect_getBoundAsString (t_guiconnect *x)
{
    if (x->x_bound) { return x->x_bound->s_name; }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_guiconnect *guiconnect_newWithBound (t_pd *owner, t_symbol *bindTo)
{
    t_guiconnect *x = (t_guiconnect *)pd_new (guiconnect_class);
    
    PD_ASSERT (owner);
    PD_ASSERT (bindTo);
    
    x->x_owner = owner;
    x->x_bound = bindTo;
    
    pd_bind (cast_pd (x), x->x_bound);
    
    return x;
}

t_guiconnect *guiconnect_new (t_pd *owner)
{
    char t[PD_STRING] = { 0 }; t_error err = string_sprintf (t, PD_STRING, ".x%lx", owner);
    
    PD_ASSERT (!err);
    
    return guiconnect_newWithBound (owner, gensym (t));
}

static void guiconnect_free (t_guiconnect *x)
{
    if (x->x_bound) { pd_unbind (cast_pd (x), x->x_bound); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void guiconnect_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_guiconnect,
            NULL,
            (t_method)guiconnect_free,
            sizeof (t_guiconnect), 
            CLASS_NOBOX,
            A_NULL);
        
    class_addAnything (c, (t_method)guiconnect_anything);
    class_addAutorelease (c, (t_method)guiconnect_autorelease);
    
    class_addMethod (c, (t_method)guiconnect_signoff, sym__signoff, A_NULL);
    
    guiconnect_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
