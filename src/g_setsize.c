
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

static t_class  *setsize_class;                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _setsize {
    t_object    x_obj;                              /* Must be the first. */
    t_gpointer  x_gpointer;
    t_symbol    *x_templateIdentifier;
    t_symbol    *x_fieldName;
    } t_setsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void setsize_float (t_setsize *x, t_float f)
{
    int nitems, onset, type;
    t_symbol *templatesym, *fieldsym = x->x_fieldName, *elemtemplatesym;
    t_template *template;
    t_template *elemtemplate;
    t_word *w;
    t_atom at;
    t_array *array;
    int elemsize;
    int newsize = f;
    t_gpointer *gp = &x->x_gpointer;
    if (!gpointer_isValid(&x->x_gpointer))
    {
        post_error ("setsize: empty pointer");
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
        post_error ("setsize: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("setsize: field %s not of type array", fieldsym->s_name);
        return;
    }
    
    w = gpointer_getData (gp);

    if (!(elemtemplate = template_findByIdentifier(elemtemplatesym)))
    {
        post_error ("element: couldn't find field template %s",
            elemtemplatesym->s_name);
        return;
    }

    elemsize = template_getSize (elemtemplate) * ARRAY_WORD;

    array = *(t_array **)(((char *)w) + onset);

    if (elemsize != array->a_stride) { PD_BUG; }

    nitems = array->a_size;
    if (newsize < 1) newsize = 1;
    if (newsize == nitems) return;
    
        /* erase the array before resizing it.  If we belong to a
        scalar it's easy, but if we belong to an element of another
        array we have to search back until we get to a scalar to erase.
        When graphics updates become queueable this may fall apart... */

    gpointer_setVisibility (gp, 0);

        /* if shrinking, free the scalars that will disappear */
    if (newsize < nitems)
    {
        char *elem;
        int count;       
        for (elem = ((char *)array->a_vector) + newsize * elemsize,
            count = nitems - newsize; count--; elem += elemsize)
                word_free((t_word *)elem, elemtemplate);
    }
        /* resize the array  */
    array->a_vector = (char *)PD_MEMORY_RESIZE(array->a_vector,
        elemsize * nitems, elemsize * newsize);
    array->a_size = newsize;
        /* if growing, initialize new scalars */
    if (newsize > nitems)
    {
        char *elem;
        int count;       
        for (elem = ((char *)array->a_vector) + nitems * elemsize,
            count = newsize - nitems; count--; elem += elemsize)
                word_init((t_word *)elem, elemtemplate, gp);
    }
        /* invalidate all gpointers into the array */
    array->a_uniqueIdentifier++;    /* Encapsulate. */

    gpointer_setVisibility (gp, 1);
}

static void setsize_set (t_setsize *x, t_symbol *templateName, t_symbol *fieldName)
{
    x->x_templateIdentifier = template_makeTemplateIdentifier (templateName);
    x->x_fieldName          = fieldName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *setsize_new (t_symbol *templateName, t_symbol *fieldName)
{
    t_setsize *x = (t_setsize *)pd_new (setsize_class);
    
    gpointer_init (&x->x_gpointer);
        
    x->x_templateIdentifier = template_makeTemplateIdentifier (templateName);
    x->x_fieldName          = fieldName;
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    
    return x;
}

static void setsize_free (t_setsize *x)
{
    gpointer_unset (&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void setsize_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_setsize,
            (t_newmethod)setsize_new,
            (t_method)setsize_free,
            sizeof (t_setsize),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addFloat (c, setsize_float);
    
    class_addMethod (c, (t_method)setsize_set, sym_set, A_SYMBOL, A_SYMBOL, A_NULL); 
    
    setsize_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
