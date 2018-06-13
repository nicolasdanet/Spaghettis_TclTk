
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *timestamp_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _timestamp {
    t_object    x_obj;                  /* Must be the first. */
    int         x_discard;
    t_outlet    *x_outlet;
    } t_timestamp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void timestamp_float (t_timestamp *x, t_float f)
{
    t_atom a[STAMP_TAGS_SIZE]; t_stamp t;
    
    t_nano ns = PD_MILLISECONDS_TO_NANOSECONDS (PD_MAX (0.0, f));
    
    stamp_set (&t);
    
    if (ns) { stamp_addNanoseconds (&t, ns); }
    
    if (!stamp_setAsTags (STAMP_TAGS_SIZE, a, &t)) { outlet_list (x->x_outlet, STAMP_TAGS_SIZE, a); }
}

static void timestamp_bang (t_timestamp *x)
{
    timestamp_float (x, 0.0);
}

static void timestamp_list (t_timestamp *x, t_symbol *s, int argc, t_atom *argv)
{
    t_stamp t; t_error err = stamp_getWithTags (argc, argv, &t);
    
    if (err) { error_invalid (sym_timestamp, sym_stamp); }
    else {
    //
    t_stamp now; t_nano ns;
    stamp_set (&now);
    err = stamp_elapsedNanoseconds (&now, &t, &ns);
    if (!err || !x->x_discard) { outlet_float (x->x_outlet, PD_NANOSECONDS_TO_MILLISECONDS (ns)); }
    //
    }
}

static void timestamp_anything (t_timestamp *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)timestamp_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void timestamp_discard (t_timestamp *x, t_float f)
{
    x->x_discard = (f != 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *timestamp_new (t_symbol *s, int argc, t_atom *argv)
{
    t_timestamp *x = (t_timestamp *)pd_new (timestamp_class);
    
    x->x_outlet = outlet_newAnything (cast_object (x));
    
    while (argc > 0) {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);

    if (t == sym___dash__d || t == sym___dash__discard) {
        timestamp_discard (x, 1.0);
        argc--;
        argv++;
        
    } else {
        break;
    }
    //
    }

    error__options (s, argc, argv);
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void timestamp_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_timestamp,
            (t_newmethod)timestamp_new,
            NULL,
            sizeof (t_timestamp),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);

    class_addBang (c, (t_method)timestamp_bang);
    class_addFloat (c, (t_method)timestamp_float);
    class_addList (c, (t_method)timestamp_list);
    class_addAnything (c, (t_method)timestamp_anything);
    
    class_addMethod (c, (t_method)timestamp_discard, sym_discard, A_FLOAT, A_NULL);
    
    timestamp_class = c;
}

void timestamp_destroy (void)
{
    class_free (timestamp_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
