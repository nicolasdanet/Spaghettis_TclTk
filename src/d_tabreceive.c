
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabreceive_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabreceive_tilde {
    t_object    x_obj;                          /* Must be the first. */
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabreceive_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabreceive_tilde_set (t_tabreceive_tilde *x, t_symbol *s)
{
    tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector, sym_tabreceive__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *tabreceive_tilde_perform (t_int *w)
{
    t_tabreceive_tilde *x = (t_tabreceive_tilde *)(w[1]);
    PD_RESTRICTED out = (t_sample *)(w[2]);
    int n = (int)w[3];
    
    t_word *data = x->x_vector;
    
    if (data) {
    //
    int size = PD_MIN (n, x->x_size);
    int pad  = n - size;
    
    while (size--) { *out++ = (t_sample)WORD_FLOAT (data); data++; }
    while (pad--)  { *out++ = (t_sample)0.0; }
    //
    } else { while (n--) { *out++ = (t_sample)0.0; } }
    
    return (w + 4);
}

static void tabreceive_tilde_dsp (t_tabreceive_tilde *x, t_signal **sp)
{
    tabreceive_tilde_set (x, x->x_name);
    
    dsp_add (tabreceive_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabreceive_tilde_new (t_symbol *s)
{
    t_tabreceive_tilde *x = (t_tabreceive_tilde *)pd_new (tabreceive_tilde_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabreceive_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabreceive__tilde__,
            (t_newmethod)tabreceive_tilde_new,
            NULL,
            sizeof (t_tabreceive_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addDSP (c, (t_method)tabreceive_tilde_dsp);
    
    class_addMethod (c, (t_method)tabreceive_tilde_set, sym_set, A_SYMBOL, A_NULL);
        
    tabreceive_tilde_class = c;
}

void tabreceive_tilde_destroy (void)
{
    CLASS_FREE (tabreceive_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
