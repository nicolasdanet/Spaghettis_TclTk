
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

t_class *voutlet_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int voutlet_isSignal (t_voutlet *x)
{
    return (x->vo_buffer != NULL);
}

t_outlet *voutlet_getOutlet (t_pd *x)
{
    PD_ASSERT (pd_class (x) == voutlet_class); return (((t_voutlet *)x)->vo_outlet);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void voutlet_bang (t_voutlet *x)
{
    outlet_bang (x->vo_outlet);
}

static void voutlet_float (t_voutlet *x, t_float f)
{
    outlet_float (x->vo_outlet, f);
}

static void voutlet_symbol (t_voutlet *x, t_symbol *s)
{
    outlet_symbol (x->vo_outlet, s);
}

static void voutlet_pointer (t_voutlet *x, t_gpointer *gp)
{
    outlet_pointer (x->vo_outlet, gp);
}

static void voutlet_list (t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->vo_outlet, argc, argv);
}

static void voutlet_anything (t_voutlet *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->vo_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *voutlet_newSignal (t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new (voutlet_class);
    
    resample_init (&x->vo_resample, s);
    
    x->vo_bufferSize = 0;
    x->vo_buffer     = (t_sample *)PD_MEMORY_GET (0);
    x->vo_bufferEnd  = x->vo_buffer;
    x->vo_owner      = canvas_getCurrent();
    x->vo_outlet     = canvas_addOutlet (x->vo_owner, cast_pd (x), &s_signal);
    
    inlet_new (cast_object (x), cast_pd (x), &s_signal, &s_signal);

    return x;
}

static void *voutlet_new (t_symbol *s)
{
    t_voutlet *x = (t_voutlet *)pd_new (voutlet_class);
    
    x->vo_owner  = canvas_getCurrent();
    x->vo_outlet = canvas_addOutlet (x->vo_owner, cast_pd (x), &s_anything);
    
    inlet_new (cast_object (x), cast_pd (x), NULL, NULL);

    return x;
}

static void voutlet_free (t_voutlet *x)
{
    canvas_removeOutlet (x->vo_owner, x->vo_outlet);
    
    if (x->vo_buffer) { PD_MEMORY_FREE (x->vo_buffer); }
    
    resample_free (&x->vo_resample);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void voutlet_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_outlet,
            (t_newmethod)voutlet_new,
            (t_method)voutlet_free,
            sizeof (t_voutlet),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)voutlet_newSignal, sym_outlet__tilde__, A_DEFSYMBOL, A_NULL);
    
    class_addDSP (c, voutlet_dsp);
    class_addBang (c, voutlet_bang);
    class_addFloat (c, voutlet_float);
    class_addSymbol (c, voutlet_symbol);
    class_addPointer (c, voutlet_pointer);
    class_addList (c, voutlet_list);
    class_addAnything (c, voutlet_anything);
    
    class_setHelpName (c, sym_pd);
    
    voutlet_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
