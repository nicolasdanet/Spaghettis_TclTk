
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

static t_class  *element_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _element {
    t_object        x_obj;
    t_gpointer      x_gpointer;
    t_gpointer      x_gpointerParent;
    t_symbol        *x_templateIdentifier;
    t_symbol        *x_fieldName;
    } t_element;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void element_float (t_element *x, t_float f)
{
    int indx = f, nitems, onset;
    t_symbol *templatesym, *fieldsym = x->x_fieldName, *elemtemplatesym;
    t_template *template;
    t_template *elemtemplate;
    t_gpointer *gparent = &x->x_gpointerParent;
    t_word *w;
    t_array *array;
    int elemsize, type;
    
    if (!gpointer_isValid(gparent))
    {
        post_error ("element: empty pointer");
        return;
    }
    if (*x->x_templateIdentifier->s_name)
    {
        if ((templatesym = x->x_templateIdentifier) !=
            gpointer_getTemplateIdentifier(gparent))
        {
            post_error ("elem %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gparent)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gparent);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("elem: couldn't find template %s", templatesym->s_name);
        return;
    }
    w = gpointer_getData (gparent);
    if (!template)
    {
        post_error ("element: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!template_findField(template, fieldsym,
        &onset, &type, &elemtemplatesym))
    {
        post_error ("element: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("element: field %s not of type array", fieldsym->s_name);
        return;
    }
    if (!(elemtemplate = template_findByIdentifier(elemtemplatesym)))
    {
        post_error ("element: couldn't find field template %s",
            elemtemplatesym->s_name);
        return;
    }

    elemsize = template_getSize (elemtemplate) * ARRAY_WORD;

    array = *(t_array **)(((char *)w) + onset);

    nitems = array->a_size;
    if (indx < 0) indx = 0;
    if (indx >= nitems) indx = nitems-1;

    gpointer_setAsWord(&x->x_gpointer, array, 
        (t_word *)((char *)(array->a_vector) + indx * elemsize));
    outlet_pointer(x->x_obj.te_outlet, &x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void element_set (t_element *x, t_symbol *templateIdentifier, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeBindSymbolWithWildcard (templateIdentifier);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *element_new (t_symbol *templateIdentifier, t_symbol *fieldName)
{
    t_element *x = (t_element *)pd_new (element_class);
    
    gpointer_init (&x->x_gpointer);
    gpointer_init (&x->x_gpointerParent);
    
    x->x_templateIdentifier = template_makeBindSymbolWithWildcard (templateIdentifier);
    x->x_fieldName          = fieldName;
    
    inlet_newPointer (cast_object (x), &x->x_gpointerParent);
    outlet_new (cast_object (x), &s_pointer);
    
    return x;
}

static void element_free (t_element *x, t_gpointer *gp)
{
    gpointer_unset (&x->x_gpointer);
    gpointer_unset (&x->x_gpointerParent);
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
