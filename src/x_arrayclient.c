
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

void arrayclient_free (t_arrayclient *x)
{
    gpointer_unset (&x->ac_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/*
t_buffer *textclient_fetchBuffer (t_textclient *x)
{
    if (x->tc_name) {

        t_textbuffer *y = (t_textbuffer *)pd_findByClass (x->tc_name, textdefine_class);

        if (y) { return textbuffer_getBuffer (y); }
        else {
            error_canNotFind (sym_text, x->tc_name);
        }

    } else if (x->tc_templateIdentifier) {

        if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
            if (gpointer_hasField (&x->tc_gpointer, x->tc_fieldName)) {
                if (gpointer_fieldIsText (&x->tc_gpointer, x->tc_fieldName)) {
                    return gpointer_getText (&x->tc_gpointer, x->tc_fieldName);
                    
        } else { error_invalid (sym_text, x->tc_fieldName); }
        } else { error_missingField (sym_text, x->tc_fieldName); }
        } else { error_invalid (sym_text, &s_pointer); }
    }
    
    return NULL;
}

t_glist *textclient_fetchView (t_textclient *x)
{
    if (x->tc_name) {

        t_textbuffer *y = (t_textbuffer *)pd_findByClass (x->tc_name, textdefine_class);

        if (y) { return textbuffer_getView (y); }
        else {
            error_canNotFind (sym_text, x->tc_name);
        }

    } else if (x->tc_templateIdentifier) {

        if (gpointer_isValidInstanceOf (&x->tc_gpointer, x->tc_templateIdentifier)) {
            if (gpointer_hasField (&x->tc_gpointer, x->tc_fieldName)) {
                if (gpointer_fieldIsText (&x->tc_gpointer, x->tc_fieldName)) {
                    return gpointer_getView (&x->tc_gpointer);
                    
        } else { error_invalid (sym_text, x->tc_fieldName); }
        } else { error_missingField (sym_text, x->tc_fieldName); }
        } else { error_invalid (sym_text, &s_pointer); }
    }
    
    return NULL;
}
*/

t_array *arrayclient_fetchArray(t_arrayclient *x, t_glist **glist)
{
    if (x->ac_name)
    {
        t_garray *y = (t_garray *)pd_findByClass (x->ac_name, garray_class);
        
        if (y)
        {
            *glist = garray_getOwner(y);
            return (garray_getArray(y));
        }
        else
        {
            post_error ("array: couldn't find named array '%s'",
                x->ac_name->s_name);
            *glist = 0;
            return (0);
        }
    }
    else if (x->ac_templateIdentifier)   /* by pointer */
    {
        t_template *template = template_findByIdentifier(x->ac_templateIdentifier);
        t_word *vec; 
        int onset, type;
        t_symbol *arraytype;
        if (!template)
        {
            post_error ("array: couldn't find struct %s", x->ac_templateIdentifier->s_name);
            return (0);
        }
        if (!gpointer_isValid(&x->ac_gpointer))
        {
            post_error ("array: stale or empty pointer");
            return (0);
        }
        vec = gpointer_getData (&x->ac_gpointer);

        if (!template_findField(template,   /* Remove template_findField ASAP !!! */
            x->ac_fieldName, &onset, &type, &arraytype))
        {
            post_error ("array: no field named %s", x->ac_fieldName->s_name);
            return (0);
        }
        if (type != DATA_ARRAY)
        {
            post_error ("array: field %s not of type array",
                x->ac_fieldName->s_name);
            return (0);
        }
        *glist = gpointer_getView (&x->ac_gpointer);

        return (*(t_array **)(((char *)vec) + onset));
    }
    else return (0);    /* shouldn't happen */
}

void arrayclient_update (t_arrayclient *x)
{
    t_glist *glist = 0;
    t_array *a = arrayclient_fetchArray(x, &glist);
    array_redraw (a, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
