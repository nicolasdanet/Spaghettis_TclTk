
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

static void getsize_pointer(t_getsize *x, t_gpointer *gp)
{
    int nitems, onset, type;
    t_symbol *templatesym, *fieldsym = x->x_fieldName, *elemtemplatesym;
    t_template *template;
    t_word *w;
    t_array *array;
    int elemsize;
    if (!gpointer_isValid(gp))
    {
        post_error ("getsize: stale or empty pointer");
        return;
    }
    if (*x->x_templateIdentifier->s_name)
    {
        if ((templatesym = x->x_templateIdentifier) !=
            gpointer_getTemplateIdentifier(gp))
        {
            post_error ("elem %s: got wrong template (%s)",
                templatesym->s_name, gpointer_getTemplateIdentifier(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_getTemplateIdentifier(gp);
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("elem: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!template_findField(template, fieldsym,
        &onset, &type, &elemtemplatesym))
    {
        post_error ("getsize: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("getsize: field %s not of type array", fieldsym->s_name);
        return;
    }
    w = gpointer_getData (gp);
    array = *(t_array **)(((char *)w) + onset);
    outlet_float(x->x_obj.te_outlet, (t_float)(array->a_size));
}

static void getsize_set (t_getsize *x, t_symbol *templateName, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeBindSymbolWithWildcard (templateName);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *getsize_new (t_symbol *templateName, t_symbol *fieldName)
{
    t_getsize *x = (t_getsize *)pd_new (getsize_class);
    
    x->x_templateIdentifier = template_makeBindSymbolWithWildcard (templateName);
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
