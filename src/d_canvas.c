
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
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
