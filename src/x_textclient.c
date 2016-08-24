
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

extern t_class *textdefine_class; 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void textclient_init (t_textclient *x, int *ac, t_atom **av, char *dummy)
{
    int argc = *ac;
    t_atom *argv = *av;
    
    x->tc_name               = NULL;
    x->tc_templateIdentifier = NULL;
    x->tc_fieldName          = NULL;
    
    gpointer_init (&x->tc_gpointer);
    
    while (argc && IS_SYMBOL (argv)) {
    //
    if (GET_SYMBOL (argv) == sym___dash__s || GET_SYMBOL (argv) == sym___dash__symbol) {
        if (argc >= 3 && IS_SYMBOL (argv + 1) && IS_SYMBOL (argv + 2)) {
            x->tc_templateIdentifier = utils_makeTemplateIdentifier (GET_SYMBOL (argv + 1));
            x->tc_fieldName = GET_SYMBOL (argv + 2);
            argc -= 3; argv += 3;
        }
    }
    
    break;
    //
    }
    
    if (argc && IS_SYMBOL (argv) && !x->tc_templateIdentifier) {
        x->tc_name = GET_SYMBOL (argv); 
        argc--; argv++;
    }
    
    *ac = argc;
    *av = argv;
}

    /* find the binbuf for this object.  This should be reusable for other
    objects.  Prints an error  message and returns 0 on failure. */
t_buffer *textclient_fetchBuffer(t_textclient *x)
{
    if (x->tc_name)       /* named text object */
    {
        t_textbuffer *y = (t_textbuffer *)pd_findByClass(x->tc_name,
            textdefine_class);
        if (y)
            return (y->tb_buffer);
        else
        {
            post_error ("text: couldn't find text buffer '%s'",
                x->tc_name->s_name);
            return (0);
        }
    }
    else if (x->tc_templateIdentifier)   /* by pointer */
    {
        t_template *template = template_findByIdentifier(x->tc_templateIdentifier);
        t_word *vec; 
        int onset, type;
        t_symbol *arraytype;
        if (!template)
        {
            post_error ("text: couldn't find struct %s", x->tc_templateIdentifier->s_name);
            return (0);
        }
        if (!gpointer_isValid(&x->tc_gpointer))
        {
            post_error ("text: stale or empty pointer");
            return (0);
        }
        vec = gpointer_getData (&x->tc_gpointer);

        if (!template_findField(template,   /* Remove template_findField ASAP !!! */
            x->tc_fieldName, &onset, &type, &arraytype))
        {
            post_error ("text: no field named %s", x->tc_fieldName->s_name);
            return (0);
        }
        if (type != DATA_TEXT)
        {
            post_error ("text: field %s not of type text", x->tc_fieldName->s_name);
            return (0);
        }
        return (*(t_buffer **)(((char *)vec) + onset));
    }
    else return (0);    /* shouldn't happen */
}

void textclient_send(t_textclient *x)
{
    if (x->tc_name)       /* named text object */
    {
        t_textbuffer *y = (t_textbuffer *)pd_findByClass(x->tc_name,
            textdefine_class);
        if (y)
            textbuffer_send(y);
        else { PD_BUG; }
    }
    else if (x->tc_templateIdentifier)   /* by pointer */
    {
        t_template *template = template_findByIdentifier(x->tc_templateIdentifier);
        if (!template)
        {
            post_error ("text: couldn't find struct %s", x->tc_templateIdentifier->s_name);
            return;
        }
        if (!gpointer_isValid(&x->tc_gpointer))
        {
            post_error ("text: stale or empty pointer");
            return;
        }
        gpointer_redraw (&x->tc_gpointer);
    }
}

void textclient_free (t_textclient *x)
{
    gpointer_unset (&x->tc_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
