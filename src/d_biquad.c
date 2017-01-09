
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

/* ---------------- biquad~ - raw biquad filter ----------------- */

typedef struct biquadctl
{
    t_sample c_x1;
    t_sample c_x2;
    t_sample c_fb1;
    t_sample c_fb2;
    t_sample c_ff1;
    t_sample c_ff2;
    t_sample c_ff3;
} t_biquadctl;

typedef struct sigbiquad
{
    t_object x_obj;
    t_float x_f;
    t_biquadctl x_cspace;
    t_biquadctl *x_ctl;
} t_sigbiquad;

t_class *sigbiquad_class;

static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv);

static void *sigbiquad_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sigbiquad *x = (t_sigbiquad *)pd_new(sigbiquad_class);
    outlet_new(&x->x_obj, &s_signal);
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_x1 = x->x_cspace.c_x2 = 0;
    sigbiquad_list(x, s, argc, argv);
    x->x_f = 0;
    return x;
}

static t_int *sigbiquad_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    t_biquadctl *c = (t_biquadctl *)(w[3]);
    int n = (t_int)(w[4]);
    int i;
    t_sample last = c->c_x1;
    t_sample prev = c->c_x2;
    t_sample fb1 = c->c_fb1;
    t_sample fb2 = c->c_fb2;
    t_sample ff1 = c->c_ff1;
    t_sample ff2 = c->c_ff2;
    t_sample ff3 = c->c_ff3;
    for (i = 0; i < n; i++)
    {
        t_sample output =  *in++ + fb1 * last + fb2 * prev;
        if (PD_IS_BIG_OR_SMALL(output))
            output = 0; 
        *out++ = ff1 * output + ff2 * last + ff3 * prev;
        prev = last;
        last = output;
    }
    c->c_x1 = last;
    c->c_x2 = prev;
    return (w+5);
}

static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float fb1 = atom_getFloatAtIndex(0, argc, argv);
    t_float fb2 = atom_getFloatAtIndex(1, argc, argv);
    t_float ff1 = atom_getFloatAtIndex(2, argc, argv);
    t_float ff2 = atom_getFloatAtIndex(3, argc, argv);
    t_float ff3 = atom_getFloatAtIndex(4, argc, argv);
    t_float discriminant = fb1 * fb1 + 4 * fb2;
    t_biquadctl *c = x->x_ctl;
    if (discriminant < 0) /* imaginary roots -- resonant filter */
    {
            /* they're conjugates so we just check that the product
            is less than one */
        if (fb2 >= -1.0f) goto stable;
    }
    else    /* real roots */
    {
            /* check that the parabola 1 - fb1 x - fb2 x^2 has a
                vertex between -1 and 1, and that it's nonnegative
                at both ends, which implies both roots are in [1-,1]. */
        if (fb1 <= 2.0f && fb1 >= -2.0f &&
            1.0f - fb1 -fb2 >= 0 && 1.0f + fb1 - fb2 >= 0)
                goto stable;
    }
        /* if unstable, just bash to zero */
    fb1 = fb2 = ff1 = ff2 = ff3 = 0;
stable:
    c->c_fb1 = fb1;
    c->c_fb2 = fb2;
    c->c_ff1 = ff1;
    c->c_ff2 = ff2;
    c->c_ff3 = ff3;
}

static void sigbiquad_set(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv)
{
    t_biquadctl *c = x->x_ctl;
    c->c_x1 = atom_getFloatAtIndex(0, argc, argv);
    c->c_x2 = atom_getFloatAtIndex(1, argc, argv);
}

static void sigbiquad_dsp(t_sigbiquad *x, t_signal **sp)
{
    dsp_add(sigbiquad_perform, 4,
        sp[0]->s_vector, sp[1]->s_vector, 
            x->x_ctl, sp[0]->s_vectorSize);

}

void sigbiquad_setup(void)
{
    sigbiquad_class = class_new(sym_biquad__tilde__, (t_newmethod)sigbiquad_new,
        0, sizeof(t_sigbiquad), 0, A_GIMME, 0);
    CLASS_SIGNAL(sigbiquad_class, t_sigbiquad, x_f);
    class_addMethod(sigbiquad_class, (t_method)sigbiquad_dsp,
        sym_dsp, A_CANT, 0);
    class_addList(sigbiquad_class, (t_method)sigbiquad_list);
    class_addMethod(sigbiquad_class, (t_method)sigbiquad_set, sym_set,
        A_GIMME, 0);
    class_addMethod(sigbiquad_class, (t_method)sigbiquad_set, sym_clear,
        A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
