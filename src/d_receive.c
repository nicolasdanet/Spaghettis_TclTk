
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_class *sigsend_class;
/* ----------------------------- receive~ ----------------------------- */
static t_class *sigreceive_class;

typedef struct _sigreceive
{
    t_object x_obj;
    t_symbol *x_sym;
    t_sample *x_wherefrom;
    int x_n;
} t_sigreceive;

static void *sigreceive_new(t_symbol *s)
{
    t_sigreceive *x = (t_sigreceive *)pd_new(sigreceive_class);
    x->x_n = DSP_SEND_SIZE;             /* LATER find our vector size correctly */
    x->x_sym = s;
    x->x_wherefrom = 0;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *sigreceive_perform(t_int *w)
{
    t_sigreceive *x = (t_sigreceive *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *in = x->x_wherefrom;
    if (in)
    {
        while (n--)
            *out++ = *in++; 
    }
    else
    {
        while (n--)
            *out++ = 0; 
    }
    return (w+4);
}

/* tb: vectorized receive function */
static t_int *sigreceive_perf8(t_int *w)
{
    t_sigreceive *x = (t_sigreceive *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *in = x->x_wherefrom;
    if (in)
    {
        for (; n; n -= 8, in += 8, out += 8)
        {
            out[0] = in[0]; out[1] = in[1]; out[2] = in[2]; out[3] = in[3]; 
            out[4] = in[4]; out[5] = in[5]; out[6] = in[6]; out[7] = in[7]; 
        }
    }
    else
    {
        for (; n; n -= 8, in += 8, out += 8)
        {
            out[0] = 0; out[1] = 0; out[2] = 0; out[3] = 0; 
            out[4] = 0; out[5] = 0; out[6] = 0; out[7] = 0; 
        }
    }
    return (w+4);
}

static void sigreceive_set(t_sigreceive *x, t_symbol *s)
{
    t_sigsend *sender = (t_sigsend *)pd_getThingByClass((x->x_sym = s),
        sigsend_class);
    if (sender)
    {
        if (sender->x_n == x->x_n)
            x->x_wherefrom = sender->x_vec;
        else
        {
            post_error ("receive~ %s: vector size mismatch", x->x_sym->s_name);
            x->x_wherefrom = 0;
        }
    }
    else
    {
        post_error ("receive~ %s: no matching send", x->x_sym->s_name);
        x->x_wherefrom = 0;
    }
}

static void sigreceive_dsp(t_sigreceive *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != x->x_n)
    {
        post_error ("receive~ %s: vector size mismatch", x->x_sym->s_name);
    }
    else
    {
        sigreceive_set(x, x->x_sym);
        if (sp[0]->s_vectorSize&7)
            dsp_add(sigreceive_perform, 3,
                x, sp[0]->s_vector, sp[0]->s_vectorSize);
        else dsp_add(sigreceive_perf8, 3,
            x, sp[0]->s_vector, sp[0]->s_vectorSize);
    }
}

void sigreceive_setup(void)
{
    sigreceive_class = class_new(sym_receive__tilde__,
        (t_newmethod)sigreceive_new, 0,
        sizeof(t_sigreceive), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)sigreceive_new, sym_r__tilde__, A_DEFSYMBOL, 0);
    class_addMethod(sigreceive_class, (t_method)sigreceive_set, sym_set,
        A_SYMBOL, 0);
    class_addMethod(sigreceive_class, (t_method)sigreceive_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(sigreceive_class, sym_send__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
