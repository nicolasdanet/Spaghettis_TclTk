
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
#pragma mark -

static t_class *getsize_class;

typedef struct _getsize
{
    t_object x_obj;
    t_symbol *x_templatesym;
    t_symbol *x_fieldsym;
} t_getsize;

static void *getsize_new(t_symbol *templatesym, t_symbol *fieldsym)
{
    t_getsize *x = (t_getsize *)pd_new(getsize_class);
    x->x_templatesym = template_makeIdentifierWithWildcard(templatesym);
    x->x_fieldsym = fieldsym;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void getsize_set(t_getsize *x, t_symbol *templatesym, t_symbol *fieldsym)
{
    x->x_templatesym = template_makeIdentifierWithWildcard(templatesym);
    x->x_fieldsym = fieldsym;
}

static void getsize_pointer(t_getsize *x, t_gpointer *gp)
{
    int nitems, onset, type;
    t_symbol *templatesym, *fieldsym = x->x_fieldsym, *elemtemplatesym;
    t_template *template;
    t_word *w;
    t_array *array;
    int elemsize;
    if (!gpointer_isValid(gp))
    {
        post_error ("getsize: stale or empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) !=
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

void getsize_setup(void)
{
    getsize_class = class_new(sym_getsize, (t_newmethod)getsize_new, 0,
        sizeof(t_getsize), 0, A_DEFSYMBOL, A_DEFSYMBOL, 0);
    class_addPointer(getsize_class, getsize_pointer); 
    class_addMethod(getsize_class, (t_method)getsize_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
