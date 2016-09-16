
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error arrayclient_init (t_arrayclient *x, int *ac, t_atom **av)
{
    int argc = *ac;
    t_atom *argv = *av;
    
    x->ac_name               = NULL;
    x->ac_templateIdentifier = NULL;
    x->ac_fieldName          = NULL;
    
    gpointer_init (&x->ac_gpointer);
    
    while (argc && IS_SYMBOL (argv)) {
    //
    t_symbol *t = GET_SYMBOL (argv);
    
    if (t == sym___dash__s || t == sym___dash__t || t == sym___dash__template) {
        if (argc >= 3 && IS_SYMBOL (argv + 1) && IS_SYMBOL (argv + 2)) {
            x->ac_templateIdentifier = template_makeIdentifierWithWildcard (GET_SYMBOL (argv + 1));
            x->ac_fieldName = GET_SYMBOL (argv + 2);
            argc -= 3; argv += 3;
        } else {
            return PD_ERROR;
        }
    } 

    break;
    //
    }

    if (!x->ac_templateIdentifier && argc) {
        if (!IS_SYMBOL (argv)) { return PD_ERROR; }
        else {
            x->ac_name = GET_SYMBOL (argv); 
            argc--; argv++;
        }
    }
    
    *ac = argc;
    *av = argv;
    
    return PD_ERROR_NONE;
}

void arrayclient_free (t_arrayclient *x)
{
    gpointer_unset (&x->ac_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_array *arrayclient_fetchArray (t_arrayclient *x)
{
    if (x->ac_name) {
    
        t_garray *y = (t_garray *)pd_findByClass (x->ac_name, garray_class);
        
        if (y) { return garray_getArray (y); }
        else {
            error_canNotFind (sym_array, x->ac_name);
        }
        
    } else if (x->ac_templateIdentifier) {
    
        if (gpointer_isValidInstanceOf (&x->ac_gpointer, x->ac_templateIdentifier)) {
            if (gpointer_hasField (&x->ac_gpointer, x->ac_fieldName)) {
                if (gpointer_fieldIsArrayAndValid (&x->ac_gpointer, x->ac_fieldName)) {
                    return gpointer_getArray (&x->ac_gpointer, x->ac_fieldName);
                    
        } else { error_invalid (sym_array, x->ac_fieldName); }
        } else { error_missingField (sym_array, x->ac_fieldName); }
        } else { error_invalid (sym_array, &s_pointer); }
    }
    
    return NULL;
}

t_garray *arrayclient_fetchOwnerIfName (t_arrayclient *x)
{
    PD_ASSERT (x->ac_name); return (t_garray *)pd_findByClass (x->ac_name, garray_class);
}

t_glist *arrayclient_fetchView (t_arrayclient *x)
{
    if (x->ac_name) {
    
        t_garray *y = (t_garray *)pd_findByClass (x->ac_name, garray_class);
        
        if (y) { return garray_getView (y); }
        else {
            error_canNotFind (sym_array, x->ac_name);
        }
        
    } else if (x->ac_templateIdentifier) {
    
        if (gpointer_isValidInstanceOf (&x->ac_gpointer, x->ac_templateIdentifier)) {
            if (gpointer_hasField (&x->ac_gpointer, x->ac_fieldName)) {
                if (gpointer_fieldIsArrayAndValid (&x->ac_gpointer, x->ac_fieldName)) {
                    return gpointer_getView (&x->ac_gpointer);
                    
        } else { error_invalid (sym_array, x->ac_fieldName); }
        } else { error_missingField (sym_array, x->ac_fieldName); }
        } else { error_invalid (sym_array, &s_pointer); }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arrayclient_update (t_arrayclient *x)
{
    t_array *array = arrayclient_fetchArray (x);
    t_glist *view  = arrayclient_fetchView (x);
    
    PD_ASSERT (array);
    PD_ASSERT (view);
        
    array_redraw (array, view);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
