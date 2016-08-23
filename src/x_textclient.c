
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

void text_client_argparse (t_text_client *x, int *argcp, t_atom **argvp, char *name)
{
    int argc = *argcp;
    t_atom *argv = *argvp;
    x->tc_sym = x->tc_struct = x->tc_field = 0;
    gpointer_init(&x->tc_gp);
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-s") &&
            argc >= 3 && argv[1].a_type == A_SYMBOL && argv[2].a_type == A_SYMBOL)
        {
            x->tc_struct = utils_makeTemplateIdentifier(argv[1].a_w.w_symbol);
            x->tc_field = argv[2].a_w.w_symbol;
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
        if (x->tc_struct)
            post_error ("%s: extra names after -s..", name);
        else x->tc_sym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    *argcp = argc;
    *argvp = argv;
}

    /* find the binbuf for this object.  This should be reusable for other
    objects.  Prints an error  message and returns 0 on failure. */
t_buffer *text_client_getbuf(t_text_client *x)
{
    if (x->tc_sym)       /* named text object */
    {
        t_textbuf *y = (t_textbuf *)pd_findByClass(x->tc_sym,
            textdefine_class);
        if (y)
            return (y->b_binbuf);
        else
        {
            post_error ("text: couldn't find text buffer '%s'",
                x->tc_sym->s_name);
            return (0);
        }
    }
    else if (x->tc_struct)   /* by pointer */
    {
        t_template *template = template_findByIdentifier(x->tc_struct);
        t_word *vec; 
        int onset, type;
        t_symbol *arraytype;
        if (!template)
        {
            post_error ("text: couldn't find struct %s", x->tc_struct->s_name);
            return (0);
        }
        if (!gpointer_isValid(&x->tc_gp))
        {
            post_error ("text: stale or empty pointer");
            return (0);
        }
        vec = gpointer_getData (&x->tc_gp);

        if (!template_findField(template,   /* Remove template_findField ASAP !!! */
            x->tc_field, &onset, &type, &arraytype))
        {
            post_error ("text: no field named %s", x->tc_field->s_name);
            return (0);
        }
        if (type != DATA_TEXT)
        {
            post_error ("text: field %s not of type text", x->tc_field->s_name);
            return (0);
        }
        return (*(t_buffer **)(((char *)vec) + onset));
    }
    else return (0);    /* shouldn't happen */
}

void text_client_senditup(t_text_client *x)
{
    if (x->tc_sym)       /* named text object */
    {
        t_textbuf *y = (t_textbuf *)pd_findByClass(x->tc_sym,
            textdefine_class);
        if (y)
            textbuf_senditup(y);
        else { PD_BUG; }
    }
    else if (x->tc_struct)   /* by pointer */
    {
        t_template *template = template_findByIdentifier(x->tc_struct);
        if (!template)
        {
            post_error ("text: couldn't find struct %s", x->tc_struct->s_name);
            return;
        }
        if (!gpointer_isValid(&x->tc_gp))
        {
            post_error ("text: stale or empty pointer");
            return;
        }
        gpointer_redraw (&x->tc_gp);
    }
}

void text_client_free(t_text_client *x)
{
    gpointer_unset(&x->tc_gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
