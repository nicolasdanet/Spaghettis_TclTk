
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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

void canvas_dspProceed (t_glist *glist, int isTopLevel, t_signal **sp)
{
    t_gobj       *y = NULL;
    t_dspcontext *context = NULL;
    t_outconnect *connection = NULL;   
    t_traverser  t;
    
    int m = object_getNumberOfSignalInlets (cast_object (glist));
    int n = object_getNumberOfSignalOutlets (cast_object (glist));
    
    context = ugen_graphStart (isTopLevel, sp, m, n);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_object *o = cast_objectIfConnectable (y);
    if (o && class_hasDSP (pd_class (y))) { ugen_graphAdd (context, o); }
    //
    }

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
        if (object_isSignalOutlet (traverser_getSource (&t), traverser_getIndexOfOutlet (&t))) {
            ugen_graphConnect (context, 
                traverser_getSource (&t), 
                traverser_getIndexOfOutlet (&t), 
                traverser_getDestination (&t), 
                traverser_getIndexOfInlet (&t));
        }
    }

    ugen_graphClose (context);
}

void canvas_dsp (t_glist *x, t_signal **sp)
{
    canvas_dspProceed (x, 0, sp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_block *canvas_getBlockIfContainsAny (t_glist **p)
{
    t_block *block = NULL;
    t_glist *glist = *p;
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == block_class) {
            if (block) { error_ignored (sym_block__tilde__); }
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

t_float canvas_getSampleRate (t_glist *glist)
{
    t_float sampleRate = audio_getSampleRate();
    
    t_glist *p = glist;
    
    while (p) {
        t_block *b = canvas_getBlockIfContainsAny (&p);
        if (b) {
            sampleRate *= block_getResamplingRatio (b);
        }
    }
    
    return sampleRate;
}

t_float canvas_getBlockSize (t_glist *glist)
{
    t_float blockSize = INTERNAL_BLOCKSIZE;
    
    t_glist *p = glist;
    
    while (p) {
        t_block *b = canvas_getBlockIfContainsAny (&p);
        if (b) {
            int n = block_getBlockSize (b);
            if (n > 0) {
                blockSize = (t_float)n;
                break;
            }
        }
    }
    
    return blockSize;
}

t_block *canvas_getBlock (t_glist *glist)
{
    t_glist *p = glist;
    
    while (p) {
        t_block *b = canvas_getBlockIfContainsAny (&p);
        if (b) {
            return b;
        }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
