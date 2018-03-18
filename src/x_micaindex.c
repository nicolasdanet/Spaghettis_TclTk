
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

static t_class *micaindex_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micaindex {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_tag;
    t_outlet    *x_outlet;
    } t_micaindex;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micaindex_bang (t_micaindex *x)
{
    outlet_symbol (x->x_outlet, x->x_tag);
}

static void micaindex_list (t_micaindex *x, t_symbol *s, int argc, t_atom *argv)
{
    mica::Concept t;
    
    if (argc > 1) {
    //
    mica::Concept a (concept_fetch (atom_getSymbolAtIndex (0, argc, argv)));
    mica::Concept b (concept_fetch (atom_getSymbolAtIndex (1, argc, argv)));
    mica::Concept c (concept_fetch (atom_getSymbolAtIndex (2, argc, argv)));
    
    if (argc > 2) { t = mica::index (a, b, c); }
    else {
        t = mica::index (a, b);
    }
    //
    }

    x->x_tag = concept_tag (t);
    
    micaindex_bang (x);
}

static void micaindex_anything (t_micaindex *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micaindex_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micaindex_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micaindex *x = (t_micaindex *)pd_new (micaindex_class);
    
    x->x_tag    = concept_tag (mica::Undefined);
    x->x_outlet = outlet_newSymbol (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micaindex_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__index,
            (t_newmethod)micaindex_new,
            NULL,
            sizeof (t_micaindex),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micaindex_bang);
    class_addList (c, (t_method)micaindex_list);
    class_addAnything (c, (t_method)micaindex_anything);
    
    class_setHelpName (c, sym_mica);
    
    micaindex_class = c;
}

void micaindex_destroy (void)
{
    class_free (micaindex_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#else

void micaindex_setup (void)
{
}

void micaindex_destroy (void)
{
}

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

