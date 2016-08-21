
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *getsize_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _getsize {
    t_object    x_obj;                      /* Must be the first. */
    t_symbol    *x_templateIdentifier;
    t_symbol    *x_fieldName;
    } t_getsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void getsize_pointer (t_getsize *x, t_gpointer *gp)
{
    if (!gpointer_isValidInstanceOf (gp, x->x_templateIdentifier)) { pointer_error (sym_getsize); }
    else {
    //
    if (gpointer_hasField (gp, x->x_fieldName)) {
        if (gpointer_fieldIsArrayAndValid (gp, x->x_fieldName)) {
            t_float size = (t_float)array_getSize (gpointer_getArray (gp, x->x_fieldName));
            outlet_float (cast_object (x)->te_outlet, size);
        }
    }
    //
    }
}

static void getsize_set (t_getsize *x, t_symbol *templateName, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *getsize_new (t_symbol *templateName, t_symbol *fieldName)
{
    t_getsize *x = (t_getsize *)pd_new (getsize_class);
    
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
    
    outlet_new (cast_object (x), &s_float);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void getsize_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_getsize,
            (t_newmethod)getsize_new,
            NULL,
            sizeof (t_getsize),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addPointer (c, getsize_pointer);
    
    class_addMethod (c, (t_method)getsize_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL);
    
    getsize_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
