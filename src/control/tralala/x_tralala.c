
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tll_algorithm.c"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_MAXIMUM_SIZE    128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tralala {
    t_object        x_obj;
    int             x_hasLearned;
    int             *x_cache;
    t_atom          *x_out;
    tll_array       *x_data;
    t_rand48        *x_random;
    tll_algorithm   *x_algorithm;
    t_glist         *x_owner;
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_tralala;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *tralala_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void tralala_float (t_tralala *x, t_float f)
{
    int size = PD_MIN ((int)f, TLL_MAXIMUM_SIZE);
    
    if (size) {
    //
    int i;
    
    if (x->x_hasLearned && !tll_algorithmProceed (x->x_algorithm, size, x->x_cache)) {
        for (i = 0; i < size; i++) { SET_FLOAT (x->x_out + i, x->x_cache[i]); }
        outlet_list (x->x_outletLeft, size, x->x_out);
    } else {
        outlet_bang (x->x_outletRight);
    }
    //
    }
}

static void tralala_clear (t_tralala *x)
{
    tll_algorithmClear (x->x_algorithm); x->x_hasLearned = 0;
}

static void tralala_learn (t_tralala *x, t_symbol *s, int argc, t_atom *argv)
{
    int size = PD_MIN (argc, TLL_MAXIMUM_SIZE);
    
    if (size) {
    //
    int i;
    
    for (i = 0; i < size; i++) { x->x_cache[i] = (int)atom_getFloat (argv + i); }
    
    tll_algorithmAdd (x->x_algorithm, size, x->x_cache); x->x_hasLearned = 1;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tralala_load (t_tralala *x)
{
    if (x->x_data) { tll_algorithmDeserialize (x->x_algorithm, x->x_data); x->x_hasLearned = 1; }
}

static void tralala_save (t_tralala *x)
{
    int dirty = 0;
    
    if (x->x_data) { tll_arrayFree (x->x_data); x->x_data = NULL; dirty = 1; }
    if (x->x_hasLearned) { x->x_data = tll_algorithmSerialize (x->x_algorithm); dirty = 1; }
    
    if (dirty) { glist_setDirty (x->x_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tralala_restore (t_tralala *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!x->x_data && argc) {
    //
    int i, size = atom_getFloat (argv);
    x->x_data = tll_arrayNew (size);
    for (i = 1; i < argc; i++) { tll_arrayAppend (x->x_data, (int)atom_getFloat (argv + i)); }
    //
    }
}

static t_error tralala_data (t_gobj *z, t_buffer *b, int flags)
{
    t_tralala *x = (t_tralala *)z;
    
    if (x->x_data) {
    //
    int i, size = tll_arrayGetSize (x->x_data);
    buffer_reserve (b, size + 2);
    buffer_appendSymbol (b, sym__restore);
    buffer_appendFloat (b, size);
    for (i = 0; i < size; i++) { buffer_appendFloat (b, tll_arrayGetAtIndex (x->x_data, i)); }
    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tralala_new (t_symbol *s)
{
    t_tralala *x = (t_tralala *)pd_new (tralala_class);
    
    int type = (s == sym_lattice) ? TLL_ALGORITHM_TYPE_LATTICE : TLL_ALGORITHM_TYPE_ORACLE;
    
    x->x_cache       = (int *)PD_MEMORY_GET (TLL_MAXIMUM_SIZE * sizeof (int));
    x->x_out         = (t_atom *)PD_MEMORY_GET (TLL_MAXIMUM_SIZE * sizeof (t_atom));
    x->x_random      = tll_randomNew();
    x->x_algorithm   = TLL_ALGORITHM_NEW (type, x->x_random);
    x->x_owner       = instance_contextGetCurrent();
    x->x_outletLeft  = outlet_newList (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    return x;
}

static void tralala_free (t_tralala *x)
{
    if (x->x_data) { tll_arrayFree (x->x_data); x->x_data = NULL; }
    
    tll_algorithmFree (x->x_algorithm);
    tll_randomFree (x->x_random);
    
    PD_MEMORY_FREE (x->x_out);
    PD_MEMORY_FREE (x->x_cache);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tralala_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tralala,
            (t_newmethod)tralala_new,
            (t_method)tralala_free,
            sizeof (t_tralala),
            CLASS_BOX,
            A_DEFSYMBOL,
            A_NULL);
    
    class_addFloat (c, (t_method)tralala_float);
    
    class_addMethod (c, (t_method)tralala_clear,    sym_clear,      A_NULL);
    class_addMethod (c, (t_method)tralala_load,     sym_load,       A_NULL);
    class_addMethod (c, (t_method)tralala_save,     sym_save,       A_NULL);
    class_addMethod (c, (t_method)tralala_learn,    sym_learn,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)tralala_restore,  sym__restore,   A_GIMME, A_NULL);
    
    class_setDataFunction (c, tralala_data);
    
    tralala_class = c;
}

void tralala_destroy (void)
{
    class_free (tralala_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
