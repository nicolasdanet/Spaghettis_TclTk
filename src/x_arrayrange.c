
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_array.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *arrayrange_new (t_class *class, int argc, t_atom *argv, int makeOnsetInlet, int makeSizeInlet)
{
    t_arrayrange *x = (t_arrayrange *)pd_new (class);

    t_error err = arrayclient_init (&x->ar_arrayclient, &argc, &argv);      /* It may consume arguments. */
    
    if (!err) {

        x->ar_first     = 0;
        x->ar_size      = -1;
        x->ar_fieldName = sym_y; 
            
        while (argc && IS_SYMBOL (argv)) {
        //
        t_symbol *t = GET_SYMBOL (argv);

        if (t == sym___dash__f || t == sym___dash__field) {
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
    
    error__options (class_getName (class), argc, argv);
    
    if (!err) { 
    
        if (makeOnsetInlet) { inlet_newFloat (cast_object (x), &x->ar_first); }
        if (makeSizeInlet)  { inlet_newFloat (cast_object (x), &x->ar_size); }
        
        if (argc && IS_FLOAT (argv)) { x->ar_first = GET_FLOAT (argv); argc--; argv++; }
        if (argc && IS_FLOAT (argv)) { x->ar_size  = GET_FLOAT (argv); argc--; argv++; }

        if (argc) { warning_unusedArguments (class_getName (class), argc, argv); }
        
        if (ARRAYCLIENT_ASPOINTER (&x->ar_arrayclient)) {
            inlet_newPointer (cast_object (x), ARRAYCLIENT_GETPOINTER (&x->ar_arrayclient));
        } else {
            inlet_newSymbol (cast_object (x),  ARRAYCLIENT_GETNAME    (&x->ar_arrayclient));
        }
     
        return x;
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void arrayrange_update (t_arrayrange *x)
{
    arrayclient_update (&x->ar_arrayclient);
}


void arrayrange_setFirst (t_arrayrange *x, t_float f)
{
    x->ar_first = f;
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
    int count, first = PD_CLAMP (x->ar_first, 0, size);
    
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
