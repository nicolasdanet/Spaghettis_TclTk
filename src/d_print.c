
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

static t_class *print_tilde_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _print_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    int         x_count;
    t_symbol    *x_name;
    } t_print_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void print_tilde_float (t_print_tilde *, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void print_tilde_bang (t_print_tilde *x)
{
    print_tilde_float (x, 1.0);
}

static void print_tilde_float (t_print_tilde *x, t_float f)
{
    x->x_count = PD_MAX (0, (int)f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *print_tilde_perform (t_int *w)
{
    t_print_tilde *x = (t_print_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    if (x->x_count) {
    //
    /*
    int i=0;
    post("%s:", x->x_name->s_name);
    for(i=0; i<n; i++) {
      post("%.4g  ", in[i]);
    }
    */
    x->x_count--;
    //
    }
    
    return (w + 4);
}

static void print_tilde_dsp (t_print_tilde *x, t_signal **sp)
{
    dsp_add (print_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_blockSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *print_tilde_new (t_symbol *s)
{
    t_print_tilde *x = (t_print_tilde *)pd_new (print_tilde_class);
    
    x->x_f     = 0;
    x->x_count = 0;
    x->x_name  = (s != &s_ ? s : sym_print__tilde__);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void print_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_print__tilde__,
            (t_newmethod)print_tilde_new,
            NULL,
            sizeof (t_print_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    CLASS_SIGNAL (c, t_print_tilde, x_f);
    
    class_addDSP (c, print_tilde_dsp);
    class_addBang (c, print_tilde_bang);
    class_addFloat (c, print_tilde_float);

    print_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
