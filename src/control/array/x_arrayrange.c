
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

void *arrayrange_new (t_class *c, int argc, t_atom *argv, int makeOnsetInlet, int makeSizeInlet)
{
    t_arrayrange *x = (t_arrayrange *)pd_new (c);

    t_error err = arrayclient_init (&x->ar_arrayclient, &argc, &argv);      /* It may consume arguments. */
    
    if (!err) {

        x->ar_onset     = 0;
        x->ar_size      = -1;
        x->ar_fieldName = sym_y; 
            
        while (argc && IS_SYMBOL (argv)) {
        //
        t_symbol *t = GET_SYMBOL (argv);

        if (t == sym___dash__field) {
            if (argc >= 3 && IS_SYMBOL (argv + 1) && IS_SYMBOL (argv + 2)) {
                x->ar_fieldName = GET_SYMBOL (argv + 2);
                argc -= 3; argv += 3;
            } else {
                err = PD_ERROR;
            }
        }

        break;
        //
        }
    }
    
    error__options (class_getName (c), argc, argv);
    
    if (!err) { 
    
        if (makeOnsetInlet) { inlet_newFloat (cast_object (x), &x->ar_onset); }
        if (makeSizeInlet)  { inlet_newFloat (cast_object (x), &x->ar_size); }
        
        if (argc && IS_FLOAT (argv)) { x->ar_onset = GET_FLOAT (argv); argc--; argv++; }
        if (argc && IS_FLOAT (argv)) { x->ar_size  = GET_FLOAT (argv); argc--; argv++; }

        if (argc) { warning_unusedArguments (class_getName (c), argc, argv); }
        
        if (ARRAYCLIENT_HAS_POINTER (&x->ar_arrayclient)) {
            inlet_newPointer (cast_object (x), ARRAYCLIENT_ADDRESS_POINTER (&x->ar_arrayclient));
        } else {
            inlet_newSymbol (cast_object (x),  ARRAYCLIENT_ADDRESS_NAME    (&x->ar_arrayclient));
        }
    }
    
    x->ar_error = err;
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayrange_update (t_arrayrange *x)
{
    arrayclient_update (&x->ar_arrayclient);
}

void arrayrange_setOnset (t_arrayrange *x, t_float f)
{
    x->ar_onset = f;
}

void arrayrange_setSize (t_arrayrange *x, t_float f)
{
    x->ar_size = f;
}

t_float arrayrange_getOnset (t_arrayrange *x)
{
    return x->ar_onset;
}

t_float arrayrange_getSize (t_arrayrange *x)
{
    return x->ar_size;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int arrayrange_isValid (t_arrayrange *x)
{
    t_array *a = arrayclient_fetchArray (&x->ar_arrayclient);
    
    if (a) { return template_fieldIsFloat (array_getTemplate (a), x->ar_fieldName); }
    
    return 0;
}

t_array *arrayrange_getRange (t_arrayrange *x, int *i, int *n)
{
    t_array *a = arrayclient_fetchArray (&x->ar_arrayclient);
    
    PD_ASSERT (a);
    
    int size = array_getSize (a);
    int count, first = PD_CLAMP (x->ar_onset, 0, size);
    
    if (x->ar_size < 0) { count = size - first; }
    else {
        count = x->ar_size; if (first + count > size) { count = size - first; }
    }
    
    *i = first;
    *n = count;
    
    return a;
}

t_symbol *arrayrange_getFieldName (t_arrayrange *x)
{
    return x->ar_fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float arrayrange_quantile (t_arrayrange *x, t_float f)
{
    PD_ASSERT (arrayrange_isValid (x));
    
    int i, start, n;
    t_array *a = arrayrange_getRange (x, &start, &n);
    
    double sum = 0.0;
    
    for (i = 0; i < n; i++) {
        t_float t = array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (x));
        sum += (t > 0.0) ? t : 0.0; 
    }
    
    sum *= f;
    
    for (i = 0; i < n - 1; i++) {
        t_float t = array_getFloatAtIndex (a, start + i, arrayrange_getFieldName (x));
        sum -= (t > 0.0) ? t : 0.0;
        if (sum < 0) {
            break; 
        }
    }
    
    return i;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer *arrayrange_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_arrayrange *x = (t_arrayrange *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendSymbol (b, arrayclient_getName (&x->ar_arrayclient));
    buffer_appendFloat (b,  arrayrange_getOnset (x));
    buffer_appendFloat (b,  arrayrange_getSize (x));
    
    return b;
    //
    }
    
    return NULL;
}

void arrayrange_restore (t_arrayrange *x, t_symbol *s, int argc, t_atom *argv)
{
    t_arrayrange *old = (t_arrayrange *)instance_pendingFetch (cast_gobj (x));

    t_symbol *t   = atom_getSymbolAtIndex (0, argc, argv);
    t_float onset = old ? old->ar_onset : atom_getFloatAtIndex (1, argc, argv);
    t_float size  = old ? old->ar_size  : atom_getFloatAtIndex (2, argc, argv);
    
    arrayrange_setOnset (x, onset);
    arrayrange_setSize (x,  size);
    
    if (old) { arrayclient_restore (&x->ar_arrayclient, &old->ar_arrayclient); }
    else {
        arrayclient_setName (&x->ar_arrayclient, t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
