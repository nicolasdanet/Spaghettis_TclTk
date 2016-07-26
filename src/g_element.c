
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

static t_class *elem_class;

typedef struct _elem
{
    t_object x_obj;
    t_symbol *x_templatesym;
    t_symbol *x_fieldsym;
    t_gpointer x_gp;
    t_gpointer x_gparent;
} t_elem;

static void *elem_new(t_symbol *templatesym, t_symbol *fieldsym)
{
    t_elem *x = (t_elem *)pd_new(elem_class);
    x->x_templatesym = template_makeIdentifierWithWildcard(templatesym);
    x->x_fieldsym = fieldsym;
    gpointer_init(&x->x_gp);
    gpointer_init(&x->x_gparent);
    inlet_newPointer(&x->x_obj, &x->x_gparent);
    outlet_new(&x->x_obj, &s_pointer);
    return (x);
}

static void elem_set(t_elem *x, t_symbol *templatesym, t_symbol *fieldsym)
{
    x->x_templatesym = template_makeIdentifierWithWildcard(templatesym);
    x->x_fieldsym = fieldsym;
}

static void elem_float(t_elem *x, t_float f)
{
    int indx = f, nitems, onset;
    t_symbol *templatesym, *fieldsym = x->x_fieldsym, *elemtemplatesym;
    t_template *template;
    t_template *elemtemplate;
    t_gpointer *gparent = &x->x_gparent;
    t_word *w;
    t_array *array;
    int elemsize, type;
    
    if (!gpointer_isValid(gparent))
    {
        post_error ("element: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) !=
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

    gpointer_setAsWord(&x->x_gp, array, 
        (t_word *)((char *)(array->a_vector) + indx * elemsize));
    outlet_pointer(x->x_obj.te_outlet, &x->x_gp);
}

static void elem_free(t_elem *x, t_gpointer *gp)
{
    gpointer_unset(&x->x_gp);
    gpointer_unset(&x->x_gparent);
}

void elem_setup(void)
{
    elem_class = class_new(sym_element, (t_newmethod)elem_new,
        (t_method)elem_free, sizeof(t_elem), 0, A_DEFSYMBOL, A_DEFSYMBOL, 0);
    class_addFloat(elem_class, elem_float); 
    class_addMethod(elem_class, (t_method)elem_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
