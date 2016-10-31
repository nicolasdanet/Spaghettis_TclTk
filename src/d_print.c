/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  miscellaneous: print~; more to come.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"
#include <stdio.h>
#include <string.h>

static t_class *print_tilde_class;

typedef struct _print_tilde
{
    t_object x_obj;
    t_float x_f;
    t_symbol *x_sym;
    int x_count;
} t_print_tilde;

static t_int *print_tilde_perform(t_int *w)
{
    t_print_tilde *x = (t_print_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    if (x->x_count)
    {
        int i=0;
        post("%s:", x->x_sym->s_name);
        for(i=0; i<n; i++) {
          post("%.4g  ", in[i]);
        }
        x->x_count--;
    }
    return (w+4);
}

static void print_tilde_dsp(t_print_tilde *x, t_signal **sp)
{
    dsp_add(print_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_blockSize);
}

static void print_tilde_float(t_print_tilde *x, t_float f)
{
    if (f < 0) f = 0;
    x->x_count = f;
}

static void print_tilde_bang(t_print_tilde *x)
{
    x->x_count = 1;
}

static void *print_tilde_new(t_symbol *s)
{
    t_print_tilde *x = (t_print_tilde *)pd_new(print_tilde_class);
    x->x_sym = (s->s_name[0]? s : sym_print__tilde__);
    x->x_count = 0;
    x->x_f = 0;
    return (x);
}

void print_tilde_setup(void)
{
    print_tilde_class = class_new(sym_print__tilde__, (t_newmethod)print_tilde_new, 0,
        sizeof(t_print_tilde), 0, A_DEFSYMBOL, 0);
    CLASS_SIGNAL(print_tilde_class, t_print_tilde, x_f);
    class_addMethod(print_tilde_class, (t_method)print_tilde_dsp, sym_dsp, 0);
    class_addBang(print_tilde_class, print_tilde_bang);
    class_addFloat(print_tilde_class, print_tilde_float);
}







