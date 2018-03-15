
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *micaset_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micaset {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_concept;
    t_outlet    *x_outlet;
    } t_micaset;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micaset_bang (t_micaset *x)
{

}

static void micaset_float (t_micaset *x, t_float f)
{

}

static void micaset_list (t_micaset *x, t_symbol *s, int argc, t_atom *argv)
{

}

static void micaset_anything (t_micaset *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micaset_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micaset_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micaset *x = (t_micaset *)pd_new (micaset_class);
    
    x->x_outlet = outlet_newSymbol (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micaset_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__set,
            (t_newmethod)micaset_new,
            NULL,
            sizeof (t_micaset),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micaset_bang);
    class_addFloat (c, (t_method)micaset_float);
    class_addList (c, (t_method)micaset_list);
    class_addAnything (c, (t_method)micaset_anything);
    
    class_setHelpName (c, sym_mica);
    
    micaset_class = c;
}

void micaset_destroy (void)
{
    class_free (micaset_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
