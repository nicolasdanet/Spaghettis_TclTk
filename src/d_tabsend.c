
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabsend_tilde_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabsend_tilde {
    t_object    x_obj;                      /* Must be the first. */
    t_float     x_f;
    int         x_redraw;
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    } t_tabsend_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabsend_tilde_polling (t_tabsend_tilde *x)
{
    if (x->x_redraw) {
    //
    t_garray *a = (t_garray *)pd_getThingByClass (x->x_name, garray_class);
    
    if (a) { garray_redraw (a); }

    x->x_redraw = 0;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabsend_tilde_set (t_tabsend_tilde *x, t_symbol *s)
{
    tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector, sym_tabsend__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *tabsend_tilde_perform (t_int *w)
{
    t_tabsend_tilde *x = (t_tabsend_tilde *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)w[3];
    
    t_word *data = x->x_vector;

    if (data) {
    //
    n = PD_MIN (x->x_size, n);

    while (n--) {
    //  
    t_sample f = *in++;
    if (PD_IS_BIG_OR_SMALL (f)) { f = (t_sample)0.0; }
    WORD_FLOAT (data) = f;
    data++;
    //
    }
    
    x->x_redraw = 1;
    //
    }

    return (w + 4);
}

static void tabsend_tilde_dsp (t_tabsend_tilde *x, t_signal **sp)
{
    tabsend_tilde_set (x, x->x_name);
    
    dsp_add (tabsend_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabsend_tilde_new (t_symbol *s)
{
    t_tabsend_tilde *x = (t_tabsend_tilde *)pd_new (tabsend_tilde_class);
    
    x->x_name = s;
    
    instance_pollingRegister (cast_pd (x));
    
    return x;
}

static void tabsend_tilde_free (t_tabsend_tilde *x)
{
    instance_pollingUnregister (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabsend_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabsend__tilde__,
            (t_newmethod)tabsend_tilde_new,
            (t_method)tabsend_tilde_free,
            sizeof (t_tabsend_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    CLASS_SIGNAL (c, t_tabsend_tilde, x_f);
    
    class_addDSP (c, (t_method)tabsend_tilde_dsp);
    class_addPolling (c, (t_method)tabsend_tilde_polling);
    
    class_addMethod (c, (t_method)tabsend_tilde_set, sym_set, A_SYMBOL, A_NULL);
    
    tabsend_tilde_class = c;
}

void tabsend_tilde_destroy (void)
{
    CLASS_FREE (tabsend_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
