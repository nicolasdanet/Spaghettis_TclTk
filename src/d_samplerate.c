
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
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

extern t_class *block_class;
static t_class *samplerate_tilde_class;

typedef struct _samplerate
{
    t_object x_obj;
    t_float x_sr;
    t_glist *x_canvas;
} t_samplerate;

static void *canvas_getblock (t_class *blockclass, t_glist **canvasp)
{
    t_glist *canvas = *canvasp;
    t_gobj *g;
    void *ret = 0;
    for (g = canvas->gl_graphics; g; g = g->g_next)
    {
        if (g->g_pd == blockclass)
            ret = g;
    }
    *canvasp = canvas->gl_parent;
    return(ret);
}

static void samplerate_tilde_bang(t_samplerate *x)
{
    t_float srate = audio_getSampleRate();
    t_glist *canvas = x->x_canvas;
    while (canvas)
    {
        t_block *b = (t_block *)canvas_getblock(block_class, &canvas);
        if (b) 
            srate *= (t_float)(b->x_upsample) / (t_float)(b->x_downsample); 
    }
    outlet_float(x->x_obj.te_outlet, srate);
}

static void *samplerate_tilde_new(t_symbol *s)
{
    t_samplerate *x = (t_samplerate *)pd_new(samplerate_tilde_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_canvas = canvas_getCurrent();
    return (x);
}

void samplerate_tilde_setup(void)
{
    samplerate_tilde_class = class_new(sym_samplerate__tilde__,
        (t_newmethod)samplerate_tilde_new, 0, sizeof(t_samplerate), 0, 0);
    class_addBang(samplerate_tilde_class, samplerate_tilde_bang);
}

