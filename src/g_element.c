
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

static t_class  *element_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _element {
    t_object        x_obj;                      /* Must be the first. */
    t_gpointer      x_gpointerWord;
    t_gpointer      x_gpointer;
    t_symbol        *x_templateIdentifier;
    t_symbol        *x_fieldName;
    } t_element;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void element_float (t_element *x, t_float f)
{
    if (!gpointer_isValidInstanceOf (&x->x_gpointer, x->x_templateIdentifier)) { 
        pointer_error (sym_element);
    } else {
    //
    if (gpointer_hasField (&x->x_gpointer, x->x_fieldName)) {
        if (gpointer_fieldIsArrayAndValid (&x->x_gpointer, x->x_fieldName)) {
            t_array *array = gpointer_getArray (&x->x_gpointer, x->x_fieldName);
            gpointer_setAsWord (&x->x_gpointerWord, array, array_getElementAtIndex (array, (int)f));
            outlet_pointer (cast_object (x)->te_outlet, &x->x_gpointerWord);
        }
    }
    //
    }
}

static void element_set (t_element *x, t_symbol *templateName, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *element_new (t_symbol *templateName, t_symbol *fieldName)
{
    t_element *x = (t_element *)pd_new (element_class);
    
    gpointer_init (&x->x_gpointerWord);
    gpointer_init (&x->x_gpointer);
    
    x->x_templateIdentifier = template_makeIdentifierWithWildcard (templateName);
    x->x_fieldName          = fieldName;
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    outlet_new (cast_object (x), &s_pointer);
    
    return x;
}

static void element_free (t_element *x, t_gpointer *gp)
{
    gpointer_unset (&x->x_gpointerWord);
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void element_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_element,
            (t_newmethod)element_new,
            (t_method)element_free,
            sizeof (t_element),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, element_float);
     
    class_addMethod (c, (t_method)element_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL); 
    
    element_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
