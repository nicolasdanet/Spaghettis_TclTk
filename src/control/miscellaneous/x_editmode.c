
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

static t_class *editmode_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _editmode {
    t_object    x_obj;                  /* Must be the first. */
    int         x_editmode;
    int         x_global;
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    } t_editmode;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void editmode_setProceed (t_editmode *x)
{
    x->x_editmode = x->x_global ? instance_hasOpenedWindowInEditMode() : glist_hasEditMode (x->x_owner);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void editmode_bang (t_editmode *x)
{
    outlet_float (x->x_outlet, (t_float)x->x_editmode);
}

static void editmode_set (t_editmode *x, t_symbol *s, int argc, t_atom *argv)
{
    int old = x->x_editmode; editmode_setProceed (x); if (x->x_editmode != old) { editmode_bang (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *editmode_new (t_symbol *s, int argc, t_atom *argv)
{
    t_editmode *x = (t_editmode *)pd_new (editmode_class);
    
    if (argc && atom_getSymbol (argv) == sym___dash__global) { x->x_global = 1; argc--; argv++; }

    error__options (s, argc, argv);
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_owner  = instance_contextGetCurrent();
    x->x_outlet = outlet_newFloat (cast_object (x));
    
    pd_bind (cast_pd (x), sym__editmode);
    
    editmode_setProceed (x);
    
    return x;
}

static void editmode_free (t_editmode *x)
{
    pd_unbind (cast_pd (x), sym__editmode);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void editmode_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_editmode,
            (t_newmethod)editmode_new,
            (t_method)editmode_free,
            sizeof (t_editmode),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);

    class_addBang (c, (t_method)editmode_bang);
    
    class_addMethod (c, (t_method)editmode_set, sym_set, A_GIMME, A_NULL);

    editmode_class = c;
}

void editmode_destroy (void)
{
    class_free (editmode_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
