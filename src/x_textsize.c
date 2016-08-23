
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
#define x_sym x_tc.tc_sym
#define x_gp x_tc.tc_gp
#define x_struct x_tc.tc_struct
#define x_field x_tc.tc_field

/* ---------------- text_size object - output number of lines -------------- */
t_class *text_size_class;

typedef struct _text_size
{
    t_text_client x_tc;
    t_outlet *x_out1;       /* float */
} t_text_size;

void *text_size_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_size *x = (t_text_size *)pd_new(text_size_class);
    x->x_out1 = outlet_new(&x->x_obj, &s_float);
    text_client_argparse(&x->x_tc, &argc, &argv, "text size");
    if (argc)
    {
        post("warning: text size ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_tc.tc_sym);
    return (x);
}

static void text_size_bang(t_text_size *x)
{
    t_buffer *b = text_client_getbuf(&x->x_tc);
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
    outlet_float(x->x_out1, cnt);
}

static void text_size_float(t_text_size *x, t_float f)
{
    t_buffer *b = text_client_getbuf(&x->x_tc);
    int start, end, n;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (text_nthline(n, vec, f, &start, &end))
        outlet_float(x->x_out1, end-start);
    else outlet_float(x->x_out1, -1);
}

void textsize_setup (void)
{
    text_size_class = class_new(sym_text__space__size,
        (t_newmethod)text_size_new, (t_method)text_client_free,
            sizeof(t_text_size), 0, A_GIMME, 0);
    class_addBang(text_size_class, text_size_bang);
    class_addFloat(text_size_class, text_size_float);
    class_setHelpName(text_size_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
