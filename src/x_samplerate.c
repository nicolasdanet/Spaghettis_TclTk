
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *samplerate_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _samplerate {
    t_object    x_obj;                      /* Must be the first. */
    t_glist     *x_owner;
    t_outlet    *x_outlet;
    } t_samplerate;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_block *samplerate_getBlockIfContainsAny (t_glist **p)
{
    t_block *block = NULL;
    t_glist *glist = *p;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == block_class) {
            if (block) { error_unexpected (sym_samplerate, sym_block__tilde__); }
            else {
                block = (t_block *)y;
            }
        }
    }
    
    *p = glist_getParent (glist);
    
    return block;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void samplerate_bang (t_samplerate *x)
{
    t_float sampleRate = audio_getSampleRate();
    
    t_glist *glist = x->x_owner;
    
    while (glist) {
        t_block *b = samplerate_getBlockIfContainsAny (&glist);
        if (b) { 
            sampleRate *= block_getRatio (b);
        }
    }
    
    outlet_float (x->x_outlet, sampleRate);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *samplerate_new (void)
{
    t_samplerate *x = (t_samplerate *)pd_new (samplerate_class);
    
    x->x_owner  = instance_contextGetCurrent();
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void samplerate_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_samplerate,
            (t_newmethod)samplerate_new,
            NULL,
            sizeof (t_samplerate),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)samplerate_new, sym_samplerate__tilde__, A_NULL);

    class_addBang (c, (t_method)samplerate_bang);
    
    samplerate_class = c;
}

void samplerate_destroy (void)
{
    class_free (samplerate_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
