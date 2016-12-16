
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
#include "g_graphics.h"
#include "d_dsp.h"
#include "d_tab.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *tabwrite_tilde_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabwrite_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    int         x_redraw;
    int         x_phase;
    int         x_size;
    t_word      *x_vector;
    t_symbol    *x_name;
    } t_tabwrite_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tab_fetchArray (t_symbol *s, int *size, t_word **data, t_symbol *err)
{
    (*size) = 0;
    (*data) = NULL;
    
    {
        t_garray *a = (t_garray *)pd_getThingByClass (s, garray_class);
        
        if (!a) { if (s != &s_) { error_canNotFind (err, s); } }
        else {
            garray_getData (a, size, data);
            garray_setAsUsedInDSP (a);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void tabwrite_tilde_bang (t_tabwrite_tilde *x)
{
    x->x_phase = 0;
}

static void tabwrite_tilde_polling (t_tabwrite_tilde *x)
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

static void tabwrite_tilde_set (t_tabwrite_tilde *x, t_symbol *s)
{
    tab_fetchArray ((x->x_name = s), &x->x_size, &x->x_vector, sym_tabwrite__tilde__);
}

static void tabwrite_tilde_start (t_tabwrite_tilde *x, t_float f)
{
    x->x_phase = (f > 0 ? f : 0);
}

static void tabwrite_tilde_stop (t_tabwrite_tilde *x)
{
    x->x_redraw = 1; x->x_phase = PD_INT_MAX;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* No aliasing. */

static t_int *tabwrite_tilde_perform (t_int *w)
{
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    if (x->x_vector) {
    //
    int end = x->x_size;
    int phase = x->x_phase;
    
    if (end > phase) {
    //
    t_word *data = x->x_vector + phase;
    int size = PD_MIN (n, end - phase);
    
    phase += size; if (phase >= end) { x->x_redraw = 1; phase = PD_INT_MAX; }
    
    x->x_phase = phase;
    
    while (size--) {
    //
    t_sample f = *in++;
    if (PD_IS_BIG_OR_SMALL (f)) { f = 0.0; }
    WORD_FLOAT (data) = f;
    data++;
    //
    }
    //
    } else { x->x_phase = PD_INT_MAX; }
    //
    }
    
    return (w + 4);
}

static void tabwrite_tilde_dsp (t_tabwrite_tilde *x, t_signal **sp)
{
    tabwrite_tilde_set (x, x->x_name);
    
    dsp_add (tabwrite_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *tabwrite_tilde_new (t_symbol *s)
{
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)pd_new (tabwrite_tilde_class);
    
    x->x_phase = PD_INT_MAX;
    x->x_name  = s;
    
    poll_add (cast_pd (x));
    
    return x;
}

static void tabwrite_tilde_free (t_tabwrite_tilde *x)
{
    poll_remove (cast_pd (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void tabwrite_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabwrite__tilde__,
            (t_newmethod)tabwrite_tilde_new,
            (t_method)tabwrite_tilde_free,
            sizeof (t_tabwrite_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    CLASS_SIGNAL (c, t_tabwrite_tilde, x_f);
    
    class_addDSP (c, (t_method)tabwrite_tilde_dsp);
    class_addBang (c, (t_method)tabwrite_tilde_bang);
    class_addPolling (c, (t_method)tabwrite_tilde_polling);
        
    class_addMethod (c, (t_method)tabwrite_tilde_set,   sym_set,    A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)tabwrite_tilde_start, sym_start,  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)tabwrite_tilde_stop,  sym_stop,   A_NULL);
    
    tabwrite_tilde_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
