
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

/* ------- text_get object - output all or part of nth lines ----------- */
t_class *text_get_class;

typedef struct _text_get
{
    t_textclient x_tc;
    t_outlet *x_out1;       /* list */
    t_outlet *x_out2;       /* 1 if comma terminated, 0 if semi, 2 if none */
    t_float x_f1;           /* field number, -1 for whole line */
    t_float x_f2;           /* number of fields */
} t_text_get;

#define x_obj x_tc.tc_obj
#define x_sym x_tc.tc_name
#define x_gp x_tc.tc_gpointer
#define x_struct x_tc.tc_templateIdentifier
#define x_field x_tc.tc_fieldName

void *text_get_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_get *x = (t_text_get *)pd_new(text_get_class);
    x->x_out1 = outlet_new(&x->x_obj, &s_list);
    x->x_out2 = outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f1);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = -1;
    x->x_f2 = 1;
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
        post("warning: text get ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_sym);
    return (x);
}

static void text_get_float(t_text_get *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_tc);
    int start, end, n, startfield, nfield;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    startfield = x->x_f1;
    nfield = x->x_f2;
    if (buffer_getMessageAt(b, f, &start, &end))
    {
        int outc = end - start, k;
        t_atom *outv;
        if (x->x_f1 < 0)    /* negative start field for whole line */
        {
                /* tell us what terminated the line (semi or comma) */
            outlet_float(x->x_out2, (end < n && vec[end].a_type == A_COMMA));
            ATOMS_ALLOCA(outv, outc);
            for (k = 0; k < outc; k++)
                outv[k] = vec[start+k];
            outlet_list(x->x_out1, 0, outc, outv);
            ATOMS_FREEA(outv, outc);
        }
        else if (startfield + nfield > outc)
            post_error ("text get: field request (%d %d) out of range",
                startfield, nfield); 
        else
        {
            ATOMS_ALLOCA(outv, nfield);
            for (k = 0; k < nfield; k++)
                outv[k] = vec[(start+startfield)+k];
            outlet_list(x->x_out1, 0, nfield, outv);
            ATOMS_FREEA(outv, nfield);
        }
    }
    else if (x->x_f1 < 0)   /* whole line but out of range: empty list and 2 */
    {
        outlet_float(x->x_out2, 2);         /* 2 for out of range */
        outlet_list(x->x_out1, 0, 0, 0);    /* ... and empty list */
    }
}

void textget_setup (void)
{
    text_get_class = class_new(sym_text__space__get,
        (t_newmethod)text_get_new, (t_method)textclient_free,
            sizeof(t_text_get), 0, A_GIMME, 0);
    class_addFloat(text_get_class, text_get_float);
    class_setHelpName(text_get_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
