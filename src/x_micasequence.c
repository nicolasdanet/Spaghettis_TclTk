
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
#include "s_system.h"
#include "x_mica.hpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *micasequence_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micasequence {
    t_object    x_obj;                  /* Must be the first. */
    t_symbol    *x_tag;
    t_outlet    *x_outlet;
    } t_micasequence;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micasequence_bang (t_micasequence *x)
{
    mica::Concept t (concept_fetch (x->x_tag));
    
    if (!t.isSequence()) { outlet_list (x->x_outlet, 0, NULL); }
    else {
    //
    int i, n = t.length();
    t_atom *a = NULL;
    
    PD_ATOMS_ALLOCA (a, n);
    
    for (i = 0; i < n; i++) {
        t_symbol *s = concept_tag (mica::item (t, i)); SET_SYMBOL (a + i, s);
    }
    
    outlet_list (x->x_outlet, n, a);
    
    PD_ATOMS_FREEA (a, n);
    //
    }
}

static void micasequence_symbol (t_micasequence *x, t_symbol *s)
{
    x->x_tag = s; micasequence_bang (x);
}

static void micasequence_list (t_micasequence *x, t_symbol *s, int argc, t_atom *argv)
{
    int i; for (i = 0; i < argc; i++) { micasequence_symbol (x, atom_getSymbolAtIndex (i, argc, argv)); }
}

static void micasequence_anything (t_micasequence *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micasequence_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micasequence_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micasequence *x = (t_micasequence *)pd_new (micasequence_class);
    
    x->x_tag    = concept_tag (mica::Undefined);
    x->x_outlet = outlet_newList (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micasequence_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__sequence,
            (t_newmethod)micasequence_new,
            NULL,
            sizeof (t_micasequence),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micasequence_bang);
    class_addSymbol (c, (t_method)micasequence_symbol);
    class_addList (c, (t_method)micasequence_list);
    class_addAnything (c, (t_method)micasequence_anything);
    
    class_setHelpName (c, sym_mica);
    
    micasequence_class = c;
}

void micasequence_destroy (void)
{
    class_free (micasequence_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#else

void micasequence_setup (void)
{
}

void micasequence_destroy (void)
{
}

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

