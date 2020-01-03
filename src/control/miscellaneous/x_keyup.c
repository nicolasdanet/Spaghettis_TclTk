
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *keyup_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _keyup {
    t_object    x_obj;                  /* Must be the first. */
    int         x_edit;
    t_outlet    *x_outlet;
    } t_keyup;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void keyup_float (t_keyup *x, t_float f)
{
    if (x->x_edit || !instance_hasOpenedWindowInEditMode()) { outlet_float (x->x_outlet, f); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *keyup_new (t_symbol *s, int argc, t_atom *argv)
{
    t_keyup *x = (t_keyup *)pd_new (keyup_class);
    
    if (argc && atom_getSymbol (argv) == sym___dash__editmode) { x->x_edit = 1; argc--; argv++; }

    error__options (s, argc, argv);
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    pd_bind (cast_pd (x), sym__keyup);
    
    return x;
}

static void keyup_free (t_keyup *x)
{
    pd_unbind (cast_pd (x), sym__keyup);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void keyup_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_keyup,
            (t_newmethod)keyup_new,
            (t_method)keyup_free,
            sizeof (t_keyup),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
        
    class_addFloat (c, (t_method)keyup_float);

    class_setHelpName (c, sym_key);
    
    keyup_class = c;
}

void keyup_destroy (void)
{
    class_free (keyup_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
