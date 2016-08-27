
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

static t_class *textsize_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textsize {
    t_textclient    x_textclient;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_textsize;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsize_bang(t_textsize *x)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_textclient);
    int n, i, cnt = 0;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    for (i = 0; i < n; i++)
    {
        if (vec[i].a_type == A_SEMICOLON || vec[i].a_type == A_COMMA)
            cnt++;
    }
    if (n && vec[n-1].a_type != A_SEMICOLON && vec[n-1].a_type != A_COMMA)
        cnt++;
    outlet_float(x->x_outlet, cnt);
}

static void textsize_float(t_textsize *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_textclient);
    int start, end, n;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (buffer_getMessageAt(b, f, &start, &end))
        outlet_float(x->x_outlet, end-start);
    else outlet_float(x->x_outlet, -1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textsize_new(t_symbol *s, int argc, t_atom *argv)
{
    t_textsize *x = (t_textsize *)pd_new(textsize_class);
    x->x_outlet = outlet_new(cast_object (x), &s_float);
    textclient_init(&x->x_textclient, &argc, &argv);
    if (argc)
    {
        post("warning: text size ignoring extra argument: ");
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

void textsize_setup (void)
{
    textsize_class = class_new(sym_text__space__size,
        (t_newmethod)textsize_new, (t_method)textclient_free,
            sizeof(t_textsize), 0, A_GIMME, 0);
    class_addBang(textsize_class, textsize_bang);
    class_addFloat(textsize_class, textsize_float);
    class_setHelpName(textsize_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
