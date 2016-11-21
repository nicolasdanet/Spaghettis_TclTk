
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
#include "d_dsp.h"
#include "d_global.h"

extern t_class *sigcatch_class;
/* ----------------------------- throw~ ----------------------------- */
static t_class *sigthrow_class;

typedef struct _sigthrow
{
    t_object x_obj;
    t_symbol *x_sym;
    t_sample *x_whereto;
    int x_vectorSize;
    t_float x_f;
} t_sigthrow;

static void *sigthrow_new(t_symbol *s)
{
    t_sigthrow *x = (t_sigthrow *)pd_new(sigthrow_class);
    x->x_sym = s;
    x->x_whereto  = 0;
    x->x_vectorSize = DSP_SEND_SIZE;
    x->x_f = 0;
    return (x);
}

static t_int *sigthrow_perform(t_int *w)
{
    t_sigthrow *x = (t_sigthrow *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *out = x->x_whereto;
    if (out)
    {
        while (n--)
        {
            *out += (PD_BIG_OR_SMALL(*in) ? 0 : *in);
            out++;
            in++;
        }
    }
    return (w+4);
}

static void sigthrow_set(t_sigthrow *x, t_symbol *s)
{
    t_catch_tilde *catcher = (t_catch_tilde *)pd_getThingByClass((x->x_sym = s),
        sigcatch_class);
    if (catcher)
    {
        if (catcher->x_vectorSize == x->x_vectorSize)
            x->x_whereto = catcher->x_vector;
        else
        {
            post_error ("throw~ %s: vector size mismatch", x->x_sym->s_name);
            x->x_whereto = 0;
        }
    }
    else
    {
        post_error ("throw~ %s: no matching catch", x->x_sym->s_name);
        x->x_whereto = 0;
    }
}

static void sigthrow_dsp(t_sigthrow *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != x->x_vectorSize)
    {
        post_error ("throw~ %s: vector size mismatch", x->x_sym->s_name);
    }
    else
    {
        sigthrow_set(x, x->x_sym);
        dsp_add(sigthrow_perform, 3,
            x, sp[0]->s_vector, sp[0]->s_vectorSize);
    }
}

void sigthrow_setup(void)
{
    sigthrow_class = class_new(sym_throw__tilde__, (t_newmethod)sigthrow_new, 0,
        sizeof(t_sigthrow), 0, A_DEFSYMBOL, 0);
    class_addMethod(sigthrow_class, (t_method)sigthrow_set, sym_set,
        A_SYMBOL, 0);
    CLASS_SIGNAL(sigthrow_class, t_sigthrow, x_f);
    class_addMethod(sigthrow_class, (t_method)sigthrow_dsp,
        sym_dsp, A_CANT, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
