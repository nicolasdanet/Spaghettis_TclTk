
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_dspProceed (t_glist *glist, int isTopLevel, t_signal **sp)
{
    t_gobj          *y = NULL;
    t_dspcontext    *context = NULL;
    t_outconnect    *connection = NULL;   
    t_linetraverser t;
    
    int m = object_getNumberOfSignalInlets (cast_object (glist));
    int n = object_getNumberOfSignalOutlets (cast_object (glist));
    
    context = ugen_graphStart (isTopLevel, sp, m, n);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_object *o = cast_objectIfPatchable (y);
    if (o && class_hasDspMethod (pd_class (y))) { ugen_graphAdd (context, o); }
    //
    }

    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
        if (object_isSignalOutlet (linetraverser_getSource (&t), linetraverser_getIndexOfOutlet (&t))) {
            ugen_graphConnect (context, 
                linetraverser_getSource (&t), 
                linetraverser_getIndexOfOutlet (&t), 
                linetraverser_getDestination (&t), 
                linetraverser_getIndexOfInlet (&t));
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
