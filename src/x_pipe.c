
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_control.h"

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
    t_atomoutlet            *x_vector;
    t_pipecallback          *x_callbacks;
    } t_pipe;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void callback_free (t_pipecallback *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void callback_task (t_pipecallback *h)
{
    t_pipe *owner = h->h_owner;
    int i;
    
    if (owner->x_callbacks == h) { owner->x_callbacks = h->h_next; }
    else {
        t_pipecallback *m = NULL;
        t_pipecallback *n = NULL;
        for (m = owner->x_callbacks; n = m->h_next; m = n) { if (n == h) { m->h_next = n->h_next; break; } }
    }
    
    for (i = owner->x_size - 1; i >= 0; i--) {
        t_error err = atomoutlet_outputAtom (owner->x_vector + i, h->h_atoms + i);
        PD_ASSERT (!err);
    }
    
    callback_free (h);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void callback_new (t_pipe *x, int argc, t_atom *argv)
{
    t_pipecallback *h = (t_pipecallback *)PD_MEMORY_GET (sizeof (t_pipecallback));
    int i;
       
    h->h_atoms      = (t_atom *)PD_MEMORY_GET (x->x_size * sizeof (t_atom));
    h->h_gpointers  = (t_gpointer *)PD_MEMORY_GET (x->x_size * sizeof (t_gpointer));
    h->h_clock      = clock_new ((void *)h, (t_method)callback_task);
    h->h_owner      = x;
    h->h_next       = x->x_callbacks;
    
    for (i = 0; i < x->x_size; i++) {
    
        t_atomoutlet *a = x->x_vector + i;
        
        if (!atomoutlet_isPointer (a)) { atomoutlet_copyAtom (a, h->h_atoms + i); }
        else {
            SET_POINTER (&h->h_atoms[i], h->h_gpointers + i);
            gpointer_setByCopy (atomoutlet_getPointer (a), h->h_gpointers + i);
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
#pragma mark -

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
    if (!atom_typesAreEqual (atomoutlet_getAtom (x->x_vector + i), argv + i)) {  
        error_mismatch (sym_pipe, sym_type); 
        return;
    }
    //
    }

    for (i = 0; i < argc; i++) { atomoutlet_setAtom (x->x_vector + i, argv + i); }
    
    callback_new (x, argc, argv);
}

static void pipe_flush (t_pipe *x)      /* FIFO. */
{
    while (x->x_callbacks) {
    //
    t_pipecallback *m = NULL;
    t_pipecallback *n = NULL;
    
    for (m = x->x_callbacks; n = m->h_next; m = n) { }
    
    callback_task (m);
    //
    }
}

static void pipe_clear (t_pipe *x)
{
    t_pipecallback *h = NULL; while (h = x->x_callbacks) { x->x_callbacks = h->h_next; callback_free (h); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pipe_new (t_symbol *s, int argc, t_atom *argv)
{
    t_pipe *x = (t_pipe *)pd_new (pipe_class);
    int i;
    
    x->x_delay  = 0.0;
        
    if (argc) {
    //
    t_atom *a = argv + (argc - 1); if (IS_FLOAT (a)) { x->x_delay = GET_FLOAT (a); argc--; }
    //
    }

    x->x_size   = PD_MAX (1, argc);     
    x->x_vector = (t_atomoutlet *)PD_MEMORY_GET (x->x_size * sizeof (t_atomoutlet));

    if (!argc) { atomoutlet_makeFloat (x->x_vector + 0, cast_object (x), 0.0, 0, 1); }
    else {
    //
    for (i = 0; i < argc; i++) {
        if (atomoutlet_makeParse (x->x_vector + i, cast_object (x), argv + i, (i != 0), 1)) {
            warning_badType (sym_pipe, atom_getSymbol (argv + i));
        }
    }
    //
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
#pragma mark -

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
    
    class_addMethod (c, (t_method)pipe_flush,   sym_flush,  A_NULL);
    class_addMethod (c, (t_method)pipe_clear,   sym_clear,  A_NULL);
    
    pipe_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
