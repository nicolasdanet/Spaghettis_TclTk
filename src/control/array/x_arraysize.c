
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *arraysize_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arraysize {
    t_arrayclient   x_arrayclient;              /* Must be the first. */
    t_outlet        *x_outlet;
    } t_arraysize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void arraysize_bang (t_arraysize *x)
{
    t_array *a = arrayclient_fetchArray (&x->x_arrayclient);
    
    if (a) { outlet_float (x->x_outlet, array_getSize (a)); }
    else { 
        error_undefined (sym_array__space__size, sym_array); 
    }
}

static void arraysize_float (t_arraysize *x, t_float f)
{
    t_array *a = arrayclient_fetchArray (&x->x_arrayclient);
    
    if (a) {
    //
    int n = PD_MAX (1, (int)f);
    
    if (ARRAYCLIENT_HAS_POINTER (&x->x_arrayclient)) {
        array_resizeAndRedraw (a, arrayclient_fetchOwner (&x->x_arrayclient), n);
    } else {
        garray_resize (arrayclient_fetchOwnerIfName (&x->x_arrayclient), (t_float)n);
    }
    //
    } else { error_undefined (sym_array__space__size, sym_array); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *arraysize_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendSymbol (b, arrayclient_getName ((t_arrayclient *)z));
    
    return b;
    //
    }
    
    return NULL;
}

static void arraysize_restore (t_arraysize *x, t_symbol *s, int argc, t_atom *argv)
{
    arrayclient_setName ((t_arrayclient *)x, atom_getSymbolAtIndex (0, argc, argv));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *arraysize_new (t_symbol *s, int argc, t_atom *argv)
{
    t_arraysize *x = (t_arraysize *)pd_new (arraysize_class);
    
    t_error err = arrayclient_init (&x->x_arrayclient, &argc, &argv);      /* It may consume arguments. */
    
    if (!err) {
    
        error__options (sym_array__space__size, argc, argv);
        
        if (argc) { warning_unusedArguments (sym_array__space__size, argc, argv); }
        
        if (ARRAYCLIENT_HAS_POINTER (&x->x_arrayclient)) {
            inlet_newPointer (cast_object (x), ARRAYCLIENT_ADDRESS_POINTER (&x->x_arrayclient));
        } else {
            inlet_newSymbol (cast_object (x),  ARRAYCLIENT_ADDRESS_NAME    (&x->x_arrayclient));
        }
     
        x->x_outlet = outlet_newFloat (cast_object (x));
        
    } else {
    
        error_invalidArguments (sym_array__space__size, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arraysize_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__size,
            (t_newmethod)arraysize_new,
            (t_method)arrayclient_free,
            sizeof (t_arraysize),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)arraysize_bang);
    class_addFloat (c, (t_method)arraysize_float);
    
    class_addMethod (c, (t_method)arraysize_restore, sym__restore, A_GIMME, A_NULL);

    class_setDataFunction (c, arraysize_functionData);
    class_setHelpName (c, sym_array);
    
    arraysize_class = c;
}

void arraysize_destroy (void)
{
    class_free (arraysize_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
