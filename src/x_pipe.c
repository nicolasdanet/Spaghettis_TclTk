
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_atomoutlet.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pipe_class;                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pipecallback {
    t_atom                  *h_atoms;
    t_gpointer              *h_gpointers;
    t_clock                 *h_clock;
    struct _pipe            *h_owner;
    struct _pipecallback    *h_next;
    } t_pipecallback;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _pipe {
    t_object                x_obj;                  /* Must be the first. */
    t_float                 x_delay;
    int                     x_size;
    t_float                 x_unit;
    t_symbol                *x_unitName;
    t_atomoutlet            *x_vector;
    t_pipecallback          *x_callbacks;
    } t_pipe;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error clock_parseUnit (t_float,  t_symbol *, t_float *, int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void callback_free (t_pipecallback *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void callback_task (t_pipecallback *h)
{
    t_pipe *owner = h->h_owner;
    int i;
    
    if (owner->x_callbacks == h) { owner->x_callbacks = h->h_next; }
    else {
        t_pipecallback *m = NULL;
        t_pipecallback *n = NULL;
        for ((m = owner->x_callbacks); (n = m->h_next); (m = n)) { 
            if (n == h) { m->h_next = n->h_next; break; }
        }
    }
    
    for (i = owner->x_size - 1; i >= 0; i--) {
        t_error err = atomoutlet_broadcastIfTypeMatch (owner->x_vector + i, h->h_atoms + i);
        PD_UNUSED (err); PD_ASSERT (!err);
    }
    
    callback_free (h);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void callback_new (t_pipe *x, int argc, t_atom *argv)
{
    t_pipecallback *h = (t_pipecallback *)PD_MEMORY_GET (sizeof (t_pipecallback));
    int i;
       
    h->h_atoms      = (t_atom *)PD_MEMORY_GET (x->x_size * sizeof (t_atom));
    h->h_gpointers  = (t_gpointer *)PD_MEMORY_GET (x->x_size * sizeof (t_gpointer));
    h->h_clock      = clock_new ((void *)h, (t_method)callback_task);
    h->h_owner      = x;
    h->h_next       = x->x_callbacks;
    
    if (x->x_unitName != &s_) {
        t_error err = clock_setUnitParsed (h->h_clock, x->x_unit, x->x_unitName);
        PD_UNUSED (err); PD_ASSERT (!err);
    }
    
    for (i = 0; i < x->x_size; i++) {
    
        t_atomoutlet *a = x->x_vector + i;
        
        if (!atomoutlet_isPointer (a)) { atomoutlet_copyAtom (a, h->h_atoms + i); }
        else {
            SET_POINTER (&h->h_atoms[i], h->h_gpointers + i);
            gpointer_setByCopy (h->h_gpointers + i, atomoutlet_getPointer (a));
        }
    }
    
    x->x_callbacks = h;

    clock_delay (h->h_clock, PD_MAX (x->x_delay, 0.0));
}

static void callback_free (t_pipecallback *h)
{
    int i;
    
    clock_free (h->h_clock);
    
    for (i = 0; i < h->h_owner->x_size; i++) { gpointer_unset (h->h_gpointers + i); }
    
    PD_MEMORY_FREE (h->h_gpointers);
    PD_MEMORY_FREE (h->h_atoms);
    PD_MEMORY_FREE (h);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int pipe_unitIsValid (t_float f, t_symbol *unitName, int verbose)
{
    if (f != 0.0 && unitName != &s_) {
    //
    int n; t_float t; t_error err = clock_parseUnit (f, unitName, &t, &n);
    
    if (err && verbose) { error_invalid (sym_pipe, sym_unit); }
    
    return (!err);
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void pipe_list (t_pipe *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    
    if (argc > x->x_size) {
    //
    if (IS_FLOAT (argv + x->x_size)) { x->x_delay = GET_FLOAT (argv + x->x_size); }
    else {
        error_invalid (sym_pipe, sym_delay);
        return;
    }
    //
    }
    
    argc = PD_MIN (argc, x->x_size);
    
    for (i = 0; i < argc; i++) {
    //
    if (!atom_typesAreEquals (atomoutlet_getAtom (x->x_vector + i), argv + i)) {  
        error_mismatch (sym_pipe, sym_type); 
        return;
    }
    //
    }

    for (i = 0; i < argc; i++) { atomoutlet_setAtom (x->x_vector + i, argv + i); }
    
    callback_new (x, argc, argv);
}

static void pipe_anything (t_pipe *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)pipe_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void pipe_flush (t_pipe *x)      /* FIFO. */
{
    while (x->x_callbacks) {
    //
    t_pipecallback *m = NULL;
    t_pipecallback *n = NULL;
    
    for ((m = x->x_callbacks); (n = m->h_next); (m = n)) { }
    
    callback_task (m);
    //
    }
}

static void pipe_clear (t_pipe *x)
{
    t_pipecallback *h = NULL; while ((h = x->x_callbacks)) { x->x_callbacks = h->h_next; callback_free (h); }
}

/* Note that float arguments are always passed at last. */

static void pipe_unit (t_pipe *x, t_symbol *unitName, t_float f)
{
    if (pipe_unitIsValid (f, unitName, 1)) { x->x_unit = f; x->x_unitName = unitName; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *pipe_new (t_symbol *s, int argc, t_atom *argv)
{
    t_pipe *x = (t_pipe *)pd_new (pipe_class);
    int i;
    
    x->x_delay    = (t_float)0.0;
    x->x_unit     = (t_float)0.0;
    x->x_unitName = &s_;
    
    if (argc > 2) {
    //
    t_float f = atom_getFloat (argv + (argc - 2));
    t_symbol *unitName = atom_getSymbol (argv + (argc - 1));
    if (pipe_unitIsValid (f, unitName, 0)) {
        x->x_unit     = f;
        x->x_unitName = unitName;
        argc -= 2;
    }
    //
    }
    
    if (argc) {
    //
    t_atom *a = argv + (argc - 1); if (IS_FLOAT (a)) { x->x_delay = GET_FLOAT (a); argc--; }
    //
    }

    x->x_size   = PD_MAX (1, argc);
    x->x_vector = (t_atomoutlet *)PD_MEMORY_GET (x->x_size * sizeof (t_atomoutlet));

    if (!argc) {
        atomoutlet_makeFloat (x->x_vector + 0, cast_object (x), ATOMOUTLET_OUTLET, NULL, (t_float)0.0);
        
    } else {

        for (i = 0; i < argc; i++) {
            int create = (i != 0) ? ATOMOUTLET_BOTH : ATOMOUTLET_OUTLET;
            atomoutlet_makeParsed (x->x_vector + i, cast_object (x), create, argv + i);
        }
    }
    
    inlet_newFloat (cast_object (x), &x->x_delay);
    
    x->x_callbacks = NULL;

    return x;
}

static void pipe_free (t_pipe *x)
{
    int i;
    
    pipe_clear (x);
    
    for (i = 0; i < x->x_size; i++) { atomoutlet_release (x->x_vector + i); }
    
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pipe_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pipe, 
            (t_newmethod)pipe_new,
            (t_method)pipe_free,
            sizeof (t_pipe),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)pipe_list);
    class_addAnything (c, (t_method)pipe_anything);
    
    class_addMethod (c, (t_method)pipe_flush,   sym_flush,  A_NULL);
    class_addMethod (c, (t_method)pipe_clear,   sym_clear,  A_NULL);
    class_addMethod (c, (t_method)pipe_unit,    sym_unit,   A_FLOAT, A_SYMBOL, A_NULL);

    pipe_class = c;
}

void pipe_destroy (void)
{
    class_free (pipe_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
