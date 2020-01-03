
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

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
    int             x_keep;
    int             x_hasLearned;
    int             *x_cache;
    t_atom          *x_out;
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

static void tralala_dirty (t_tralala *x)
{
    if (x->x_keep) { glist_setDirty (x->x_owner, 1); }
}

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
    tll_algorithmClear (x->x_algorithm); if (x->x_hasLearned) { x->x_hasLearned = 0; tralala_dirty (x); }
}

static void tralala_learn (t_tralala *x, t_symbol *s, int argc, t_atom *argv)
{
    int size = PD_MIN (argc, TLL_MAXIMUM_SIZE);
    
    if (size) {
    //
    int i;
    
    for (i = 0; i < size; i++) { x->x_cache[i] = (int)atom_getFloat (argv + i); }
    
    tll_algorithmAdd (x->x_algorithm, size, x->x_cache); x->x_hasLearned = 1; tralala_dirty (x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *tralala_functionData (t_gobj *z, int flags)
{
    t_tralala *x = (t_tralala *)z;
    
    if (SAVED_DEEP (flags) || x->x_keep) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    
    if (x->x_hasLearned) {
    //
    tll_array *a = tll_algorithmSerialize (x->x_algorithm);
    int i, size  = tll_arrayGetSize (a);
 
    buffer_reserve (b, size + 2);
    buffer_appendFloat (b, size);
    for (i = 0; i < size; i++) { buffer_appendFloat (b, tll_arrayGetAtIndex (a, i)); }
    
    tll_arrayFree (a);
    //
    }
    
    return b;
    //
    }
    
    return NULL;
}

static void tralala_restoreEncapsulation (t_tralala *x, t_tralala *old)
{
    t_rand48 *t0      = x->x_random;
    tll_algorithm *t1 = x->x_algorithm;
    int t2            = x->x_hasLearned;
    
    x->x_random       = old->x_random;     old->x_random     = t0;
    x->x_algorithm    = old->x_algorithm;  old->x_algorithm  = t1;
    x->x_hasLearned   = old->x_hasLearned; old->x_hasLearned = t2;
}

static void tralala_restoreProceed (t_tralala *x, int argc, t_atom *argv)
{
    if (argc && IS_FLOAT (argv)) {
    //
    int i, size = atom_getFloat (argv);
    tll_array *a = tll_arrayNew (size);
    
    for (i = 1; i < argc; i++) { tll_arrayAppend (a, (int)atom_getFloat (argv + i)); }
    
    tll_algorithmDeserialize (x->x_algorithm, a); x->x_hasLearned = 1;
    
    tll_arrayFree (a);
    //
    }
}

static void tralala_restore (t_tralala *x, t_symbol *s, int argc, t_atom *argv)
{
    t_tralala *old = (t_tralala *)instance_pendingFetch (cast_gobj (x));

    if (old) { tralala_restoreEncapsulation (x, old); } else { tralala_restoreProceed (x, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tralala_new (t_symbol *s, int argc, t_atom *argv)
{
    t_tralala *x = (t_tralala *)pd_new (tralala_class);
    
    int type = TLL_ALGORITHM_TYPE_ORACLE;
    
    while (argc && IS_SYMBOL (argv)) {
    
        t_symbol *t = GET_SYMBOL (argv);
        
        if (t == sym___dash__keep) { x->x_keep = 1; argc--; argv++; }
        else {
            break;
        }
    }
    
    if (!error__options (s, argc, argv)) {
        if (argc && IS_SYMBOL (argv)) {
            if (GET_SYMBOL (argv) == sym_lattice) { type = TLL_ALGORITHM_TYPE_LATTICE; argc--; argv++; }
            if (GET_SYMBOL (argv) == sym_oracle)  { type = TLL_ALGORITHM_TYPE_ORACLE;  argc--; argv++; }
        }
    }
    
    x->x_cache       = (int *)PD_MEMORY_GET (TLL_MAXIMUM_SIZE * sizeof (int));
    x->x_out         = (t_atom *)PD_MEMORY_GET (TLL_MAXIMUM_SIZE * sizeof (t_atom));
    x->x_random      = tll_randomNew();
    x->x_algorithm   = TLL_ALGORITHM_NEW (type, x->x_random);
    x->x_owner       = instance_contextGetCurrent();
    x->x_outletLeft  = outlet_newList (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

static void tralala_free (t_tralala *x)
{
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
            A_GIMME,
            A_NULL);
    
    class_addFloat (c, (t_method)tralala_float);
    
    class_addMethod (c, (t_method)tralala_clear,    sym_clear,      A_NULL);
    class_addMethod (c, (t_method)tralala_learn,    sym_learn,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)tralala_restore,  sym__restore,   A_GIMME, A_NULL);
    
    class_setDataFunction (c, tralala_functionData);
    class_requirePending (c);

    tralala_class = c;
}

void tralala_destroy (void)
{
    class_free (tralala_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
