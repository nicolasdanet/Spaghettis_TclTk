
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *send_tilde_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *receive_tilde_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _receive_tilde {
    t_object    x_obj;
    int         x_vectorSize;
    t_sample    *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_receive_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void receive_tilde_set(t_receive_tilde *x, t_symbol *s)
{
    t_send_tilde *sender = (t_send_tilde *)pd_getThingByClass((x->x_name = s),
        send_tilde_class);
    if (sender)
    {
        if (sender->x_vectorSize == x->x_vectorSize)
            x->x_vector = sender->x_vector;
        else
        {
            post_error ("receive~ %s: vector size mismatch", x->x_name->s_name);
            x->x_vector = 0;
        }
    }
    else
    {
        post_error ("receive~ %s: no matching send", x->x_name->s_name);
        x->x_vector = 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *receive_tilde_perform(t_int *w)
{
    t_receive_tilde *x = (t_receive_tilde *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    t_sample *in = x->x_vector;
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

static void receive_tilde_dsp(t_receive_tilde *x, t_signal **sp)
{
    if (sp[0]->s_vectorSize != x->x_vectorSize)
    {
        post_error ("receive~ %s: vector size mismatch", x->x_name->s_name);
    }
    else
    {
        receive_tilde_set(x, x->x_name);
        dsp_add(receive_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *receive_tilde_new(t_symbol *s)
{
    t_receive_tilde *x = (t_receive_tilde *)pd_new(receive_tilde_class);
    x->x_vectorSize = DSP_SEND_SIZE;             /* LATER find our vector size correctly */
    x->x_name = s;
    x->x_vector = 0;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void receive_tilde_setup(void)
{
    receive_tilde_class = class_new(sym_receive__tilde__,
        (t_newmethod)receive_tilde_new, 0,
        sizeof(t_receive_tilde), 0, A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)receive_tilde_new, sym_r__tilde__, A_DEFSYMBOL, 0);
    class_addMethod(receive_tilde_class, (t_method)receive_tilde_set, sym_set,
        A_SYMBOL, 0);
    class_addMethod(receive_tilde_class, (t_method)receive_tilde_dsp,
        sym_dsp, A_CANT, 0);
    class_setHelpName(receive_tilde_class, sym_send__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
