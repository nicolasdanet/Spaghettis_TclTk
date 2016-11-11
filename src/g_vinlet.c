
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *vinlet_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int vinlet_isSignal (t_vinlet *x)
{
    return (x->vi_buffer != NULL);
}

t_inlet *vinlet_getInlet (t_pd *x)
{
    PD_ASSERT (pd_class (x) == vinlet_class); return (((t_vinlet *)x)->vi_inlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vinlet_bang (t_vinlet *x)
{
    outlet_bang (x->vi_outlet);
}

static void vinlet_float (t_vinlet *x, t_float f)
{
    outlet_float (x->vi_outlet, f);
}

static void vinlet_symbol (t_vinlet *x, t_symbol *s)
{
    outlet_symbol (x->vi_outlet, s);
}

static void vinlet_pointer (t_vinlet *x, t_gpointer *gp)
{
    outlet_pointer (x->vi_outlet, gp);
}

static void vinlet_list (t_vinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->vi_outlet, argc, argv);
}

static void vinlet_anything (t_vinlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->vi_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vinlet_newSignal (t_symbol *s)
{
    t_vinlet *x = (t_vinlet *)pd_new (vinlet_class);
    
    resample_init (&x->vi_resample, s);

    x->vi_bufferSize   = 0;
    x->vi_buffer       = (t_sample *)PD_MEMORY_GET (0);
    x->vi_bufferEnd    = x->vi_buffer;
    x->vi_owner        = canvas_getCurrent();
    x->vi_outlet       = outlet_new (cast_object (x), &s_signal);
    x->vi_inlet        = canvas_addInlet (x->vi_owner, cast_pd (x), &s_signal);
    x->vi_directSignal = NULL;
    
    return x;
}

static void *vinlet_new (t_symbol *s)
{
    t_vinlet *x = (t_vinlet *)pd_new (vinlet_class);
    
    x->vi_owner  = canvas_getCurrent();
    x->vi_outlet = outlet_new (cast_object (x), &s_anything);
    x->vi_inlet  = canvas_addInlet (x->vi_owner, cast_pd (x), NULL);
    
    return x;
}

static void vinlet_free (t_vinlet *x)
{
    canvas_removeInlet (x->vi_owner, x->vi_inlet);
    
    if (x->vi_buffer) { PD_MEMORY_FREE (x->vi_buffer); }
    
    resample_free (&x->vi_resample);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vinlet_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_inlet,
            (t_newmethod)vinlet_new,
            (t_method)vinlet_free,
            sizeof (t_vinlet),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)vinlet_newSignal, sym_inlet__tilde__, A_DEFSYMBOL, A_NULL);
    
    class_addDSP (c, vinlet_dsp);
    class_addBang (c, vinlet_bang);
    class_addFloat (c, vinlet_float);
    class_addSymbol (c, vinlet_symbol);
    class_addPointer (c, vinlet_pointer);
    class_addList (c, vinlet_list);
    class_addAnything (c, vinlet_anything);
    
    class_setHelpName (c, sym_pd);
    
    vinlet_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
