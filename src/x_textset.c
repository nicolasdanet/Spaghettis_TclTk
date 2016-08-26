
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

typedef struct _textset {
    t_textclient    x_textclient;           /* Must be the first. */
    t_float         x_line;
    t_float         x_field;
    } t_textset;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *textset_class;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void textset_list(t_textset *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_textclient);
    int start, end, n, lineno = x->x_line, fieldno = x->x_field, i;
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
    textclient_update(&x->x_textclient);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textset_new(t_symbol *s, int argc, t_atom *argv)
{
    t_textset *x = (t_textset *)pd_new(textset_class);
    inlet_newFloat(cast_object (x), &x->x_line);
    inlet_newFloat(cast_object (x), &x->x_field);
    x->x_line = 0;
    x->x_field = -1;
    textclient_init(&x->x_textclient, &argc, &argv);
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_line = argv->a_w.w_float;
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
            x->x_field = argv->a_w.w_float;
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
    if (TEXTCLIENT_ASPOINTER (&x->x_textclient))
        inlet_newPointer(cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
    else inlet_newSymbol(cast_object (x), TEXTCLIENT_GETNAME (&x->x_textclient));
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textset_setup (void)
{
    textset_class = class_new(sym_text__space__set,
        (t_newmethod)textset_new, (t_method)textclient_free,
            sizeof(t_textset), 0, A_GIMME, 0);
    class_addList(textset_class, textset_list);
    class_setHelpName(textset_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
