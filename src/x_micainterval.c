
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

static t_class *micainterval_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _micainterval {
    t_object            x_obj;              /* Must be the first. */
    mica::MIR::Interval x_interval;
    t_outlet            *x_outlet;
    } t_micainterval;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void micainterval_bang (t_micainterval *x)
{
    outlet_symbol (x->x_outlet, concept_tag (x->x_interval.getName()));
}

static void micainterval_list (t_micainterval *x, t_symbol *s, int argc, t_atom *argv)
{
    mica::Concept t;
    
    if (argc == 2) {
    //
    mica::Concept a (concept_fetch (atom_getSymbolAtIndex (0, argc, argv)));
    mica::Concept b (concept_fetch (atom_getSymbolAtIndex (1, argc, argv)));
    
    if (!mica::index (mica::ChromaticPitches, a).isUndefined()) {
    if (!mica::index (mica::ChromaticPitches, b).isUndefined()) {
    //
    x->x_interval = mica::MIR::Interval::withNotes (a, b);
    //
    }
    }
    //
    }
    
    micainterval_bang (x);
}

static void micainterval_anything (t_micainterval *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)micainterval_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *micainterval_new (t_symbol *s, int argc, t_atom *argv)
{
    t_micainterval *x = (t_micainterval *)pd_new (micainterval_class);
    
    /* Calling the placement constructor should not be necessary (but in case). */
    
    try { new (x) t_micainterval; } catch (...) { PD_BUG; }
    
    x->x_outlet = outlet_newSymbol (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

static void micainterval_free (t_micainterval *x)
{
    /* Ditto for the destructor. */
    
    x->~t_micainterval();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void micainterval_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_mica__space__interval,
            (t_newmethod)micainterval_new,
            (t_method)micainterval_free,
            sizeof (t_micainterval),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)micainterval_bang);
    class_addList (c, (t_method)micainterval_list);
    class_addAnything (c, (t_method)micainterval_anything);
    
    class_setHelpName (c, sym_mica);
    
    micainterval_class = c;
}

void micainterval_destroy (void)
{
    class_free (micainterval_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#else

void micainterval_setup (void)
{
}

void micainterval_destroy (void)
{
}

#endif // PD_WITH_BELLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

