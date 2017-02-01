
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
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *block_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *samplerate_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _samplerate_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    } t_samplerate_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_block *samplerate_tilde_getBlockIfContainsAny (t_glist **p)
{
    t_block *block = NULL;
    t_glist *glist = *p;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == block_class) {
            if (block) { error_unexpected (sym_samplerate__tilde__, sym_block__tilde__); }
            else {
                block = (t_block *)y;
            }
        }
    }
    
    *p = glist->gl_parent;
    
    return block;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void samplerate_tilde_bang (t_samplerate_tilde *x)
{
    t_float sampleRate = audio_getSampleRate();
    
    t_glist *glist = x->x_owner;
    
    while (glist) {
        t_block *b = samplerate_tilde_getBlockIfContainsAny (&glist);
        if (b) { 
            sampleRate *= block_getRatio (b);
        }
    }
    
    outlet_float (x->x_outlet, sampleRate);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *samplerate_tilde_new (void)
{
    t_samplerate_tilde *x = (t_samplerate_tilde *)pd_new (samplerate_tilde_class);
    
    x->x_owner  = canvas_getCurrent();
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void samplerate_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_samplerate__tilde__,
            (t_newmethod)samplerate_tilde_new,
            NULL,
            sizeof (t_samplerate_tilde),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, (t_method)samplerate_tilde_bang);
    
    samplerate_tilde_class = c;
}

void samplerate_tilde_destroy (void)
{
    CLASS_FREE (samplerate_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
