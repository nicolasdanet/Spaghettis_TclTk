
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void            ugen_start          (void);
void            ugen_stop           (void);
void            ugen_add            (t_dspcontext *, t_object *);
void            ugen_connect        (t_dspcontext *, t_object *, int, t_object *, int);
void            ugen_done_graph     (t_dspcontext *);

t_dspcontext    *ugen_start_graph   (int, t_signal **, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_dspPerform (t_glist *glist, int isTopLevel, t_signal **sp)
{
    t_gobj          *y = NULL;
    t_dspcontext    *context = NULL;
    t_outconnect    *connection = NULL;   
    t_linetraverser t;
    
    int m = object_numberOfSignalInlets (cast_object (glist));
    int n = object_numberOfSignalOutlets (cast_object (glist));
    
    context = ugen_start_graph (isTopLevel, sp, m, n);
    
    for (y = glist->gl_list; y; y = y->g_next) {
    //
    t_object *o = canvas_castToObjectIfBox (y);
    if (o && class_hasMethod (pd_class (y), gensym ("dsp"))) { ugen_add (context, o); }
    //
    }

    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
        if (object_isSignalOutlet (t.tr_srcObject, t.tr_srcIndexOfOutlet)) {
            ugen_connect (context, 
                t.tr_srcObject, 
                t.tr_srcIndexOfOutlet, 
                t.tr_destObject, 
                t.tr_destIndexOfInlet);
        }
    }

    ugen_done_graph (context);
}

void canvas_dsp (t_glist *x, t_signal **sp)
{
    canvas_dspPerform (x, 0, sp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dsp_start (void)
{
    t_glist *glist;
    
    if (pd_this->pd_dspState) { ugen_stop(); }

    ugen_start();
    
    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) { canvas_dspPerform (glist, 1, NULL); }
    
    pd_this->pd_dspState = 1;
}


static void dsp_stop (void)
{
    PD_ASSERT (pd_this->pd_dspState);
    
    ugen_stop();
    
    pd_this->pd_dspState = 0;
}

static void dsp_notify (int n)
{
    sys_vGui ("set ::var(isDsp) %d\n", n);  // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_state (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int n = (int)atom_getFloatAtIndex (0, argc, argv);
    
    if (n != pd_this->pd_dspState) {
    //
    if (n) { if (audio_startDSP() == PD_ERROR_NONE) { dsp_start(); } }
    else {
        dsp_stop(); audio_stopDSP();
    }
    
    dsp_notify (pd_this->pd_dspState);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dsp_update (void)
{
    if (pd_this->pd_dspState) { dsp_start(); }
}

int dsp_suspend (void)
{
    int oldState = pd_this->pd_dspState;
    
    if (oldState) { dsp_stop(); }
    
    return oldState;
}

void dsp_resume (int oldState)
{
    if (oldState) { dsp_start(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
