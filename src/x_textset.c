
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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define x_obj x_tc.tc_obj
#define x_sym x_tc.tc_name
#define x_gp x_tc.tc_gpointer
#define x_struct x_tc.tc_templateIdentifier
#define x_field x_tc.tc_fieldName

typedef struct _text_set
{
    t_textclient x_tc;
    t_float x_f1;           /* line number */
    t_float x_f2;           /* field number, -1 for whole line */
} t_text_set;

t_class *text_set_class;

void *text_set_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_set *x = (t_text_set *)pd_new(text_set_class);
    inlet_newFloat(&x->x_obj, &x->x_f1);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = -1;
    textclient_init(&x->x_tc, &argc, &argv);
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_f1 = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field number");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_f2 = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field count");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text set ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_sym);
    return (x);
}

static void text_set_list(t_text_set *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_tc);
    int start, end, n, lineno = x->x_f1, fieldno = x->x_f2, i;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (lineno < 0)
    {
        post_error ("text set: line number (%d) < 0", lineno);
        return;
    }
    if (buffer_getMessageAt(b, lineno, &start, &end))
    {
        if (fieldno < 0)
        {
            if (end - start != argc)  /* grow or shrink */
            {
                int oldn = n;
                n = n + (argc - (end-start));
                if (n > oldn)
                    buffer_resize(b, n);
                vec = buffer_atoms(b);
                memmove(&vec[start + argc], &vec[end],
                    sizeof(*vec) * (oldn - end));
                if (n < oldn) {
                    buffer_resize(b, n);
                    vec = buffer_atoms(b);
                }
            }
        }
        else
        {
            if (fieldno >= end - start)
            {
                post_error ("text set: field number (%d) past end of line",
                    fieldno);
                return;
            }
            if (fieldno + argc > end - start)
                argc = (end - start) - fieldno;
            start += fieldno;
        }
    }
    else if (fieldno < 0)  /* if line number too high just append to end */
    {
        int addsemi = (n && vec[n-1].a_type != A_SEMICOLON &&
            vec[n-1].a_type != A_COMMA), newsize = n + addsemi + argc + 1;
        buffer_resize(b, newsize);
        vec = buffer_atoms(b);
        if (addsemi)
            SET_SEMICOLON(&vec[n]);
        SET_SEMICOLON(&vec[newsize-1]);
        start = n+addsemi;
    }
    else
    {
        post("text set: %d: line number out of range", lineno);
        return;
    }
    for (i = 0; i < argc; i++)
    {
        if (argv[i].a_type == A_POINTER)
            SET_SYMBOL(&vec[start+i], sym___parenthesis__pointer__parenthesis__);
        else vec[start+i] = argv[i];
    }
    textclient_update(&x->x_tc);
}

void textset_setup (void)
{
    text_set_class = class_new(sym_text__space__set,
        (t_newmethod)text_set_new, (t_method)textclient_free,
            sizeof(t_text_set), 0, A_GIMME, 0);
    class_addList(text_set_class, text_set_list);
    class_setHelpName(text_set_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
