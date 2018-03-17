
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_core.h"
#include "x_mica.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *micaget_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micaget {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_tag;
    t_outlet    *x_outlet;
    } t_micaget;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micaget_bang (t_micaget *x)
{
    if (x->x_tag) {
    //
    mica::Concept t (concept_fetch (x->x_tag));
    
    if (t.isInteger())     { outlet_float (x->x_outlet, t.getNumerator()); }
    else if (t.isNumber()) {
        t_atom a[2];
        SET_FLOAT (a + 0, t.getNumerator());
        SET_FLOAT (a + 1, t.getDenominator());
        outlet_list (x->x_outlet, 2, a);
    } else {
        outlet_symbol (x->x_outlet, gensym (t.toString().c_str()));
    }
    //
    }
}

static void micaget_symbol (t_micaget *x, t_symbol *s)
{
    x->x_tag = s; micaget_bang (x);
}

static void micaget_list (t_micaget *x, t_symbol *s, int argc, t_atom *argv)
{
    int i; for (i = 0; i < argc; i++) { micaget_symbol (x, atom_getSymbolAtIndex (i, argc, argv)); }
}

static void micaget_anything (t_micaget *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micaget_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micaget_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micaget *x = (t_micaget *)pd_new (micaget_class);
    
    x->x_tag     = &s_;
    x->x_outlet  = outlet_newAnything (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micaget_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__get,
            (t_newmethod)micaget_new,
            NULL,
            sizeof (t_micaget),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micaget_bang);
    class_addSymbol (c, (t_method)micaget_symbol);
    class_addList (c, (t_method)micaget_list);
    class_addAnything (c, (t_method)micaget_anything);
    
    class_setHelpName (c, sym_mica);
    
    micaget_class = c;
}

void micaget_destroy (void)
{
    class_free (micaget_class);
}

#else

void micaget_setup (void)
{
}

void micaget_destroy (void)
{
}

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

