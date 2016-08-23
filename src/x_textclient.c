
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
#pragma mark -

void text_client_argparse (t_textclient *x, int *argcp, t_atom **argvp, char *name)
{
    int argc = *argcp;
    t_atom *argv = *argvp;
    x->tc_symbol = x->tc_templateIdentifier = x->tc_fieldName = 0;
    gpointer_init(&x->tc_gpointer);
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-s") &&
            argc >= 3 && argv[1].a_type == A_SYMBOL && argv[2].a_type == A_SYMBOL)
        {
            x->tc_templateIdentifier = utils_makeTemplateIdentifier(argv[1].a_w.w_symbol);
            x->tc_fieldName = argv[2].a_w.w_symbol;
            argc -= 2; argv += 2;
        }
        else
        {
            post_error ("%s: unknown flag '%s'...", name,
                argv->a_w.w_symbol->s_name);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        if (x->tc_templateIdentifier)
            post_error ("%s: extra names after -s..", name);
        else x->tc_symbol = argv->a_w.w_symbol;
        argc--; argv++;
    }
    *argcp = argc;
    *argvp = argv;
}

    /* find the binbuf for this object.  This should be reusable for other
    objects.  Prints an error  message and returns 0 on failure. */
t_buffer *text_client_getbuf(t_textclient *x)
{
    if (x->tc_symbol)       /* named text object */
    {
        t_textbuffer *y = (t_textbuffer *)pd_findByClass(x->tc_symbol,
            textdefine_class);
        if (y)
            return (y->tb_buffer);
        else
        {
            post_error ("text: couldn't find text buffer '%s'",
                x->tc_symbol->s_name);
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

void text_client_senditup(t_textclient *x)
{
    if (x->tc_symbol)       /* named text object */
    {
        t_textbuffer *y = (t_textbuffer *)pd_findByClass(x->tc_symbol,
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

void text_client_free(t_textclient *x)
{
    gpointer_unset(&x->tc_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
