
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

static t_class *tabread_tilde_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabread_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabread_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabread_tilde_set (t_tabread_tilde *x, t_symbol *s)
{
    tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector, sym_tabread__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *tabread_tilde_perform (t_int *w)
{
    t_tabread_tilde *x = (t_tabread_tilde *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    PD_RESTRICTED out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    
    int size = x->x_size;
    t_word *data = x->x_vector;
    
    if (data && size > 0) {
    //
    while (n--) {
    //
    t_sample f = *in++;
    int position = PD_CLAMP ((int)f, 0, size - 1);
    t_sample g = WORD_FLOAT (data + position);
    *out++ = g;
    //
    }
    //
    } else { while (n--) { *out++ = 0; } }

    return (w + 5);
}

static void tabread_tilde_dsp (t_tabread_tilde *x, t_signal **sp)
{
    tabread_tilde_set (x, x->x_name);

    PD_ASSERT (sp[0]->s_vector != sp[1]->s_vector);
    
    dsp_add (tabread_tilde_perform, 4, x, sp[0]->s_vector, sp[1]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabread_tilde_new (t_symbol *s)
{
    t_tabread_tilde *x = (t_tabread_tilde *)pd_new (tabread_tilde_class);
    
    x->x_name   = s;
    x->x_outlet = outlet_new (cast_object (x), &s_signal);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabread_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabread__tilde__,
            (t_newmethod)tabread_tilde_new,
            NULL,
            sizeof (t_tabread_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
        
    CLASS_SIGNAL (c, t_tabread_tilde, x_f);
    
    class_addDSP (c, (t_method)tabread_tilde_dsp);
    
    class_addMethod (c, (t_method)tabread_tilde_set, sym_set, A_SYMBOL, A_NULL);
    
    tabread_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
