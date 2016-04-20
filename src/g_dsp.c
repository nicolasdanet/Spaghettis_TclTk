
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

static void canvas_dspNotify (int n)
{
    sys_vGui ("set ::var(isDsp) %d\n", n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_dodsp (t_glist *x, int toplevel, t_signal **sp)
{
    t_linetraverser t;
    t_outconnect *oc;
    t_gobj *y;
    t_object *ob;
    t_symbol *dspsym = gensym("dsp");
    t_dspcontext *dc;    

        /* create a new "DSP graph" object to use in sorting this canvas.
        If we aren't toplevel, there are already other dspcontexts around. */

    dc = ugen_start_graph(toplevel, sp,
        object_numberOfSignalInlets(&x->gl_obj),
        object_numberOfSignalOutlets(&x->gl_obj));

        /* find all the "dsp" boxes and add them to the graph */
    
    for (y = x->gl_list; y; y = y->g_next)
        if ((ob = canvas_castToObjectIfBox(&y->g_pd)) && class_hasMethod (pd_class (&y->g_pd), dspsym))
            ugen_add(dc, ob);

        /* ... and all dsp interconnections */
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
        if (object_isSignalOutlet(t.tr_srcObject, t.tr_srcIndexOfOutlet))
            ugen_connect(dc, t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);

        /* finally, sort them and add them to the DSP chain */
    ugen_done_graph(dc);
}

void canvas_dsp (t_glist *x, t_signal **sp)
{
    canvas_dodsp (x, 0, sp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_dspStart (void)
{
    t_glist *glist;
    
    if (pd_this->pd_dspState) { ugen_stop(); }

    ugen_start();
    
    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) { canvas_dodsp (glist, 1, NULL); }
    
    pd_this->pd_dspState = 1;
}


static void canvas_dspStop (void)
{
    PD_ASSERT (pd_this->pd_dspState);
    
    ugen_stop();
    
    pd_this->pd_dspState = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_dspState (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    int n = (int)atom_getFloatAtIndex (0, argc, argv);
    
    if (n != pd_this->pd_dspState) {
    //
    if (n) { if (audio_startDSP() == PD_ERROR_NONE) { canvas_dspStart(); } }
    else {
        canvas_dspStop(); audio_stopDSP();
    }
    
    canvas_dspNotify (pd_this->pd_dspState);
    //
    }
    //
    }
}

int canvas_dspSuspend (void)
{
    int oldState = pd_this->pd_dspState;
    
    if (oldState) { canvas_dspStop(); }
    
    return oldState;
}

void canvas_dspResume (int oldState)
{
    if (oldState) { canvas_dspStart(); }
}

void canvas_dspUpdate (void)
{
    if (pd_this->pd_dspState) { canvas_dspStart(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
